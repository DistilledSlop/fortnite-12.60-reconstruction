# Cooked asset compatibility probe

The new UE 4.25 Linux game target was built, cooked for `LinuxNoEditor`, and
staged. The stage contains only newly cooked engine content by default.

For a focused reuse test, `cue4inspect --save-folder` recovered the supplied
Windows cooked packages in these folders into ignored generated output:

- `FortniteGame/Content/Sounds/FortSoundClassesAndModes`: 292 packages
- `FortniteGame/Content/Athena/Sounds/Events/SoundClasses`: 14 packages
- `FortniteGame/Content/Athena/Sounds/Events/RiskyReels`: 3 packages

The recovered `.uasset` and `.uexp` files were copied into the corresponding
`FortniteLinux/Content/` paths in the ignored staged build. The network-isolated
headless run used:

```sh
./FortniteLinux.sh -nullrhi -nosound -unattended \
  -FortniteProbeAsset=/Game/Sounds/FortSoundClassesAndModes/Master.Master \
  -ExecCmds=Quit
```

The runtime logged `Master.Master -> SoundClass`, loaded `/Engine/Maps/Entry`,
and exited with code 0. The final log had no error or warning lines. Earlier
runs with only one package reported missing dependencies; adding the recovered
folder trees removed those errors. This confirms direct loading for this small
dependency graph. It does not establish compatibility for maps, GPU textures,
audio payloads, skeletal meshes, Blueprints, custom Fortnite classes, or the
full 12.60 game. The staged assets and logs are generated, ignored files and
are not part of the repository.

## Blueprint boundary

The recovered UI Blueprint `BP_BannerEditorTile` was also staged alone and
requested by the runtime probe. Its dependencies include absent content and
native script packages such as `/Script/CommonUI`, `/Script/FortniteUI`, and
`/Script/FortniteGame`. Loading stopped with a fatal missing dependency on
`CommonUI.CommonButton:BP_OnHovered` (exit code 139). The newly authored
`FortniteGame` module name is therefore insufficient to run this Blueprint;
the referenced native classes, functions, and content graph must be provided
or the Blueprint must be reimplemented. This probe does not indicate that the
Blueprint bytes themselves are corrupt.
