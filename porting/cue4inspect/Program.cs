using CUE4Parse.Encryption.Aes;
using CUE4Parse.FileProvider;
using CUE4Parse.UE4.Objects.Core.Misc;
using CUE4Parse.UE4.Versions;

bool showAll = args.Contains("--all");
string[] positional = args.Where(arg => arg != "--all").ToArray();
if (positional.Length < 1 || positional.Length > 2)
{
    Console.Error.WriteLine("Usage: Cue4Inspect <asset-path-without-extension> [pak-directory] [--all]");
    return 2;
}

string assetPath = positional[0];
string pakDirectory = positional.Length == 2
    ? positional[1]
    : Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "../../../../../Fortnite/FortniteGame/Content/Paks"));

if (!Directory.Exists(pakDirectory))
{
    Console.Error.WriteLine($"Pak directory does not exist: {pakDirectory}");
    return 2;
}

try
{
    var provider = new DefaultFileProvider(
        pakDirectory,
        SearchOption.TopDirectoryOnly,
        new VersionContainer(EGame.GAME_UE4_25_Plus),
        StringComparer.OrdinalIgnoreCase);
    provider.Initialize();

    string key = ReadHiddenKey("Primary AES key: ");
    int mounted = provider.SubmitKey(new FGuid(), new FAesKey("0x" + key));
    Console.WriteLine($"Mounted {mounted} pak(s) with the primary key.");

    var exports = provider.LoadPackage(assetPath).GetExports();
    Console.WriteLine($"Package: {assetPath}");
    Console.WriteLine($"Exports: {exports.Count()}");
    foreach (var group in exports.GroupBy(item => item.GetType().Name).OrderByDescending(group => group.Count()))
        Console.WriteLine($"  {group.Key}: {group.Count()}");
    if (!showAll) Console.WriteLine("First 20 exports (use --all for the full list):");
    foreach (var item in showAll ? exports : exports.Take(20))
        Console.WriteLine($"{item.GetType().Name}: {item.Name}");
    return 0;
}
catch (Exception error)
{
    Console.Error.WriteLine($"Inspection failed: {error.GetType().Name}: {error.Message}");
    return 1;
}

static string ReadHiddenKey(string prompt)
{
    Console.Error.Write(prompt);
    var characters = new List<char>();
    while (true)
    {
        var key = Console.ReadKey(intercept: true);
        if (key.Key == ConsoleKey.Enter)
        {
            Console.Error.WriteLine();
            break;
        }
        if (key.Key == ConsoleKey.Backspace)
        {
            if (characters.Count > 0) characters.RemoveAt(characters.Count - 1);
            continue;
        }
        if (Uri.IsHexDigit(key.KeyChar)) characters.Add(key.KeyChar);
    }
    string result = new string(characters.ToArray());
    if (result.Length != 64)
        throw new ArgumentException("Expected a 64-character AES-256 hex key");
    return result;
}
