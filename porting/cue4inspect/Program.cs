using CUE4Parse.Encryption.Aes;
using CUE4Parse.FileProvider;
using CUE4Parse.UE4.AssetRegistry;
using CUE4Parse.UE4.Objects.Core.Misc;
using CUE4Parse.UE4.Objects.UObject;
using CUE4Parse.UE4.Readers;
using CUE4Parse.UE4.Versions;

bool showAll = args.Contains("--all");
bool showPseudo = args.Contains("--pseudo");
bool registryMode = args.Contains("--registry");
bool allKeys = args.Contains("--all-keys");
bool savePackage = args.Contains("--save-package");
bool saveFolder = args.Contains("--save-folder");
string[] positional = args.Where(arg => arg is not ("--all" or "--pseudo" or "--registry" or "--all-keys" or "--save-package" or "--save-folder")).ToArray();
if (registryMode ? positional.Length > 1 : positional.Length is < 1 or > 2)
{
    Console.Error.WriteLine("Usage: Cue4Inspect <asset-path-without-extension> [pak-directory] [--all] [--pseudo] [--save-package] [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect <folder-prefix> [pak-directory] --save-folder [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect --registry [pak-directory] [--all-keys]");
    return 2;
}

string? assetPath = registryMode ? null : positional[0];
string pakDirectory = positional.Length == (registryMode ? 1 : 2)
    ? positional[^1]
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
    provider.ReadScriptData = showPseudo;
    provider.Initialize();

    string key = ReadHiddenKey("Primary AES key: ");
    int mounted = provider.SubmitKey(new FGuid(), new FAesKey("0x" + key));
    Console.WriteLine($"Mounted {mounted} pak(s) with the primary key.");
    if (allKeys)
    {
        foreach (var guid in provider.RequiredKeys.Where(guid => guid.IsValid()).OrderBy(guid => guid.ToString(EGuidFormats.Digits)))
        {
            string extraKey = ReadHiddenKey($"AES key for GUID {guid.ToString(EGuidFormats.Digits)}: ");
            mounted += provider.SubmitKey(guid, new FAesKey("0x" + extraKey));
        }
        Console.WriteLine($"Mounted {mounted} pak(s) in total.");
    }

    if (registryMode)
    {
        var registryBytes = provider.SaveAsset("FortniteGame/AssetRegistry.bin");
        var registry = new FAssetRegistryState(new FByteArchive("AssetRegistry.bin", registryBytes, provider.Versions));
        Console.WriteLine($"Registry assets: {registry.PreallocatedAssetDataBuffers.Length}");
        Console.WriteLine($"Dependency nodes: {registry.PreallocatedDependsNodeDataBuffers.Length}");
        Console.WriteLine($"Package records: {registry.PreallocatedPackageDataBuffers.Length}");
        foreach (var group in registry.PreallocatedAssetDataBuffers
            .GroupBy(asset => asset.AssetClass.Text)
            .OrderByDescending(group => group.Count())
            .Take(showAll ? int.MaxValue : 25))
            Console.WriteLine($"{group.Key}: {group.Count()}");
        return 0;
    }

    if (saveFolder)
    {
        var prefix = assetPath!.TrimEnd('/') + "/";
        var packages = provider.Files.Values
            .Where(file => file.Path.StartsWith(prefix, StringComparison.OrdinalIgnoreCase) &&
                           file.Path.EndsWith(".uasset", StringComparison.OrdinalIgnoreCase))
            .OrderBy(file => file.Path)
            .ToArray();
        Console.WriteLine($"Recovering {packages.Length} package(s) under {prefix}");
        foreach (var package in packages)
        {
            SaveRecoveredFiles(provider.SavePackage(package));
        }
        return 0;
    }

    if (savePackage)
    {
        SaveRecoveredFiles(provider.SavePackage(assetPath!));
    }

    var exports = provider.LoadPackage(assetPath!).GetExports();
    Console.WriteLine($"Package: {assetPath}");
    Console.WriteLine($"Exports: {exports.Count()}");
    foreach (var group in exports.GroupBy(item => item.GetType().Name).OrderByDescending(group => group.Count()))
        Console.WriteLine($"  {group.Key}: {group.Count()}");
    if (!showAll) Console.WriteLine("First 20 exports (use --all for the full list):");
    foreach (var item in showAll ? exports : exports.Take(20))
        Console.WriteLine($"{item.GetType().Name}: {item.Name}");
    if (showPseudo)
    {
        var functions = exports.OfType<UFunction>().ToArray();
        Console.WriteLine($"Decoded function scripts: {functions.Count(function => function.ScriptBytecode is { Length: > 0 })}/{functions.Length}");
        foreach (var blueprintClass in exports.OfType<UClass>())
        {
            Console.WriteLine($"\nBlueprint pseudocode: {blueprintClass.Name}");
            Console.WriteLine(blueprintClass.DecompileBlueprintToPseudo());
        }
    }
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

static void SaveRecoveredFiles(IReadOnlyDictionary<string, byte[]> files)
{
    var destination = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "../../../../generated/extracted"));
    foreach (var (name, bytes) in files)
    {
        var relative = name.Replace('\\', '/');
        if (Path.IsPathRooted(relative) || relative.Split('/').Any(part => part is ".." or "."))
            throw new InvalidDataException("Package contains an unsafe path");
        var target = Path.GetFullPath(Path.Combine(destination, relative));
        if (!target.StartsWith(destination + Path.DirectorySeparatorChar, StringComparison.Ordinal))
            throw new InvalidDataException("Package path escapes output directory");
        Directory.CreateDirectory(Path.GetDirectoryName(target)!);
        File.WriteAllBytes(target, bytes);
        Console.WriteLine($"Saved {relative} ({bytes.Length} bytes)");
    }
}
