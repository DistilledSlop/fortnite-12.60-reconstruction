# Original Athena game-mode declaration migration

This bundle replaces the **declaration** of the existing manual Athena fixture
with the original reflected class, preserving its current constructor,
StartPlay, RestartPlayer, PositionPawnOnTerrain and SpawnApolloBackground
definitions byte for byte in Private/FortniteAthenaGameplay.cpp. Those five
methods remain existing Linux compatibility/runtime support. They are not newly
recovered original native bodies. Original StartPlay and the complete original
constructor remain open; this migration does not claim their recovery.
The default script plan preserves the .cpp byte for byte. A separately guarded
optional flag now adds the recovered original GameStateClass write; see
[constructor-defaults.md](constructor-defaults.md) for the exact source/provider
and owner command. Existing pawn/controller fixture statements are preserved.

Original chain, from class-parent reflection:

`AFortGameModeAthena → AFortGamePvPBase → AFortGameModeZone →
AFortGameMode_InGame → AFortGameMode → AFortGameModeBase →
APlayspaceGameMode → AGameMode → AGameModeBase → AInfo → AActor → UObject`.

`original-hierarchy.json` retains the original ancestor records, including source
and block addresses. `original-class.json` retains all 197 property-table records
(150 actual properties after container-inner and enum-underlying pairing), all
25 reflected function records and original class flags/configuration. No field
or unrecovered function metadata is removed to make the class generate.
`validate_emitted_properties` rejects any silently omitted original property.
Unrecovered ordinary native functions remain unported under the existing
generator contract; retaining their metadata does not credit their bodies.

The original FClassParams also has **two native interfaces**, recovered from its
array at 0x1463cf228: UGameplayMutatorObserverInterface, original native offset
0xa10, and UFortMutatorOwner, original native offset 0xa18. Both K2 bits are
false. ClassFunc 0x142b64290 jumps to singleton builder 0x142b640e0, whose RDX
UTF16 literal at 0x146587b62 is GameplayMutatorObserverInterface. ClassFunc
0x1429f89f0 jumps to builder 0x1429f85f0, whose RDX literal at 0x14646d1b2 is
FortMutatorOwner. Both use /Script/FortniteGame at 0x146232ea8. The policy retains
both native C++ interface bases, using their existing generated headers. Their
unrecovered methods remain open; the existing generated interface declarations
are shells. Original offsets describe Windows provenance, not a Linux ABI to
force with padding. `migration-contract.json` pins the source windows used for
this recovery. Image: `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`,
file offset = VA minus 0x140000000.

## Native owner integration

Only the shared Native/generator/build owner should perform these steps. This
bundle itself does not mutate those paths or run generation/builds.

1. Import `source_policy.py` into gen_native_classes.py as
   `_athena_mode_policy`, following the adjacent `_native_gameplay_tags_policy`
   importlib pattern, using behavior/athena_mode_reconstruction_runtime/source_policy.py.
   In promotion, after blocks_by_va/name_to_block/parents are available, call
   `_athena_mode_policy.validate_classes(blocks_by_va, name_to_block, parents)`.
2. In build_class_promoted, after the initial model exists and before returning
   it, add:

   ```python
   model = _athena_mode_policy.apply(model, block, owner_fns)
   _athena_mode_policy.validate_emitted_properties(model, split_properties)
   ```

   In render_promoted_header before the final class `};`, add:

   ```python
   lines.extend(_athena_mode_policy.declarations(model).splitlines())
   ```

   No emitted .cpp definitions are added by this policy. Its interface header
   dependencies enter the existing splice header_includes set, so they survive
   the later own_includes assignment.
3. Run the image guard against the named source image, then the read-only plan:

   ```bash
   python3 - <<'PY'
   import importlib.util
   p = 'porting/behavior/athena_mode_reconstruction_runtime/source_policy.py'
   spec = importlib.util.spec_from_file_location('athena_mode_policy', p)
   policy = importlib.util.module_from_spec(spec)
   spec.loader.exec_module(policy)
   policy.validate_original_image('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
   PY
   python3 porting/behavior/athena_mode_reconstruction_runtime/migrate_fixture.py
   ```

4. Review fixture-header.expected.h versus fixture-header.after.h, then execute
   the same script with `--apply`. It validates all inputs before writing three
   files: Public/FortniteAthenaGameplay.h, native_class_blocklist.txt and
   native_class_set.txt. It removes only the old class declaration, adds the
   generated AFortGameModeAthena.h include before the last generated.h include,
   removes the exact Athena blocklist entry and adds Athena to the explicit
   class list. All sky/terrain/background actor declarations survive unchanged.
   The complete manual .cpp checksum is guarded and the script never writes it.
   Unexpected existing edits cause an error for owner review; no fuzzy matching.
5. Add the callback candidate from callback-manifest.proposed.json to the owner’s
   scratch manifest as `status: ported` only for actual promotion validation.
   Its existing canonical include contains the complete original native body
   0x1419520f0 reached by exec 0x1428f0440. Ensure the canonical aircraft field
   NativeInDropZone and GetAircraft/SetAircraftLocked dependencies are included.
   The ignored parameter spelling is FortAthenaAircraft. Do not replace the
   callback’s unchecked GameState cast or its full aircraft iteration.
6. Run the owner’s existing scratch promotion command against a scratch copy of
   the native source root, using the modified class list/blocklist and complete
   canonical manifest. Require no newly rejected entries, all 150 original
   Athena properties, both native interface bases and one callback definition;
   inspect the report for unresolved field dependencies and chase their source.
   Do not remove original fields or weaken the completeness gate. Verify only
   one Athena UCLASS declaration and one definition of each retained manual
   method across the combined source tree. Once scratch promotion passes, run
   `bash porting/promote_native_classes.sh` under sole owner control.
7. Build both client and server using the established owner commands, since the
   original parent chain changes server gameplay dispatch. Run the existing
   native default join and affected Apollo terrain/spawn check as one coherent
   batch. Confirm current terrain/background behavior, restart pawn positioning,
   and inherited GameMode match-state behavior. Retain commands/logs under the
   owner’s validation directory. This bundle does not establish a green build.

## Remaining original-native dependencies

* Callback execution is not a completed aircraft feature: original tick
  0x1418d7fb0, producer state, native delegate storage/broadcast/binding and the
  other tick effects remain the meaningful remaining boundary. See
  ../aircraft_dropzone_runtime/README.md. No invented tick or binding is added.
* The original GameStateClass assignment is now recovered from constructor
  0x14192dab0 and available through the optional guarded migration flag. The
  default fixture constructor only assigns pawn/controller classes; ancestry
  alone still does not supply the needed GameState setup. The full constructor
  remains unported. Do not turn the callback's intentional failed-cast
  dereference into a fallback.
* Moving AGameModeBase to the original AGameMode ancestry changes Super dispatch
  through the existing generated ancestors. Runtime fixture behavior must be
  checked by the owner; the unchanged method bodies alone are not proof of
  equivalent inherited behavior.

## Bounded validation performed

2026-11-02: original metadata/ancestor guards and every pinned original image
window PASS; existing generator split_properties reports 197 records, 150 actual
properties, no pairing notes. Read-only migration plan PASS against the current
manual header/.cpp and class-list/blocklist inputs. Receipt:
`/tmp/gameplay-athena-mode/validation.json`. No shared generation, build, runtime
or source promotion was performed by this task. No function count credit yet.
The scratch transition also passed: all four Apollo actor declarations remain,
the .cpp is byte-identical, and repeat migration fails closed.

Primary has wired the generator import, optional selected-class identity/hierarchy validation, class-model/property gate and header-declaration hook. They are inactive while AFortGameModeAthena remains blocked/manual. Owner steps1–2 above are now done; do not duplicate the hooks. Shared Native migration and class selection, followed by actual scratch generation and coherent client/server builds/joins, are still required.

Primary scratch generation now passes with original Athena selected: all150 properties retained and full original interface/parent declarations emitted. `/tmp/gameplay-athena-mode/generation/callback.command.json` and `callback-generation.log` also accept the recovered callback:292 accepted entries,0 rejected, using a scratch-only manifest/class list/blocklist. `constructor-transition.command.json` / `constructor-transition.log` PASS the guarded original GameStateClass restoration on a copied project. This is generation/transition validation only: no migration into shared Native, no migrated-class UHT/compile/link/runtime yet. The canonical callback remains uncredited until owner integration.
