# Vtable slot 0x270 (index 78) for UImage: target and behavior

Scope: the virtual call `jmp *0x270(%rax)` when the receiver is the UImage at UFortConfirmationWindow +0x480 (Image_MainIcon). Evidence: flat disassembly of /mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin (VA = 0x140000000 + file offset), Ghidra pseudo-C from porting/generated/decomp-imports/Imports/SetBoolSlot270__143d91830__143d91830.c, reflection JSON, and UE 4.25 engine source. Nothing built or run; nothing committed.

## UImage vtable (VERIFIED)

- UImage class getter 0x143dac430 (see MEMBERS.md). Its GetPrivateStaticClass call (disassembly of 0x143dac430, stores at r11-0x40 and r11-0x38) passes InternalConstructor = 0x143dac230 (10th argument) and InternalVTableHelperCtorCaller = 0x140680300 (11th argument, returns null).
- 0x143dac230 loads the object pointer and tail-jumps to 0x143d34b60.
- 0x143d34b60 calls 0x143d38d40 (base constructor), then stores vptr 0x14682cf08 at offset 0 (`lea 0x2af8388(%rip),%rax` at 0x143d34b79, `mov %rax,(%rdi)` at 0x143d34b80). This is the UImage vptr.
- The reflection candidate 0x1468284a8 (native_class_index.json, UImage) is shared by 11 classes (UImage, UListView, UHorizontalBox, ...). It is NOT UImage-specific and is not used here.
- Table at 0x14682cf08: 100 code pointers (slots 0..99). Slot 78 (0x270) = 0x143d91830.

## Slot 0x270 target: 0x143d91830 (VERIFIED behavior)

Disassembly and Ghidra pseudo-C:
1. `*(uint8*)(this+0xc3) = arg` (arg = dl, the ESlateVisibility index). Reflection lists "Visibility", kind Enum, offset 195 (0xc3), in the class-property records.
2. `FUN_143d5a140(this, &out)`; name UNKNOWN (engine GetCachedWidget is a candidate, not verified). If `out` is null, return.
3. Map arg through a 5-entry table, default Visible: 0 -> 0x19 (25), 1 -> 0x02, 2 -> 0x04, 3 -> 0x01, 4 -> 0x11 (17). Table at 0x147c522f0..f4. These are EVisibility values (Visible=VIS_Visible=0x19, Collapsed=2, Hidden=4, HitTestInvisible=1, SelfHitTestInvisible=0x11; Visibility.h).
4. `(**(code**)(*out + 0x1d8))(out, {value, 1, 0, 0})`: a virtual call on the Slate widget with the EVisibility value. Slot 0x1d8 identity UNKNOWN.

Engine comparison (UE 4.25): `UWidget::SetVisibility(ESlateVisibility)` (Runtime/UMG/Private/Components/Widget.cpp:304-314) is declared `virtual` (Runtime/UMG/Public/Components/Widget.h:523). It stores `Visibility = InVisibility`, calls GetCachedWidget, and if valid calls `ConvertSerializedVisibilityToRuntime` (Widget.cpp:1376-1395) then `SafeWidget->SetVisibility`. The 0..4 mapping order matches the ESlateVisibility enum (SlateWrapperTypes.h:23-32) and the default case returns Visible. Every step matches, so slot 0x270 is named UWidget::SetVisibility (engine name, address 0x143d91830 recorded). Caveat: the image is a stripped UE build; the only checked identity is the body.

## Call sites and flag meaning

- u2 at 0x144f7601e (receiver [W+0x480]): dl = ([W+0x4f8]==0) ? 1 : 0 (from NOTES.md, disassembly). So if W+0x4f8 == 0, the target gets index 1, and the EVisibility is Collapsed (2). Otherwise index 0, Visible (0x19). W+0x4f8 meaning UNKNOWN.
- Helper 0x144f5fcc0: dl = (arg != 0) ? 4 : 1 (index 4 SelfHitTestInvisible, or index 1 Collapsed). Its receiver is [arg+0x2c0] and its class is UNKNOWN, so whether slot 0x270 here reaches 0x143d91830 is UNKNOWN. Caller UNKNOWN.

## Summary

- Slot 0x270 of UImage = 0x143d91830 = UWidget::SetVisibility(ESlateVisibility) body: stores the visibility byte at +0xc3, converts to EVisibility, and applies it to the cached Slate widget.
- For u2, the effect on Image_MainIcon is Collapsed when W+0x4f8 == 0, otherwise Visible.
