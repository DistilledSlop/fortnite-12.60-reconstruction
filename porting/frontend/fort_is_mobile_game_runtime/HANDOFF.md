# FortKismetLibrary IsMobileGame

Unintegrated Win64 shipping candidate. This class is currently handwritten in
`FortniteMapClasses.h`, so generator model insertion is not provided. The exact
native-exec record and complete ambiguous original reflection function record are in
`original-policy.json`; `policy.validate(block, native_execs, image_path)` rejects
owner, reflection, native-exec or original instruction changes using bounded `pread`.
The class block is `0x14641a2c0`, no-register function `0x14295ff10`. Full extracted
record retains both same-name candidates; the Kismet registration has flags
`0x14022403`, exec `0x1429729a0`, create `0x14295ed90`, one UObject pointer input
`WorldContextObject` and one bool return. It is not zero-argument. Its peer
FortGlobals function is a distinct owner/exec.

Exec `0x1429729a0` steps the world-context object through either frame path,
advances bytecode when present, passes the object in RCX to `0x1406801e0`, and
stores AL in the result. The entire body at that target is bytes `32 c0 c3`
(`xor al, al; ret`); there are no global reads, conditionals or override paths in
this body. The original Win64 shipping target thus returns false for all contexts.
This does not establish other platforms' behavior. A source candidate for this
specific port is `static bool IsMobileGame(const UObject* WorldContextObject)`
returning false, exposed BlueprintPure and BlueprintCallable; confirm exact UHT
metadata against cooked import at integration. Do not substitute the distinct
zero-argument IsMobilePlatform API.

Evidence command: `python3 /mnt/vibrato/fortnite-work/common-action-widget-refresh-identity-1/decode.py 1429729a0:7f 1406801e0:3`.
Image `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`
was read using seek/pread only. This is static source evidence; no integration,
compile, runtime verification, Native/generator/Engine changes, or commit performed.
