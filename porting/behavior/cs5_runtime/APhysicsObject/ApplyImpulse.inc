// Original body 0x141b38a90 (thunk 0x142b83970): RCX=this, R8=ImpulseToApply, RDX=ImpulseInstigator unused. TArray<FVector> at object +0x790 (Data +0x790, Num +0x798, Max +0x79c), gap in APhysicsObject own region; asm is Num+1, grow via 0x140694640 if Num+1 > Max, then store FVector at old Num = TArray Add
// CONFIDENCE: HIGH
(void)ImpulseInstigator;
reinterpret_cast<TArray<FVector>*>(reinterpret_cast<uint8*>(this) + 0x790)->Add(ImpulseToApply);
