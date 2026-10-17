using System;
using Godot;

namespace AbeHDLauncher.Core;

public partial class Launcher : Control
{
    [Export] private TextureButton _discordButton;
    [Export] private Button _installButton;
    [Export] private TextureRect _backgroundTexture;
    [Export] private TextureButton _aoButton;
    [Export] private TextureButton _aeButton;
    [Export] private Label _downloadLabel;
    [Export] private ProgressBar _downloadProgressBar;
    [Export] private Button _playButton;
    [Export] private Label _progressBarLabel;

    private AssetsManager _assetsManager = new();
    private SelectAssetsDialog _selectAssetsDialog;
    private GameType _selectedGame;
    private readonly ReliveProcess _reliveProcess = new();
    private ConfirmationDialog _quitDialog = new();
    private const double TextInterval = 0.5;
    private double _sinceTextUpdate;
    private long _lastDone;
    private double _speed;

    // Called when the node enters the scene tree for the first time.
    public override async void _Ready()
    {
        var aoInstallPath = GameDetector.GetGameInstallPath(GameType.AO);
        var aeInstallPath = GameDetector.GetGameInstallPath(GameType.AE);
        if (/*aoInstallPath == null && */aeInstallPath == null)
        {
            //OS.Alert("Couldn't find AO or AE game path.\nPlease install the game through steam and try again.");
            OS.Alert("Couldn't find AE game path.\nPlease install the game through steam and try again.");
            QuitLauncher();
            return;
        }

        //if (aoInstallPath == null)
        {
            _aoButton.Disabled = true;
            _aoButton.Modulate = Colors.DarkSlateGray;
        }

        if (aeInstallPath == null)
        {
            _aeButton.Disabled = true;
            _aeButton.Modulate = Colors.DarkSlateGray;
        }

        GD.Print($"AO install path: {aoInstallPath}");
        GD.Print($"AE install path: {aeInstallPath}");

        _discordButton.Pressed += () =>
        {
            OS.ShellOpen("https://discord.com/invite/Ca5uB4JByA");
        };

        AddChild(_assetsManager);
        if (!await _assetsManager.LoadManifestAsync())
        {
            QuitLauncher();
            return;
        }

        _assetsManager.StageChanged += OnStageChanged;

        _selectAssetsDialog = GD.Load<PackedScene>("uid://cag4lhasmyks1").Instantiate<SelectAssetsDialog>();
        AddChild(_selectAssetsDialog);
        _installButton.Pressed += () => _selectAssetsDialog.PopupCenteredClamped();

        _selectAssetsDialog.SelectionAccepted += (Godot.Collections.Array<string> selectedAssetIds) =>
        {
            _assetsManager.StartInstallation(_selectedGame, selectedAssetIds);
        };

        _aoButton.Pressed += () => SetSelectedGame(GameType.AO);
        _aeButton.Pressed += () => SetSelectedGame(GameType.AE);

        _playButton.Pressed += () => _reliveProcess.LaunchGame(_selectedGame, false);

        SetSelectedGame(GameType.AE);

        GetTree().AutoAcceptQuit = false;

        AddChild(_quitDialog);

        _quitDialog.ForceNative = true;

        _quitDialog.Title = "QUIT";
        _quitDialog.DialogText = "DO YOU REALLY WANT TO QUIT?\nTHE ONGOING INSTALLATION WILL BE ABORTED";

        _quitDialog.OkButtonText = "YES";
        _quitDialog.CancelButtonText = "NO";

        _quitDialog.Confirmed += () =>
        {
            _assetsManager.AbortInstallation();
            QuitLauncher();
        };
        _quitDialog.Canceled += _quitDialog.Hide;

        //await _assetsManager.TestInstall(GameType.AE);
        //_assetsManager.ConvertReliveData();
    }

    public override void _Notification(int what)
    {
        if (what == NotificationWMCloseRequest)
        {
            if (_assetsManager.IsInstallationRunning())
            {
                _quitDialog.PopupCenteredClamped();
                return;
            }

            QuitLauncher();
        }
    }

    private void SetSelectedGame(GameType gameType)
    {
        if (gameType == GameType.AO)
        {
            OS.Alert("AO is currently not supported.");
            return;
        }

        _selectedGame = gameType;
        _backgroundTexture.Texture = gameType == GameType.AO ? GD.Load<Texture2D>("uid://qkkp8ytj3ng4") : GD.Load<Texture2D>("uid://bcyv8yv8q4ese");
    }

    public override void _Process(double delta)
    {
        if (_assetsManager.Stage == AssetsManager.InstallStage.Downloading)
        {
            var p = _assetsManager.GetDownloadProgress();
            if (p.TotalBytes <= 0)
            {
                return;
            }

            long done = Math.Min(p.CompletedBytes + p.Received, p.TotalBytes);

            _downloadProgressBar.Value = done * 100.0 / p.TotalBytes;

            _sinceTextUpdate += delta;
            if (_sinceTextUpdate < TextInterval)
            {
                return;
            }

            double instant = (done - _lastDone) / _sinceTextUpdate;
            _speed = _speed <= 0 ? instant : _speed * 0.7 + instant * 0.3;
            _lastDone = done;
            _sinceTextUpdate = 0;

            _progressBarLabel.Text = $"{done * 100 / p.TotalBytes}% - {FormatSize(done)}/{FormatSize(p.TotalBytes)}";

            string eta = _speed > 1 ? FormatEta((p.TotalBytes - done) / _speed) : "CALCULATING...";
            _downloadLabel.Text = $"{FormatSize((long)_speed)}/S - {eta} - FILE {Math.Min(p.Completed + 1, p.TotalFiles)}/{p.TotalFiles}";
        }
    }

    private static string FormatEta(double seconds)
    {
        if (seconds < 10)
        {
            return "A FEW SECONDS LEFT";
        }

        if (seconds < 60)
        {
            return $"{Math.Round(seconds / 5) * 5:F0} S LEFT";
        }

        if (seconds < 3600)
        {
            return $"ABOUT {Math.Ceiling(seconds / 60):F0} MIN LEFT";
        }

        return $"ABOUT {seconds / 3600:F1} H LEFT";
    }

    private static string FormatSize(long bytes)
    {
        return bytes switch
        {
            >= 1_000_000_000 => $"{bytes / 1e9:F2} GB",
            >= 1_000_000 => $"{bytes / 1e6:F1} MB",
            _ => $"{bytes / 1e3:F0} KB",
        };
    }

    private static string FormatTime(double seconds)
    {
        var t = TimeSpan.FromSeconds(seconds);

        if (t.TotalHours >= 1)
        {
            return $"{(int)t.TotalHours}:{t.Minutes:D2}:{t.Seconds:D2}";
        }

        return $"{t.Minutes}:{t.Seconds:D2}";
    }

    private void OnStageChanged(int stageValue)
    {
        var stage = (AssetsManager.InstallStage)stageValue;

        _installButton.Disabled = stage != AssetsManager.InstallStage.Idle && stage != AssetsManager.InstallStage.Finished;

        _downloadProgressBar.Indeterminate = stage != AssetsManager.InstallStage.Downloading && stage != AssetsManager.InstallStage.Finished;
        _progressBarLabel.Visible = !_downloadProgressBar.Indeterminate;

        switch (stage)
        {
            case AssetsManager.InstallStage.InstallingRelive:
                _downloadLabel.Text = "INSTALLING RELIVE...";
                break;
            case AssetsManager.InstallStage.ConvertingData:
                _downloadLabel.Text = "CONVERTING GAME DATA... (THIS MAY TAKE UP TO 30 MIN)";
                break;
            case AssetsManager.InstallStage.ExtractingAssets:
                _downloadLabel.Text = "EXTRACTING ASSETS...";
                break;
            case AssetsManager.InstallStage.Finished:
                _downloadProgressBar.Value = 100.0;
                _downloadLabel.Text = "FINISHED";
                _progressBarLabel.Text = "100%";
                break;
        }
    }

    private void QuitLauncher()
    {
        _reliveProcess?.Dispose();
        GetTree().Quit();
    }
}
