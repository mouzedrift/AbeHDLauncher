using System;
using System.Collections.Generic;
using System.IO;
using System.Text.Json;
using Godot;

namespace AbeHDLauncher.Core;

public class InstalledAssetEntry
{
    public string FileId { get; set; }
    public string Sha256 { get; set; }
}

public class InstalledAssetsRecord
{
    private static readonly JsonSerializerOptions JsonOptions = new() { WriteIndented = true };
    public Dictionary<string, InstalledAssetEntry> Entries { get; set; } = [];

    private static string GetRecordPath(string gameInstallPath)
    {
        return Path.Combine(gameInstallPath, "installed_hd_assets.json");
    }

    public static InstalledAssetsRecord Load(string gameInstallPath)
    {
        var path = GetRecordPath(gameInstallPath);
        if (File.Exists(path))
        {
            return JsonSerializer.Deserialize<InstalledAssetsRecord>(File.ReadAllText(path)) ?? new();
        }

        GD.Print("no installed assets found");

        return new();
    }

    public void Save(string gameInstallPath)
    {
        File.WriteAllText(GetRecordPath(gameInstallPath), JsonSerializer.Serialize(this, JsonOptions));
    }

    public bool IsInstalled(AssetFile asset)
    {
        return Entries.TryGetValue(asset.FileId, out var e) && string.Equals(asset.Sha256, e.Sha256, StringComparison.OrdinalIgnoreCase);
    }

    public void MarkInstalled(AssetFile asset)
    {
        Entries[asset.FileId] = new InstalledAssetEntry
        {
            FileId = asset.FileId,
            Sha256 = asset.Sha256
        };
    }
}