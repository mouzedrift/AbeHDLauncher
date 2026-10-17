using Godot;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;

using FileAccess = Godot.FileAccess;

namespace AbeHDLauncher.Core;

public enum GameType
{
	AO,
	AE
}

public static class GameDetector
{
	public static string GetSteamInstallationPath()
	{
		if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
		{
			var steamPath = Microsoft.Win32.Registry.CurrentUser.OpenSubKey(@"Software\Valve\Steam")?.GetValue("SteamPath")?.ToString();
			if (steamPath != null && DirAccess.DirExistsAbsolute(steamPath))
			{
				return steamPath;
			}
		}
		else if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
		{
			string home = OS.GetEnvironment("HOME");
			string[] possiblePaths =
			{
				Path.Combine(home, ".local/share/Steam"),
				Path.Combine(home, ".steam/steam"),
			};

			foreach (string path in possiblePaths)
			{
				if (DirAccess.DirExistsAbsolute(path))
				{
					return path;
				}
			}
		}

		return null;
	}

	private static List<string> ParseLibraryFoldersVdf(string vdfPath)
	{
		using var file = FileAccess.Open(vdfPath, FileAccess.ModeFlags.Read);
		if (file == null)
		{
			return [];
		}

		List<string> paths = [];
		while (!file.EofReached())
		{
			string line = file.GetLine().Trim();

			// extract library path
			if (line.StartsWith("\"path\""))
			{
				int firstQuote = line.IndexOf('"', 6);
				int secondQuote = line.IndexOf('"', firstQuote + 1);

				string path = line.Substring(firstQuote + 1, secondQuote - firstQuote - 1);
				if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
				{
					path = path.Replace("\\\\", "/");
				}

				paths.Add(path);
			}
		}

		return paths;
	}

	public static string GetGameInstallPath(GameType gameType)
	{
		var steamPath = GetSteamInstallationPath();
		if (steamPath == null)
		{
			GD.Print("couldn't detect steam path");
			return null;
		}

		string gameFolderName = gameType == GameType.AE ? "Oddworld Abes Exoddus" : "Oddworld Abes Oddysee";
		string appPath = Path.Combine(steamPath, "steamapps", "common", gameFolderName);
		if (DirAccess.DirExistsAbsolute(appPath))
		{
			return appPath;
		}

		// fallback
		string vdfPath = Path.Combine(steamPath, "steamapps", "libraryfolders.vdf");
		var libs = ParseLibraryFoldersVdf(vdfPath);
		if (libs.Count == 0)
		{
			GD.Print("couldn't detect any steam library folders");
			return null;
		}

		foreach (var lib in libs)
		{
			appPath = Path.Combine(lib, "steamapps", "common", gameFolderName);
			if (DirAccess.DirExistsAbsolute(appPath))
			{
				return appPath;
			}
		}

		return null;
	}
}
