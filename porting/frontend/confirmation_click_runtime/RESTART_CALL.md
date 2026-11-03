# 141ed81a0: restart platform call (bool, byte)

Source: `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin` (VA - 0x140000000), disassembled with objdump in small windows. Not built, not run, not committed.

Caller: `UFortRestartClientModal::HandleButtonClicked` (body 144f5f9f0) calls `141ed81a0(true, this[0x448])`, where `0x448` is the reflected `bShouldExit`/`InbShouldExit` byte set by the setter `144f7b54f`. Note: HANDOFF's `bShouldShow` pairing is not used by this call.

## What the function does (Windows x64 ABI, cl = arg1 bool, dl = arg2 byte)

1. Saves `dl` into `edi`. The byte is the exit flag.
2. If `cl == 0` (launcher path, not used by the modal):
   - Builds a name from the literal `"LauncherCheck"` (0x1459a50a0) through `142cd49c0`, resolves an object through `142cbc9d0`/`142cc0750`.
   - Calls its vtable slot `+0x48` with `ELauncherAction = 1` (AppUpdateCheck).
   - Logs `HandleClientUpdateRequired: RunLauncher(ELauncherAction::AppUpdateCheck)` at Log verbosity (0x1481348e0 gate >= 4).
   - Falls through to step 4.
3. If `cl != 0` (restart path, the modal's case):
   - Calls `142bcb050` (platform restart). Body: if verbosity gate 0x148247e70 >= 4, logs `Restart application is not supported or implemented in current platform`, then returns `false` unconditionally (`xor al,al; ret`).
   - If it returned true, return (restart happened).
4. If `edi` (exit flag) is nonzero: tail-jump to `142ce8ac0(0)` = `FPlatformMisc::RequestExit(false)`.
   - Body: verbosity gate 0x1482477f8 >= 5 logs `FPlatformMisc::RequestExit(%i)`; then calls `142c7e110` and `140684de0` (unnamed); then, since the arg is 0, tail-jumps through the global slot 0x1458fd170, which resolves to `0x6ffffe1a7ee0`. That address is outside the image range and looks like a Wine/Windows import thunk, so the non-forced exit is a process-exit import, not a Linux path.
5. Otherwise return.

## Characterization on Linux-relevant terms

- Arg 1 (`bool`): `true` selects the platform restart attempt; `false` selects the Epic launcher update-check path (`LauncherCheck` object, `RunLauncher(AppUpdateCheck)`). The modal always passes `true`.
- Arg 2 (`byte`): exit-on-failure flag. When the restart does not happen, the function requests process exit with `RequestExit(false)` (non-forced).
- Platform request: `FPlatformMisc` restart hook (`142bcb050`), which in the captured build is a stub that returns false. Engine API: `FPlatformMisc::RequestExit(bool)` (`142ce8ac0`) for exit.
- No delegate is invoked on the modal's path.

## Windows-only check?

No. The restart stub `142bcb050` contains no platform check. Its only gate is a log-verbosity compare. It is a plain generic stub that returns false, so the unsupported result is the captured runtime's behavior and does not depend on a Windows-only gate. Whether a Linux `FPlatformMisc` override exists is not determined by this dump. Porting should therefore keep the call, not drop it.

## Proposed portable body (Linux port)

```cpp
// Portable equivalent of 141ed81a0(bool bRestartPlatform, uint8 bExitIfNotRestarted).
// Modal caller passes bRestartPlatform = true and bExitIfNotRestarted = bShouldExit.
static bool PlatformRestartApplicationRequest()
{
    // Captured stub 142bcb050: logs the unsupported message and returns false.
    // Replace this body only when a Linux restart implementation exists.
    UE_LOG(LogFortUI, Log, TEXT("Restart application is not supported or implemented in current platform"));
    return false;
}

static void RequestPlatformRestartOrExit(bool bRestartPlatform, bool bExitIfNotRestarted)
{
    if (bRestartPlatform)
    {
        if (PlatformRestartApplicationRequest())
        {
            return;
        }
    }
    else
    {
        // Launcher path (not reached from HandleButtonClicked). The Linux port has no
        // Epic launcher; this branch is intentionally omitted with a log.
        UE_LOG(LogFortUI, Log, TEXT("HandleClientUpdateRequired: RunLauncher skipped (no launcher on this platform)"));
    }

    if (bExitIfNotRestarted)
    {
        FPlatformMisc::RequestExit(false);
    }
}
```

Caveats: the log category and verbosity are inferred (the captured category struct is not named). The launcher branch is an intentional omission because the original calls an Epic launcher object that the port does not have. The modal's behavior is preserved: restart unsupported, then non-forced exit when the modal's exit flag is set.

## Open items

- `142c7e110` and `140684de0` (run before the exit in step 4) are unnamed. Their effect must be checked before claiming the exit sequence is equivalent.
- The vtable `+0x48` target (launcher) and the lookup `142cbc9d0`/`142cc0750` are unnamed.
- Confirm whether a Linux `FPlatformMisc::RestartApplication` override exists. If it does, the stub replacement above must call it.
