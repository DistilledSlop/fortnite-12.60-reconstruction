using CUE4Parse;
using CUE4Parse.Encryption.Aes;
using CUE4Parse.FileProvider;
using CUE4Parse.UE4.Assets;
using CUE4Parse.UE4.AssetRegistry;
using CUE4Parse.UE4.Assets.Exports.StaticMesh;
using CUE4Parse.UE4.Assets.Exports.Actor;
using CUE4Parse.UE4.Assets.Exports.Component.Landscape;
using CUE4Parse.UE4.Assets.Exports.Component;
using CUE4Parse.UE4.Assets.Exports.Component.StaticMesh;
using CUE4Parse.UE4.Assets.Exports.Material;
using CUE4Parse.UE4.Assets.Exports.Texture;
using CUE4Parse.UE4.Objects.Core.Misc;
using CUE4Parse.UE4.Objects.Engine;
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
bool scanNative = args.Contains("--scan-native");
bool allKeys = args.Contains("--all-keys");
bool savePackage = args.Contains("--save-package");
bool saveList = args.Contains("--save-list");
bool saveFolder = args.Contains("--save-folder");
bool saveClosure = args.Contains("--save-closure");
bool exportGlb = args.Contains("--export-glb");
bool exportLandscape = args.Contains("--export-landscape");
bool exportLandscapeGrid = args.Contains("--export-landscape-grid");
bool exportLandscapeWeightmapGrid = args.Contains("--export-landscape-weightmap-grid");
bool landscapeReport = args.Contains("--landscape-report");
bool actorReport = args.Contains("--actor-report");
bool exportPng = args.Contains("--export-png");
bool exportLandscapeWeightmaps = args.Contains("--export-landscape-weightmaps");
bool materialReport = args.Contains("--material-report");
bool functionReport = args.Contains("--function-report");
bool versionReport = args.Contains("--versions");
bool sizeList = args.Contains("--sizes");
bool ue426 = args.Contains("--ue426");
bool loose = args.Contains("--loose");
Globals.FatalObjectSerializationErrors = exportGlb || exportLandscape || exportLandscapeGrid || exportLandscapeWeightmapGrid || exportPng || exportLandscapeWeightmaps || materialReport || functionReport || landscapeReport || actorReport;
string? keyFileArgument = args.SingleOrDefault(arg => arg.StartsWith("--key-file=", StringComparison.Ordinal));
string? keyFilePath = keyFileArgument is not null ? keyFileArgument["--key-file=".Length..] : Environment.GetEnvironmentVariable("FORTNITE_AES_KEY_FILE");
string? maxArgument = args.SingleOrDefault(arg => arg.StartsWith("--max-packages=", StringComparison.Ordinal));
int maxPackages = 250;
if (maxArgument is not null && (!int.TryParse(maxArgument[15..], out maxPackages) || maxPackages < 1))
{
    Console.Error.WriteLine("--max-packages must be a positive integer");
    return 2;
}
string[] positional = args.Where(arg => arg != maxArgument && arg != keyFileArgument && arg is not ("--all" or "--pseudo" or "--imports" or "--registry" or "--scan-maps" or "--scan-native" or "--all-keys" or "--save-package" or "--save-list" or "--sizes" or "--save-folder" or "--save-closure" or "--export-glb" or "--export-landscape" or "--export-landscape-grid" or "--export-landscape-weightmap-grid" or "--export-png" or "--export-landscape-weightmaps" or "--material-report" or "--function-report" or "--landscape-report" or "--actor-report" or "--versions" or "--ue426" or "--loose")).ToArray();
bool packageLess = registryMode || scanMaps || scanNative || exportLandscapeGrid || exportLandscapeWeightmapGrid;
if (new[] { registryMode, scanMaps, scanNative, exportLandscapeGrid, exportLandscapeWeightmapGrid }.Count(mode => mode) > 1 || (packageLess ? positional.Length > 1 : positional.Length is < 1 or > 2))
{
    Console.Error.WriteLine("Usage: Cue4Inspect <asset-path-without-extension> [pak-directory] [--all] [--pseudo] [--imports] [--save-package] [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect <folder-prefix> [pak-directory] --save-folder [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect <newline-list-file> [pak-directory] --save-list [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect <newline-list-file> [pak-directory] --sizes [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect <asset-path> [pak-directory] --save-closure [--max-packages=250] [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect <static-mesh-path> [pak-directory] --export-glb [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect <landscape-map-path> [pak-directory] --export-landscape [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect --export-landscape-grid [pak-directory] [--max-packages=64] [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect --export-landscape-weightmap-grid [pak-directory] [--max-packages=64] [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect <texture-path> [pak-directory] --export-png [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect <texture-path> [extracted-root] --export-png --loose");
    Console.Error.WriteLine("   or: Cue4Inspect <landscape-map-path> [source-directory] --export-landscape-weightmaps [--loose]");
    Console.Error.WriteLine("   or: Cue4Inspect <material-path> [pak-directory] --material-report [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect <landscape-map-path> [pak-directory] --landscape-report [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect <map-path> [pak-directory] --actor-report [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect <blueprint-path> [pak-directory] --function-report [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect <package-path> [pak-directory] --versions [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect --registry [pak-directory] [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect --scan-maps [pak-directory] [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect --scan-native [pak-directory] [--all-keys]");
    return 2;
}

string? assetPath = packageLess ? null : positional[0];
string sourceDirectory = positional.Length == (packageLess ? 1 : 2)
    ? positional[^1]
    : Path.GetFullPath(Path.Combine(AppContext.BaseDirectory,
        loose ? "../../../../generated/extracted" : "../../../../../Fortnite/FortniteGame/Content/Paks"));

if (!Directory.Exists(sourceDirectory))
{
    Console.Error.WriteLine($"Source directory does not exist: {sourceDirectory}");
    return 2;
}

try
{
    var versions = new VersionContainer(ue426 ? EGame.GAME_UE4_26 : EGame.GAME_UE4_25_Plus,
        optionOverrides: new Dictionary<string, bool>
        {
            ["StaticMesh.Fortnite1260NoForceOpaque"] = true,
            ["RawIndexBuffer.HasShouldExpandTo32Bit"] = false,
            ["StaticMesh.HasRayTracingGeometry"] = false
        });
    var provider = loose
        ? new DefaultFileProvider(new DirectoryInfo(Path.Combine(sourceDirectory, "FortniteGame")),
            [new DirectoryInfo(Path.Combine(sourceDirectory, "Engine"))],
            SearchOption.AllDirectories, versions, StringComparer.OrdinalIgnoreCase)
        : new DefaultFileProvider(sourceDirectory, SearchOption.TopDirectoryOnly,
            versions, StringComparer.OrdinalIgnoreCase);
    provider.ReadScriptData = showPseudo;
    provider.Initialize();

    if (loose)
    {
        Console.WriteLine($"Indexed {provider.LooseFileCount} loose package(s).");
    }
    else
    {
        var fileKeys = LoadKeyFile(keyFilePath);
        string key = fileKeys.TryGetValue("primary", out var filePrimary) ? filePrimary : ReadHiddenKey("Primary AES key: ");
        int mounted = provider.SubmitKey(new FGuid(), new FAesKey("0x" + key));
        Console.WriteLine($"Mounted {mounted} pak(s) with the primary key.");
        if (allKeys)
        {
            foreach (var guid in provider.RequiredKeys.Where(guid => guid.IsValid()).OrderBy(guid => guid.ToString(EGuidFormats.Digits)))
            {
                string guidDigits = guid.ToString(EGuidFormats.Digits).ToLowerInvariant();
                string extraKey = fileKeys.TryGetValue(guidDigits, out var fileExtra) ? fileExtra : ReadHiddenKey($"AES key for GUID {guid.ToString(EGuidFormats.Digits)}: ");
                mounted += provider.SubmitKey(guid, new FAesKey("0x" + extraKey));
            }
            Console.WriteLine($"Mounted {mounted} pak(s) in total.");
        }
    }

    if (exportLandscapeGrid)
    {
        var manifest = new List<object>();
        int succeeded = 0, empty = 0, absent = 0, failed = 0;
        foreach (char row in "ABCDEFGH")
        {
            for (int column = 1; column <= 8; column++)
            {
                if (succeeded + empty + absent + failed >= maxPackages) break;
                var tile = $"{row}{column}";
                var path = $"FortniteGame/Content/Athena/Apollo/Maps/Landscape/Apollo_Terrain_LS_{tile}";
                try
                {
                    var package = provider.LoadPackage(path);
                    var actors = package.GetExports().OfType<ALandscapeProxy>().ToArray();
                    if (actors.Length == 0)
                        throw new InvalidDataException("No landscape actor export");
                    if (actors.All(actor => actor.LandscapeComponents.Length == 0))
                    {
                        Console.WriteLine($"Landscape {tile} has no components; skipping mesh export");
                        empty++;
                        continue;
                    }
                    ExportLandscapes(package, path);
                    foreach (var actor in actors.Where(actor => actor.LandscapeComponents.Length > 0))
                    {
                        var root = actor.GetOrDefault<FPackageIndex?>("RootComponent")?.Load<USceneComponent>()
                            ?? throw new InvalidDataException($"No root component for {actor.Name}");
                        manifest.Add(new
                        {
                            tile,
                            actor = actor.Name,
                            mesh = $"/Game/Athena/Apollo/Maps/Landscape/Converted/{actor.Name}.{actor.Name}",
                            location = new[] { root.RelativeLocation.X, root.RelativeLocation.Y, root.RelativeLocation.Z },
                            scale = new[] { root.RelativeScale3D.X, root.RelativeScale3D.Y, root.RelativeScale3D.Z },
                            components = actor.LandscapeComponents.Length
                        });
                    }
                    succeeded++;
                }
                catch (KeyNotFoundException)
                {
                    Console.WriteLine($"Landscape {tile} has no package in this pak set");
                    absent++;
                }
                catch (Exception error)
                {
                    Console.Error.WriteLine($"Landscape {tile} failed: {error.GetType().Name}: {error.Message}");
                    failed++;
                }
            }
        }
        var manifestPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "../../../../generated/converted/apollo-landscape-grid.json"));
        Directory.CreateDirectory(Path.GetDirectoryName(manifestPath)!);
        File.WriteAllText(manifestPath, JsonSerializer.Serialize(manifest, new JsonSerializerOptions { WriteIndented = true }));
        Console.WriteLine($"Landscape grid exported: {succeeded} meshes, {empty} empty, {absent} absent, {failed} failed; manifest: {manifestPath}");
        return failed == 0 ? 0 : 1;
    }

    if (exportLandscapeWeightmapGrid)
    {
        int succeeded = 0, empty = 0, absent = 0, failed = 0;
        foreach (char row in "ABCDEFGH")
        {
            for (int column = 1; column <= 8; column++)
            {
                if (succeeded + empty + absent + failed >= maxPackages) break;
                var tile = $"{row}{column}";
                var path = $"FortniteGame/Content/Athena/Apollo/Maps/Landscape/Apollo_Terrain_LS_{tile}";
                try
                {
                    var package = provider.LoadPackage(path);
                    if (!package.GetExports().OfType<ALandscapeProxy>()
                        .Any(actor => actor.LandscapeComponents.Length > 0))
                    {
                        Console.WriteLine($"Landscape {tile} has no components; skipping weightmaps");
                        empty++;
                        continue;
                    }
                    ExportLandscapeWeightmaps(package, path);
                    succeeded++;
                }
                catch (KeyNotFoundException)
                {
                    Console.WriteLine($"Landscape {tile} has no package in this pak set");
                    absent++;
                }
                catch (Exception error)
                {
                    Console.Error.WriteLine($"Landscape {tile} failed: {error.GetType().Name}: {error.Message}");
                    failed++;
                }
            }
        }
        Console.WriteLine($"Landscape weightmap grid exported: {succeeded} tiles, {empty} empty, {absent} absent, {failed} failed");
        return failed == 0 ? 0 : 1;
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

    if (scanMaps || scanNative)
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
                if (scanNative)
                {
                    var nativeImports = package.ImportMap
                        .Select(import => new { root = RootImportName(package, import), kind = import.ClassName.Text, name = import.ObjectName.Text })
                        .Where(import => import.root.StartsWith("/Script/", StringComparison.Ordinal) &&
                                         import.kind is "Class" or "Struct" or "Enum" or "Function")
                        .Select(import => $"{import.root}.{import.name}|{import.kind}")
                        .Distinct()
                        .Order()
                        .ToArray();
                    Console.WriteLine(JsonSerializer.Serialize(new { path = map.Path, nativeImports }));
                }
                else
                {
                    Console.WriteLine(JsonSerializer.Serialize(new
                    {
                        path = map.Path,
                        imports = package.ImportMap.Length,
                        exports = package.ExportMap.Length,
                        scriptModules = roots.Where(root => root.StartsWith("/Script/", StringComparison.Ordinal)).Distinct().Order().ToArray(),
                        assetImports = roots.Count(root => root.StartsWith("/Game/", StringComparison.Ordinal))
                    }));
                }
            }
            catch (Exception error)
            {
                Console.WriteLine(JsonSerializer.Serialize(new { path = map.Path, error = error.GetType().Name + ": " + error.Message }));
            }
            if (++scanned % 100 == 0) Console.Error.WriteLine($"Scanned {scanned}/{maps.Length}");
        }
        return 0;
    }

    if (sizeList)
    {
        var suffixes = new[] { ".umap", ".uasset", ".uexp", ".ubulk", ".uptnl" };
        foreach (var package in File.ReadLines(Path.GetFullPath(assetPath!))
            .Select(line => line.Trim())
            .Where(line => line.Length > 0 && !line.StartsWith('#'))
            .Distinct(StringComparer.OrdinalIgnoreCase))
        {
            var files = suffixes
                .Select(suffix => provider.Files.TryGetValue(package + suffix, out var file)
                    ? new { suffix, bytes = file.Size }
                    : null)
                .Where(file => file is not null)
                .ToArray();
            Console.WriteLine(JsonSerializer.Serialize(new
            {
                path = package,
                bytes = files.Sum(file => file!.bytes),
                files
            }));
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

    if (saveList)
    {
        var listPath = Path.GetFullPath(assetPath!);
        var packages = File.ReadLines(listPath)
            .Select(line => line.Trim())
            .Where(line => line.Length > 0 && !line.StartsWith('#'))
            .Distinct(StringComparer.OrdinalIgnoreCase)
            .ToArray();
        int failed = 0;
        foreach (var package in packages)
        {
            if (!(package.StartsWith("FortniteGame/Content/", StringComparison.Ordinal) ||
                  package.StartsWith("Engine/Content/", StringComparison.Ordinal)) ||
                package.Split('/').Any(part => part is ".." or "."))
            {
                Console.Error.WriteLine($"Unsafe package path: {package}");
                failed++;
                continue;
            }
            try
            {
                SaveRecoveredFiles(provider.SavePackage(package));
            }
            catch (Exception error)
            {
                Console.Error.WriteLine($"Could not recover {package}: {error.GetType().Name}: {error.Message}");
                failed++;
            }
        }
        Console.Error.WriteLine($"List recovery: {packages.Length - failed}/{packages.Length} packages");
        return failed == 0 ? 0 : 1;
    }

    if (savePackage)
    {
        SaveRecoveredFiles(provider.SavePackage(assetPath!));
    }

    var packageData = provider.LoadPackage(assetPath!);
    if (versionReport)
    {
        if (packageData is not Package legacy)
            throw new InvalidDataException("Not a legacy package");
        var summary = legacy.Summary;
        Console.WriteLine(JsonSerializer.Serialize(new
        {
            path = assetPath,
            ue4 = summary.FileVersionUE.FileVersionUE4,
            licensee = (int) summary.FileVersionLicenseeUE,
            fortniteMain = summary.CustomVersionContainer?.GetVersion(FFortniteMainBranchObjectVersion.GUID),
            release = summary.CustomVersionContainer?.GetVersion(FReleaseObjectVersion.GUID),
            customVersions = summary.CustomVersionContainer?.Versions.Length,
            exports = summary.ExportCount,
            imports = summary.ImportCount
        }));
        return 0;
    }
    if (functionReport)
    {
        var functionExports = packageData.GetExports().ToArray();
        Console.WriteLine(JsonSerializer.Serialize(new
        {
            path = assetPath,
            classes = functionExports.OfType<UClass>().Select(blueprint => new
            {
                name = blueprint.Name,
                super = blueprint.SuperStruct.Name,
                properties = blueprint.ChildProperties?.Select(property => new
                {
                    name = property.Name.Text,
                    type = property.GetType().Name
                }).ToArray()
            }).ToArray(),
            functions = functionExports.OfType<UFunction>().Select(function => new
            {
                name = function.Name,
                super = function.SuperStruct.Name,
                flags = function.FunctionFlags.ToString(),
                parameters = function.ChildProperties?.OfType<FProperty>()
                    .Where(property => property.PropertyFlags.HasFlag(EPropertyFlags.Parm))
                    .Select(property => new
                    {
                        name = property.Name.Text,
                        type = property.GetType().Name,
                        flags = property.PropertyFlags.ToString()
                    }).ToArray()
            }).ToArray()
        }, new JsonSerializerOptions { WriteIndented = true }));
        return 0;
    }
    if (actorReport)
    {
        var mapExports = packageData.GetExports().ToArray();
        var mapPackage = packageData as Package;
        Console.WriteLine(JsonSerializer.Serialize(new
        {
            path = assetPath,
            exportTypes = mapExports.GroupBy(item => item.ExportType)
                .OrderByDescending(group => group.Count())
                .Select(group => new { type = group.Key, count = group.Count() }).ToArray(),
            staticMeshComponents = mapExports.OfType<UStaticMeshComponent>().Select(component =>
            {
                var mesh = component.GetStaticMesh();
                var transform = component.GetComponentTransform();
                return new
                {
                    name = component.Name,
                    path = component.GetPathName(),
                    owner = component.GetTypedOuter<AActor>()?.Name ?? component.Outer?.Name.Text,
                    mesh = mesh.Name,
                    meshPackage = mesh.IsImport && mapPackage is not null
                        ? RootImportName(mapPackage, mapPackage.ImportMap[-mesh.Index - 1]) : null,
                    overrideMaterials = component.OverrideMaterials.Select(material => new
                    {
                        name = material?.Name,
                        package = material is { IsImport: true } && mapPackage is not null
                            ? RootImportName(mapPackage, mapPackage.ImportMap[-material.Index - 1]) : null
                    }).ToArray(),
                    location = new[] { transform.Translation.X, transform.Translation.Y, transform.Translation.Z },
                    rotation = new[] { transform.Rotator().Pitch, transform.Rotator().Yaw, transform.Rotator().Roll },
                    scale = new[] { transform.Scale3D.X, transform.Scale3D.Y, transform.Scale3D.Z }
                };
            }).ToArray()
        }, new JsonSerializerOptions { WriteIndented = true }));
        return 0;
    }
    if (landscapeReport)
    {
        var actors = packageData.GetExports().OfType<ALandscapeProxy>().ToArray();
        Console.WriteLine(JsonSerializer.Serialize(new
        {
            path = assetPath,
            actors = actors.Select(actor => new
            {
                name = actor.Name,
                type = actor.GetType().Name,
                material = actor.LandscapeMaterial.Name,
                componentSizeQuads = actor.ComponentSizeQuads,
                root = actor.GetOrDefault<FPackageIndex?>("RootComponent")?.Load<USceneComponent>() is { } root
                    ? new
                    {
                        name = root.Name,
                        location = new[] { root.RelativeLocation.X, root.RelativeLocation.Y, root.RelativeLocation.Z },
                        scale = new[] { root.RelativeScale3D.X, root.RelativeScale3D.Y, root.RelativeScale3D.Z }
                    }
                    : null,
                components = actor.LandscapeComponents.Select(index =>
                {
                    var component = index.Load<ULandscapeComponent>();
                    return component is null ? null : new
                    {
                        name = component.Name,
                        sectionBase = new[] { component.SectionBaseX, component.SectionBaseY },
                        location = new[] { component.RelativeLocation.X, component.RelativeLocation.Y, component.RelativeLocation.Z },
                        scale = new[] { component.RelativeScale3D.X, component.RelativeScale3D.Y, component.RelativeScale3D.Z },
                        quads = component.ComponentSizeQuads,
                        materialInstances = component.GetOrDefault<FPackageIndex[]>("MaterialInstances", [])
                            .Select(reference => reference.Name).ToArray(),
                        weightmapTextures = component.GetOrDefault<FPackageIndex[]>("WeightmapTextures", [])
                            .Select(reference => reference.Name).ToArray(),
                        layers = component.WeightmapLayerAllocations.Select(layer => new
                        {
                            name = layer.GetLayerName(),
                            textureIndex = layer.WeightmapTextureIndex,
                            channel = layer.WeightmapTextureChannel
                        }).ToArray()
                    };
                }).ToArray()
            }).ToArray()
        }, new JsonSerializerOptions { WriteIndented = true }));
        return 0;
    }
    if (exportLandscape)
    {
        ExportLandscapes(packageData, assetPath!);
        return 0;
    }
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
    if (exportLandscapeWeightmaps)
    {
        ExportLandscapeWeightmaps(packageData, assetPath!);
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
            materialProperties = materials.Select(material => new
            {
                name = material.Name,
                expressionRefs = (material as UMaterial)?.Expressions.Select(reference => new
                {
                    name = reference.Name,
                    index = reference.Index,
                    isImport = reference.IsImport
                }).ToArray(),
                rootInputs = new[] { "BaseColor", "EmissiveColor", "Opacity", "OpacityMask", "Normal", "WorldPositionOffset" }
                    .Select(inputName => new
                    {
                        name = inputName,
                        input = material.GetOrDefault<FExpressionInput?>(inputName)
                    })
                    .Where(item => item.input is not null)
                    .Select(item => new
                    {
                        item.name,
                        expression = item.input!.Expression?.Name,
                        expressionIndex = item.input.Expression?.Index,
                        expressionName = item.input.ExpressionName.Text,
                        item.input.OutputIndex,
                        item.input.Mask,
                        item.input.MaskR,
                        item.input.MaskG,
                        item.input.MaskB,
                        item.input.MaskA
                    }).ToArray(),
                properties = material.Properties.Select(property => new
                {
                    name = property.Name.Text,
                    type = property.PropertyType.Text,
                    value = property.Tag?.ToString()
                }).ToArray()
            }).ToArray(),
            instanceParameters = packageData.GetExports().OfType<UMaterialInstanceConstant>()
                .Select(instance => new
                {
                    name = instance.Name,
                    textures = instance.TextureParameterValues.Select(value => new
                    {
                        parameter = value.Name,
                        texture = value.ParameterValue?.Name,
                        package = value.ParameterValue is { IsImport: true } && packageData is Package p
                            ? RootImportName(p, p.ImportMap[-value.ParameterValue.Index - 1]) : null
                    }).ToArray(),
                    scalars = instance.ScalarParameterValues.Select(value => new
                    {
                        parameter = value.Name,
                        value = value.ParameterValue
                    }).ToArray(),
                    vectors = instance.VectorParameterValues.Select(value => new
                    {
                        parameter = value.Name,
                        value = value.ParameterValue is not { } color ? null : new[]
                        {
                            color.R, color.G, color.B, color.A
                        }
                    }).ToArray()
                }).ToArray(),
            expressionExports = packageData.GetExports()
                .Where(item => item.ExportType.StartsWith("MaterialExpression", StringComparison.Ordinal))
                .Select(item => new
                {
                    name = item.Name,
                    exportType = item.ExportType,
                    parserType = item.GetType().Name,
                    properties = item.Properties.Select(property => new
                    {
                        name = property.Name.Text,
                        type = property.PropertyType.Text,
                        value = property.Tag?.ToString()
                    }).ToArray()
                }).ToArray(),
            textureSamples = packageData.GetExports().OfType<UMaterialExpressionTextureSample>()
                .Select(sample => new
                {
                    name = sample.Name,
                    texture = sample.Texture?.Name,
                    samplerType = sample.SamplerType.ToString(),
                    properties = sample.Properties.Select(property => property.Name.Text).ToArray()
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

static void ExportLandscapes(IPackage package, string assetPath)
{
    var actors = package.GetExports().OfType<ALandscapeProxy>()
        .Where(actor => actor.LandscapeComponents.Length > 0).ToArray();
    if (actors.Length == 0)
        throw new InvalidDataException("Package has no landscape actor with components");
    var destination = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "../../../../generated/converted"));
    var relative = assetPath.Replace('\\', '/').TrimStart('/');
    if (relative.Split('/').Any(part => part is ".." or "."))
        throw new InvalidDataException("Unsafe asset path");
    var options = new ExportOptions(meshFormat: EMeshFormat.Gltf2, exportMaterials: false);
    foreach (var actor in actors)
    {
        using var dto = new LandscapeMeshDto(actor);
        var files = new GltfMeshFormat().BuildStaticMesh(actor.Name, assetPath, options, dto);
        foreach (var file in files)
        {
            var target = Path.GetFullPath(Path.Combine(destination, relative + "/" + actor.Name + file.NameSuffix + "." + file.Extension));
            if (!target.StartsWith(destination + Path.DirectorySeparatorChar, StringComparison.Ordinal))
                throw new InvalidDataException("Export path escapes output directory");
            Directory.CreateDirectory(Path.GetDirectoryName(target)!);
            File.WriteAllBytes(target, file.Data);
            Console.WriteLine($"Exported {target} ({file.Data.Length} bytes)");
        }
    }
}

static void ExportLandscapeWeightmaps(IPackage package, string assetPath)
{
    var destination = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "../../../../generated/converted"));
    var relative = assetPath.Replace('\\', '/').TrimStart('/');
    if (relative.Split('/').Any(part => part is ".." or "."))
        throw new InvalidDataException("Unsafe asset path");
    var folder = Path.GetFullPath(Path.Combine(destination, relative, "weightmaps"));
    if (!folder.StartsWith(destination + Path.DirectorySeparatorChar, StringComparison.Ordinal))
        throw new InvalidDataException("Export path escapes output directory");
    Directory.CreateDirectory(folder);

    var components = new List<object>();
    foreach (var actor in package.GetExports().OfType<ALandscapeProxy>())
    {
        foreach (var reference in actor.LandscapeComponents)
        {
            var component = reference.Load<ULandscapeComponent>()
                ?? throw new InvalidDataException($"Missing landscape component in {actor.Name}");
            if (!component.Name.All(character => char.IsLetterOrDigit(character) || character == '_'))
                throw new InvalidDataException($"Unsafe component name: {component.Name}");
            var filenames = new List<string>();
            var textures = component.GetWeightmapTextures();
            for (var index = 0; index < textures.Length; index++)
            {
                var decoded = textures[index].Decode()
                    ?? throw new InvalidDataException($"Weightmap {index} on {component.Name} has no decodable mip");
                var png = EncodeRgbaPng(decoded);
                var filename = $"{component.Name}-{index}.png";
                File.WriteAllBytes(Path.Combine(folder, filename), png);
                filenames.Add(filename);
                Console.WriteLine($"Exported {filename} ({decoded.Width}x{decoded.Height}, {png.Length} bytes)");
            }
            components.Add(new
            {
                actor = actor.Name,
                component = component.Name,
                sectionBase = new[] { component.SectionBaseX, component.SectionBaseY },
                textures = filenames,
                layers = component.WeightmapLayerAllocations.Select(layer => new
                {
                    name = layer.GetLayerName(),
                    textureIndex = layer.WeightmapTextureIndex,
                    channel = layer.WeightmapTextureChannel
                }).ToArray()
            });
        }
    }
    if (components.Count == 0)
        throw new InvalidDataException("Package has no landscape components");
    var manifest = Path.Combine(folder, "layers.json");
    File.WriteAllText(manifest, JsonSerializer.Serialize(components, new JsonSerializerOptions { WriteIndented = true }));
    Console.WriteLine($"Exported {components.Count} component layer records to {manifest}");
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

// Reads "name=<64 hex>" lines (name is "primary" or a GUID in digits form). Blank lines and lines
// starting with '#' are ignored. The file must live outside the repository; key values are never printed.
static Dictionary<string, string> LoadKeyFile(string? path)
{
    var keys = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
    if (string.IsNullOrWhiteSpace(path)) return keys;
    foreach (var raw in File.ReadAllLines(path))
    {
        var line = raw.Trim();
        if (line.Length == 0 || line[0] == '#') continue;
        int separator = line.IndexOf('=');
        if (separator <= 0) throw new FormatException("Key file lines must look like name=<64 hex characters>");
        string name = line[..separator].Trim().ToLowerInvariant().Replace("-", "");
        string value = line[(separator + 1)..].Trim();
        if (value.StartsWith("0x", StringComparison.OrdinalIgnoreCase)) value = value[2..];
        if (value.Length != 64 || !value.All(Uri.IsHexDigit))
            throw new FormatException($"Key '{name}' in the key file is not a 64-character AES-256 hex key");
        keys[name] = value;
    }
    return keys;
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
