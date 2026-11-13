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
dependency graph. It does not establish compatibility for maps, GPU texture upload,
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

`cue4inspect --imports` found 77 script imports in that Blueprint: 30 from
`UMG`, 18 from `CommonUI`, 10 from `FortniteGame`, 9 from `Engine`, 5 from
`CoreUObject`, and 5 from `FortniteUI`. The missing modules supply concrete
types and methods such as `CommonButton`, `BP_OnHovered`,
`FortPlayerProfileBannerEditorTile`, and `FortBannerUtilities`. These counts
apply to this one package and are not a full project-wide class census.

## Texture probe

The recovered `T_UI_Icon_Mic_64` package loaded in the same staged Linux
runtime as `Texture2D`. Its platform data reported 64×64 pixels, one mip, and
4,096 bytes for that mip. The headless run exited with code 0 and no error or
warning lines. This verifies package and mip metadata access for one texture;
`-nullrhi` did not test Vulkan upload or visible rendering.

A separate `-vulkan` run under Xvfb with Mesa's Lavapipe software ICD created a
Vulkan swapchain, loaded the same texture, and exited with code 0. UE 4.25
logged a handled ensure because Lavapipe's vendor ID was unknown to this old
engine. The run confirms Vulkan initialization in this environment, but the
texture was not drawn on screen, so GPU upload and visual correctness remain
untested.
