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
