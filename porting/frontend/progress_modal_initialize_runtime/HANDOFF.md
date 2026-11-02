# Progress modal InitializeModal

Reflection identifies protected BlueprintCallable final native `void InitializeModal()` (flags `0x4080401`), owner construct `0x1452ad6f0`, registration thunk `0x1452b1900`, native body `0x144f651e0`. No parameters, return, delegate or callback is involved. The body copies Description, Title and Icon, reads `ProgressWidget_Main` at +0x508, and calls the non-reflected widget initializer at `0x144e0d2b0`. That routine calls the title and description text blocks' virtual SetText, then the icon image's virtual SetBrush, then SetVisibility with HitTestInvisible when the brush resource object is not null and not pending kill, otherwise Collapsed. It does not check the widget or child pointers. The reflected fields and offsets for both classes are pinned in original-policy.json. The UE branch's `IsValid` tests the same pending-kill flag, and `GetResourceObject` returns the raw brush resource.

`definitions.inc` inlines that small native initializer, retaining value copies before calling virtual UI methods, and preserves null-pointer behavior. Include `UFortProgressWidget.h` and `Components/Image.h` in the generated modal cpp. Existing widget header provides UCommonTextBlock. No synthetic widget fields or delegates are required. The splice does not implement the other reflected modal or widget methods.

Integrate by importing policy.py, calling `validate_original`, `validate_classes(blocks, name_to_block, native_execs)` before rendering, and `apply(model, block, owner_fns)` on the modal. Emit declarations/definitions, add the cpp includes, and run `validate_output` on rendered output. Ensure the widget generated model retains all three matching reflected members and is available to the modal module. `native_execs` is the native_funcs.json mapping. Verify the module/API ownership and build after integration. Original evidence windows can be rechecked with:

```
python3 porting/frontend/progress_modal_initialize_runtime/policy.py /mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin
```

Only source-window hashes and reflection gates have been verified here; this isolated policy has not been integrated or compiled, and no interactive UI behavior has been exercised.
