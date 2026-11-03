# Native controller loss and placement broadcasts

ClientNotifyLost1419c57a0 writes the reflected full-byte bClientNotifiedOfLoss
at2c35 then broadcasts one byte, the original R8B LostReason, through native
multicast31b0. The WinningPawn argument is unused. Placement1419c73e0 broadcasts
the original EDX Placement and R8D PointsEarned through native multicast3240.
The decompiler parameter labels and BattleLabInputComponent array expressions
are misleading; native instructions use controller-relative addresses directly.
Current reflected/generated declarations supply the correct argument order.

Both bodies increment delegate lock count+14, walk16-byte invocation entries in
reverse, check stale/null entries, call ExecuteIfSafe virtual+50, decrement lock
and tail-call140685370 when compaction is needed. This matches UE4.25
TBaseMulticastDelegate::Broadcast in Core/Public/Delegates/DelegateSignatureImpl.inl
lines998..1024, including callback mutation/reentry, initial bound and compaction.
Delegates/Delegate.h supplies the typed native multicast. These assigned field
names expose the native contracts without inventing reflected notifications.

Both24-byte delegate fields lie after the last nearby reflected object pointer
BattleLabInputComponent3140..3148 and do not overlap canonical native fields at
31f8..3210 or3210..3228. Source bytes themselves prove complete delegate storage
and operations, with original object ownership unchanged. bool_bits.json maps
2c35 to bClientNotifiedOfLoss (full-byte setter mask1). No field defaults,
constructor, serialization or unrelated observer/writer claims are added.

Root probe outputs under porting/generated/controller-delegates84 and log
/tmp/controller-delegates84.log; source and complete native windows through RET
pinned in evidence.json. No build, runtime or tests; generation is required.

Root reviewed and integrated the proposals in generation84:2,300 accepted
entries,0 rejected. No native build/runtime validation before checkpoint3,040.
