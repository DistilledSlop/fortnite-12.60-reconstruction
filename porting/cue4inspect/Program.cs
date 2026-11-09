using System.Diagnostics;
using CUE4Parse;
using CUE4Parse.Encryption.Aes;
using CUE4Parse.FileProvider;
using CUE4Parse.FileProvider.Objects;
using CUE4Parse.UE4.VirtualFileSystem;
using CUE4Parse.UE4.Assets;
using CUE4Parse.UE4.Assets.Objects;
using CUE4Parse.UE4.Assets.Objects.Properties;
using CUE4Parse.UE4.Assets.Readers;
using CUE4Parse.UE4.Assets.Exports;
using CUE4Parse.UE4.AssetRegistry;
using CUE4Parse.UE4.Assets.Exports.StaticMesh;
using CUE4Parse.UE4.Assets.Exports.SkeletalMesh;
using CUE4Parse.UE4.Assets.Exports.Animation;
using CUE4Parse.UE4.Assets.Exports.Actor;
using CUE4Parse.UE4.Assets.Exports.Component.Landscape;
using CUE4Parse.UE4.Assets.Exports.Component;
using CUE4Parse.UE4.Assets.Exports.Component.StaticMesh;
using CUE4Parse.UE4.Assets.Exports.Material;
using CUE4Parse.UE4.Assets.Exports.Texture;
using CUE4Parse.UE4.Objects.Core.Misc;
using CUE4Parse.UE4.Objects.Core.Math;
using CUE4Parse.UE4.Objects.Engine;
using CUE4Parse.UE4.Objects.GameplayTags;
using CUE4Parse.UE4.Objects.Core.i18N;
using CUE4Parse.UE4.Objects.UObject;
using CUE4Parse.UE4.Readers;
using CUE4Parse.UE4.Versions;
using System.Text.Json;
using CUE4Parse_Conversion.Dto;
using CUE4Parse_Conversion.Formats.Meshes;
using CUE4Parse_Conversion.Formats.Animations;
using CUE4Parse_Conversion.Animations;
using CUE4Parse_Conversion.Options;
using CUE4Parse_Conversion.Textures;
using SixLabors.ImageSharp;
using SixLabors.ImageSharp.PixelFormats;

bool showAll = args.Contains("--all");
bool showPseudo = args.Contains("--pseudo");
bool showImports = args.Contains("--imports");
bool importsJson = args.Contains("--imports-json");
bool exportMapJson = args.Contains("--export-map-json");
bool propertyReport = args.Contains("--property-report");
bool registryMode = args.Contains("--registry");
bool pakSources = args.Contains("--pak-sources");
bool registryPaths = args.Contains("--registry-paths");
bool seedList = args.Contains("--seed-list");
bool followSoft = args.Contains("--follow-soft");
bool scanMaps = args.Contains("--scan-maps");
bool scanNative = args.Contains("--scan-native");
bool scanExportClasses = args.Contains("--scan-export-classes");
string? classesOutArgument = args.SingleOrDefault(arg => arg.StartsWith("--classes-out=", StringComparison.Ordinal));
bool allKeys = args.Contains("--all-keys");
bool savePackage = args.Contains("--save-package");
bool saveList = args.Contains("--save-list");
bool saveFolder = args.Contains("--save-folder");
bool saveClosure = args.Contains("--save-closure");
bool exportGlb = args.Contains("--export-glb");
bool exportAnim = args.Contains("--export-anim");
bool exportLandscape = args.Contains("--export-landscape");
bool exportLandscapeGrid = args.Contains("--export-landscape-grid");
bool exportLandscapeWeightmapGrid = args.Contains("--export-landscape-weightmap-grid");
bool landscapeReport = args.Contains("--landscape-report");
bool actorReport = args.Contains("--actor-report");
bool streamingReport = args.Contains("--streaming-report");
bool exportPng = args.Contains("--export-png");
bool exportLandscapeWeightmaps = args.Contains("--export-landscape-weightmaps");
bool materialReport = args.Contains("--material-report");
bool legacyMaterialStart = args.Contains("--legacy-material-start");
bool textureReport = args.Contains("--texture-report");
string? textureOutputArgument = args.SingleOrDefault(arg => arg.StartsWith("--texture-output=", StringComparison.Ordinal));
bool functionReport = args.Contains("--function-report");
bool versionReport = args.Contains("--versions");
bool sizeList = args.Contains("--sizes");
bool ue426 = args.Contains("--ue426");
bool loose = args.Contains("--loose");
bool extractAll = args.Contains("--extract-all");
bool extractDryRun = args.Contains("--dry-run");
bool extractVerify = args.Contains("--verify");
int extractFlagIndex = Array.IndexOf(args, "--extract-all");
string? extractOutput = extractFlagIndex >= 0 && extractFlagIndex + 1 < args.Length ? args[extractFlagIndex + 1] : null;
string? extractWorkersArgument = args.SingleOrDefault(arg => arg.StartsWith("--workers=", StringComparison.Ordinal));
string? extractPlanArgument = args.SingleOrDefault(arg => arg.StartsWith("--plan-out=", StringComparison.Ordinal));
string? extractSummaryArgument = args.SingleOrDefault(arg => arg.StartsWith("--summary-out=", StringComparison.Ordinal));
string? extractSampleArgument = args.SingleOrDefault(arg => arg.StartsWith("--sample=", StringComparison.Ordinal));
string? extractSeedArgument = args.SingleOrDefault(arg => arg.StartsWith("--seed=", StringComparison.Ordinal));
bool auditDuplicates = args.Contains("--audit-duplicates");
bool repairDuplicates = args.Contains("--repair");
string? duplicatesOutArgument = args.SingleOrDefault(arg => arg.StartsWith("--duplicates-out=", StringComparison.Ordinal));
Globals.FatalObjectSerializationErrors = exportGlb || exportAnim || exportLandscape || exportLandscapeGrid || exportLandscapeWeightmapGrid || exportPng || exportLandscapeWeightmaps || materialReport || functionReport || landscapeReport || actorReport || streamingReport || propertyReport;
string? keyFileArgument = args.SingleOrDefault(arg => arg.StartsWith("--key-file=", StringComparison.Ordinal));
string? keyFilePath = keyFileArgument is not null ? keyFileArgument["--key-file=".Length..] : Environment.GetEnvironmentVariable("FORTNITE_AES_KEY_FILE");
string? maxArgument = args.SingleOrDefault(arg => arg.StartsWith("--max-packages=", StringComparison.Ordinal));
string? closureOutputArgument = args.SingleOrDefault(arg => arg.StartsWith("--closure-output=", StringComparison.Ordinal));
int maxPackages = 250;
if (maxArgument is not null && (!int.TryParse(maxArgument[15..], out maxPackages) || maxPackages < 1))
{
    Console.Error.WriteLine("--max-packages must be a positive integer");
    return 2;
}
string[] positional = args.Where(arg => arg != maxArgument && arg != closureOutputArgument && arg != keyFileArgument && arg != extractOutput && arg != extractWorkersArgument && arg != extractPlanArgument && arg != extractSummaryArgument && arg != extractSampleArgument && arg != extractSeedArgument && arg != duplicatesOutArgument && arg != textureOutputArgument && arg != classesOutArgument && arg is not ("--all" or "--pseudo" or "--imports" or "--imports-json" or "--export-map-json" or "--property-report" or "--registry" or "--pak-sources" or "--registry-paths" or "--seed-list" or "--follow-soft" or "--scan-maps" or "--scan-native" or "--scan-export-classes" or "--all-keys" or "--save-package" or "--save-list" or "--sizes" or "--save-folder" or "--save-closure" or "--export-glb" or "--export-anim" or "--export-landscape" or "--export-landscape-grid" or "--export-landscape-weightmap-grid" or "--export-png" or "--export-landscape-weightmaps" or "--material-report" or "--legacy-material-start" or "--texture-report" or "--function-report" or "--landscape-report" or "--actor-report" or "--streaming-report" or "--versions" or "--ue426" or "--loose" or "--extract-all" or "--dry-run" or "--verify" or "--audit-duplicates" or "--repair")).ToArray();
bool packageLess = registryMode || pakSources || registryPaths || scanMaps || scanNative || scanExportClasses || exportLandscapeGrid || exportLandscapeWeightmapGrid || extractAll;
if (new[] { registryMode, pakSources, registryPaths, scanMaps, scanNative, scanExportClasses, exportLandscapeGrid, exportLandscapeWeightmapGrid, extractAll }.Count(mode => mode) > 1 || (packageLess ? positional.Length > 1 : positional.Length is < 1 or > 2))
{
    Console.Error.WriteLine("Usage: Cue4Inspect <asset-path-without-extension> [pak-directory] [--all] [--pseudo] [--imports] [--save-package] [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect <package-path> [source-directory] --imports-json [--loose]");
    Console.Error.WriteLine("   or: Cue4Inspect <package-path> [source-directory] --export-map-json [--loose]");
    Console.Error.WriteLine("   or: Cue4Inspect <package-path> [source-directory] --property-report [--loose]");
    Console.Error.WriteLine("   or: Cue4Inspect <folder-prefix> [pak-directory] --save-folder [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect <newline-list-file> [pak-directory] --save-list [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect <newline-list-file> [pak-directory] --sizes [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect <asset-path> [source-directory] --save-closure [--max-packages=250] [--closure-output=<new frontend-probes directory>] [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect <static-mesh-path> [pak-directory] --export-glb [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect <anim-sequence-path> [source-directory] --export-anim [--loose]");
    Console.Error.WriteLine("   or: Cue4Inspect <landscape-map-path> [pak-directory] --export-landscape [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect --export-landscape-grid [pak-directory] [--max-packages=64] [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect --export-landscape-weightmap-grid [pak-directory] [--max-packages=64] [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect <texture-path> [pak-directory] --export-png [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect <texture-path> [extracted-root] --export-png --loose");
    Console.Error.WriteLine("   or: Cue4Inspect <landscape-map-path> [source-directory] --export-landscape-weightmaps [--loose]");
    Console.Error.WriteLine("   or: Cue4Inspect <material-path> [pak-directory] --material-report [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect <landscape-map-path> [pak-directory] --landscape-report [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect <map-path> [pak-directory] --actor-report [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect <map-path> [source-directory] --streaming-report [--loose]");
    Console.Error.WriteLine("   or: Cue4Inspect <blueprint-path> [pak-directory] --function-report [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect <package-path> [pak-directory] --versions [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect --registry [pak-directory] [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect --scan-maps [pak-directory] [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect --scan-native [pak-directory] [--all-keys]");
    Console.Error.WriteLine("   or: Cue4Inspect --scan-export-classes <extracted-root> --classes-out=<tsv> [--workers=N]");
    return 2;
}
if (scanExportClasses && (classesOutArgument is null || classesOutArgument.Length == "--classes-out=".Length || positional.Length != 1 ||
    (extractWorkersArgument is not null && (!int.TryParse(extractWorkersArgument["--workers=".Length..], out int scanWorkers) || scanWorkers is < 1 or > 32))))
{
    Console.Error.WriteLine("Usage: Cue4Inspect --scan-export-classes <extracted-root> --classes-out=<tsv> [--workers=N] (workers 1-32)");
    return 2;
}

if (extractAll)
{
    if (extractOutput is null || extractOutput.StartsWith("--", StringComparison.Ordinal) || !allKeys || loose || (extractDryRun && extractVerify)
        || (auditDuplicates && (extractDryRun || extractVerify || duplicatesOutArgument is null)) || (repairDuplicates && !auditDuplicates))
    {
        Console.Error.WriteLine("Usage: Cue4Inspect [pak-directory] --extract-all <output-directory> --all-keys [--dry-run | --verify | --audit-duplicates --duplicates-out=<json> [--repair]] [--workers=N] [--plan-out=<json>] [--summary-out=<json>] [--sample=N] [--seed=N] [--key-file=<path>]");
        return 2;
    }
    if (extractWorkersArgument is not null && (!int.TryParse(extractWorkersArgument["--workers=".Length..], out int workers) || workers is < 1 or > 32))
    {
        Console.Error.WriteLine("--workers must be an integer from 1 to 32");
        return 2;
    }
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

string? closureOutput = null;
if (closureOutputArgument is not null)
{
    // An explicit closure destination is isolated from the extracted source and
    // existing staged assets. Refuse existing destinations and symlinked parents.
    string root = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "../../../../generated/frontend-probes"));
    closureOutput = Path.GetFullPath(closureOutputArgument["--closure-output=".Length..]);
    if (!saveClosure || !closureOutput.StartsWith(root + Path.DirectorySeparatorChar, StringComparison.Ordinal) ||
        Directory.Exists(closureOutput) || File.Exists(closureOutput))
    {
        Console.Error.WriteLine("--closure-output must be a new directory under porting/generated/frontend-probes, with --save-closure");
        return 2;
    }
    for (DirectoryInfo? parent = new DirectoryInfo(closureOutput).Parent; parent is not null; parent = parent.Parent)
        if (parent.LinkTarget is not null)
        {
            Console.Error.WriteLine("--closure-output may not have a symlinked parent");
            return 2;
        }
}

try
{
    // Fortnite 12.60 cooked packages are unversioned, so CUE4Parse guesses custom
    // versions from EGame. Its 4.25 guess for FAnimObjectVersion includes
    // IncreaseBoneIndexLimitPerChunk/UnlimitedBoneInfluences, but 12.60 skin
    // weight buffers still use the legacy layout (bExtraBoneInfluences, stride,
    // NumVertices, bulk weights; no bUse16BitBoneIndex).
    var customVersions = new CUE4Parse.UE4.Objects.Core.Serialization.FCustomVersionContainer(
    [
        new CUE4Parse.UE4.Objects.Core.Serialization.FCustomVersion(FAnimObjectVersion.GUID,
            (int) FAnimObjectVersion.Type.SerializeRigVMRegisterArrayState)
    ]);
    var versions = new VersionContainer(ue426 ? EGame.GAME_UE4_26 : EGame.GAME_UE4_25_Plus,
        customVersions: customVersions,
        optionOverrides: new Dictionary<string, bool>
        {
            ["StaticMesh.Fortnite1260NoForceOpaque"] = true,
            ["RawIndexBuffer.HasShouldExpandTo32Bit"] = false,
            ["StaticMesh.HasRayTracingGeometry"] = false,
            ["SkeletalMesh.HasRayTracingData"] = false,
            // 12.60 still cooks the 4.23/4.24 FCompressedAnimSequence layout
            // (FUECompressedAnimData header before the byte stream, no bone codec
            // DDC handle), although its custom versions otherwise match 4.25.
            ["AnimSequence.UE424CompressedDataLayout"] = true
        });
    if (scanExportClasses)
        return ScanExportClasses(sourceDirectory, Path.GetFullPath(classesOutArgument!["--classes-out=".Length..]),
            extractWorkersArgument is null ? 16 : int.Parse(extractWorkersArgument["--workers=".Length..]), versions);

    var provider = loose
        ? new DefaultFileProvider(new DirectoryInfo(Path.Combine(sourceDirectory, "FortniteGame")),
            SearchOption.AllDirectories, versions, StringComparer.OrdinalIgnoreCase)
        : new DefaultFileProvider(sourceDirectory, SearchOption.TopDirectoryOnly,
            versions, StringComparer.OrdinalIgnoreCase);
    provider.ReadScriptData = showPseudo;
    provider.Initialize();

    if (loose)
    {
        // DefaultFileProvider derives extra-directory paths relative to its
        // primary directory. Mount Engine independently to preserve Engine/Content paths.
        var engineDirectory = new DirectoryInfo(Path.Combine(sourceDirectory, "Engine"));
        int enginePackages = 0;
        if (engineDirectory.Exists)
        {
            var engineProvider = new DefaultFileProvider(engineDirectory, SearchOption.AllDirectories, versions,
                                                         StringComparer.OrdinalIgnoreCase);
            engineProvider.Initialize();
            provider.Files.AddFiles(engineProvider.Files);
            enginePackages = engineProvider.LooseFileCount;
        }
        (importsJson || propertyReport ? Console.Error : Console.Out).WriteLine($"Indexed {provider.LooseFileCount + enginePackages} loose package(s).");
    }
    else
    {
        var fileKeys = LoadKeyFile(keyFilePath);
        string key = fileKeys.TryGetValue("primary", out var filePrimary) ? filePrimary : ReadHiddenKey("Primary AES key: ");
        int mounted = provider.SubmitKey(new FGuid(), new FAesKey("0x" + key));
        (importsJson ? Console.Error : Console.Out).WriteLine($"Mounted {mounted} pak(s) with the primary key.");
        if (allKeys)
        {
            foreach (var guid in provider.RequiredKeys.Where(guid => guid.IsValid()).OrderBy(guid => guid.ToString(EGuidFormats.Digits)))
            {
                string guidDigits = guid.ToString(EGuidFormats.Digits).ToLowerInvariant();
                string extraKey = fileKeys.TryGetValue(guidDigits, out var fileExtra) ? fileExtra : ReadHiddenKey($"AES key for GUID {guid.ToString(EGuidFormats.Digits)}: ");
                mounted += provider.SubmitKey(guid, new FAesKey("0x" + extraKey));
            }
            (importsJson ? Console.Error : Console.Out).WriteLine($"Mounted {mounted} pak(s) in total.");
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

    if (registryPaths)
    {
        // One row per registry asset: resolved package path (or a #unresolved marker), asset class, presence.
        var registryBytes = provider.SaveAsset("FortniteGame/AssetRegistry.bin");
        var registry = new FAssetRegistryState(new FByteArchive("AssetRegistry.bin", registryBytes, provider.Versions));
        var pluginRoots = BuildPluginContentRoots(provider.Files.Keys);
        bool PackageExists(string path) => provider.Files.ContainsKey(path + ".uasset") || provider.Files.ContainsKey(path + ".umap");
        foreach (var asset in registry.PreallocatedAssetDataBuffers)
        {
            var packageName = asset.PackageName.Text;
            var target = ResolvePackageRoot(packageName, pluginRoots, PackageExists);
            bool present = target is not null && PackageExists(target);
            Console.WriteLine($"{target ?? "#unresolved:" + packageName}\t{asset.AssetClass.Text}\t{(present ? "present" : "absent")}");
        }
        return 0;
    }

    if (pakSources)
    {
        // One row per recovered .uasset: package path, then the pak whose entry the provider serves.
        foreach (var entry in provider.Files.Values
            .OfType<CUE4Parse.UE4.VirtualFileSystem.VfsEntry>()
            .Where(file => file.Path.EndsWith(".uasset", StringComparison.OrdinalIgnoreCase))
            .OrderBy(file => file.Path, StringComparer.Ordinal))
            Console.WriteLine($"{entry.Path[..^".uasset".Length]}\t{entry.Vfs.Name}");
        return 0;
    }

    if (extractAll)
    {
        // Raw export of every path the provider serves (one winner per path). Relative paths are kept; nothing is converted.
        return ExtractAllFiles(provider, Path.GetFullPath(extractOutput!), new ExtractOptions(
            DryRun: extractDryRun,
            Verify: extractVerify,
            Workers: extractWorkersArgument is null ? 32 : int.Parse(extractWorkersArgument["--workers=".Length..]),
            PlanOut: extractPlanArgument?["--plan-out=".Length..],
            SummaryOut: extractSummaryArgument?["--summary-out=".Length..],
            SampleCount: extractSampleArgument is null ? 200 : int.Parse(extractSampleArgument["--sample=".Length..]),
            Seed: extractSeedArgument is null ? 20261031 : int.Parse(extractSeedArgument["--seed=".Length..]),
            PakDirectory: sourceDirectory,
            AuditDuplicates: auditDuplicates,
            Repair: repairDuplicates,
            DuplicatesOut: duplicatesOutArgument?["--duplicates-out=".Length..]));
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
        var queued = new HashSet<string>(StringComparer.OrdinalIgnoreCase); // unique packages ever queued: pending is a frontier count, not queue entries
        var pluginRoots = BuildPluginContentRoots(provider.Files.Keys);
        var unresolved = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        var unresolvedSoftCandidates = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        bool PackageExists(string path) => provider.Files.ContainsKey(path + ".uasset") || provider.Files.ContainsKey(path + ".umap");
        void RecordUnresolved(string root, bool softCandidate = false)
        {
            var roots = softCandidate ? unresolvedSoftCandidates : unresolved;
            if (root.StartsWith("/", StringComparison.Ordinal) &&
                !root.StartsWith("/Script/", StringComparison.OrdinalIgnoreCase) && roots.Add(root))
                Console.Error.WriteLine($"Unresolved {(softCandidate ? "raw soft-path candidate" : "imported package root")}: {root}");
        }
        if (seedList)
        {
            // --seed-list: the positional argument is a file of package paths (FortniteGame/Content/... or Engine/Content/...).
            foreach (var seed in File.ReadLines(Path.GetFullPath(assetPath!))
                .Select(line => line.Trim())
                .Where(line => line.Length > 0 && !line.StartsWith('#'))
                .Distinct(StringComparer.OrdinalIgnoreCase))
                if (queued.Add(seed)) pending.Enqueue(seed);
        }
        else
        {
            if (queued.Add(assetPath!)) pending.Enqueue(assetPath!);
        }
        int failed = 0;
        int softFollowed = 0;
        while (pending.Count > 0 && visited.Count < maxPackages)
        {
            var current = pending.Dequeue();
            if (!visited.Add(current)) continue;
            try
            {
                if (provider.LoadPackage(current) is not Package package)
                    throw new InvalidDataException("Not a legacy package");
                var files = provider.SavePackage(current);
                SaveRecoveredFiles(files, closureOutput);
                foreach (var root in package.ImportMap.Select(import => RootImportName(package, import)).Distinct())
                {
                    var target = ResolvePackageRoot(root, pluginRoots, PackageExists);
                    if (target is not null && PackageExists(target))
                    {
                        if (queued.Add(target)) pending.Enqueue(target);
                    }
                    else
                        RecordUnresolved(root);
                }
                if (followSoft)
                {
                    foreach (var soft in SoftPackageRoots(files))
                    {
                        var target = ResolvePackageRoot(soft, pluginRoots, PackageExists);
                        if (target is null || !PackageExists(target))
                            RecordUnresolved(soft, softCandidate: true);
                        else if (!visited.Contains(target) && queued.Add(target))
                        {
                            pending.Enqueue(target);
                            softFollowed++;
                        }
                    }
                }
            }
            catch (Exception error)
            {
                failed++;
                Console.Error.WriteLine($"Could not recover {current}: {error.GetType().Name}: {error.Message}");
            }
            if (visited.Count % 25 == 0) Console.Error.WriteLine($"Recovered {visited.Count} package(s); pending {pending.Count}");
        }
        Console.Error.WriteLine($"Closure: {visited.Count} visited, {failed} failed, {pending.Count} pending, {unresolved.Count} unresolved roots");
        if (followSoft) Console.Error.WriteLine($"Soft references enqueued: {softFollowed}; unresolved raw candidates: {unresolvedSoftCandidates.Count} (heuristic, not proven dependencies)");
        return failed == 0 && pending.Count == 0 && unresolved.Count == 0 && unresolvedSoftCandidates.Count == 0 ? 0 : 1;
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
                  package.StartsWith("Engine/Content/", StringComparison.Ordinal) ||
                  package.StartsWith("FortniteGame/Plugins/", StringComparison.Ordinal) ||
                  package.StartsWith("Engine/Plugins/", StringComparison.Ordinal)) ||
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
    if (legacyMaterialStart)
    {
        if (!loose || packageData is not Package p || p.Summary.FileVersionUE.FileVersionUE4 != 379)
            throw new InvalidDataException("Legacy start probe only supports loose version 379 packages");
        var matches = p.ExportMap.Select((e, i) => (e, i)).Where(x => x.e.ClassName == "MaterialInstanceConstant").ToArray();
        if (matches.Length != 1) throw new InvalidDataException("Expected one material instance constant");
        var (entry, index) = matches[0];
        if (p.HasFlags(EPackageFlags.PKG_Cooked) || p.HasFlags(EPackageFlags.PKG_FilterEditorOnly) ||
            ((EObjectFlags) entry.ObjectFlags).HasFlag(EObjectFlags.RF_ClassDefaultObject))
            throw new InvalidDataException("Expected uncooked non-default editor instance");
        var material = p.ExportsLazy[index].Value as UMaterialInstanceConstant
            ?? throw new InvalidDataException("Invalid instance export");
        if (!material.bHasStaticPermutationResource) throw new InvalidDataException("Expected static permutation resource");
        var path = Path.GetFullPath(Path.Combine(sourceDirectory, assetPath!.TrimStart('/').Replace('/', Path.DirectorySeparatorChar) + ".uasset"));
        var bytes = File.ReadAllBytes(path);
        if (entry.SerialOffset < 0 || entry.SerialSize < 0 || entry.SerialOffset + entry.SerialSize > bytes.Length)
            throw new InvalidDataException("Export outside standalone uasset");
        var ver = new VersionContainer(EGame.GAME_UE4_25_Plus, ver: p.Summary.FileVersionUE, customVersions: p.Summary.CustomVersionContainer);
        var ar = new FAssetArchive(new FByteArchive(path, bytes, ver), p);
        ar.Position = entry.SerialOffset;
        // UObject::Serialize reads NetIndex for this UE4 version.
        int? netIndex = null;
        if (ar.Ver < EUnrealEngineObjectUE4Version.REMOVE_NET_INDEX) netIndex = ar.Read<int>();
        var names = new List<string>();
        while (true)
        {
            if (ar.Position >= entry.SerialOffset + entry.SerialSize || names.Count > 512) throw new InvalidDataException("Invalid tagged property bounds");
            var tag = new FPropertyTag(ar, true);
            if (tag.Name.IsNone) break;
            names.Add(tag.Name.Text);
        }
        var guidFlag = ar.Read<int>();
        if (guidFlag is not (0 or 1)) throw new InvalidDataException("Invalid object GUID flag");
        if (guidFlag == 1) ar.Position += 16;
        if (ar.Position > entry.SerialOffset + entry.SerialSize) throw new InvalidDataException("Tags exceed export bounds");
        var parsed = material.Properties.Select(x => x.Name.Text).ToArray();
        if (!names.SequenceEqual(parsed)) throw new InvalidDataException("Tag list disagrees with normal object parser");
        Console.WriteLine(JsonSerializer.Serialize(new { package = assetPath, sourceSha256 = Convert.ToHexString(System.Security.Cryptography.SHA256.HashData(bytes)).ToLowerInvariant(), ue4 = p.Summary.FileVersionUE.FileVersionUE4, customVersions = p.Summary.CustomVersionContainer?.Versions.Length, cooked = false, filterEditorOnly = false, classDefaultObject = false, staticPermutation = material.bHasStaticPermutationResource, exportOffset = entry.SerialOffset, exportSize = entry.SerialSize, netIndex, tagNames = names, guidFlag, nativeStart = ar.Position, constants = new { removeNetIndex = (int) EUnrealEngineObjectUE4Version.REMOVE_NET_INDEX, overrides = (int) EUnrealEngineObjectUE4Version.MATERIAL_INSTANCE_BASE_PROPERTY_OVERRIDES, phase2 = (int) EUnrealEngineObjectUE4Version.MATERIAL_INSTANCE_BASE_PROPERTY_OVERRIDES_PHASE_2, fixedSerialization = (int) EUnrealEngineObjectUE4Version.FIX_MATERIAL_PROPERTY_OVERRIDE_SERIALIZE, shadowDepth = (int) EUnrealEngineObjectUE4Version.STATIC_SHADOW_DEPTH_MAPS, dither = (int) EUnrealEngineObjectUE4Version.MATERIAL_INSTANCE_BASE_PROPERTY_OVERRIDES_DITHERED_LOD_TRANSITION, purged = (int) EUnrealEngineObjectUE4Version.PURGED_FMATERIAL_COMPILE_OUTPUTS, collections = (int) EUnrealEngineObjectUE4Version.COLLECTIONS_IN_SHADERMAPID } }));
        return 0;
    }
    if (propertyReport)
    {
        Console.WriteLine(JsonSerializer.Serialize(new
        {
            package = assetPath,
            exports = packageData.GetExports().Select(item => new
            {
                name = item.Name,
                exportType = item.ExportType,
                parserType = item.GetType().Name,
                properties = item.Properties.Select(property => new
                {
                    name = property.Name.Text,
                    type = property.PropertyType.Text,
                    value = MaterialValue(property.Tag)
                }).ToArray()
            }).ToArray()
        }, new JsonSerializerOptions { WriteIndented = true }));
        return 0;
    }
    if (exportMapJson)
    {
        if (packageData is not Package exportPackage)
            throw new InvalidDataException("Export map requires a legacy package");
        Console.WriteLine(JsonSerializer.Serialize(new
        {
            package = assetPath,
            headerBytes = exportPackage.Summary.TotalHeaderSize,
            exports = exportPackage.ExportMap.Select((item, index) => new
            {
                index = index + 1,
                name = item.ObjectName.Text,
                serialOffset = item.SerialOffset,
                serialSize = item.SerialSize
            }).ToArray()
        }));
        return 0;
    }
    if (importsJson)
    {
        if (packageData is not Package importPackage)
            throw new InvalidDataException("Import chain requires a legacy package");
        var imports = importPackage.ImportMap.Select((item, index) => new
        {
            index = -(index + 1),
            name = item.ObjectName.Text,
            kind = item.ClassName.Text,
            outerIndex = item.OuterIndex?.Index ?? 0,
            chain = ImportChain(importPackage, index)
        }).ToArray();
        Console.WriteLine(JsonSerializer.Serialize(new { package = assetPath, imports }));
        return 0;
    }
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
    if (streamingReport)
    {
        if (packageData is not Package mapPackage)
            throw new InvalidDataException("Streaming report requires a legacy package");
        // Deserialize only streaming exports, avoiding unrelated map actor and bulk data.
        var entries = mapPackage.ExportMap.Select((export, index) => (export, index))
            .Where(entry => entry.export.ClassName.StartsWith("LevelStreaming", StringComparison.Ordinal))
            .Select(entry =>
            {
                var level = mapPackage.ExportsLazy[entry.index].Value as ULevelStreaming
                    ?? throw new InvalidDataException($"Unsupported streaming export {entry.export.ObjectName.Text} ({entry.export.ClassName})");
                var transformTag = level.Properties.SingleOrDefault(p => p.Name.Text == "LevelTransform");
                var transform = FTransform.Identity;
                if (transformTag is not null)
                {
                    if (transformTag.Tag?.GenericValue is not FScriptStruct { StructType: FStructFallback fields })
                        throw new InvalidDataException($"Unsupported LevelTransform on {level.Name}");
                    // Tagged struct fields omitted by Unreal retain FTransform's identity defaults.
                    transform.Translation = fields.GetOrDefault("Translation", transform.Translation);
                    transform.Rotation = fields.GetOrDefault("Rotation", transform.Rotation);
                    transform.Scale3D = fields.GetOrDefault("Scale3D", transform.Scale3D);
                }
                return new
                {
                    index = entry.index + 1,
                    name = level.Name,
                    type = entry.export.ClassName,
                    worldAsset = level.WorldAsset?.AssetPathName.Text,
                    worldSubPath = level.WorldAsset?.SubPathString,
                    packageName = level.PackageName?.Text,
                    transformSerialized = transformTag is not null,
                    translation = new[] { transform.Translation.X, transform.Translation.Y, transform.Translation.Z },
                    rotation = new[] { transform.Rotation.X, transform.Rotation.Y, transform.Rotation.Z, transform.Rotation.W },
                    scale = new[] { transform.Scale3D.X, transform.Scale3D.Y, transform.Scale3D.Z }
                };
            }).ToArray();
        // Foundation actors reference additional worlds via soft paths rather than
        // LevelStreaming exports. Report their component transforms as evidence;
        // whether a runtime loader applies those transforms is a separate question.
        var foundations = mapPackage.ExportMap.Select((export, index) => (export, index))
            .Where(entry => entry.export.ClassName.StartsWith("LF_", StringComparison.Ordinal))
            .Select(entry =>
            {
                var actor = mapPackage.ExportsLazy[entry.index].Value;
                var worldsTag = actor.Properties.SingleOrDefault(p => p.Name.Text == "AdditionalWorlds");
                if (worldsTag is null) return null;
                if (worldsTag.Tag?.GenericValue is not UScriptArray worlds)
                    throw new InvalidDataException($"Unsupported AdditionalWorlds on {actor.Name}");
                var paths = worlds.Properties.Select(value => value.GenericValue is FSoftObjectPath path
                    ? path.ToString()
                    : throw new InvalidDataException($"Unsupported AdditionalWorlds element on {actor.Name}")).ToArray();
                var component = actor.GetOrDefault<FPackageIndex?>("RootComponent")?.Load<USceneComponent>();
                var transform = component?.GetComponentTransform();
                return new
                {
                    index = entry.index + 1,
                    name = actor.Name,
                    type = entry.export.ClassName,
                    additionalWorlds = paths,
                    rootComponent = component?.Name,
                    translation = transform is { } t ? new[] { t.Translation.X, t.Translation.Y, t.Translation.Z } : null,
                    rotation = transform is { } r ? new[] { r.Rotation.X, r.Rotation.Y, r.Rotation.Z, r.Rotation.W } : null,
                    scale = transform is { } s ? new[] { s.Scale3D.X, s.Scale3D.Y, s.Scale3D.Z } : null
                };
            }).Where(entry => entry is not null).ToArray();
        Console.WriteLine(JsonSerializer.Serialize(new { path = assetPath, streamingLevels = entries, foundations },
            new JsonSerializerOptions { WriteIndented = true }));
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
    if (textureReport)
    {
        if (textureOutputArgument is null)
            throw new InvalidDataException("--texture-report requires --texture-output=<fresh directory under porting/generated>");
        var destination = Path.GetFullPath(textureOutputArgument["--texture-output=".Length..]);
        var generated = Path.GetFullPath(Path.Combine(Directory.GetCurrentDirectory(), "porting/generated"));
        if (!destination.StartsWith(generated + Path.DirectorySeparatorChar, StringComparison.Ordinal) || Directory.Exists(destination))
            throw new InvalidDataException("Texture output must be a fresh directory under porting/generated; run from repository root");
        var texture = packageData.GetExports().OfType<UTexture2D>().Single();
        var decoded = texture.Decode() ?? throw new InvalidDataException("Texture has no decodable mip data");
        var rgba = DecodeRgbaPixels(decoded);
        var png = EncodeRgbaPng(decoded);
        Directory.CreateDirectory(destination);
        var rgbaPath = Path.Combine(destination, texture.Name + ".rgba");
        var pngPath = Path.Combine(destination, texture.Name + ".png");
        File.WriteAllBytes(rgbaPath, rgba);
        File.WriteAllBytes(pngPath, png);
        var report = new
        {
            package = assetPath,
            name = texture.Name,
            width = decoded.Width, height = decoded.Height,
            pixelFormat = texture.Format.ToString(), decodedPixelFormat = decoded.PixelFormat.ToString(),
            srgb = texture.SRGB, filter = texture.Filter.ToString(), lodGroup = texture.LODGroup.ToString(),
            addressX = texture.AddressX.ToString(), addressY = texture.AddressY.ToString(),
            firstMip = texture.PlatformData.FirstMipToSerialize,
            mips = texture.PlatformData.Mips.Select(mip => new
            {
                width = mip.SizeX, height = mip.SizeY, depth = mip.SizeZ,
                bulkFlags = mip.BulkData?.BulkDataFlags.ToString(),
                bulkOffset = mip.BulkData?.Header.OffsetInFile, bulkBytes = mip.BulkData?.Header.SizeOnDisk
            }).ToArray(),
            rgba = rgbaPath, rgbaSha256 = Convert.ToHexString(System.Security.Cryptography.SHA256.HashData(rgba)).ToLowerInvariant(),
            png = pngPath, pngSha256 = Convert.ToHexString(System.Security.Cryptography.SHA256.HashData(png)).ToLowerInvariant(),
            properties = texture.Properties.Select(property => new { name = property.Name.Text, value = property.Tag?.ToString() }).ToArray()
        };
        var json = JsonSerializer.Serialize(report, new JsonSerializerOptions { WriteIndented = true });
        File.WriteAllText(Path.Combine(destination, "texture-report.json"), json);
        Console.WriteLine(json);
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
        // Metadata is unrelated to the report and can fail on uncooked editor
        // packages. Do not deserialize it; disclose every skipped export.
        var skipped = packageData is Package mp ? mp.ExportMap
            .Where(e => e.ClassName == "MetaData").Select(e => e.ObjectName.Text).ToArray() : [];
        var materialExports = packageData is Package ep ? ep.ExportMap.Select((e, i) => (e, i))
            .Where(x => x.e.ClassName != "MetaData").Select(x => packageData.ExportsLazy[x.i].Value).ToArray()
            : packageData.GetExports().ToArray();
        var materials = materialExports.OfType<UMaterialInterface>().ToArray();
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
            skippedMetadataExports = skipped,
            materials = materials.Select(material => new
            {
                name = material.Name,
                type = material.GetType().Name,
                blendMode = (material as UMaterial)?.BlendMode.ToString(),
                shadingModel = (material as UMaterial)?.ShadingModel.ToString(),
                twoSided = (material as UMaterial)?.TwoSided,
                expressionCount = (material as UMaterial)?.Expressions.Length,
                opacityMaskClipValue = (material as UMaterial)?.OpacityMaskClipValue,
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
                    value = property.Tag?.ToString(),
                    structured = MaterialValue(property.Tag)
                }).ToArray()
            }).ToArray(),
            instanceParameters = materialExports.OfType<UMaterialInstanceConstant>()
                .Select(instance => new
                {
                    name = instance.Name,
                    textures = instance.TextureParameterValues.Select(value => new
                    {
                        parameter = value.Name,
                        parameterInfo = value.ParameterInfo is not { } info ? null : new
                        {
                            name = new { name = info.Name.PlainText, number = info.Name.Number },
                            association = (byte) info.Association, index = info.Index
                        },
                        texture = value.ParameterValue?.Name,
                        package = value.ParameterValue is { IsImport: true } && packageData is Package p
                            ? RootImportName(p, p.ImportMap[-value.ParameterValue.Index - 1]) : null
                    }).ToArray(),
                    scalars = instance.ScalarParameterValues.Select(value => new
                    {
                        parameter = value.Name,
                        parameterInfo = value.ParameterInfo is not { } info ? null : new
                        {
                            name = new { name = info.Name.PlainText, number = info.Name.Number },
                            association = (byte) info.Association,
                            index = info.Index
                        },
                        value = value.ParameterValue
                    }).ToArray(),
                    vectors = instance.VectorParameterValues.Select(value => new
                    {
                        parameter = value.Name,
                        parameterInfo = value.ParameterInfo is not { } info ? null : new
                        {
                            name = new { name = info.Name.PlainText, number = info.Name.Number },
                            association = (byte) info.Association,
                            index = info.Index
                        },
                        value = value.ParameterValue is not { } color ? null : new[]
                        {
                            color.R, color.G, color.B, color.A
                        }
                    }).ToArray()
                }).ToArray(),
            expressionExports = materialExports
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
                        value = property.Tag?.ToString(),
                        structured = MaterialValue(property.Tag)
                    }).ToArray()
                }).ToArray(),
            textureSamples = materialExports.OfType<UMaterialExpressionTextureSample>()
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
    if (exportAnim)
    {
        // Resolve only AnimSequence exports; the skeleton is loaded through the import.
        var sequence = packageData is Package animPackage
            ? animPackage.ExportMap.Select((export, index) => (export, index))
                .Where(entry => entry.export.ClassName == "AnimSequence")
                .Select(entry => animPackage.ExportsLazy[entry.index].Value)
                .OfType<UAnimSequence>().FirstOrDefault()
            : packageData.GetExports().OfType<UAnimSequence>().FirstOrDefault();
        if (sequence is null)
            throw new InvalidDataException("Package has no AnimSequence export");
        var animSet = sequence.ConvertAnims();
        var animFiles = new UEFormatAnimFormat().Build(sequence.Name, assetPath!, new ExportOptions(), animSet);
        var animDestination = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "../../../../generated/converted"));
        var animRelative = assetPath!.Replace('\\', '/').TrimStart('/');
        if (animRelative.Split('/').Any(part => part is ".." or "."))
            throw new InvalidDataException("Unsafe asset path");
        Console.WriteLine($"Anim frames={sequence.NumFrames}, tracks={sequence.CompressedTrackToSkeletonMapTable?.Length ?? 0}, " +
                          $"codec={sequence.CompressedDataStructure?.GetType().Name ?? "none"}, curves={sequence.CompressedCurveData?.FloatCurves?.Length ?? 0}/{sequence.CompressedCurveNames?.Length ?? 0}, sequences={animSet.Sequences.Count}");
        foreach (var file in animFiles)
        {
            var target = Path.GetFullPath(Path.Combine(animDestination, animRelative + file.NameSuffix + "." + file.Extension));
            if (!target.StartsWith(animDestination + Path.DirectorySeparatorChar, StringComparison.Ordinal))
                throw new InvalidDataException("Export path escapes output directory");
            Directory.CreateDirectory(Path.GetDirectoryName(target)!);
            File.WriteAllBytes(target, file.Data);
            Console.WriteLine($"Exported {target} ({file.Data.Length} bytes)");
        }
        return 0;
    }
    if (exportGlb)
    {
        // Resolve only SkeletalMesh exports so unrelated exports (anim sequences,
        // physics assets) are not deserialized just to locate the mesh.
        var skinned = packageData is Package skeletalPackage
            ? skeletalPackage.ExportMap.Select((export, index) => (export, index))
                .Where(entry => entry.export.ClassName == "SkeletalMesh")
                .Select(entry => skeletalPackage.ExportsLazy[entry.index].Value)
                .OfType<USkeletalMesh>().FirstOrDefault()
            : packageData.GetExports().OfType<USkeletalMesh>().FirstOrDefault();
        if (skinned is not null)
        {
            using var skeletalDto = new SkeletalMeshDto(skinned, EMeshQuality.Highest, exportMorphTarget: false);
            IReadOnlyList<CUE4Parse_Conversion.ExportFile> skeletalFiles;
            try
            {
                var skeletalOptions = new ExportOptions(meshFormat: EMeshFormat.Gltf2, exportMaterials: false);
                skeletalFiles = new GltfMeshFormat().BuildSkeletalMesh(skinned.Name, assetPath!, skeletalOptions, skeletalDto);
            }
            catch (NotFiniteNumberException)
            {
                // Some morph targets carry non-finite deltas; export the skinned geometry without them.
                Console.WriteLine("Non-finite morph target data; retrying without morph targets");
                var fallbackOptions = new ExportOptions(meshFormat: EMeshFormat.Gltf2, exportMaterials: false, exportMorphTargets: false);
                skeletalFiles = new GltfMeshFormat().BuildSkeletalMesh(skinned.Name, assetPath!, fallbackOptions, skeletalDto);
            }
            var skeletalDestination = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "../../../../generated/converted"));
            var skeletalRelative = assetPath!.Replace('\\', '/').TrimStart('/');
            if (skeletalRelative.Split('/').Any(part => part is ".." or "."))
                throw new InvalidDataException("Unsafe asset path");
            foreach (var file in skeletalFiles)
            {
                var target = Path.GetFullPath(Path.Combine(skeletalDestination, skeletalRelative + file.NameSuffix + "." + file.Extension));
                if (!target.StartsWith(skeletalDestination + Path.DirectorySeparatorChar, StringComparison.Ordinal))
                    throw new InvalidDataException("Export path escapes output directory");
                Directory.CreateDirectory(Path.GetDirectoryName(target)!);
                File.WriteAllBytes(target, file.Data);
                Console.WriteLine($"Exported {target} ({file.Data.Length} bytes)");
            }
            return 0;
        }
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
    Console.Error.WriteLine(exportGlb || exportAnim || exportPng || materialReport ? $"Inspection failed: {error}" : $"Inspection failed: {error.GetType().Name}: {error.Message}");
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
    var rgba = DecodeRgbaPixels(texture);
    using var image = Image.LoadPixelData<Rgba32>(rgba, texture.Width, texture.Height);
    using var buffer = new MemoryStream();
    image.SaveAsPng(buffer);
    return buffer.ToArray();
}

static byte[] DecodeRgbaPixels(CTexture texture)
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
    return rgba;
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

// Plugin content mounts as /<Plugin>/... in the engine but is stored under Engine/Plugins/**/<Plugin>/Content
// (or FortniteGame/Plugins/**). Returns plugin name -> content folder prefixes (e.g. Engine/Plugins/FX/Niagara/Content).
static Dictionary<string, List<string>> BuildPluginContentRoots(IEnumerable<string> keys)
{
    var roots = new Dictionary<string, List<string>>(StringComparer.OrdinalIgnoreCase);
    foreach (var key in keys)
    {
        if (!(key.StartsWith("Engine/Plugins/", StringComparison.OrdinalIgnoreCase) ||
              key.StartsWith("FortniteGame/Plugins/", StringComparison.OrdinalIgnoreCase)))
            continue;
        int index = key.IndexOf("/Content/", StringComparison.OrdinalIgnoreCase);
        if (index < 0) continue;
        string prefix = key[..(index + "/Content".Length)];
        string pluginFolder = prefix[..^"/Content".Length];
        string name = pluginFolder[(pluginFolder.LastIndexOf('/') + 1)..];
        if (!roots.TryGetValue(name, out var list)) roots[name] = list = new List<string>();
        if (!list.Contains(prefix, StringComparer.OrdinalIgnoreCase)) list.Add(prefix);
    }
    return roots;
}

// Maps an engine import/soft root to the package path (no extension). /Game and /Engine map directly;
// /<Plugin>/... maps to the first plugin content folder that holds the package. Null when unresolvable.
static string? ResolvePackageRoot(string root, Dictionary<string, List<string>> pluginRoots, Func<string, bool> exists)
{
    if (root.StartsWith("/Game/", StringComparison.Ordinal)) return "FortniteGame/Content" + root[5..];
    if (root.StartsWith("/Engine/", StringComparison.Ordinal)) return "Engine/Content" + root[7..];
    if (!root.StartsWith('/') || root.StartsWith("/Script/", StringComparison.Ordinal)) return null;
    int slash = root.IndexOf('/', 1);
    if (slash < 0) return null;
    string rest = root[slash..];
    if (!pluginRoots.TryGetValue(root[1..slash], out var prefixes)) return null;
    foreach (var prefix in prefixes)
        if (exists(prefix + rest)) return prefix + rest;
    return null;
}

// Candidate package roots stored as data: ASCII and UTF-16 text inside the package bytes, e.g. soft object
// paths or string asset references. Returns root-form paths such as /Game/UI/Foo/Bar (no object suffix).
static HashSet<string> SoftPackageRoots(IReadOnlyDictionary<string, byte[]> files)
{
    var found = new HashSet<string>(StringComparer.Ordinal);
    var pattern = new System.Text.RegularExpressions.Regex("/[A-Za-z][A-Za-z0-9_]*/[A-Za-z0-9_/\\-]+");
    foreach (var bytes in files.Values)
    {
        var texts = new List<string> { System.Text.Encoding.Latin1.GetString(bytes) };
        for (int offset = 0; offset < 2 && offset < bytes.Length; offset++)
            texts.Add(System.Text.Encoding.Unicode.GetString(bytes, offset, (bytes.Length - offset) - ((bytes.Length - offset) % 2)));
        foreach (var text in texts)
            foreach (System.Text.RegularExpressions.Match match in pattern.Matches(text))
                found.Add(match.Value.TrimEnd('/'));
    }
    return found;
}

static void SaveRecoveredFiles(IReadOnlyDictionary<string, byte[]> files, string? output = null)
{
    var destination = output ?? Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "../../../../generated/extracted"));
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

static object[] ImportChain(Package package, int index)
{
    var chain = new List<object>();
    var seen = new HashSet<int>();
    while (true)
    {
        if (index < 0 || index >= package.ImportMap.Length || !seen.Add(index))
            throw new InvalidDataException("Invalid or cyclic import outer chain");
        var item = package.ImportMap[index];
        int outer = item.OuterIndex?.Index ?? 0;
        chain.Add(new { index = -(index + 1), name = item.ObjectName.Text,
                        kind = item.ClassName.Text, outerIndex = outer });
        if (outer >= 0) break; // zero: no outer; positive: export outer, retained in outerIndex
        index = -outer - 1;
    }
    chain.Reverse();
    return chain.ToArray();
}

// Reads only each loose package header (summary, name, import and export maps);
// exports stay lazy and are never deserialized, and .uexp/.ubulk are not opened.
static int ScanExportClasses(string root, string output, int workers, VersionContainer versions)
{
    root = Path.GetFullPath(root);
    if (output.StartsWith(root + Path.DirectorySeparatorChar, StringComparison.Ordinal))
    {
        Console.Error.WriteLine("--classes-out must be outside the extracted root");
        return 2;
    }
    var files = Directory.EnumerateFiles(root, "*", SearchOption.AllDirectories)
        .Where(file => file.EndsWith(".uasset", StringComparison.OrdinalIgnoreCase) || file.EndsWith(".umap", StringComparison.OrdinalIgnoreCase))
        .ToArray();
    Console.Error.WriteLine($"Scanning export classes of {files.Length} package(s) with {workers} worker(s)");
    var rows = new System.Collections.Concurrent.ConcurrentBag<string>();
    var errors = new System.Collections.Concurrent.ConcurrentBag<string>();
    int scanned = 0;
    Parallel.ForEach(files, new ParallelOptions { MaxDegreeOfParallelism = workers }, file =>
    {
        string name = Path.GetRelativePath(root, file).Replace('\\', '/');
        name = name[..name.LastIndexOf('.')];
        try
        {
            var archive = new FByteArchive(file, File.ReadAllBytes(file), versions);
            var package = new Package(archive, null, (FArchive?) null);
            var classes = package.ExportMap.Select(export => export.ClassIndex switch
                {
                    { IsImport: true } index when -index.Index - 1 < package.ImportMap.Length => package.ImportMap[-index.Index - 1].ObjectName.Text,
                    { IsExport: true } index when index.Index - 1 < package.ExportMap.Length => package.ExportMap[index.Index - 1].ObjectName.Text,
                    { IsNull: true } => "Class",
                    _ => "?"
                })
                .Distinct(StringComparer.Ordinal)
                .Order(StringComparer.Ordinal);
            rows.Add(name + "\t" + string.Join(',', classes));
        }
        catch (Exception error)
        {
            errors.Add(name + "\t" + error.GetType().Name + ": " + error.Message.ReplaceLineEndings(" "));
        }
        int done = Interlocked.Increment(ref scanned);
        if (done % 20000 == 0) Console.Error.WriteLine($"Scanned {done}/{files.Length}");
    });
    Directory.CreateDirectory(Path.GetDirectoryName(output)!);
    string temp = output + ".partial";
    File.WriteAllLines(temp, rows.Order(StringComparer.Ordinal));
    File.Move(temp, output, overwrite: true);
    foreach (var error in errors.Order(StringComparer.Ordinal).Take(50))
        Console.Error.WriteLine("error\t" + error);
    Console.Error.WriteLine($"Wrote {rows.Count} package row(s) to {output}; {errors.Count} unreadable package(s)");
    return errors.IsEmpty ? 0 : 1;
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

// --extract-all: raw export of every path the provider serves (one winner per path, as the provider resolves duplicates).
// Each file is the decrypted, decompressed pak payload exactly as CUE4Parse returns it (GameFile.Read); nothing is converted.
// Re-running skips files whose target already exists with the index size; writes go through a .partial file and a rename.
const long ExtractGiB = 1L << 30;
const long ExtractOutputFloor = 100 * ExtractGiB; // /mnt/vibrato keeps at least this much free
const long ExtractRootFloor = 12 * ExtractGiB;    // / keeps at least this much free
const long ExtractBigFileBytes = 256L << 20;      // files this large are written by at most 4 workers at once

static int ExtractAllFiles(DefaultFileProvider provider, string outputRoot, ExtractOptions options)
{
    var clock = Stopwatch.StartNew();
    if (provider.UnloadedVfs.Count > 0)
    {
        Console.Error.WriteLine($"Refusing to run: {provider.UnloadedVfs.Count} pak(s) did not mount.");
        return 2;
    }

    var mounted = provider.MountedVfs.OrderBy(reader => reader.Name, StringComparer.Ordinal).ToArray();
    // One entry per distinct path. Enumerating provider.Files yields every pak entry in descending read order (newest patch
    // first), so the first entry seen for a key is the winner that provider[path] returns; later entries for the same key
    // are shadowed copies in older or lower-priority paks and are never planned. (--audit-duplicates checks this against provider[path].)
    var winners = new Dictionary<string, GameFile>(StringComparer.Ordinal);
    var duplicatePaths = new HashSet<string>(StringComparer.Ordinal);
    var shadowedByPak = new Dictionary<string, ExtractTally>(StringComparer.Ordinal);
    long enumeratedEntries = 0, duplicateEntries = 0;
    foreach (var entry in provider.Files)
    {
        enumeratedEntries++;
        if (winners.TryAdd(entry.Key, entry.Value)) continue;
        duplicateEntries++;
        duplicatePaths.Add(entry.Key);
        ExtractTally.Add(shadowedByPak, PakNameOf(entry.Value), entry.Value.Size);
    }
    // Pak order, then offset inside the pak, so reads stay roughly sequential per pak.
    var files = winners.Values
        .OrderBy(file => PakNameOf(file), StringComparer.Ordinal)
        .ThenBy(file => OffsetOf(file))
        .ToArray();
    long indexEntries = mounted.Sum(reader => (long) reader.Files.Count);

    var servedByPak = new Dictionary<string, ExtractTally>(StringComparer.Ordinal);
    var byExtension = new Dictionary<string, ExtractTally>(StringComparer.Ordinal);
    var byFolder = new Dictionary<string, ExtractTally>(StringComparer.Ordinal);
    long plannedBytes = 0;
    foreach (var file in files)
    {
        plannedBytes += file.Size;
        ExtractTally.Add(servedByPak, PakNameOf(file), file.Size);
        ExtractTally.Add(byExtension, ExtensionOf(file.Path), file.Size);
        ExtractTally.Add(byFolder, TopFolderOf(file.Path), file.Size);
    }

    static bool IsAudioPath(string path) => path.Contains("Sound", StringComparison.OrdinalIgnoreCase)
        || path.Contains("Audio", StringComparison.OrdinalIgnoreCase)
        || path.Contains("Music", StringComparison.OrdinalIgnoreCase);
    var audioBulk = new ExtractTally();
    long audioUasset = 0;
    foreach (var file in files)
    {
        if (!IsAudioPath(file.Path)) continue;
        var extension = ExtensionOf(file.Path);
        if (extension == ".ubulk")
        {
            audioBulk.Files++;
            audioBulk.Bytes += file.Size;
        }
        else if (extension == ".uasset")
        {
            audioUasset++;
        }
    }

    var registryClasses = new Dictionary<string, object?>();
    long registryAssets = -1;
    try
    {
        var registryBytes = provider.SaveAsset("FortniteGame/AssetRegistry.bin");
        var registry = new FAssetRegistryState(new FByteArchive("AssetRegistry.bin", registryBytes, provider.Versions));
        var groups = registry.PreallocatedAssetDataBuffers
            .GroupBy(asset => asset.AssetClass.Text)
            .ToDictionary(group => group.Key, group => group.Count());
        registryAssets = registry.PreallocatedAssetDataBuffers.Length;
        foreach (var name in new[] { "SoundWave", "SoundCue", "World", "WidgetBlueprint", "Blueprint" })
            registryClasses[name] = groups.TryGetValue(name, out var count) ? count : 0;
    }
    catch (Exception error)
    {
        Console.Error.WriteLine($"Registry read failed: {error.GetType().Name}: {error.Message}");
    }

    long rootFree = FreeBytesFor("/");
    long outputFree = FreeBytesFor(outputRoot);
    var todo = new List<GameFile>();
    long presentFiles = 0, presentBytes = 0;
    foreach (var file in files)
    {
        var target = ResolveTarget(outputRoot, file.Path);
        if (File.Exists(target) && new FileInfo(target).Length == file.Size)
        {
            presentFiles++;
            presentBytes += file.Size;
        }
        else
        {
            todo.Add(file);
        }
    }
    long todoBytes = todo.Sum(file => file.Size);
    long outputFreeAfter = outputFree - todoBytes;
    bool floorBreach = outputFreeAfter < ExtractOutputFloor || rootFree < ExtractRootFloor;

    var manifest = new Dictionary<string, object?>
    {
        ["generatedUtc"] = DateTime.UtcNow.ToString("yyyy-MM-ddTHH:mm:ssZ"),
        ["mode"] = options.DryRun ? "dry-run" : options.Verify ? "verify" : "extract",
        ["pakDirectory"] = options.PakDirectory,
        ["outputRoot"] = outputRoot,
        ["pakCount"] = mounted.Length,
        ["indexEntries"] = indexEntries,
        ["enumeratedEntries"] = enumeratedEntries,
        ["entryCountsAgree"] = indexEntries == enumeratedEntries,
        ["uniquePaths"] = files.Length,
        ["duplicatePaths"] = duplicatePaths.Count,
        ["duplicateEntries"] = duplicateEntries,
        ["plannedBytes"] = plannedBytes,
        ["pakRows"] = mounted.Select(reader => new
        {
            pak = reader.Name,
            indexEntries = (long) reader.Files.Count,
            served = ExtractTally.Get(servedByPak, reader.Name).Files,
            servedBytes = ExtractTally.Get(servedByPak, reader.Name).Bytes,
            shadowed = ExtractTally.Get(shadowedByPak, reader.Name).Files
        }).ToArray(),
        ["byExtension"] = SummarizeTallies(byExtension),
        ["byTopFolder"] = SummarizeTallies(byFolder),
        ["umapFiles"] = ExtractTally.Get(byExtension, ".umap").Files,
        ["audio"] = new
        {
            ubulkFiles = audioBulk.Files,
            ubulkBytes = audioBulk.Bytes,
            soundPathUassetFiles = audioUasset,
            wemFiles = ExtractTally.Get(byExtension, ".wem").Files,
            bnkFiles = ExtractTally.Get(byExtension, ".bnk").Files,
            oggFiles = ExtractTally.Get(byExtension, ".ogg").Files,
            wavFiles = ExtractTally.Get(byExtension, ".wav").Files
        },
        ["registry"] = new { assets = registryAssets, classes = registryClasses },
        ["largestFiles"] = files.OrderByDescending(file => file.Size).Take(10)
            .Select(file => new { path = file.Path, pak = PakNameOf(file), bytes = file.Size }).ToArray(),
        ["output"] = new
        {
            alreadyPresentFiles = presentFiles,
            alreadyPresentBytes = presentBytes,
            toWriteFiles = todo.Count,
            toWriteBytes = todoBytes,
            outputFsFreeBeforeBytes = outputFree,
            outputFsFreeAfterProjectedBytes = outputFreeAfter,
            outputFloorBytes = ExtractOutputFloor,
            rootFreeBytes = rootFree,
            rootFloorBytes = ExtractRootFloor,
            floorBreachProjected = floorBreach
        }
    };

    if (options.DryRun)
    {
        Console.WriteLine($"Dry run: {mounted.Length} paks, {indexEntries:N0} index entries -> {files.Length:N0} unique paths " +
                          $"({duplicateEntries:N0} shadowed entries over {duplicatePaths.Count:N0} duplicated paths), {plannedBytes:N0} bytes (winner index sizes). " +
                          $"Already present: {presentFiles:N0} files. To write: {todo.Count:N0} files, {todoBytes:N0} bytes. " +
                          $"Output fs free {outputFree:N0} -> {outputFreeAfter:N0} projected (floor {ExtractOutputFloor:N0}); / free {rootFree:N0} (floor {ExtractRootFloor:N0}). Floor breach projected: {floorBreach}.");
        WriteJson(options.PlanOut, manifest);
        return 0;
    }

    if (options.AuditDuplicates)
    {
        var audit = AuditDuplicates(provider, mounted, winners, duplicatePaths, duplicateEntries, outputRoot, options);
        WriteJson(options.DuplicatesOut, audit);
        Console.WriteLine($"Duplicate audit: {audit["duplicatePaths"]:N0} duplicated paths, {audit["identicalAcrossCopies"]:N0} identical across copies, " +
                          $"{audit["differingAcrossCopies"]:N0} differing; disk ok {audit["diskOk"]:N0}, disk differs from winner {audit["diskDiffersFromWinner"]:N0} " +
                          $"(missing {audit["diskMissing"]:N0}, size {audit["diskSizeDiffers"]:N0}, content {audit["diskContentDiffers"]:N0}); repaired {audit["repaired"]:N0}, repair failed {audit["repairFailed"]:N0}; ok {audit["ok"]}.");
        return (bool) audit["ok"]! ? 0 : 1;
    }

    if (options.Verify)
    {
        var verification = VerifyOutput(files, plannedBytes, outputRoot, options);
        manifest["verification"] = verification;
        WriteJson(options.SummaryOut, manifest);
        return (bool) verification["ok"]! ? 0 : 1;
    }

    if (floorBreach)
    {
        Console.Error.WriteLine($"ABORT: writing {todoBytes:N0} bytes would leave {outputFreeAfter:N0} bytes free on the output filesystem " +
                                $"(floor {ExtractOutputFloor:N0}), or / has {rootFree:N0} free (floor {ExtractRootFloor:N0}). Nothing was written.");
        WriteJson(options.PlanOut, manifest);
        return 3;
    }

    Directory.CreateDirectory(outputRoot);
    var extraction = WriteFiles(outputRoot, todo, options.Workers, clock);
    manifest["extraction"] = extraction;
    WriteJson(options.SummaryOut, manifest);
    return (bool) extraction["ok"]! ? 0 : 1;
}

static Dictionary<string, object?> WriteFiles(string outputRoot, List<GameFile> todo, int workers, Stopwatch clock)
{
    long total = todo.Count;
    long totalBytes = todo.Sum(file => file.Size);
    long written = 0, failed = 0, writtenBytes = 0, done = 0;
    int stop = 0;
    var bigGate = new SemaphoreSlim(4);
    var consoleLock = new object();
    var started = clock.Elapsed;
    Console.WriteLine($"Writing {total:N0} files ({totalBytes:N0} bytes) with {workers} workers into {outputRoot}");

    Parallel.ForEach(todo, new ParallelOptions { MaxDegreeOfParallelism = workers }, (file, loopState) =>
    {
        if (Volatile.Read(ref stop) != 0)
        {
            loopState.Stop();
            return;
        }
        try
        {
            var target = ResolveTarget(outputRoot, file.Path);
            bool big = file.Size >= ExtractBigFileBytes;
            if (big) bigGate.Wait();
            try
            {
                byte[] data = file.Read();
                if (data.LongLength != file.Size)
                    throw new InvalidDataException($"read {data.LongLength} bytes, index says {file.Size}");
                Directory.CreateDirectory(Path.GetDirectoryName(target)!);
                string partial = target + ".partial";
                File.WriteAllBytes(partial, data);
                File.Move(partial, target, overwrite: true);
            }
            finally
            {
                if (big) bigGate.Release();
            }
            Interlocked.Increment(ref written);
            Interlocked.Add(ref writtenBytes, file.Size);
        }
        catch (Exception error)
        {
            Interlocked.Increment(ref failed);
            lock (consoleLock) Console.Error.WriteLine($"FAILED {file.Path} [{PakNameOf(file)}]: {error.GetType().Name}: {error.Message}");
        }

        long completed = Interlocked.Increment(ref done);
        if (completed % 2000 == 0)
        {
            long rootFree = FreeBytesFor("/");
            long outputFree = FreeBytesFor(outputRoot);
            if (rootFree < ExtractRootFloor || outputFree < ExtractOutputFloor)
            {
                Interlocked.Exchange(ref stop, 1);
                lock (consoleLock) Console.Error.WriteLine($"STOP: free-space floor reached (/ {rootFree:N0}, output {outputFree:N0}). Rerun to resume.");
            }
        }
        if (completed % 5000 == 0)
        {
            double seconds = Math.Max(1.0, (clock.Elapsed - started).TotalSeconds);
            long bytesNow = Interlocked.Read(ref writtenBytes);
            double mibPerSecond = bytesNow / seconds / (1 << 20);
            double remainingSeconds = bytesNow > 0 ? (totalBytes - bytesNow) / (bytesNow / seconds) : 0;
            lock (consoleLock)
                Console.WriteLine($"progress {completed:N0}/{total:N0} files, written {Interlocked.Read(ref written):N0}, failed {Interlocked.Read(ref failed):N0}, " +
                                  $"{bytesNow / (double) ExtractGiB:F2} GiB of {totalBytes / (double) ExtractGiB:F2}, {mibPerSecond:F1} MiB/s, eta {TimeSpan.FromSeconds(remainingSeconds):hh\\:mm\\:ss}");
        }
    });

    bool stopped = Volatile.Read(ref stop) != 0;
    bool ok = failed == 0 && !stopped && written == total;
    Console.WriteLine($"Extraction {(ok ? "finished" : "stopped")}: written {written:N0}, failed {failed:N0}, {writtenBytes:N0} bytes, {clock.Elapsed:hh\\:mm\\:ss}");
    return new Dictionary<string, object?>
    {
        ["attempted"] = total,
        ["written"] = written,
        ["failed"] = failed,
        ["writtenBytes"] = writtenBytes,
        ["stoppedByFloor"] = stopped,
        ["ok"] = ok
    };
}

// --audit-duplicates: every path served by more than one pak entry is read from each of its entries and hashed (SHA-256). The copies
// are compared with each other, with the provider winner (provider[path] must be the same object), and with the file on disk.
// --repair rewrites a disk file that is missing or differs from the winner (through .partial + rename) and re-hashes it.
// Lists in the report are capped at 500 rows and sorted by path; all counts are complete.
static Dictionary<string, object?> AuditDuplicates(DefaultFileProvider provider, IAesVfsReader[] readers, Dictionary<string, GameFile> winners,
    HashSet<string> duplicatePaths, long duplicateEntries, string outputRoot, ExtractOptions options)
{
    const int listCap = 500;
    var paths = duplicatePaths.OrderBy(path => path, StringComparer.Ordinal).ToArray();
    var copies = new Dictionary<string, List<GameFile>>(StringComparer.Ordinal);
    foreach (var path in paths) copies[path] = new List<GameFile>();
    long gathered = 0;
    foreach (var reader in readers)
    {
        foreach (var entry in reader.Files)
        {
            if (!copies.TryGetValue(entry.Key, out var list)) continue;
            list.Add(entry.Value);
            gathered++;
        }
    }
    long expectedGathered = duplicateEntries + paths.Length;

    var histogram = new Dictionary<int, long>();
    foreach (var path in paths)
    {
        int count = copies[path].Count;
        histogram[count] = histogram.GetValueOrDefault(count) + 1;
    }

    var tally = new DuplicateAuditTally();
    var tallyLock = new object();
    var differingRows = new System.Collections.Concurrent.ConcurrentBag<(string Path, object Row)>();
    var diskRows = new System.Collections.Concurrent.ConcurrentBag<(string Path, object Row)>();
    var errorRows = new System.Collections.Concurrent.ConcurrentBag<string>();

    Parallel.ForEach(paths, new ParallelOptions { MaxDegreeOfParallelism = options.Workers }, path =>
    {
        try
        {
            var winner = winners[path];
            var list = copies[path]
                .OrderBy(file => PakNameOf(file), StringComparer.Ordinal)
                .ThenBy(file => OffsetOf(file))
                .ToArray();
            bool lookupMatches = provider.Files.TryGetValue(path, out var served) && object.ReferenceEquals(served, winner);

            var hashes = new string[list.Length];
            long lengthBad = 0;
            string? winnerSha = null;
            byte[]? winnerBytes = null;
            for (int i = 0; i < list.Length; i++)
            {
                byte[] data = list[i].Read();
                hashes[i] = Sha256Hex(data);
                if (data.LongLength != list[i].Size) lengthBad++;
                if (object.ReferenceEquals(list[i], winner))
                {
                    winnerSha = hashes[i];
                    winnerBytes = data;
                }
            }
            if (winnerSha is null || winnerBytes is null)
            {
                lock (tallyLock)
                {
                    tally.CopiesRead += list.Length;
                    tally.WinnerNotAmongCopies++;
                }
                errorRows.Add($"winner not among copies: {path}");
                return;
            }

            bool identicalCopies = hashes.All(hash => hash == hashes[0]);
            var target = ResolveTarget(outputRoot, path);
            string diskState;
            long diskSize = -1;
            string? diskSha = null;
            if (!File.Exists(target))
            {
                diskState = "missing";
            }
            else
            {
                diskSize = new FileInfo(target).Length;
                using (var stream = File.OpenRead(target)) diskSha = Sha256HexStream(stream);
                diskState = diskSize != winner.Size ? "size-differs" : diskSha == winnerSha ? "ok" : "content-differs";
            }

            bool repairedNow = false, repairVerified = false;
            string? repairError = null;
            if (options.Repair && diskState != "ok")
            {
                try
                {
                    Directory.CreateDirectory(Path.GetDirectoryName(target)!);
                    string partial = target + ".partial";
                    File.WriteAllBytes(partial, winnerBytes);
                    File.Move(partial, target, overwrite: true);
                    repairedNow = true;
                    using var check = File.OpenRead(target);
                    repairVerified = check.Length == winner.Size && Sha256HexStream(check) == winnerSha;
                }
                catch (Exception error)
                {
                    repairError = $"{error.GetType().Name}: {error.Message}";
                }
            }

            lock (tallyLock)
            {
                tally.CopiesRead += list.Length;
                tally.LengthMismatch += lengthBad;
                if (!lookupMatches) tally.ProviderLookupMismatch++;
                if (identicalCopies) tally.Identical++; else tally.Differing++;
                switch (diskState)
                {
                    case "ok": tally.DiskOk++; break;
                    case "missing": tally.DiskMissing++; break;
                    case "size-differs": tally.DiskSizeDiffers++; break;
                    default: tally.DiskContentDiffers++; break;
                }
                if (repairedNow) tally.Repaired++;
                if (repairVerified) tally.RepairVerified++;
                if (options.Repair && diskState != "ok" && !repairVerified) tally.RepairFailed++;
            }

            if (!identicalCopies)
            {
                differingRows.Add((path, new
                {
                    path,
                    winnerPak = PakNameOf(winner),
                    winnerBytes = winner.Size,
                    winnerSha256 = winnerSha,
                    lookupMatchesWinner = lookupMatches,
                    copies = list.Select((file, i) => new
                    {
                        pak = PakNameOf(file),
                        offset = OffsetOf(file),
                        bytes = file.Size,
                        sha256 = hashes[i],
                        winner = object.ReferenceEquals(file, winner)
                    }).ToArray()
                }));
            }
            if (diskState != "ok")
            {
                diskRows.Add((path, new
                {
                    path,
                    state = diskState,
                    diskBytes = diskSize,
                    winnerBytes = winner.Size,
                    diskSha256 = diskSha,
                    winnerSha256 = winnerSha,
                    repaired = repairedNow,
                    repairVerified,
                    repairError
                }));
            }
        }
        catch (Exception error)
        {
            lock (tallyLock) tally.AuditErrors++;
            errorRows.Add($"{path}: {error.GetType().Name}: {error.Message}");
        }
    });

    var differingList = differingRows.OrderBy(row => row.Path, StringComparer.Ordinal).Take(listCap).Select(row => row.Row).ToArray();
    var diskList = diskRows.OrderBy(row => row.Path, StringComparer.Ordinal).Take(listCap).Select(row => row.Row).ToArray();
    long diskDiffers = tally.DiskMissing + tally.DiskSizeDiffers + tally.DiskContentDiffers;
    bool copyCountsAgree = gathered == expectedGathered;
    bool integrityOk = copyCountsAgree && tally.LengthMismatch == 0 && tally.WinnerNotAmongCopies == 0
        && tally.ProviderLookupMismatch == 0 && tally.AuditErrors == 0;
    bool ok = integrityOk && (options.Repair ? tally.RepairFailed == 0 : diskDiffers == 0);

    return new Dictionary<string, object?>
    {
        ["generatedUtc"] = DateTime.UtcNow.ToString("yyyy-MM-ddTHH:mm:ssZ"),
        ["mode"] = "audit-duplicates",
        ["repair"] = options.Repair,
        ["pakDirectory"] = options.PakDirectory,
        ["outputRoot"] = outputRoot,
        ["workers"] = options.Workers,
        ["duplicatePaths"] = paths.Length,
        ["duplicateEntries"] = duplicateEntries,
        ["copiesGathered"] = gathered,
        ["expectedCopies"] = expectedGathered,
        ["copyCountHistogram"] = histogram.OrderBy(entry => entry.Key).ToDictionary(entry => entry.Key.ToString(), entry => (object) entry.Value),
        ["copiesRead"] = tally.CopiesRead,
        ["identicalAcrossCopies"] = tally.Identical,
        ["differingAcrossCopies"] = tally.Differing,
        ["diskOk"] = tally.DiskOk,
        ["diskDiffersFromWinner"] = diskDiffers,
        ["diskMissing"] = tally.DiskMissing,
        ["diskSizeDiffers"] = tally.DiskSizeDiffers,
        ["diskContentDiffers"] = tally.DiskContentDiffers,
        ["repaired"] = tally.Repaired,
        ["repairVerified"] = tally.RepairVerified,
        ["repairFailed"] = tally.RepairFailed,
        ["lengthMismatches"] = tally.LengthMismatch,
        ["winnerNotAmongCopies"] = tally.WinnerNotAmongCopies,
        ["providerLookupMismatches"] = tally.ProviderLookupMismatch,
        ["auditErrors"] = tally.AuditErrors,
        ["copyCountsAgree"] = copyCountsAgree,
        ["listCap"] = listCap,
        ["differingAcrossCopiesList"] = differingList,
        ["diskDiffersList"] = diskList,
        ["errors"] = errorRows.OrderBy(text => text, StringComparer.Ordinal).Take(50).ToArray(),
        ["ok"] = ok
    };
}

static string Sha256Hex(byte[] data) => Convert.ToHexString(System.Security.Cryptography.SHA256.HashData(data)).ToLowerInvariant();

static string Sha256HexStream(Stream stream) => Convert.ToHexString(System.Security.Cryptography.SHA256.HashData(stream)).ToLowerInvariant();

static Dictionary<string, object?> VerifyOutput(GameFile[] files, long plannedBytes, string outputRoot, ExtractOptions options)
{
    // Walk the output tree once; compare every planned path's size with its index size; count extras and leftovers.
    var onDisk = new Dictionary<string, long>(StringComparer.Ordinal);
    long partialLeft = 0;
    if (Directory.Exists(outputRoot))
    {
        foreach (var path in Directory.EnumerateFiles(outputRoot, "*", SearchOption.AllDirectories))
        {
            var relative = Path.GetRelativePath(outputRoot, path).Replace(Path.DirectorySeparatorChar, '/');
            if (relative.EndsWith(".partial", StringComparison.Ordinal))
            {
                partialLeft++;
                continue;
            }
            onDisk[relative] = new FileInfo(path).Length;
        }
    }
    long treeBytes = onDisk.Values.Sum();

    var planned = new HashSet<string>(StringComparer.Ordinal);
    long matchedFiles = 0, matchedBytes = 0, missing = 0, sizeMismatch = 0;
    var examples = new List<string>();
    foreach (var file in files)
    {
        planned.Add(file.Path);
        if (!onDisk.TryGetValue(file.Path, out var size))
        {
            missing++;
            AddExample(examples, $"missing {file.Path}");
        }
        else if (size != file.Size)
        {
            sizeMismatch++;
            AddExample(examples, $"size {size} != index {file.Size}: {file.Path}");
        }
        else
        {
            matchedFiles++;
            matchedBytes += size;
        }
    }
    long extraFiles = onDisk.Keys.Count(key => !planned.Contains(key));

    // Random sample: re-read each chosen entry from its pak and compare the bytes with the file on disk.
    var random = new Random(options.Seed);
    var picked = new HashSet<int>();
    int sampleTarget = Math.Min(options.SampleCount, files.Length);
    while (picked.Count < sampleTarget) picked.Add(random.Next(files.Length));
    long sampleChecked = 0, sampleMatched = 0, sampleBytes = 0;
    foreach (var index in picked.OrderBy(value => value))
    {
        var file = files[index];
        var target = ResolveTarget(outputRoot, file.Path);
        byte[] fromPak = file.Read();
        byte[] fromDisk = File.ReadAllBytes(target);
        sampleChecked++;
        sampleBytes += fromPak.LongLength;
        if (fromPak.AsSpan().SequenceEqual(fromDisk)) sampleMatched++;
        else AddExample(examples, $"sample mismatch {file.Path}");
    }
    Console.WriteLine($"Verify: tree {onDisk.Count:N0} files / {treeBytes:N0} bytes vs index {files.Length:N0} / {plannedBytes:N0}; missing {missing}, size mismatch {sizeMismatch}, extra {extraFiles}, partial leftovers {partialLeft}; sample {sampleMatched}/{sampleChecked} byte-identical.");

    bool ok = missing == 0 && sizeMismatch == 0 && extraFiles == 0 && partialLeft == 0
        && onDisk.Count == files.Length && treeBytes == plannedBytes && sampleMatched == sampleChecked;
    return new Dictionary<string, object?>
    {
        ["ok"] = ok,
        ["plannedFiles"] = files.Length,
        ["plannedBytes"] = plannedBytes,
        ["treeFiles"] = onDisk.Count,
        ["treeBytes"] = treeBytes,
        ["matchedFiles"] = matchedFiles,
        ["matchedBytes"] = matchedBytes,
        ["missing"] = missing,
        ["sizeMismatch"] = sizeMismatch,
        ["extraFiles"] = extraFiles,
        ["partialLeftovers"] = partialLeft,
        ["sample"] = new { seed = options.Seed, requested = options.SampleCount, compared = sampleChecked, matched = sampleMatched, bytes = sampleBytes },
        ["examples"] = examples.Take(20).ToArray()
    };
}

static void AddExample(List<string> examples, string text)
{
    if (examples.Count < 20) examples.Add(text);
}

static Dictionary<string, object> SummarizeTallies(Dictionary<string, ExtractTally> map) =>
    map.OrderBy(entry => entry.Key, StringComparer.Ordinal)
        .ToDictionary(entry => entry.Key, entry => (object) new { files = entry.Value.Files, bytes = entry.Value.Bytes });

static string PakNameOf(GameFile file) => file is VfsEntry entry ? entry.Vfs.Name : "(none)";

static long OffsetOf(GameFile file) => file is VfsEntry entry ? entry.Offset : 0;

static string ExtensionOf(string path)
{
    var extension = Path.GetExtension(path);
    return extension.Length == 0 ? "(none)" : extension.ToLowerInvariant();
}

// Content folder of a path: "FortniteGame/Content", "Engine/Content", "FortniteGame/Plugins", ... (first segment otherwise).
static string TopFolderOf(string path)
{
    var parts = path.Split('/');
    return parts.Length >= 3 && (parts[1] == "Content" || parts[1] == "Plugins") ? $"{parts[0]}/{parts[1]}" : parts[0];
}

static string ResolveTarget(string outputRoot, string relative)
{
    var normalized = relative.Replace('\\', '/');
    if (Path.IsPathRooted(normalized) || normalized.Split('/').Any(part => part is "" or "." or ".."))
        throw new InvalidDataException($"Unsafe path: {relative}");
    var target = Path.GetFullPath(Path.Combine(outputRoot, normalized));
    if (!target.StartsWith(outputRoot + Path.DirectorySeparatorChar, StringComparison.Ordinal))
        throw new InvalidDataException($"Path escapes output directory: {relative}");
    return target;
}

// Free bytes on the filesystem that holds the path (longest matching mount point).
static long FreeBytesFor(string path)
{
    var full = Path.GetFullPath(path);
    return DriveInfo.GetDrives()
        .Where(drive => drive.IsReady && full.StartsWith(drive.RootDirectory.FullName, StringComparison.Ordinal))
        .OrderByDescending(drive => drive.RootDirectory.FullName.Length)
        .First()
        .AvailableFreeSpace;
}

static void WriteJson(string? path, object value)
{
    if (string.IsNullOrEmpty(path)) return;
    var full = Path.GetFullPath(path);
    Directory.CreateDirectory(Path.GetDirectoryName(full)!);
    File.WriteAllText(full, JsonSerializer.Serialize(value, new JsonSerializerOptions { WriteIndented = true }));
    Console.WriteLine($"Wrote {full}");
}

// Restrict recursive reporting to known property representations. Unknown
// types remain explicit so absence of a decoder cannot masquerade as parity.
static object? MaterialValue(object? value, int depth = 0)
{
    if (value is null) return null;
    if (depth > 32) return new { unsupported = "depth limit" };
    return value switch
    {
        FPropertyTagType tag => MaterialValue(tag.GenericValue, depth + 1),
        FScriptStruct script => MaterialValue(script.StructType, depth + 1),
        FStructFallback fallback => fallback.Properties.Select(p => new { name = p.Name.Text, type = p.PropertyType.Text, value = MaterialValue(p.Tag, depth + 1) }).ToArray(),
        UScriptArray array => array.Properties.Select(p => MaterialValue(p, depth + 1)).ToArray(),
        FExpressionInput input => new { expression = input.Expression?.Name, expressionIndex = input.Expression?.Index, inputName = input.InputName.Text, expressionName = input.ExpressionName.Text, input.OutputIndex, input.Mask, input.MaskR, input.MaskG, input.MaskB, input.MaskA, fallback = MaterialValue(input.FallbackStruct, depth + 1) },
        FPackageIndex index => new { name = index.Name, index = index.Index, isImport = index.IsImport },
        FGameplayTagContainer tags => tags.GameplayTags.Select(tag => tag.TagName.Text).ToArray(),
        FText text => new { display = text.ToString() },
        FSoftObjectPath path => new { path = path.ToString() },
        FName name => name.Text,
        FGuid guid => new { type = "FGuid", value = guid.ToString() },
        string or bool or byte or sbyte or short or ushort or int or uint or long or ulong or float or double => value,
        _ when value.GetType().IsEnum => value.ToString(),
        _ => new { unsupported = value.GetType().FullName, display = value.ToString() }
    };
}

sealed record ExtractOptions(bool DryRun, bool Verify, int Workers, string? PlanOut, string? SummaryOut, int SampleCount, int Seed, string PakDirectory,
    bool AuditDuplicates = false, bool Repair = false, string? DuplicatesOut = null);

sealed class DuplicateAuditTally
{
    public long CopiesRead, LengthMismatch, WinnerNotAmongCopies, ProviderLookupMismatch, AuditErrors;
    public long Identical, Differing, DiskOk, DiskMissing, DiskSizeDiffers, DiskContentDiffers, Repaired, RepairVerified, RepairFailed;
}

sealed class ExtractTally
{
    public long Files { get; set; }
    public long Bytes { get; set; }

    public static void Add(Dictionary<string, ExtractTally> map, string key, long bytes)
    {
        if (!map.TryGetValue(key, out var tally)) map[key] = tally = new ExtractTally();
        tally.Files++;
        tally.Bytes += bytes;
    }

    public static ExtractTally Get(Dictionary<string, ExtractTally> map, string key) => map.TryGetValue(key, out var tally) ? tally : new ExtractTally();
}
