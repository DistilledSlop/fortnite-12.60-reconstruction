// Offline original atlas metadata inspection; no pixel conversion or game code.
using CUE4Parse.FileProvider;
using CUE4Parse.UE4.Assets.Exports.Texture;
using CUE4Parse.UE4.Versions;
using System.Security.Cryptography;
using System.Text.Json;
if (args.Length!=2) throw new ArgumentException("usage: <extracted-root> <new-json-output>");
var root=Path.GetFullPath(args[0]);var output=Path.GetFullPath(args[1]);
if (File.Exists(output)||Directory.Exists(output)) throw new ArgumentException("output exists");
var versions=new VersionContainer(EGame.GAME_UE4_25_Plus);
using var provider=new DefaultFileProvider(new DirectoryInfo(Path.Combine(root,"FortniteGame")),SearchOption.AllDirectories,versions,StringComparer.OrdinalIgnoreCase);
provider.Initialize();
var package=provider.LoadPackage("FortniteGame/Content/UI/SocialPanel/Art/CA_SocialPanel");
var texture=package.GetExports().OfType<UTexture2D>().Single();
var inputs=new[]{".uasset",".uexp"}.Select(ext=>{var path=Path.Combine(root,"FortniteGame/Content/UI/SocialPanel/Art/CA_SocialPanel"+ext);var bytes=File.ReadAllBytes(path);return new {path,bytes=bytes.Length,sha256=Convert.ToHexString(SHA256.HashData(bytes)).ToLowerInvariant()};}).ToArray();
var report=new {schema="fortnite.social-border-atlas-metadata.v1",package="FortniteGame/Content/UI/SocialPanel/Art/CA_SocialPanel",name=texture.Name,export_type=texture.ExportType,
 pixelFormat=texture.Format.ToString(),srgb=texture.SRGB,filter=texture.Filter.ToString(),lodGroup=texture.LODGroup.ToString(),addressX=texture.AddressX.ToString(),addressY=texture.AddressY.ToString(),firstMip=texture.PlatformData.FirstMipToSerialize,
 mips=texture.PlatformData.Mips.Select(m=>new {width=m.SizeX,height=m.SizeY,depth=m.SizeZ,bulkFlags=m.BulkData?.BulkDataFlags.ToString(),bulkOffset=m.BulkData?.Header.OffsetInFile,bulkBytes=m.BulkData?.Header.SizeOnDisk}).ToArray(),
 properties=texture.Properties.Select(p=>new {name=p.Name.Text,value=p.Tag?.ToString()}).ToArray(),inputs};
using var file=new FileStream(output,FileMode.CreateNew,FileAccess.Write,FileShare.None);JsonSerializer.Serialize(file,report,new JsonSerializerOptions{WriteIndented=true});Console.WriteLine(JsonSerializer.Serialize(report));
