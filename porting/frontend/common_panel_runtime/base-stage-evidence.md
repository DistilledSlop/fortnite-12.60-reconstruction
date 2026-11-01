# CommonActivatableWidget native activation candidate: first complete piece

This first piece restores original **base widget activation behavior**, retaining
constructor defaults, virtual request/notification phases, Blueprint events,
native multicast notifications and reflected dynamic notifications. It does not
register Panel OnBeginIntro/Outro yet and does not resolve the Root runtime
blocker by itself. Panel and input queue integration are the next separate piece.

Generator base SHA-256 is
`5acb33fde99149da9bf6596476901f174529ea31db840ec77fa7fbb64b75487a`.
The GetContext candidate must be composed/rebased before shared generation.
All source changes here are off workspace. No shared Native/Engine/generator,
stage or material fixture was changed; the sole shared build owner applies only
after review.

Original base constructor 140808180 explicitly sets autoactivate/back-handler/
modal/autorestore/visibility-management false, supports-activation-focus true,
ActivatedVisibility SelfHitTestInvisible, DeactivatedVisibility Collapsed and
inactive state. Original Bool setter windows independently bind each property.
The predecessor CommonUserWidget constructor 140798270 calls Engine UserWidget
constructor and initializes its own storage, including bConsumePointerInput,
to zero. UObject allocation supplies that zero default in the existing generated
parent; this patch does not replace parent construction.

ActivateWidget/DeactivateWidget enter virtual request phases only when inactive/
active respectively. The default request changes active state before invoking
the virtual notification phase. Each base notification phase checks state once,
then applies enabled visibility, invokes BP_OnActivated/BP_OnDeactivated,
broadcasts the separate native multicast, and broadcasts the reflected dynamic
multicast. A synchronous Blueprint state change does not suppress the remaining
notifications in that original phase. Project-local names label unreflected
native delegates and virtual phases; no source byte offsets appear in C++ field
accesses.

The policy pins exact original reflection records and owner. Wrong owner,
class, parameter table, flags and duplicate emitted APIs fail closed. The
existing BP_OnActivated remains the established protected Blueprint event.

Validation performed entirely under /tmp:

- Shared C++ activation core runs under ASan/UBSan. Controls cover active guards,
  notification order, synchronous Blueprint state mutation, completion-before-
  state-test and unconditional outro completion. The latter core functions are
  prepared for the later Panel piece; no Panel method is emitted here.
- Seven signature policy tests pass, including rejection controls and no
  accidental Panel event stub.
- Isolated generation emits two classes and their required signature types.
- **Fresh isolated UHT passes** on the candidate headers. Every manifest output,
  generated CPP base, dependencies file and log path is redirected below
  `/tmp/common-panel-native-runtime-candidate-1/uht-base-1`; all other manifest
  modules have SaveExportedHeaders=false. This is not a shared UHT/build.
- Both actual generated widget C++ and fresh UHT widget C++ pass the original
  UE Clang syntax command, using a textual copy of the Engine PCH source,
  without a compiled PCH or stale-UHT line aliases.
- Fresh UHT emits original function flags: ActivateWidget/DeactivateWidget
  04020401, IsActivated 54020401, BP_OnActivated/BP_OnDeactivated 08080800.

Future build diagnostics should instantiate a real test widget subclass whose
Blueprint event synchronously reenters activation, record visibility/BP/native/
dynamic phases, and compare CDO defaults. Both built target ELFs must independently
bind registration names, exec symbols, FunctionParams flags/owner/property
counts and original return Bool signature. Fresh UHT/syntax and portable tests
do not establish widget construction, interaction or runtime parity.

Original follow-up Ghidra batches base-followup-1/2 were read-only, never saved
or renamed, and record five bookkeeping changes each. No missing function was
created. First isolated generation attempt failed before output because a
candidate-local Build.cs default was absent; retry explicitly reads the existing
project Build.cs and succeeds. Ghidra lease was subsequently released to the
separate CommonInput dependency owner.

## UHT startup side effects, discovered by trace

The initial manifest redirection does not imply that UHT's entire process writes
only there. A second fresh run (`uht-base-traced-2`) records exact file syscalls:
UHT startup writes Engine/Programs/UnrealHeaderTool/Saved/Config (including its
per-session CrashReportClient INI and Linux config files) and opens its
`/tmp/UnrealHeaderTool` instance mutex. These runs remain preserved.

One final traced run (`uht-base-isolated-3`) uses the current Engine's supported
`-UserDir=<owned>/user` and `-neverfirst`. FPaths::ProjectUserDir/ProjectSavedDir
prove the former; FUnixPlatformProcess::IsFirstInstance proves the latter.
This run passes and redirects the program config/eliminates its instance-lock
write. Engine config initialization still writes the 23-byte version-global
`~/.config/Epic/UnrealEngine/4.25/Saved/Config/Linux/Manifest.ini`.
**Blanket no-shared-write is false.** The exact conservative write-intent audit
is `write-audit-final.json`, backed by the full trace. No source, generated code,
Native or stage write appears. Shared config was not automatically restored
without a pre-run byte snapshot. No HOME/CODEX_HOME variable changed. UHT reruns
stopped after identifying this residual write.

The original first receipt is immutable. A supplementary receipt binds the
traces, final command/manifest, exact UHT/compiler binary hashes, this disclosure
and revised stage patch. The two successful C++ syntax units use the first
fresh UHT output, with the side effects above fully disclosed.
