// Behaviour snippet: UFortLinkToActorComponent::SetRegisterWithOwnerMovementComponent (body only; spliced into the generated member function).
// Original address: 0x14297f9f0 (exec; the FFrame read and the store are in the raw decompile)
// Decompile source: porting/generated/native225-fresh-bulk/0x14297f9f0.c
// Proxy score: none (exec body verified against the raw decompile; no portability record)
// Inferred: the exec reads one parameter from the FFrame stack, then byte +0xd0: and ~0x2, then or (argument != 0) * 0x2. bool_bits.py maps (0xd0, 0x2) to bRegisterWithOwnerMovementComponent.
// Declared in generated header: no (the generator emits this declaration when the behaviour splice is on)
// CONFIDENCE: HIGH
bRegisterWithOwnerMovementComponent = bRegisterWithMovementComp;
