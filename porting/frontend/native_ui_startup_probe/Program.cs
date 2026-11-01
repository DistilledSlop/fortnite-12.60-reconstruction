// Bounded read-only original UI startup package evidence. No keys or asset writes.
using CUE4Parse.FileProvider;
using CUE4Parse.UE4.Assets;
using CUE4Parse.UE4.Versions;
using Newtonsoft.Json;
using System.Security.Cryptography;

if (args.Length is < 2 or > 3 || args.Length == 3 && args[2] != "--br-only")
    throw new ArgumentException("Usage: <loose-extraction-root> <fresh-report-directory> [--br-only]");
var root = Path.GetFullPath(args[0]);
var output = Path.GetFullPath(args[1]);
if (Directory.Exists(output) || File.Exists(output))
    throw new ArgumentException("Report destination must not exist");
var targets = new[] { "FortniteGame/Content/UI/Foundation/UIManager_Startup", "FortniteGame/Content/UI/Foundation/RootLayout_Athena",
    args.Length == 3 ? "FortniteGame/Content/UI/Foundation/Config/BROnlyUIDataConfiguration" :
        "FortniteGame/Content/UI/Foundation/Config/DefaultUIDataConfiguration",
    "FortniteGame/Content/UI/Foundation/StateLayouts/LoginScreen" };
var versions = new VersionContainer(EGame.GAME_UE4_25_Plus);
using var provider = new DefaultFileProvider(new DirectoryInfo(Path.Combine(root, "FortniteGame")),
    SearchOption.AllDirectories, versions, StringComparer.OrdinalIgnoreCase);
provider.Initialize();
using var engine = new DefaultFileProvider(new DirectoryInfo(Path.Combine(root, "Engine")),
    SearchOption.AllDirectories, versions, StringComparer.OrdinalIgnoreCase);
engine.Initialize();
provider.Files.AddFiles(engine.Files);
Directory.CreateDirectory(output);
foreach (var path in targets)
{
    var package = provider.LoadPackage(path);
    if (package is not Package exports)
        throw new InvalidDataException("Expected original legacy package");
    var objects = exports.ExportMap.Select((entry, index) => (entry, index))
        .Where(x => x.entry.ClassName != "MetaData")
        .Select(x => new { export_index = x.index + 1, name = x.entry.ObjectName.Text,
            declared_class = x.entry.ClassName, value = package.ExportsLazy[x.index].Value }).ToArray();
    var assetName = path.Split('/').Last();
    if (objects.Count(x => x.name == assetName + "_C") != 1 ||
        objects.Count(x => x.name == "Default__" + assetName + "_C") != 1)
        throw new InvalidDataException("Missing or duplicate original generated class/CDO");
    var report = new
    {
        schema = "fortnite.original-native-ui-startup-package.v1",
        path,
        total_header_size = exports.Summary.TotalHeaderSize,
        name_map = exports.NameMap.Select(x => x.Name).ToArray(),
        inputs = new[] { ".uasset", ".uexp" }.Select(suffix =>
        {
            var input = Path.Combine(root, path + suffix);
            return new { path = input, sha256 = Convert.ToHexString(SHA256.HashData(File.ReadAllBytes(input))).ToLowerInvariant() };
        }).ToArray(),
        export_map = exports.ExportMap.Select((entry, index) => new { index = index + 1,
            name = entry.ObjectName.Text, declared_class = entry.ClassName,
            serial_offset = entry.SerialOffset, serial_size = entry.SerialSize }).ToArray(),
        objects
    };
    File.WriteAllText(Path.Combine(output, assetName + ".json"),
        JsonConvert.SerializeObject(report, Formatting.Indented) + "\n");
    Console.WriteLine(path + " exports=" + objects.Length);
}
