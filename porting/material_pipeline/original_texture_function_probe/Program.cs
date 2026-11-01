// Offline, loose-package texture-context evidence. No keys or asset writes.
using CUE4Parse.FileProvider;
using CUE4Parse.UE4.Assets;
using CUE4Parse.UE4.Versions;
using Newtonsoft.Json;
using System.Security.Cryptography;

if (args.Length < 3)
    throw new ArgumentException("Usage: <extracted-root> <new-report-directory> [--instance] <package-path>...");
var root = Path.GetFullPath(args[0]);
var output = Path.GetFullPath(args[1]);
if (Directory.Exists(output) || File.Exists(output))
    throw new ArgumentException("Report directory must not exist");
bool instanceMode = args.Skip(2).Contains("--instance");
var paths = args.Skip(2).Where(x => x != "--instance").Distinct(StringComparer.Ordinal).ToArray();
if (paths.Length == 0)
    throw new ArgumentException("At least one package path is required");
string expectedType = instanceMode ? "MaterialInstanceConstant" : "MaterialFunction";
foreach (var path in paths)
    if (!(path.StartsWith("Engine/Content/", StringComparison.Ordinal) ||
          path.StartsWith("FortniteGame/Content/", StringComparison.Ordinal)) ||
        path.Contains('\\') || path.Split('/').Any(x => x is "." or ".." or ""))
        throw new ArgumentException("Unsafe package path");
var versions = new VersionContainer(EGame.GAME_UE4_25_Plus);
using var provider = new DefaultFileProvider(new DirectoryInfo(Path.Combine(root, "FortniteGame")),
    SearchOption.AllDirectories, versions, StringComparer.OrdinalIgnoreCase);
provider.Initialize();
using var engine = new DefaultFileProvider(new DirectoryInfo(Path.Combine(root, "Engine")),
    SearchOption.AllDirectories, versions, StringComparer.OrdinalIgnoreCase);
engine.Initialize();
provider.Files.AddFiles(engine.Files);
Directory.CreateDirectory(output);
foreach (var path in paths)
{
    var package = provider.LoadPackage(path);
    if (package is not Package exports)
        throw new InvalidDataException("Expected legacy package");
    var objects = exports.ExportMap.Select((entry, index) => (entry, index))
        .Where(x => x.entry.ClassName != "MetaData")
        .Select(x => package.ExportsLazy[x.index].Value).ToArray();
    if (objects.Count(x => x.ExportType == expectedType) != 1)
        throw new InvalidDataException("Expected exactly one " + expectedType + " export: " + path);
    var report = new
    {
        schema = instanceMode ? "fortnite.original-texture-instance-package.v1" : "fortnite.original-texture-function-package.v1",
        path,
        inputs = new[] { ".uasset", ".uexp" }.Select(suffix =>
        {
            var input = Path.Combine(root, path + suffix);
            return new { path = input, sha256 = Convert.ToHexString(SHA256.HashData(File.ReadAllBytes(input))).ToLowerInvariant() };
        }).ToArray(),
        skippedMetadata = exports.ExportMap.Where(x => x.ClassName == "MetaData").Select(x => x.ObjectName.Text).ToArray(),
        // UObject's CUE converter preserves serialized ordered property arrays.
        objects
    };
    File.WriteAllText(Path.Combine(output, path.Replace('/', '_') + ".json"),
        JsonConvert.SerializeObject(report, Formatting.Indented) + "\n");
    Console.WriteLine(path + " exports=" + objects.Length);
}
