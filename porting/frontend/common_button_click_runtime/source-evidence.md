# CommonButton click and routing source evidence

The supplied Win64 memory-layout image is pinned as SHA256
338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680.
Evidence below is static original-image analysis unless an explicit runtime receipt
is cited. Scratch evidence is included in the frozen base-click phase artifacts.

## Base click and analytics

Original base +4d0 is 0x1407d2580. It calls BP_OnClicked wrapper140808bd0,
native multicast140684de0 on +ad8, dynamic OnButtonClicked(this) when its list has
entries, then fresh virtual+508. Default+508 (1407c1e30) fills GetName with numeric
suffix retained and None/None, returning true. Three real FString outputs are
cleaned up in reverse declaration order after the conditional context lookup.

Accessor143d5c530 is UWidget::GetGameInstance, using GetWorld and World+188.
Resolver14078c7b0 is GI->GetSubsystem<UCommonUISubsystem>(), with collection
fallback capable of populating cached subsystem arrays. The earlier local-player
label was incorrect for these two functions; other separately proven local-player
input-subsystem paths are unaffected. Replays: button-base-click-contract-1/ and
button-click-context-resolver-1/ under /mnt/vibrato/fortnite-work.

Native multicast broadcast walks reverse and locks its invocation list. On an
invalid/empty entry it unlocks and calls compactor140685370, which uses swap
removal140690780. A removed first entry can therefore be replaced by an appended
last entry before the next reverse broadcast. The source-backed fixture expects
BP,Second,Dynamic,Analytics on the first pass and BP,Second,Added,Dynamic,Analytics
on the next. Original fragments and local engine headers are independently pinned
in button-click-compaction-proof-1/. It corrects a stable-removal assumption in an
earlier failed probe; the actual native multicast implementation was unchanged.

## Legacy +498: concrete argument proof and native row producer

Original1407bd360 resolves the game-instance subsystem, takes its CommonInputManager
(+50), scans ActivatablePanelStack (+b8) backward and checks panel NativeActionRecords
(+360). The existing FCommonPanelActionRecord is the same map value used by panel
outro. The relevant commit instance is entry+28/count+30, not entry+10's progress
multicast. It stops on the first active matching panel even if its delegate is invalid.
The reused map slot supplies the initial argument byte; it is not initialized false.

Panel fallback registration1407f6930 clones its R8 delegate into source value+18;
map insertion14078aed0 and pair constructor140785640 clone it into entry+28.
Caller1407f7220 constructs a concrete delegate with vtable1459fe450. Slot+50 calls
1407bca80 then1407bc8a0, which normalizes the byte pointed to by RDX, dispatches a
one-byte ProcessEvent parameter and copies it back. This directly supports the
maintained NativeCommit(bool&) signature. It does not identify the dynamic target
as any particular button callback. panel-commit-producer-trace-1/replay.py pins the
producer, insertion, vtable and execution spans. Earlier reflected bool-out metadata
alone did not establish the panel linkage; the manager global handler is distinct.

The button's +498 row is an unreflected native row at +2c0, distinct from reflected
TriggeringInputAction at +2b0. SetTriggeredInputAction exec14080d7f0 calls body1407fa760,
which copies the supplied row to +2c0. Its full behavior additionally removes and
adds panel/manager held bindings, invokes action-widget virtual+4a0 and a reflected
changed event. Exact dynamic names are NativeOnActionProgress/NativeOnActionComplete,
from UTF16 literals1459f1a40/1459f1a90. These producer effects are still unported;
never alias this native row to TriggeringInputAction or silently skip those effects.
button-triggered-row-producer-1/ pins the original bodies and strings. This research
does not register legacy routing, the setter, accepted-click, or Initialize.

The same storage correction applies to native double/hover/unhover/focus slots
+af0/+b08/+b20/+b38: their complete pinned base bodies call or tail-jump to the
same multicast broadcaster140684de0. Earlier singlecast labels in dependency
notes were wrong. button-native-multicast-correction-1/ records exact call sites
and source windows. Binding ownership still depends on each actual producer.
