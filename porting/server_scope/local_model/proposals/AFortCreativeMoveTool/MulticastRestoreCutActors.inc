// Original address: 0x142757d50.
// CONFIDENCE: high
// The reflected exec entry is a virtual-dispatch thunk, not a self-contained body.
// It (1) performs a non-zero-guarded 64-bit increment of the script-frame field at
// +0x20 (FFrame* passed as the native Parms argument in RDX): value 0 stays 0, non-zero
// n becomes n+1, and (2) TAIL-CALLS (JMP, not CALL) the class vtable entry at byte
// offset 0xb40 (index 288). The vtable pointer is loaded from [this] (RCX); the
// dispatch is virtual and must be preserved as such (a derived override is not
// substituted here).
//
// Resolved concrete target: primary vtable 0x146204508 slot 0xb40 (0x146205048) =
// 0x14267b260. It iterates the replicated TArray CutActors (data ptr byte 0x10a0, count
// byte 0x10a8, 0x60-byte entries) and RESTORES (unhides) every non-null cut actor:
//   for (i = 0; i < CutActors.Num(); ++i) {
//       AActor* actor = CutActors[i].Actor;            // 8 bytes at entry+0x00
//       if (actor == nullptr) continue;
//       // entry+0x50 is a 16-byte saved-state block; its first byte records whether the
//       // actor's collision was enabled when it was cut.
//       if (CutActors[i].bWasCollisionEnabled != 0) {  // byte at entry+0x50
//           actor->SetActorEnableCollision(true);      // 0x143f83090(actor, 1)
//       }
//       actor->SetActorHiddenInGame(false);            // actor->vtable[0x2f8](actor, 0)
//   }
// 0x143f83090 is AActor::SetActorEnableCollision(bool): it no-ops when AActor byte 0x5c
// bit 2 (bActorEnableCollision) already equals the argument, otherwise sets that bit and
// reapplies collision on the actor's components (STAT_GetComponentsTime, component
// iteration + SetCollisionEnabled on each). vtable[0x2f8] is AActor::SetActorHiddenInGame
// (bool bHidden): here called with 0 = unhide = restore.
// MulticastRestoreCutActors has NO authority guard (it restores on every machine), in
// contrast to MulticastHideCutActors (slot 0xb48), which is guarded by a virtual bool
// (vtable[0x790], aHasAuthority-style gate) before iterating.
// See the evidence JSON for the full proof chain.
*(uint8**)((uint8*)&Stack + 0x20) += (*(uint8**)((uint8*)&Stack + 0x20) != nullptr);
(*(void (**)(void*))(*(uint8**)this + 0xb40))(this);
