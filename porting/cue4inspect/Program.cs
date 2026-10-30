using CUE4Parse;
using CUE4Parse.Encryption.Aes;
using CUE4Parse.FileProvider;
using CUE4Parse.UE4.Assets;
using CUE4Parse.UE4.AssetRegistry;
using CUE4Parse.UE4.Assets.Exports.StaticMesh;
using CUE4Parse.UE4.Assets.Exports.Material;
using CUE4Parse.UE4.Assets.Exports.Texture;
using CUE4Parse.UE4.Objects.Core.Misc;
using CUE4Parse.UE4.Objects.UObject;
using CUE4Parse.UE4.Readers;
using CUE4Parse.UE4.Versions;
using System.Text.Json;
using CUE4Parse_Conversion.Dto;
using CUE4Parse_Conversion.Formats.Meshes;
using CUE4Parse_Conversion.Options;
using CUE4Parse_Conversion.Textures;
using SixLabors.ImageSharp;
using SixLabors.ImageSharp.PixelFormats;

bool showAll = args.Contains("--all");
bool showPseudo = args.Contains("--pseudo");
bool showImports = args.Contains("--imports");
bool registryMode = args.Contains("--registry");
bool scanMaps = args.Contains("--scan-maps");
bool allKeys = args.Contains("--all-keys");
bool savePackage = args.Contains("--save-package");
bool saveFolder = args.Contains("--save-folder");
bool saveClosure = args.Contains("--save-closure");
bool exportGlb = args.Contains("--export-glb");
bool exportPng = args.Contains("--export-png");
bool materialReport = args.Contains("--material-report");
bool ue426 = args.Contains("--ue426");
Globals.FatalObjectSerializationErrors = exportGlb || exportPng || materialReport;
string? maxArgument = args.SingleOrDefault(arg => arg.StartsWith("--max-packages=", StringComparison.Ordinal));
int maxPackages = 250;
if (maxArgument is not null && (!int.TryParse(maxArgument[15..], out maxPackages) || maxPackages < 1))
{
    Console.Error.WriteLine("--max-packages must be a positive integer");
    return 2;
}
string[] positional = args.Where(arg => arg != maxArgument && arg is not ("--all" or "--pseudo" or "--imports" or "--registry" or "--scan-maps" or "--all-keys" or "--save-package" or "--save-folder" or "--save-closure" or "--export-glb" or "--export-png" or "--material-report" or "--ue426")).ToArray();
bool packageLess = registryMode || scanMaps;
if ((registryMode && scanMaps) || (packageLess ? positional.Length > 1 : positional.Length is < 1 or > 2))
{
    Console.Error.WriteLine("Usage: Cue4Inspect <asset-path-without-extension> [pak-directory] [--all] [--pseudo] [--imports] [--save-package] [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect <folder-prefix> [pak-directory] --save-folder [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect <asset-path> [pak-directory] --save-closure [--max-packages=250] [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect <static-mesh-path> [pak-directory] --export-glb [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect <texture-path> [pak-directory] --export-png [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect <material-path> [pak-directory] --material-report [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect --registry [pak-directory] [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect --scan-maps [pak-directory] [--all-keys]");
    return 2;
}

string? assetPath = packageLess ? null : positional[0];
string pakDirectory = positional.Length == (packageLess ? 1 : 2)
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
        new VersionContainer(ue426 ? EGame.GAME_UE4_26 : EGame.GAME_UE4_25_Plus,
            optionOverrides: new Dictionary<string, bool>
            {
                ["StaticMesh.Fortnite1260NoForceOpaque"] = true,
                ["RawIndexBuffer.HasShouldExpandTo32Bit"] = false,
                ["StaticMesh.HasRayTracingGeometry"] = false
            }),
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

    if (scanMaps)
    {
        var maps = provider.Files.Values
            .Where(file => file.Path.EndsWith(".umap", StringComparison.OrdinalIgnoreCase))
            .OrderBy(file => file.Path)
            .ToArray();
        Console.Error.WriteLine($"Scanning {maps.Length} map package(s)");
        int scanned = 0;
        foreach (var map in maps)
        {
            try
            {
                if (provider.LoadPackage(map) is not Package package)
                    throw new InvalidDataException("Not a legacy package");
                var roots = package.ImportMap.Select(import => RootImportName(package, import)).ToArray();
                Console.WriteLine(JsonSerializer.Serialize(new
                {
                    path = map.Path,
                    imports = package.ImportMap.Length,
                    exports = package.ExportMap.Length,
                    scriptModules = roots.Where(root => root.StartsWith("/Script/", StringComparison.Ordinal)).Distinct().Order().ToArray(),
                    assetImports = roots.Count(root => root.StartsWith("/Game/", StringComparison.Ordinal))
                }));
            }
            catch (Exception error)
            {
                Console.WriteLine(JsonSerializer.Serialize(new { path = map.Path, error = error.GetType().Name + ": " + error.Message }));
            }
            if (++scanned % 100 == 0) Console.Error.WriteLine($"Scanned {scanned}/{maps.Length}");
        }
        return 0;
    }

    if (saveClosure)
    {
        var pending = new Queue<string>();
        var visited = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        pending.Enqueue(assetPath!);
        int failed = 0;
        while (pending.Count > 0 && visited.Count < maxPackages)
        {
            var current = pending.Dequeue();
            if (!visited.Add(current)) continue;
            try
            {
                if (provider.LoadPackage(current) is not Package package)
                    throw new InvalidDataException("Not a legacy package");
                SaveRecoveredFiles(provider.SavePackage(current));
                foreach (var root in package.ImportMap.Select(import => RootImportName(package, import)).Distinct())
                {
                    if (root.StartsWith("/Game/", StringComparison.Ordinal))
                        pending.Enqueue("FortniteGame/Content" + root[5..]);
                    else if (root.StartsWith("/Engine/", StringComparison.Ordinal))
                        pending.Enqueue("Engine/Content" + root[7..]);
                    else if (root.StartsWith("/", StringComparison.Ordinal) &&
                             !root.StartsWith("/Script/", StringComparison.Ordinal))
                        Console.Error.WriteLine($"Unmapped package root: {root}");
                }
            }
            catch (Exception error)
            {
                failed++;
                Console.Error.WriteLine($"Could not recover {current}: {error.GetType().Name}: {error.Message}");
            }
            if (visited.Count % 25 == 0) Console.Error.WriteLine($"Recovered {visited.Count} package(s); pending {pending.Count}");
        }
        Console.Error.WriteLine($"Closure: {visited.Count} visited, {failed} failed, {pending.Count} pending");
        return failed == 0 && pending.Count == 0 ? 0 : 1;
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

    var packageData = provider.LoadPackage(assetPath!);
    if (exportPng)
    {
        var texture = packageData.GetExports().OfType<UTexture2D>().FirstOrDefault()
            ?? throw new InvalidDataException("Package has no Texture2D export");
        var decoded = texture.Decode()
            ?? throw new InvalidDataException("Texture has no decodable mip data");
        var png = EncodeRgbaPng(decoded);
        var destination = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "../../../../generated/converted"));
        var relative = assetPath!.Replace('\\', '/').TrimStart('/');
        if (relative.Split('/').Any(part => part is ".." or "."))
            throw new InvalidDataException("Unsafe asset path");
        var target = Path.GetFullPath(Path.Combine(destination, relative + ".png"));
        if (!target.StartsWith(destination + Path.DirectorySeparatorChar, StringComparison.Ordinal))
            throw new InvalidDataException("Export path escapes output directory");
        Directory.CreateDirectory(Path.GetDirectoryName(target)!);
        File.WriteAllBytes(target, png);
        Console.WriteLine($"Exported {target} ({decoded.Width}x{decoded.Height}, {png.Length} bytes)");
        return 0;
    }
    if (materialReport)
    {
        var materials = packageData.GetExports().OfType<UMaterialInterface>().ToArray();
        var imports = packageData is Package materialPackage
            ? materialPackage.ImportMap
                .Select(import => new
                {
                    root = RootImportName(materialPackage, import),
                    className = import.ClassName.Text,
                    objectName = import.ObjectName.Text
                })
                .Where(import => import.root.StartsWith("/Game/", StringComparison.Ordinal) ||
                                 import.root.StartsWith("/Engine/", StringComparison.Ordinal))
                .ToArray()
            : [];
        Console.WriteLine(JsonSerializer.Serialize(new
        {
            path = assetPath,
            materials = materials.Select(material => new
            {
                name = material.Name,
                type = material.GetType().Name,
                blendMode = (material as UMaterial)?.BlendMode.ToString(),
                shadingModel = (material as UMaterial)?.ShadingModel.ToString(),
                twoSided = (material as UMaterial)?.TwoSided,
                expressionCount = (material as UMaterial)?.Expressions.Length,
                referencedTextures = (material as UMaterial)?.ReferencedTextures.Select(reference => reference?.Name).ToArray(),
                parent = (material as UMaterialInstance)?.Parent?.Name,
                staticPermutation = (material as UMaterialInstance)?.bHasStaticPermutationResource
            }).ToArray(),
            imports
        }, new JsonSerializerOptions { WriteIndented = true }));
        return 0;
    }
    if (exportGlb)
    {
        var mesh = packageData.GetExports().OfType<UStaticMesh>().FirstOrDefault()
            ?? throw new InvalidDataException("Package has no static mesh export");
        if (packageData is Package meshPackage)
            Console.WriteLine($"Package flags: {meshPackage.Summary.PackageFlags}");
        Console.WriteLine($"Mesh cooked={mesh.bCooked}, render data={mesh.RenderData is not null}, render LODs={mesh.RenderData?.LODs?.Length ?? 0}, materials={mesh.StaticMaterials.Length}");
        using var dto = new StaticMeshDto(mesh, EMeshQuality.Highest);
        var options = new ExportOptions(meshFormat: EMeshFormat.Gltf2, exportMaterials: false);
        var files = new GltfMeshFormat().BuildStaticMesh(mesh.Name, assetPath!, options, dto);
        var destination = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "../../../../generated/converted"));
        var relative = assetPath!.Replace('\\', '/').TrimStart('/');
        if (relative.Split('/').Any(part => part is ".." or "."))
            throw new InvalidDataException("Unsafe asset path");
        foreach (var file in files)
        {
            var target = Path.GetFullPath(Path.Combine(destination, relative + file.NameSuffix + "." + file.Extension));
            if (!target.StartsWith(destination + Path.DirectorySeparatorChar, StringComparison.Ordinal))
                throw new InvalidDataException("Export path escapes output directory");
            Directory.CreateDirectory(Path.GetDirectoryName(target)!);
            File.WriteAllBytes(target, file.Data);
            Console.WriteLine($"Exported {target} ({file.Data.Length} bytes)");
        }
        return 0;
    }
    if (showImports && packageData is Package legacyPackage)
    {
        Console.WriteLine($"Imports: {legacyPackage.ImportMap.Length}");
        foreach (var import in legacyPackage.ImportMap)
        {
            var root = RootImportName(legacyPackage, import);
            if (root.StartsWith("/Script/", StringComparison.Ordinal))
                Console.WriteLine($"{root} | {import.ClassName.Text} | {import.ObjectName.Text}");
        }
    }

    var exports = packageData.GetExports();
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
    Console.Error.WriteLine(exportGlb || exportPng || materialReport ? $"Inspection failed: {error}" : $"Inspection failed: {error.GetType().Name}: {error.Message}");
    return 1;
}

static byte[] EncodeRgbaPng(CTexture texture)
{
    var pixelCount = checked(texture.Width * texture.Height);
    byte[] rgba;
    switch (texture.PixelFormat)
    {
        case EPixelFormat.PF_R8G8B8A8:
            rgba = texture.Data;
            break;
        case EPixelFormat.PF_B8G8R8A8:
        case EPixelFormat.PF_A8R8G8B8:
            rgba = (byte[])texture.Data.Clone();
            for (var i = 0; i < rgba.Length; i += 4)
                (rgba[i], rgba[i + 2]) = (rgba[i + 2], rgba[i]);
            break;
        case EPixelFormat.PF_R8:
        case EPixelFormat.PF_G8:
            rgba = new byte[checked(pixelCount * 4)];
            for (var i = 0; i < pixelCount; i++)
            {
                rgba[4 * i] = rgba[4 * i + 1] = rgba[4 * i + 2] = texture.Data[i];
                rgba[4 * i + 3] = 255;
            }
            break;
        default:
            throw new NotSupportedException($"PNG conversion does not support decoded {texture.PixelFormat}");
    }
    if (rgba.Length != checked(pixelCount * 4))
        throw new InvalidDataException("Decoded texture byte count does not match its dimensions");
    using var image = Image.LoadPixelData<Rgba32>(rgba, texture.Width, texture.Height);
    using var buffer = new MemoryStream();
    image.SaveAsPng(buffer);
    return buffer.ToArray();
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

static string RootImportName(Package package, FObjectImport import)
{
    var root = import;
    for (int depth = 0; depth < package.ImportMap.Length; depth++)
    {
        if (root.OuterIndex is not { IsImport: true } outer) break;
        int index = -outer.Index - 1;
        if (index < 0 || index >= package.ImportMap.Length) break;
        root = package.ImportMap[index];
    }
    return root.ObjectName.Text;
}
