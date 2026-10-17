using AbeHDLauncher.Core;
using System.Text.Json.Serialization;

public class AbeHDMod(GameType gameType)
{
    [JsonPropertyName("author")] public string Author { get; set; } = "AbeHDTeam";
    [JsonPropertyName("name")] public string Name { get; set; } = "AbeHD";
    [JsonPropertyName("target_game")] public string TargetGame { get; set; } = gameType == GameType.AO ? "AO" : "AE";
}
