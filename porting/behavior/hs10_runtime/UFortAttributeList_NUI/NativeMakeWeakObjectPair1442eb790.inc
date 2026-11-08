// Helper 0x1442eb790 (9 callers; 0x144de6540 passes a stack block, an object from 0x14468f1a0 and null).
// Asm: [0]=-1,[4]=0; weak-assign(+8,R8=Secondary); weak-assign(temp,RDX=Primary); [0]=temp qword (the 8-byte weak pointer); return RCX.
// 0x142ee3b30 = TWeakObjectPtr assign; the initial -1/0 stores are overwritten by the Primary weak pointer.
// Offset evidence: block has no recorded owning class; two 8-byte TWeakObjectPtr slots.
// CONFIDENCE: MEDIUM
TWeakObjectPtr<UObject>& SecondarySlot = *reinterpret_cast<TWeakObjectPtr<UObject>*>(OutPair + 0x8);
SecondarySlot = Secondary;
TWeakObjectPtr<UObject>& PrimarySlot = *reinterpret_cast<TWeakObjectPtr<UObject>*>(OutPair);
PrimarySlot = Primary;
return OutPair;
