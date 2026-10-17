using Godot;
using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using FileAccess = Godot.FileAccess;

namespace AbeHDLauncher.Core;

public enum AssetType
{
    Exe,
    Sprite,
    Skin,
    Cam
}

public class AssetManifest
{
    public List<AssetFile> Assets { get; } = [];

    public List<AssetFile> AssetsFor(GameType game)
    {
        return Assets.Where(a => a.AssetType != AssetType.Exe && a.GameType == game).ToList();
    }

    public AssetFile ExeForCurrentPlatform()
    {
        var id = RuntimeInformation.IsOSPlatform(OSPlatform.Windows) ? "relive-windows" : "relive-linux";
        return Assets.FirstOrDefault(a => a.AssetType == AssetType.Exe && a.FileId == id);
    }
}

public class AssetFile
{
    public string FileId { get; init; }
    public AssetType AssetType { get; init; }
    public GameType? GameType { get; init; }
    public long Size { get; init; }
    public string Sha256 { get; init; }
    public string DownloadUrl { get; init; }

    public override bool Equals(object obj)
    {
        return obj is AssetFile other && other.FileId == FileId && other.Sha256 == Sha256;
    }

    public override int GetHashCode()
    {
        return FileId.GetHashCode();
    }

    public string GetFileName()
    {
        if (!string.IsNullOrEmpty(DownloadUrl))
        {
            return Path.GetFileName(new Uri(DownloadUrl).AbsolutePath);
        }

        return $"{FileId}.zip";
    }
};

public partial class AssetsManager : Node
{
    public enum InstallStage
    {
        Idle,
        Downloading,
        InstallingRelive,
        ConvertingData,
        ExtractingAssets,
        Finished
    }

    private AssetManifest _manifest = new();
    private BatchDownloader _batchDownloader = new();
    private ReliveProcess _reliveConversionProcess;
    private GameType _gameToInstall;
    private HashSet<AssetFile> _assetsToInstall = [];
    private InstalledAssetsRecord _installed = new();

    public static string AssetsDownloadPath => "user://assets";
    public static string GlobalizedAssetsDownloadPath => ProjectSettings.GlobalizePath(AssetsDownloadPath);
    private const string ManifestUrl = "https://raw.githubusercontent.com/mouzedrift/abe-hd-assets/refs/heads/main/manifest.json";

    [Signal] public delegate void DownloadsFinishedEventHandler();
    [Signal] public delegate void InstallationFinishedEventHandler();
    [Signal] public delegate void StageChangedEventHandler(int stage);

    public record struct DownloadProgress(string FileName, long Received, int Completed, int TotalFiles, long CompletedBytes, long TotalBytes);
    public InstallStage Stage { get; private set; } = InstallStage.Idle;

    public DownloadProgress GetDownloadProgress()
    {
        return new DownloadProgress(
            _batchDownloader.CurrentDownloadFile?.GetFileName() ?? "",
            _batchDownloader.GetDownloadedBytes(),
            _batchDownloader.CompletedDownloads,
            _batchDownloader.TotalDownloads,
            _batchDownloader.CompletedBytes,
            _batchDownloader.TotalBytes);
    }

    private void SetStage(InstallStage stage)
    {
        Stage = stage;
        CallDeferred(GodotObject.MethodName.EmitSignal, SignalName.StageChanged, (int)stage);
    }

    public override void _Ready()
    {
        AddChild(_batchDownloader);

        _batchDownloader.AllDownloadsFinished += () =>
        {
            EmitSignal(SignalName.DownloadsFinished);

            if (!InstallReliveExe(_gameToInstall))
            {
                OS.Alert("RELIVE executable installation failed!", "Error");
                return;
            }

            ConvertReliveData();
        };

        SetStage(InstallStage.Idle);
    }

    static AssetType ParseAssetType(string s) => s switch
    {
        "exe" => AssetType.Exe,
        "sprite" => AssetType.Sprite,
        "skin" => AssetType.Skin,
        "cam" => AssetType.Cam,
        _ => throw new FormatException($"Unknown asset type: {s}")
    };

    static GameType ParseGameType(string s) => s switch
    {
        "ao" => GameType.AO,
        "ae" => GameType.AE,
        _ => throw new FormatException($"Unknown game type: {s}")
    };

    public async Task<bool> LoadManifestAsync()
    {
        Directory.CreateDirectory(ProjectSettings.GlobalizePath(AssetsDownloadPath));

        var httpRequest = new HttpRequest();
        httpRequest.Timeout = 15.0;
        AddChild(httpRequest);

        var error = httpRequest.Request(ManifestUrl);
        if (error != Error.Ok)
        {
            httpRequest.QueueFree();
            OS.Alert($"Fetching manifest.json failed!\n{error}", "Error");
            return false;
        }

        var response = await ToSignal(httpRequest, HttpRequest.SignalName.RequestCompleted);
        httpRequest.QueueFree();

        var result = (long)response[0];
        var responseCode = (long)response[1];
        var body = (byte[])response[3];

        if (result != (long)HttpRequest.Result.Success || responseCode != 200)
        {
            OS.Alert($"Couldn't download the manifest.\nCheck your internet connection.\n(result {result}, HTTP {responseCode})", "Error");
            return false;
        }

        GD.Print("successfully fetched manifest.json");

        var parsedManifest = Json.ParseString(body.GetStringFromUtf8());
        if (parsedManifest.VariantType != Variant.Type.Array)
        {
            OS.Alert("Failed to parse manifest.json!", "Error");
            return false;
        }

        var root = parsedManifest.AsGodotArray();

        _manifest = new AssetManifest();

        foreach (var entry in root)
        {
            var d = entry.AsGodotDictionary();

            _manifest.Assets.Add(new AssetFile
            {
                FileId = d["id"].AsString(),
                AssetType = ParseAssetType(d["type"].AsString()),
                GameType = d.ContainsKey("game") ? ParseGameType(d["game"].AsString()) : null,
                Size = d["size"].AsInt64(),
                Sha256 = d["sha256"].AsString(),
                DownloadUrl = d["url"].AsString()
            });
        }

        return true;
    }

    public async Task TestInstall(GameType gameType)
    {
        _assetsToInstall.Clear();

        _gameToInstall = gameType;

        _assetsToInstall.Add(_manifest.ExeForCurrentPlatform());
        InstallReliveExe(gameType);

        foreach (var asset in _manifest.AssetsFor(gameType))
        {
            _assetsToInstall.Add(asset);
        }

        await InstallSpritesAndCamsAsync(gameType);
    }

    public AssetManifest GetManifest()
    {
        return _manifest;
    }

    public void StartInstallation(GameType gameType, Godot.Collections.Array<string> skinIds)
    {
        string gameInstallPath = GameDetector.GetGameInstallPath(gameType);

        _gameToInstall = gameType;
        _installed = InstalledAssetsRecord.Load(gameInstallPath);

        _assetsToInstall.Clear();

        foreach (var asset in _manifest.AssetsFor(gameType))
        {
            if (asset.AssetType != AssetType.Skin || skinIds.Contains(asset.FileId))
            {
                _assetsToInstall.Add(asset);
            }
        }

        _assetsToInstall.Add(_manifest.ExeForCurrentPlatform());

        // skip everything that's already installed and unchanged
        _assetsToInstall.RemoveWhere(_installed.IsInstalled);

        foreach (var assetFile in _assetsToInstall)
        {
            _batchDownloader.AddDownload(assetFile, AssetsDownloadPath);
        }

        _batchDownloader.StartDownloads();

        SetStage(InstallStage.Downloading);
    }

    public void ConvertReliveData()
    {
        SetStage(InstallStage.ConvertingData);

        GD.Print("starting relive data conversion");

        _reliveConversionProcess?.KillProcess();

        _reliveConversionProcess = new ReliveProcess();
        _reliveConversionProcess.Exited += () =>
        {
            GD.Print("relive conversion process exited");
            _ = InstallSpritesAndCamsAsync(_gameToInstall);
        };

        if (!_reliveConversionProcess.LaunchGame(_gameToInstall, true))
        {
            GD.Print("couldn't launch relive to do data conversion");
            return;
        }
    }

    private bool InstallReliveExe(GameType gameType)
    {
        SetStage(InstallStage.InstallingRelive);

        var zipPath = Path.Combine(GlobalizedAssetsDownloadPath, _manifest.ExeForCurrentPlatform().GetFileName());
        var outPath = GameDetector.GetGameInstallPath(gameType);
        if (outPath == null)
        {
            return false;
        }

        ZipFile.ExtractToDirectory(zipPath, outPath, true);

        if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
        {
            OS.Execute("chmod", ["+x", Path.Combine(outPath, "relive-x86_64.AppImage")]);
        }

        return true;
    }

    private static string CamNameToReliveDataPath(string camName, string gameInstallPath, GameType gameType)
    {
        // should be something like: MIP04C26.png
        if (camName.Length < 12)
        {
            return null;
        }

        string lvlAcronym = camName.Substr(0, 2);

        int pathNum = int.Parse(camName.Substr(3, 2));
        int camNum = int.Parse(camName.Substr(6, 2));

        // remove leading zero if any
        string pathNumStr = pathNum.ToString().TrimStart('0');
        string camNumStr = camNum.ToString().TrimStart('0');

        lvlAcronym = lvlAcronym.ToLower();

        string lvlFolderName = string.Empty;
        if (gameType == GameType.AE)
        {
            if (lvlAcronym == "mi")
            {
                lvlFolderName = "mines";
            }
            else if (lvlAcronym == "ne")
            {
                lvlFolderName = "necrum";
            }
            else if (lvlAcronym == "pv")
            {
                if (pathNum == 13)
                {
                    lvlFolderName = "mudomo_vault_ender";
                }
                else
                {
                    lvlFolderName = "mudomo_vault";
                }
            }
            else if (lvlAcronym == "sv")
            {
                // 14 = demo path
                if (pathNum == 9 || pathNum == 10 || pathNum == 11 || pathNum == 14)
                {
                    lvlFolderName = "mudanchee_vault_ender";
                }
                else
                {
                    lvlFolderName = "mudanchee_vault";
                }
            }
            else if (lvlAcronym == "fd")
            {
                if (pathNum == 11)
                {
                    lvlFolderName = "feeco_depot_ender";
                }
                else
                {
                    lvlFolderName = "feeco_depot";
                }
            }
            else if (lvlAcronym == "ba")
            {
                if (pathNum == 11 || pathNum == 16)
                {
                    lvlFolderName = "barracks_ender";
                }
                else
                {
                    lvlFolderName = "barracks";
                }
            }
            else if (lvlAcronym == "bw")
            {
                if (pathNum == 14 || pathNum == 9 || pathNum == 12)
                {
                    lvlFolderName = "bonewerkz_ender";
                }
                else
                {
                    lvlFolderName = "bonewerkz";
                }
            }
            else if (lvlAcronym == "br")
            {
                lvlFolderName = "brewery";
            }
            else if (lvlAcronym == "bm")
            {
                lvlFolderName = "brewery_ender";
            }
            else if (lvlAcronym == "cr")
            {
                lvlFolderName = "credits";
            }
            else if (lvlAcronym == "st")
            {
                lvlFolderName = "menu";
            }
            else
            {
                return null;
            }
        }
        else if (gameType == GameType.AO)
        {
            // TODO: ao lvl names
        }

        string gameStr = gameType == GameType.AE ? "ae" : "ao";
        return Path.Combine(gameInstallPath, "relive_data", gameStr, "levels", lvlFolderName, pathNumStr, $"{camNumStr}.png");
    }

    private static bool ExtractCams(string zipFilePath, AssetFile assetFile, string gameInstallPath, GameType gameType)
    {
        bool anyFailed = false;

        using var zip = ZipFile.OpenRead(zipFilePath);
        foreach (var entry in zip.Entries)
        {
            string filename = entry.Name;
            if (filename == "")
            {
                continue;
            }

            string camOutPath = CamNameToReliveDataPath(filename, gameInstallPath, gameType);
            if (camOutPath == null)
            {
                GD.Print($"couldn't resolve camera output path for cam {filename}");
                anyFailed = true;
                continue;
            }

            try
            {
                Directory.CreateDirectory(Path.GetDirectoryName(camOutPath)!);
                entry.ExtractToFile(camOutPath, true);
            }
            catch (Exception e)
            {
                GD.Print($"couldn't write {camOutPath}: {e.Message}");
                anyFailed = true;
                continue;
            }

            GD.Print($"extracting {assetFile.GetFileName()} to {camOutPath}");
        }

        return !anyFailed;
    }

    private async Task InstallSpritesAndCamsAsync(GameType gameType)
    {
        SetStage(InstallStage.ExtractingAssets);

        GD.Print("installing sprites and cams");

        string gameInstallPath = GameDetector.GetGameInstallPath(gameType);
        if (gameInstallPath == null)
        {
            return;
        }

        string zipAnimOutPath = Path.Combine(gameInstallPath, "relive_data", gameType == GameType.AE ? "ae" : "ao", "animations");
        Directory.CreateDirectory(zipAnimOutPath);

        var assets = _assetsToInstall;

        var installedAssets = await Task.Run(() =>
        {
            var installed = new List<AssetFile>();

            foreach (var assetFile in assets)
            {
                string zipFilePath = Path.Combine(GlobalizedAssetsDownloadPath, assetFile.GetFileName());
                if (!FileAccess.FileExists(zipFilePath))
                {
                    GD.Print($"skipping missing file during installation: {zipFilePath}");
                    continue;
                }

                if (assetFile.AssetType == AssetType.Sprite || assetFile.AssetType == AssetType.Skin)
                {
                    GD.Print($"extracting {assetFile.GetFileName()} to {zipAnimOutPath}");
                    ZipFile.ExtractToDirectory(zipFilePath, zipAnimOutPath, true);

                    installed.Add(assetFile);
                }
                else if (assetFile.AssetType == AssetType.Cam)
                {
                    if (ExtractCams(zipFilePath, assetFile, gameInstallPath, gameType))
                    {
                        installed.Add(assetFile);
                    }
                }
            }

            return installed;
        });

        foreach (var asset in installedAssets)
        {
            _installed.MarkInstalled(asset);
        }

        _installed.Save(gameInstallPath);

        SetStage(InstallStage.Finished);
        CallDeferred(GodotObject.MethodName.EmitSignal, SignalName.InstallationFinished);
    }

    public void AbortInstallation()
    {
        _reliveConversionProcess?.KillProcess();
        SetStage(InstallStage.Idle);
    }

    public bool IsInstallationRunning()
    {
        return Stage != InstallStage.Idle && Stage != InstallStage.Finished;
    }
}
