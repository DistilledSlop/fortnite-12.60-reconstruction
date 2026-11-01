# Frontend asset readiness, 2026-10-31

This document combines the original static audit with the later native class-load probes. A generated header or successful class load does not establish complete Blueprint behavior or a working menu.

## Current probe snapshot (2026-11-01)

Original shader recovery is now tracked in `../material_pipeline/README.md`: the actual library ABI differs from public UE 4.25. Four failing material files expose candidate hashes for 44 original shader payloads; all recover and structurally validate after SPIR-V translation, but twelve modifier diagnostics, inline field decoding, resource bindings and actual rendering remain unresolved. This changes the recovery path, not the milestone status.

The three actual startup UI packages were recovered with their hard imports in an isolated closure. The first loose-source run (`generated/frontend-probes/closure-1000`) visited 716 packages but failed to resolve 24 `/Engine` packages because CUE4Parse indexed the extra Engine directory under the wrong root. The corrected independent Engine mount produced `generated/frontend-probes/closure-fixed`: **721 visited, 0 failed, 0 pending**, with 1,454 files including 59 Engine files. `generated/frontend-probes/runtime-fixed-manifest.json` records every overlay file and hash (909 copied, 540 existing identical, five existing preserved); the runtime is an isolated hard-linked copy of the baseline stage. These counts concern hard imports only.

On that corrected runtime, `UIManager_Startup_C` loaded as `BlueprintGeneratedClass` and exited 0 (`generated/frontend-probes/probe-fixed-uimanager.log`). `RootLayout_Athena_C` and `LoginScreen_C` each exited 139 during `FShaderMapBase::Serialize` of a Windows-cooked UI material (`probe-fixed-rootlayout.log`, `probe-fixed-login.log`). The first reported material varies with asynchronous load order: the corrected runs named `M-Border-PowerToastGlow` and `M_MenuButton_Master`; the earlier runs named `M_UI_DonutIcons` and `Athena_LockerTabMaster`. The reads sought roughly 1.2 to 1.7 billion bytes into `.uexp` files smaller than 27 KB. Missing `/Engine` imports no longer precede these assertions; the remaining `/Water` content errors are separate.

`generated/frontend-probes/material-donut.json` and `material-locker.json` show that CUE4Parse can read the two original material packages and their parameter exports. Their `Expressions` arrays contain 33 and 45 slots, but only 13 and 16 expression objects survive in the cooked exports; many slots are null. This report alone cannot reconstruct their full editor graphs or produce faithful Linux shader maps. The next material step is to establish a compatible shader/material conversion or recover the missing graph data before trying a Linux recook. A class load or a recooked stand-in would still require a separate interactive frontend test.

The recovered `UFortRuntimeOptions::GetGameVersion` getter is now generated from the native thunk/body evidence described in `behavior/README.md`. The generator self-test passed 141 checks; the behavior checker verified 251 of 251 splices; `FortniteLinux` compiled (`generated/frontend-probes/build-getgameversion.log`). The opt-in `-FortniteProbeGameVersion` run exited 0 and logged `v4.25 (0)` for this local engine's `++UE4+Release-4.25-CL-0` build (`generated/frontend-probes/probe-getgameversion.log`). This validates the local runtime branch and changelist path, not the original client's display string or execution of `LoginScreen_C`.

The stage 2 tile probe exited 139 with invalid cooked export index 45381 for an export map of 96 (`porting/generated/frontend-banner-probe-stage2.log`). The unversioned file actually stores the old full field path without an owner index: the following byte is a Blueprint opcode. Engine commit `f716e0cb6` adds the experimental, explicit `-FortniteLegacyFieldPaths` option for unversioned packages. With it, the tile advanced to a missing CommonButton `BP_OnHovered` SuperStruct (`porting/generated/frontend-banner-probe-legacy.log`).

The generator now emits the two observed protected non-native CommonButton Blueprint events and the static `UFortRuntimeOptions::GetRuntimeOptions` getter. With these changes and the experimental option, the tile probe exited 0 and returned `WidgetBlueprintGeneratedClass` (`porting/generated/frontend-banner-probe-events.log`). It also logged missing content packages, skipped native imports, and missing widget outers; this is a partial class-load result, not proof of widget creation, display, or gameplay. Full build and regressions passed (`porting/generated/regress/20261031T235803Z-2105216/result.json`).

The option is off by default and affects **all unversioned packages loaded while enabled**, not only 12.60 assets. A locally cooked unversioned empty map loaded with it, and a versioned locally cooked Blueprint class loaded with and without it; these controls do not prove every field reference or package is compatible. Do not enable it globally on this evidence.

## Entrypoints and scope

The recovered actual frontend map is `/Game/Maps/Frontend` (`/mnt/vibrato/fortnite-extracted-full/FortniteGame/Content/Maps/Frontend.{umap,uexp}`). The census records 290 direct asset imports and a 3,632-package hard closure with no failed or pending entries at that time. It contains four identified WidgetBlueprintGeneratedClass packages, none established as the root menu; the lobby closure has 12. Soft references and Blueprint runtime behavior remain unverified. A map load is not proof of a usable menu.

The smallest historically exercised UI probe is `/Game/UI/Foundation/Profile/BP_BannerEditorTile.BP_BannerEditorTile_C`. Its `.uasset` and `.uexp` already exist in the ignored Linux stage. This is a profile tile outside the four frontend closures, not the frontend root, but its old failure provides a controlled comparison. The prior run staged it alone, so its missing content dependencies must be assessed separately. Full extracted copies exist at `/mnt/vibrato/fortnite-extracted-full/FortniteGame/Content/UI/Foundation/Profile/`.

## Changed dependencies and remaining blocker

The earlier UI census's claims that CommonUI and FortniteUI were absent predate generation. Current source includes `UCommonButton`, `UCommonActivatableWidget`, `UFortBangWrapper_NUI`, `UFortPlayerProfileBannerEditorTile`, `UFortBannerUtilities`, `UFortAccountItem`, `UFortHomebaseBannerColorItem`, `UFortHomebaseBannerIconItem`, `AFortGameModeFrontEnd`, `AFortPlayerControllerFrontEnd`, `AFrontEndSettings`, and `UFortFrontEndContext`. This establishes declarations only. `UFortSocialNameTextBlock` and `UFortMediaSubtitlesPlayer` have no generated headers in the present tree; the latter's `SubtitlesWidgets` module is listed as missing in class coverage. They are graph-level dependencies, not proven dependencies of the selected tile.

The old `porting/generated/blueprint-asset-probe.log` reported `Could not find SuperStruct BP_OnHovered`. Reflection records that event and `BP_OnSelected` with zero parameters and flags `0x08080800`; both are now generated as protected Blueprint events, compiled and exercised by the successful partial tile class load above. Missing imports and content remain in that probe.

The field-path failures in `porting/map_compatibility.md` are separate Apollo Blueprint observations, not an observed failure of this tile: `BP_DisplayShadowProxy_01` reported unresolved `InstancedStaticMesh` in its construction script; storm and POI probes hit invalid export indexes during `UStruct::SerializeExpr` after `FFieldPath` data. The 4.26 branch-version backport changed indices but did not fix those crashes. Neither demonstrates this UI package's next failure.

## Completed banner class-load probe

The banner tile class-load probe ran from the stage directory with core dumps disabled:

```sh
cd /opt/fortnite/FortniteClient-12.60-CL-13477524/porting/generated/linux-stage/LinuxNoEditor
ulimit -c 0
./FortniteLinux.sh -nullrhi -nosound -unattended -log -FortniteLegacyFieldPaths \
  -FortniteProbeAsset=/Game/UI/Foundation/Profile/BP_BannerEditorTile.BP_BannerEditorTile_C \
  -ExecCmds=Quit > ../../frontend-banner-probe.log 2>&1
```

The resulting `generated/frontend-banner-probe-events.log` records a partial `WidgetBlueprintGeneratedClass` load and exit 0, with missing imports/outers still logged. This is a deserialization check; it did not create or display a widget. Later root probes used an isolated hard-import closure and reached a separate material shader-map blocker, as recorded at the top of this page. The actual `/Game/Maps/Frontend` runtime has not yet displayed a menu.

## Event declaration validation

The integrated generator emits protected, non-native `BlueprintImplementableEvent` declarations for the two observed `UCommonButton` imports only when the reflected flags equal `0x08080800` and the parameter list is empty; mismatches are recorded as skipped. UE 4.25 UHT assigns Event and BlueprintEvent, clears Native, and derives Protected from C++ access. No handwritten event body is emitted.

`python3 -I porting/frontend/test_ui_events.py` passed 2 tests and `python3 -I porting/gen_native_classes.py --self-test` passed 139 checks. Promotion, UHT, build, regression and the limited class-load probe passed as described above.

## Observed original frontend initialization and next root probe

The existing original-client log `porting/generated/direct-connect/runs/01-nomcp/client-FortniteGame.log` records `/Game/Maps/Frontend` loading with `FortGameModeFrontEnd`. It then logs `UFortGameInstance::InitializeUI()`, `UIManager_Startup_C` adding `RootLayout_Athena_C` to the viewport, `UI.State.Startup.Login`, and a `LoginScreen_C` state widget. This is evidence of the observed original initialization path, not proof the rebuilt runtime implements it. Loose package inspection finds `FortWorldSettings`, `FortGameModeFrontEnd`, `FortPlayerControllerFrontEnd`, and `FortLevelScriptActor` among the map's imports, with a `Frontend_C` export. Decoded `Frontend_C` pseudocode identifies `OnMatchStarted` forwarding into an ubergraph but does not establish widget creation or a WorldSettings game-mode property; the original runtime log supplies the game-mode observation.

The actual root assets are `/Game/UI/Foundation/UIManager_Startup.UIManager_Startup_C`, `/Game/UI/Foundation/RootLayout_Athena.RootLayout_Athena_C`, and `/Game/UI/Foundation/StateLayouts/LoginScreen.LoginScreen_C`. Their `.uasset` and `.uexp` exist in the full extraction. Loose CUE4Parse inspection of the latter two reports 323 and 381 imports respectively; `RootLayout_Athena` imports `FortRootViewportLayout_Athena`, and `LoginScreen` imports `FortUIStateWidget_Login` and exports one WidgetBlueprintGeneratedClass. These are stronger frontend probes than the profile tile, but generated native class headers do not establish UI-manager or widget behavior.

The root classes were probed separately in the isolated runtime. Both reached Windows-cooked inline material shader-map deserialization and crashed, as detailed above. The next root probe should follow a verified inline-material serializer correction and then test `/Game/Maps/Frontend` for the original `UFortGameInstance::InitializeUI` flow. A graphics-enabled run must show an interactive menu to meet the milestone; class loading alone cannot do so.

### Read-only native import comparison

A repeatable flat-name comparison is `python3 -I porting/frontend/audit_imports.py <Cue4Inspect-import-report> ...`. It reads `--imports` output and project headers, including handwritten declarations; it does not inspect engine headers or establish function ownership, reflected flags, signatures, registered modules, or functionality. Regenerate the three input reports with Cue4Inspect's `--loose --imports`: use `FortniteGame/Content/UI/Foundation/UIManager_Startup` with `/mnt/vibrato/fortnite-extracted-full`, and `FortniteGame/Content/UI/Foundation/RootLayout_Athena` and `FortniteGame/Content/UI/Foundation/StateLayouts/LoginScreen` with `/mnt/vibrato/fortnite-extracted`. Save stdout separately outside the extraction tree. The full root scan indexed 455,088 loose packages and took longer than the smaller closure scan.

At the earlier audit snapshot, every imported project class name in these three reports had a project declaration (including handwritten `UFortKismetLibrary`). This is a name check, not proof of runtime registration or matching ancestry. The following imported function names were absent from the corresponding project module's headers at that time; `GetRuntimeOptions` has since been generated and its body spliced:

- RootLayout: `BlueprintContext.GetContext`, `FortniteGame.ExecuteClientEvent`, `FortniteUI.QuitGame`, `FortniteUI.ShowSimpleConfirmationDialog_CustomInput`.
- LoginScreen: `BlueprintContext.GetContext`, `FortniteGame.GetGameVersion`, `FortniteGame.GetRuntimeOptions`, `FortniteUI.SetFrontEndCamera`, `FortniteUI.SetInputMode`, `FortniteUI.UpdateWatermark`.
- UIManager_Startup imports seven items, including `FortUIDirector` and `FortUIManager_Startup` classes; it has no function imports in this flat listing.

Some missing functions may be required before a root class can load; the import listing strips the owning object, so inspect the full import chain and reflection before emitting declarations. Even a found name may belong to the wrong class. The two widget packages also import numerous Engine and UMG names outside this check.

### Owner-aware inspection (later audit)

`Cue4Inspect --imports-json` preserves each legacy package import's raw index, outer index, and root-to-member chain as JSON; the existing `--imports` format remains available. The owner-aware `python3 -I porting/frontend/audit_import_owners.py /tmp/root-imports.json /tmp/login-imports.json` joins these chains to class reflection and parent chains, then searches the imported owner and its project ancestors' headers for declarations. Its output gives raw reflection flags and parameter types/construct pointers; found header text is only a candidate, not proof of matching UFUNCTION metadata or signature. Engine ancestors are outside the header scan.

At the earlier audit snapshot, none of these exact owner or project-ancestor headers declared the imported member. `FortRuntimeOptions:GetRuntimeOptions` has since been generated and its body spliced:

- RootLayout: `BlueprintContext.BlueprintContextLibrary:GetContext` (`0x14022409`, ContextObject/Class/ReturnValue); `FortniteGame.FortKismetLibrary:ExecuteClientEvent` (`0x4022409`, WorldContextObject/EventType/EventSource/EventFocus); `FortniteUI.FortAsyncAction_ShowConfirmation_NUI:ShowSimpleConfirmationDialog_CustomInput` (`0x4022401`, 11 reflected parameters including return); `FortniteUI.FortGlobalUIContext:QuitGame` (`0x4020401`, no parameters).
- LoginScreen: `BlueprintContext.BlueprintContextLibrary:GetContext` as above; `FortniteGame.FortRuntimeOptions:GetGameVersion` (`0x54020401`, string return) and `GetRuntimeOptions` (`0x14022401`, object return); `FortniteUI.FortBuildWatermark:UpdateWatermark` (`0x4080401`, no parameters); `FortniteUI.FortFrontEndContext:SetFrontEndCamera` (`0x4020401`, enum NewState); `FortniteUI.FortGlobalUIContext:SetInputMode` (`0x4020401`, enum InMode).

For `QuitGame`, the construct-function evidence selects the `UFortGlobalUIContext` no-parameter candidate over a distinct engine function of the same name. These are static missing-declaration findings, not evidence of successful loads or implemented behavior. Parameter listings above are abbreviated; use the report for reflection types, flags, and parameter order.

Reproduction (after `dotnet build porting/cue4inspect/Cue4Inspect.csproj -c Release --no-restore`):

```sh
dotnet porting/cue4inspect/bin/Release/net10.0/Cue4Inspect.dll FortniteGame/Content/UI/Foundation/RootLayout_Athena /mnt/vibrato/fortnite-extracted --loose --imports-json > /tmp/root-imports.json
dotnet porting/cue4inspect/bin/Release/net10.0/Cue4Inspect.dll FortniteGame/Content/UI/Foundation/StateLayouts/LoginScreen /mnt/vibrato/fortnite-extracted --loose --imports-json > /tmp/login-imports.json
python3 -I porting/frontend/audit_import_owners.py /tmp/root-imports.json /tmp/login-imports.json
```

The Release inspector build succeeded (49 warnings, zero errors). JSON reports had 323 and 381 import chains respectively; a chain-index consistency check passed for every entry. The owner report rejected an empty report. No UE build or native load was run for this investigation.
