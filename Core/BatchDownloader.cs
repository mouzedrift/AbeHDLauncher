using Godot;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace AbeHDLauncher.Core;

public partial class BatchDownloader : HttpRequest
{
    private readonly struct Job(AssetFile assetFile, string outPath)
    {
        public readonly AssetFile AssetFile = assetFile;
        public readonly string OutPath = outPath;
    }

    private readonly Queue<Job> _queue = [];
    private Job _currentJob;
    public bool IsDownloading { get; private set; }
    public AssetFile CurrentDownloadFile { get; private set; }
    public int TotalDownloads { get; private set; }
    public int CompletedDownloads { get; private set; }
    public long TotalBytes { get; private set; }
    public long CompletedBytes { get; private set; }

    [Signal] public delegate void AllDownloadsFinishedEventHandler();
    [Signal] public delegate void DownloadFailedEventHandler(string message);

    // Called when the node enters the scene tree for the first time.
    public override void _Ready()
    {
        RequestCompleted += OnDownloadCompleted;
    }

    private void OnDownloadCompleted(long result, long responseCode, string[] headers, byte[] body)
    {
        if (result != (long)Result.Success || responseCode != 200)
        {
            Fail($"download failed: {_currentJob.AssetFile.DownloadUrl} (result {result}, response {responseCode})");
            return;
        }

        var actual = Godot.FileAccess.GetSha256(_currentJob.OutPath);
        if (!string.Equals(actual, _currentJob.AssetFile.Sha256, StringComparison.OrdinalIgnoreCase))
        {
            File.Delete(_currentJob.OutPath);
            Fail($"hash mismatch: {_currentJob.AssetFile.GetFileName()}");
            return;
        }

        CompletedBytes += _currentJob.AssetFile.Size;
        CompletedDownloads++;
        ProcessQueue();
    }

    private void Fail(string message)
    {
        GD.PrintErr(message);
        _queue.Clear();
        IsDownloading = false;
        EmitSignal(SignalName.DownloadFailed, message);
    }

    // Called every frame. 'delta' is the elapsed time since the previous frame.
    public override void _Process(double delta)
    {
    }

    public void AddDownload(AssetFile assetFile, string outDirectory)
    {
        var outPath = Path.Combine(ProjectSettings.GlobalizePath(outDirectory), assetFile.GetFileName());
        _queue.Enqueue(new Job(assetFile, outPath));

        TotalBytes += assetFile.Size;
    }

    public void StartDownloads()
    {
        TotalDownloads = _queue.Count;
        CompletedDownloads = 0;
        TotalBytes = _queue.Sum(j => j.AssetFile.Size);
        CompletedBytes = 0;
        IsDownloading = true;
        ProcessQueue();
    }

    private void ProcessQueue()
    {
        if (_queue.Count > 0)
        {
            var job = _queue.Dequeue();
            StartDownload(job);
        }
        else
        {
            IsDownloading = false;
            EmitSignal(SignalName.AllDownloadsFinished);
        }
    }

    private void StartDownload(Job job)
    {
        GD.Print($"starting download: {job.AssetFile.DownloadUrl} out: {job.OutPath}");
        _currentJob = job;
        CurrentDownloadFile = job.AssetFile;
        DownloadFile = job.OutPath;
        Request(job.AssetFile.DownloadUrl);
    }
}
