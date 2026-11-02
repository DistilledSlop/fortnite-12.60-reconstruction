# CommonInput settings and action-widget callback

Original runtime image `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`; prior pinned SHA256 `338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680` (not rehashed). `replay.py` uses bounded `pread`, unlike the prior range script's whole-image read. Execute with `PYTHONPATH=/tmp/common-input-capstone python3 porting/frontend/common_input_settings_evidence/replay.py START END` from repository root. No build, test, generation, or native edits.

## Closed field and callback

`14081b6f0..14081b789` reads settings +0x188. Original reflection `porting/generated/reflection/fortnite_game_reflection.json` block for class no-register `14081df50` and `UCommonInputSettings` literal at `145a0e980` records `InputDataClass` offset 392 (0x188), Class, flags `0x44000000002000`, meta class `14081dce0`; `InputData` at +0x28 is SoftClass. The actual read validates class derives from `UCommonUIInputData` getter `14081dd00`, obtains its CDO (invoking class virtual+390 then reloading if necessary), and copies `DefaultClickAction` (+0x28) or returns an empty row. Do not bypass class validation or CDO creation behavior.

`1407c94c0..1407c94f8` first calls real action-widget refresh `140801d90(this)`, then passes `(input_type == 1)` to `1406a9dc0(this+0x108, &bool)`. Reflection identifies +0x108 as UCommonActionWidget `OnInputMethodChanged`, inline multicast, signature `140807760`, parameter `bUsingGamepad`; enum 1 is Gamepad. Implement as refresh followed by `OnInputMethodChanged.Broadcast(InputType == ECommonInputType::Gamepad)`, preserving order, every invocation, and real dynamic multicast semantics. The weak native subscription, removal and reentry rules remain those established in button-action-widget-update-1. Refresh itself remains a dependency.

## Module getter: strongly identified, registration link still open

`1407c4fc0` retrieves a module named CommonInput through two module-manager branches and invokes module vtable +0x40. A concrete 16-byte module factory `14081bef0..14081bf1e` initializes +8 to null and installs vtable `145a0db48`. Table +0x40 (`145a0db88`) contains `140680310`: exactly `mov rax,[rcx+8]; ret`. Table +8 is `14081d150`, which calls settings class no-register `14081df50`, calls `142ed7360` with that class, stores the returned object at module+8, and marks its global object-array item with `0x40000000` (root flag). Table +0x20 `1407fb490` clears that flag and the pointer on shutdown, subject to the global shutdown gate. This is strong structural evidence of the intended CommonInput module, but direct factory registration/name linkage has not been recovered; therefore the exact dispatch target remains conditional rather than asserted. The newly created, module-held rooted object is not proven equivalent to `GetDefault<UCommonInputSettings>()`; do not make that substitution.

The settings object's initialization path contains `14081c22e` resolving its soft class +0x28 and `14081c284` storing a checked class to +0x188; its full enclosing function, initialization trigger, soft-load behavior and rooted-class side effect are not closed. Root is independently tracing settings/platform preparation `14081d830` and `14081b550`.

Replay commands (all completed, bounded):

```
PYTHONPATH=/tmp/common-input-capstone python3 porting/frontend/common_input_settings_evidence/replay.py 1407c4fc0 1407c5054
PYTHONPATH=/tmp/common-input-capstone python3 porting/frontend/common_input_settings_evidence/replay.py 1407c94c0 1407c94f9
PYTHONPATH=/tmp/common-input-capstone python3 porting/frontend/common_input_settings_evidence/replay.py 14081b6f0 14081b78a
PYTHONPATH=/tmp/common-input-capstone python3 porting/frontend/common_input_settings_evidence/replay.py 14081bef0 14081bf1f
PYTHONPATH=/tmp/common-input-capstone python3 porting/frontend/common_input_settings_evidence/replay.py 14081d150 14081d230
PYTHONPATH=/tmp/common-input-capstone python3 porting/frontend/common_input_settings_evidence/replay.py 140680310 140680315
PYTHONPATH=/tmp/common-input-capstone python3 porting/frontend/common_input_settings_evidence/replay.py 1407fb490 1407fb4fb
PYTHONPATH=/tmp/common-input-capstone python3 porting/frontend/common_input_settings_evidence/replay.py 14081c22e 14081c33c
```

To replay vtable using only a small image slice:

```
python3 -c 'import os,struct; f=os.open("/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin",os.O_RDONLY); a=0x145a0db48; d=os.pread(f,0x48,a-0x140000000); print("\n".join(f"+{i:#x} {struct.unpack_from(chr(60)+chr(81),d,i)[0]:#x}" for i in range(0,len(d),8)))'
```

Remaining: prove module registration links CommonInput to factory `14081bef0`; identify `142ed7360` creation semantics and outer/flags; trace the full settings initialization path including +0x188 population and class lifecycle; implement real module storage and startup/shutdown behavior once proven. Existing refresh/registry/brush dependencies remain. No fabricated fallback or no-op callback.
