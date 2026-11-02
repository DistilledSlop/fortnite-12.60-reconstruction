# Restart modal click handler recovery

The missing reflected function is `UFortRestartClientModal::HandleButtonClicked()` (no parameters, flags `0x4080401`). The class block is `0x146fc2220`, class no-register function `0x1452bfc40`, and the owner function candidate has outer `0x1452bfd00` and create function `0x1452bfd30`. `native_funcs.json` identifies its exec as `0x1452c2a80`. The generated class currently omits the function.

The exec advances the script frame and tail-jumps to `0x144f5f9f0`. In Windows x64 calling convention the native function performs these operations in order:

1. Call `0x141ed81a0` with first argument byte `1` and second argument byte `this[0x448]`.
2. If byte `this[0x304]` is nonzero, tail-call the object's virtual slot at vtable byte offset `0x4c0`.
3. Otherwise tail-call `0x1407b7ed0`, which returns if byte `this[0x288]` is zero and otherwise tail-calls the object's virtual slot at `0x4a8`.

The modal constructor callback `0x145228430` initializes byte `0x448` to zero and installs primary vtable `0x146d700c8`. Setter `0x144f7b54f` writes its second argument to this byte and invokes the `OnButtonSet` wrapper `0x1452c17d0` with that same argument and `true` (the reflected parameters establish those as `InbShouldExit` and `bShouldShow`). The first call's first argument selects platform restart: `0x142bcb050` logs “Restart application is not supported or implemented in current platform” and returns zero. If that returns zero and the stored exit flag is nonzero, `0x141ed81a0` calls `0x142ce8ac0` with zero; its log literal identifies `FPlatformMisc::RequestExit(%i)`.

Byte `0x304` records membership on the activation stack, per `common_input_manager_operation_queue.md`. The modal's actual vtable entry `0x4c0` is `0x1407e1130`: if panel weak manager `0x3b0` is valid, it resolves the manager and calls its `PopActivatablePanel(this)` implementation `0x1407e0c80`. The other byte `0x288` is reflected `bIsActive`; fallback is `DeactivateWidget()`, whose guarded request dispatches virtual slot `0x4a8`. Modal slot `0x4a8` is the panel override `0x1407ce810`. See `common_activatable_panel_native_events.md` for identity and dispatch evidence.

A faithful body requires the manager's actual operation queue, stack membership state, and panel pop virtual, none currently implemented in the generated CommonUI runtime. It also requires a portable equivalent for the restart call on the target platform; simply dropping it based on the captured platform's unsupported result would change the call and potential platform behavior. The setter is a separate native function boundary, not a reflected setter, and must be recovered at its call sites when connecting this state. No body or generator policy is supplied until these dependencies exist.

For reproducibility, the following SHA-256 values cover bounded windows in `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin` at VA minus `0x140000000`:

| VA | Length | SHA-256 |
|---|---:|---|
| `0x1452c2a80` | `0x1a` | `9df58122abec40c4f5e37d9c5928e759bb3c9ab251ec45ec93dfb3b67ef944b6` |
| `0x144f5f9f0` | `0x38` | `63cfa17dd8a189d6b19fbf03b612919952c37272f98208899134323eb16483a1` |
| `0x141ed81a0` | `0xb1` | `a8092c06a63867b617fabc7aae4cbb4f3722ece47eb17e6b787f5af9b1b65bff` |
| `0x1407b7ed0` | `0x14` | `e76619ab8444a8c4f24123f0b47b383bc95fd78b7c29a170bf7e200c6f945f78` |
| `0x142bcb050` | `0x36` | `653aba0966c4d76a9d4079d628fb29cd00f27b55183471f5c8eaf434aa1dab15` |
