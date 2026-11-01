// Inspect package import/export tables only; do not deserialize any UObject exports.
using System.Security.Cryptography;
using System.Text.Json;
using CUE4Parse.FileProvider;
using CUE4Parse.UE4.Assets;
using CUE4Parse.UE4.Versions;

if (args.SequenceEqual(new[] { "--self-test" }))
{
    var good = new Dictionary<int, Node> { [-1] = new("Go", "Function", -2), [-2] = new("Widget", "Class", -3), [-3] = new("/Script/Pkg", "Package", 0) };
    if (ResolveChain(-1, good).error != null) throw new Exception("valid chain rejected");
    foreach (var pair in new[] { (-1, -9), (-2, -1), (-3, 1) })
    {
        var bad = new Dictionary<int, Node>(good); bad[pair.Item1] = bad[pair.Item1] with { outer = pair.Item2 };
        if (ResolveChain(-1, bad).error == null) throw new Exception("malformed chain accepted");
    }
    var badKind = new Dictionary<int, Node>(good); badKind[-2] = badKind[-2] with { kind = "Object" };
    if (ResolveChain(-1, badKind).error == null) throw new Exception("wrong kind accepted");
    Console.WriteLine("native super import guards: five tests passed"); return;
}
if (args.Length != 3) throw new ArgumentException("usage: <closure-root> <fixed-manifest.json> <new-report.json>");
string root = Path.GetFullPath(args[0]), manifest = Path.GetFullPath(args[1]), output = Path.GetFullPath(args[2]);
if (File.Exists(output) || Directory.Exists(output)) throw new ArgumentException("report must not exist");
using JsonDocument input = JsonDocument.Parse(File.ReadAllBytes(manifest));
var entries = input.RootElement.GetProperty("files").EnumerateArray()
    .Select(x => (path: x.GetProperty("source_path").GetString() ?? "", sha: x.GetProperty("source_sha256").GetString() ?? ""))
    .Where(x => x.path.EndsWith(".uasset", StringComparison.Ordinal)).ToArray();
var paths = entries.Select(x => x.path).ToArray();
var expectedHashes = entries.ToDictionary(x => x.path, x => x.sha, StringComparer.OrdinalIgnoreCase);
if (paths.Length != 721 || paths.Distinct(StringComparer.OrdinalIgnoreCase).Count() != 721)
    throw new InvalidDataException("expected 721 unique source package headers");
foreach (string path in paths)
    if (!(path.StartsWith("FortniteGame/Content/", StringComparison.Ordinal) || path.StartsWith("Engine/Content/", StringComparison.Ordinal) || path.StartsWith("Engine/Plugins/", StringComparison.Ordinal)) ||
        path.Contains('\\') || path.Split('/').Any(x => x is "" or "." or ".."))
        throw new InvalidDataException("unsafe package path: " + path);
var versions = new VersionContainer(EGame.GAME_UE4_25_Plus, optionOverrides: new Dictionary<string, bool> {
    ["StaticMesh.Fortnite1260NoForceOpaque"] = true, ["RawIndexBuffer.HasShouldExpandTo32Bit"] = false,
    ["StaticMesh.HasRayTracingGeometry"] = false });
using var provider = new DefaultFileProvider(new DirectoryInfo(Path.Combine(root, "FortniteGame")), SearchOption.AllDirectories, versions, StringComparer.OrdinalIgnoreCase);
provider.Initialize();
using var engine = new DefaultFileProvider(new DirectoryInfo(Path.Combine(root, "Engine")), SearchOption.AllDirectories, versions, StringComparer.OrdinalIgnoreCase);
engine.Initialize(); provider.Files.AddFiles(engine.Files);
var packages = new List<object>(); var targets = new List<object>(); var rejected = new List<object>();
int exports = 0, functionExports = 0, importedSupers = 0;
foreach (string path in paths.OrderBy(x => x, StringComparer.Ordinal))
{
    string full = Path.Combine(root, path); byte[] header = File.ReadAllBytes(full);
    string hash = Convert.ToHexString(SHA256.HashData(header)).ToLowerInvariant();
    if (!hash.Equals(expectedHashes[path], StringComparison.OrdinalIgnoreCase)) throw new InvalidDataException("source package header SHA256 differs from fixed manifest: " + path);
    string packagePath = path[..^7];
    if (provider.LoadPackage(packagePath) is not Package p) throw new InvalidDataException("not a legacy package: " + path);
    exports += p.ExportMap.Length;
    packages.Add(new { path, sha256 = hash, export_count = p.ExportMap.Length, import_count = p.ImportMap.Length });
    for (int i = 0; i < p.ExportMap.Length; i++)
    {
        var e = p.ExportMap[i];
        if (e.ClassName != "Function") continue;
        functionExports++;
        int super = e.SuperIndex.Index;
        if (super >= 0) continue;
        importedSupers++;
        var nodes = p.ImportMap.Select((x, j) => (index: -(j + 1), node: new Node(x.ObjectName.Text, x.ClassName.Text, x.OuterIndex?.Index ?? 0)))
            .ToDictionary(x => x.index, x => x.node);
        var (chain, error) = ResolveChain(super, nodes);
        if (error is not null)
        {
            rejected.Add(new { package = packagePath, export_index = i + 1, export_name = e.ObjectName.Text, super_index = super, reason = error,
                chain = chain.Select(x => new { x.index, x.name, x.kind }).ToArray() }); continue;
        }
        targets.Add(new { package = packagePath, export_index = i + 1, export_name = e.ObjectName.Text,
            export_outer_index = e.OuterIndex?.Index ?? 0, super_index = super,
            native_package = chain[0].name, native_class = chain[1].name, native_function = chain[2].name,
            path = chain[0].name + "." + chain[1].name + ":" + chain[2].name });
    }
}
var report = new { schema = "fortnite.native-super-imports.v1", scope = "fixed closure; package headers only; no UObject export deserialization", 
    manifest, manifest_sha256 = Convert.ToHexString(SHA256.HashData(File.ReadAllBytes(manifest))).ToLowerInvariant(),
    tool_binary_sha256 = Convert.ToHexString(SHA256.HashData(File.ReadAllBytes(System.Reflection.Assembly.GetExecutingAssembly().Location))).ToLowerInvariant(),
    cue_binary_sha256 = Convert.ToHexString(SHA256.HashData(File.ReadAllBytes(typeof(Package).Assembly.Location))).ToLowerInvariant(),
    root, stats = new { packages = packages.Count, exports, function_exports = functionExports, imported_function_supers = importedSupers,
        native_targets = targets.Count, rejected_import_chains = rejected.Count }, packages, targets, rejected };
using var stream = new FileStream(output, FileMode.CreateNew, FileAccess.Write, FileShare.None);
JsonSerializer.Serialize(stream, report, new JsonSerializerOptions { WriteIndented = true });
Console.WriteLine(JsonSerializer.Serialize(report.stats));

static (List<(int index, string name, string kind)> chain, string? error) ResolveChain(int index, IReadOnlyDictionary<int, Node> nodes)
{
    var chain = new List<(int index, string name, string kind)>();
    var seen = new HashSet<int>(); string? error = null;
    while (index < 0)
    {
        if (!nodes.TryGetValue(index, out var node) || !seen.Add(index)) { error = "invalid or cyclic import chain"; break; }
        chain.Add((index, node.name, node.kind)); index = node.outer;
    }
    chain.Reverse();
    if (error is null && index != 0) error = "import chain has export outer";
    if (error is null && (chain.Count != 3 || chain[0].kind != "Package" || chain[1].kind != "Class" || chain[2].kind != "Function" ||
        !chain[0].name.StartsWith("/Script/", StringComparison.Ordinal) || chain.Any(x => string.IsNullOrEmpty(x.name) || x.name.Contains('.'))))
        error = "not a canonical native function import";
    return (chain, error);
}
record Node(string name, string kind, int outer);
