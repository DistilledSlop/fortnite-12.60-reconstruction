# Legacy held-input interface: recovery notes and blockers

Image: `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`, base 0x140000000.
Reads used os.pread on single qwords and the bounded decoder (`decode.py VA:SIZE`). No whole-image load.

## Verified

- Vtable pointers read from the original image match the handoff:
  - global `1459f1338`: 1407a5da8, 140695430, 1407c8be0, 1407cab10, 1407cc2b0, 140802440, 140800b70
  - panel `1459ee038`: 1407a5d84, 1408056ac, 1407c87e0, 1407ca7f0, 1407cbf10, 140802200, 140800860
- Slot 0 thunks (`sub rcx,0x28` / `sub rcx,0x2d8`) jump to the real deleting destructors.
- Slot 8 global 140695430 is `lea rax,[rcx-0x28]; ret`. Panel 1408056ac subtracts 0x1d0 and tail-jumps to 1406a7c40, which returns `rcx-0x108`. The net adjustment is -0x2d8.
- Manager tick 1407ffc50 (via secondary table 1459ed5f8 at manager+0x28), fully read:
  - fd clear: jump to held path (return false).
  - fd set, fc clear, count(f0) zero: increment idle(+100); if >2, clear fd and broadcast false on +90.
  - fd set otherwise: return without held tick (return true).
  - held path: when pair words at +80/+88 relative to the secondary object (manager +a8/+b0) are both nonnull, `call [interface+0x28]`.
- Global/panel owner getters are declared and defined with `return this;`. The compiler adjustor supplies the offset.

## Blockers (not emitted as bodies)

1. Held tick bodies 140802440 (global, 0x3e9 bytes) and 140802200 (panel, 0x23b bytes). HANDOFF.md describes their behavior. Their asm was not re-read in this pass, so they are declared as overrides with no definition.
2. Dispatch slots +10/+18/+20: global 1407c8be0, 1407cab10, 1407cc2b0; panel 1407c87e0, 1407ca7f0, 1407cbf10. These route through 1407c8d40 per HANDOFF. Full parameter lists are not recovered (declared with `NativeBlocked*` names and no parameters).
3. Slot +30: global 140800b70, panel 140800860. Semantics and signature not recovered.
4. Operation record element type behind the Operations TArray (header e8, count f0). Name `FCommonInputOperationRecordNative` is descriptive and not recovered.
5. `FCommonInputSuspensionChangedNative` (multicast at +90) and its delegate macro are assumed to match `NativeInputSuspensionChanged` in legacy-lifecycle fragments. Not verified against the declaration.
6. Verbose log at 1407ffcb3 via 142c54b70: message text and category not recovered.
7. Secondary table 1459ed5f8 entry for GetTickableTickType (1407c5600) is not translated here.
8. Manager `FTickableGameObject` registration (constructor 144617dd0 and 140796de0) is not reproduced; policy must not enable this until registration is in place.
9. Interface source names (IHeldInput/ActionHandler) are unknown; all names are descriptive.

## Not done

- No build, no run, no commit, no edits outside this folder.
- `policy.py` is not updated. The fragments depend on blockers 1-4 above.

## VERIFIED (decompile pass, 10 targets, Ghidra output in porting/generated/decomp-imports/Imports)

Decompiled with decompile_targets.py (--create-missing --no-index): 0x140802440, 0x140802200,
0x1407c8be0, 0x1407cab10, 0x1407cc2b0, 0x1407c87e0, 0x1407ca7f0, 0x1407cbf10, 0x140800b70, 0x140800860.
All ten returned ok. Ghidra parameter IDs and prototypes are unresolved in this slim project, so
no signature below is final. The callee lists are the verified closure work still open:

- Global held tick 140802440 calls: 142bee710 (x6), 142bfd200 (x4), 142ee5dd0, 142ee7880, 1407ae750,
  143d5a140, 142ee3b30, 142cd8ed0, 1407d0160, 1407c20f0, 1407bd580, 1407ac590, 14078c640, 140684de0.
- Panel held tick 140802200 calls: 14081b540, 14081b530, 14081b480, 142ee5dd0, 142cd8ed0, 140811c70,
  1407f17d0 (popup close, matches Panel+360 path), 1407bdba0, 1407ae640 (decompiled as Helper_ae640),
  1407ac590, 14078c640, 140684de0.
- Global dispatch A 1407c8be0, B 1407cab10, C 1407cc2b0: each builds a 16-byte payload from the
  param, wraps it through 1407ae9d0 / 1407ae9a0, calls 1407c8d40 (the shared funnel), then releases
  the refcount at param_3[2]. C additionally calls 1407cc3b0 and 1407ad780 (both undecompiled).
- Panel dispatch A 1407c87e0, B 1407ca7f0, C 1407cbf10: each reads the held-action state at this-0x2d8
  (the interface-owner adjustment), iterates the sparse bit array at param_1+0x88..0xb0, and resolves
  each row via 14078c640 and 1407cf950. Mode arg 0 calls 1407ae640 and sets the held pair.
  Mode arg 1 calls 1407cf0f0 and 1407c20f0. B and C call 1407f17d0 / 1407bc6e0 on a match.
- Slot +30 global 140800b70 and panel 140800860: both call 142ee7880, 142ee5dd0, 14081b540/530,
  and 1407cac50; global also calls 14080e600, panel also calls 14081b480. Semantics unresolved.

Still BLOCKED after this pass (closure needs these helpers translated first, none of which is verified):
- Held-tick bodies 140802440 and 140802200: need 142bee710, 142bfd200, 142ee5dd0, 142ee7880, 1407d0160,
  1407ae750 (global); 14081b480/540/530, 140811c70, 1407f17d0, 1407bdba0 (panel).
- Dispatch slots +10/+18/+20 (both implementations): need 1407c8d40, 1407cf950, 1407c5190, 1407cf0f0,
  1407ae640 (translated only as Helper_ae640), 1407bc6e0, 1407cc3b0, 1407ad780.
- Slot +30 (both): need 142ee7880, 14080e600, 1407cac50, 14081b480.
- Operation record element type, manager registration (1407ffc50 caller chain), verbose-log text: unchanged.

Side effect: the decompile run reported Ghidra project files changed on disk under
/opt/fortnite/ghidra-projects/fortnite-slim.rep (idata/index and project.prp). Not reverted here.
