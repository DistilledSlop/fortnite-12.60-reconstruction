# FortMcpContext::CancelEpicPurchasingDisplay: resolution pass

Scope: close UNKNOWNs in NOTES.md / CANCEL.md. Result: no UNKNOWN closed. The writer of this+0x378 is not found.
Image: image-125.runtime.bin (file offset = VA - 0x140000000). Scratch output: /tmp/claude-1000/cancel378_resolve/.

## Verified in this pass

- R1 Class native table (porting/generated/reflection/native_funcs.json, UFortMcpContext): CancelEpicPurchasingDisplay
  exec 0x142993d30, RegisterForEpicPurchasingDisplay 0x1429971a0, PurchaseOffer 0x142996700, PurchaseRealMoneyCatalogEntry
  0x142996ca0. Constructor body 0x142991870 (see R3); class ctor wrapper 0x1429917e0 (CANCEL.md C/Z).
- R2 Window scan, all accesses to [reg+0x378] within +/-0x40000 of 0x142995000 (find_field_refs.py): 10 hits. Excluding the
  ctor, every hit lies in a function that is not a UFortMcpContext function:
  - 0x142998ed2 in 0x142998d60 (after UFortMcpProfile RemoveGiftBox create 0x142998970), `mov qword [rdi+0x378],2`.
  - 0x14299ebd4 in 0x14299ea10 (after UFortMcpProfileCampaign OnDifferentHeroLoadoutActivated create 0x14299e750),
    `mov dword [rdi+0x378],-1`.
  - 0x1429593b1 in 0x142959390 (UFortItem GetLoadedAmmo), `call [rax+0x378]`.
  - 0x1429860a4 / 0x142987a38, 0x1429b5ef4 / 0x1429b7868, 0x1429c9974 / 0x1429cb320: `[rsp+0x378]` stack slots, not members.
  Owner attribution is by nearest preceding native entry (heuristic), but the stores are not on the context path regardless.
- R3 0x142991b4a is `48 89 b7 78 03 00 00` = `mov %rsi,0x378(%rdi)` in the ctor body 0x142991870 (bytes at file offset
  0x2991b4a). RSI is zeroed at 0x142991890 (`33 f6` xor %esi,%esi). So the ctor stores NULL to +0x378. The wrapper
  0x1429917e0 has the same body plus a null check on *param_1 (decompile diff of the two files).
- R4 Wide scan, all accesses to [reg+0x378] in 0x140000000-0x144000000 (window 0x2000000 around 0x142000000): 661 hits,
  513 stores. After dropping immediate and zero stores, 441 distinct functions. Nearest-preceding-entry attribution puts
  exactly one of them in UFortMcpContext: the ctor 0x142991870 (R3). The other register stores at +0x378 attribute to
  other classes (e.g. UMcpProfile at 0x14112e080, 0x141133ca0, 0x141158460). That attribution is heuristic and was not
  checked per function.
- R5 No LEA of +0x378 occurs in the class window (R2 used all-access). Stores through a copied base, or 16-byte stores
  written as MOVUPS/MOVDQU at +0x370 that span +0x378, are not excluded by the scan.

## Consequences for the UNKNOWN list

- UNKNOWN 1 (Helper type, writer): still UNKNOWN. Only null is stored by UFortMcpContext code. The creating function
  is not found, so decompile steps (allocation size, vtable, +0x50/+0x58) cannot run on it.
- UNKNOWN 3-9, 11, 12: unchanged. No new Impl, object, or listener evidence was produced.
- UNKNOWN 2 (UFUNCTION category): unchanged. The reflection JSON carries flags only.

## UNKNOWN count

11 open items (NOTES.md list minus the resolved #10: 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12).

## Next step

Find the code that writes a non-null value to this+0x378 by a path the scans miss: lea-based base copies, or 16-byte
stores at +0x370. Alternatively, a native caller passes a UFortMcpContext* into a helper outside the class band. A
decompile-level search of the callers of RegisterForEpicPurchasingDisplay (0x1429971a0) and PurchaseOffer (0x142996700)
is the narrowest route.
