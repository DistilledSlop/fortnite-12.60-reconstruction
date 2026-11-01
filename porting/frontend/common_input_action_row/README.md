# Original CommonUI input table row

`InputActionDataTable` imports `CommonInputActionData` directly. No emitted class
property referenced the row, so the class-driven type closure never selected it.
The policy guards the full original row, nested info and enum records, then feeds
the row into the existing type gate, dependency closure and module graph. It does
not emit a substitute row or weaken type checks.

The original row is 1,064 bytes, aligned to 8, and derives from `FTableRowBase`.
Two texts and six 168-byte input-info members use normal C++ construction. Each
info defaults its state to Enabled, hold flag to false and hold time to 0.5 seconds.
Its brush uses the real `FSlateBrush` defaults with only DrawAs changed to NoDrawType.
The unusual member spelling `OverrrideState` is original reflection metadata.

Original info constructor `140794ff0..1407950a4` and row constructor
`140794b80..140794f50` agree with these defaults. Original SlateBrush reflection
places DrawAs at offset 108: the info constructor changes byte info+140 from
Image (3) to NoDrawType (0). No padding zeroing or custom copy implementation is
introduced. Hash-only original spans are recorded in `original-runtime.json`:

```
python3 porting/frontend/confirmation_runtime/verify_original.py --record porting/frontend/common_input_action_row/original-runtime.json /mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin
```

Detailed proof is under `/mnt/vibrato/fortnite-work/common-input-info-ctor-brush-proof-1`
and `common-input-action-row-type-1`. Policy tests do not establish ABI or asset
loading. The opt-in `-CommonInputActionRowProbe` checks actual UE layouts/defaults,
struct initialization/traits and loading of the original table. Nonempty rows
do not prove row-content parity or input routing.
