using Godot;
using System;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;

using FileAccess = Godot.FileAccess;

namespace AbeHDLauncher.Core;

public class ReliveProcess : IDisposable
{
    private Process _process;
    public Action Exited;

    public bool LaunchGame(bool onlyConvertData, string workingDirectory, AbeHDMod mod = null)
    {
        var relivePath = string.Empty;
        if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
        {
            relivePath = Path.Combine(workingDirectory, "relive.exe");
        }
        else if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
        {
            relivePath = Path.Combine(workingDirectory, "relive-x86_64.AppImage");
        }

        if (!FileAccess.FileExists(relivePath))
        {
            OS.Alert($"Couldn't launch relive because the executable wasn't found at:\n{relivePath}", "Error");
            return false;
        }

        var psi = new ProcessStartInfo
        {
            FileName = relivePath,
            WorkingDirectory = workingDirectory,
            UseShellExecute = false,
        };

        psi.ArgumentList.Add("-renderer=gl");

        if (onlyConvertData)
        {
            psi.ArgumentList.Add("-convert");
        }

        if (mod != null)
        {
            psi.ArgumentList.Add($"-mod={mod.Name}");
        }

        try
        {
            var process = new Process
            {
                StartInfo = psi,
                EnableRaisingEvents = true
            };

            _process = process;
            _process.Exited += OnExited;

            if (!process.Start())
            {
                _process.Dispose();
                _process = null;
                OS.Alert("Failed to start relive!", "Error");
                return false;
            }
        }
        catch (Exception e)
        {
            OS.Alert($"{e.Message}", "Error");
            return false;
        }

        return true;
    }

    private void OnExited(object sender, EventArgs e)
    {
        Exited?.Invoke();
        Dispose();
    }

    public void KillProcess()
    {
        if (_process != null && !_process.HasExited)
        {
            _process.EnableRaisingEvents = false;
            _process.Kill();
            Dispose();
        }
    }

    public void Dispose()
    {
        _process?.Dispose();
        _process = null;
    }
}
