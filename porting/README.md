# Fortnite 12.60 Linux reconstruction

This repository tracks new porting code and notes only. The supplied Windows
client and the separate Unreal Engine source checkout remain outside Git.
`../Fortnite/` is a read-only input to this work.

## Current evidence

- The client is `FortniteClient-12.60-CL-13477524`, a packaged Win64 build.
- The adjacent engine checkout is Epic's public UE 4.25.0 release at Git
  revision `f8f4b403eb682ffc055613c7caf9d2ba5df7f319`.
- All 31 game paks use pak version 10 and have encrypted indexes. Version 10
  is Unreal's path-hash index format. The public UE 4.25 source only defines
  pak versions through 9, so its `UnrealPak` cannot simply read these files.
- Some paks contain a nonzero encryption-key GUID. A GUID identifies a key;
  it is not the key itself. No keys are stored in this repository.
- The supplied candidate key successfully decrypted and validated 28 pak
  indexes. Three paks (`pakchunk1000`, `1001`, `1002`) need other keys.
- Those 28 indexes list 967,056 files, including project metadata, plugins,
  configuration, maps, and cooked assets. The static client scan found 59 API
  path templates; this is a partial view of the server interface.
- The supplied client does not include the Fortnite project source, editor
  assets, or a Linux executable.
- An adjacent FModel/CUE4Parse checkout can serve as a reference for pak and
  asset parsing. The user's `/opt/fortnite/assetextract.md` is also local
  reference material; neither checkout nor the article belongs in this Git
  repository. The article's embedded-key pattern is an example, not a
  guaranteed layout for this client.

Run `python3 porting/inspect_install.py > porting/generated/inventory.json` to
regenerate the local inventory. The generated file is ignored by Git.

Run `python3 porting/inspect_client_routes.py > porting/generated/routes.json`
to inventory API path templates embedded in the client. This is only a static
route list; it does not determine the full network protocol or server behavior.

Run `python3 porting/probe_pak_key.py` to test a candidate AES-256 key. The
script prompts with hidden input and prints only validation results. Never put
keys in source files, shell arguments, Git, or generated reports.

Run `python3 porting/list_pak_assets.py > porting/generated/assets.json` to
list package filenames from the indexes unlocked by a key. The output is
generated metadata and stays outside Git. The script does not extract files.

## Work needed for the requested result

1. **Asset access:** obtain the needed keys and mappings, read every pak index,
   and inventory package dependencies. Compare UE 4.25 and 4.26 pak readers.
2. **Linux client:** reconstruct game classes and content in a new UE project;
   convert or rebuild platform-specific assets; build and cook for Linux.
   This new client will have no EAC integration.
3. **Local server:** identify the client protocols and state machine, then
   implement the minimum local services for login, lobby, matchmaking, and a
   game session. Expand service coverage as client behavior requires.
4. **Full coverage:** verify maps, modes, UI, audio, inventory, progression,
   and gameplay against the supplied 12.60 build. The result must be tested
   end to end; file presence alone does not establish feature parity.

The first executable milestone is a Linux project that loads one recovered
level and character. It will establish which cooked assets can be reused and
which require conversion or reconstruction.

## Format references

- UE 4.25 `FPakInfo`: `../../UnrealEngine/Engine/Source/Runtime/PakFile/Public/IPlatformFilePak.h`
- [UEViewer pak version definitions](https://github.com/gildor2/UEViewer/blob/master/Unreal/FileSystem/UnArchivePak.h)
