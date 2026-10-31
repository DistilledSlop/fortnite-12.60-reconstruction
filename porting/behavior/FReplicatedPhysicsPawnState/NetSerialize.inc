// Behaviour snippet: FReplicatedPhysicsPawnState::NetSerialize (body only; spliced inline into the struct header, kind struct_net).
// Original address: 0x1423350b0
// Decompile source: porting/decompilation_notes.md, Eleventh pass (slot-14 thunk and body, Ghidra probe and asm).
// Thunk note: reached from the ops thunk 0x142a25f80. Raw layout: no flags, no quantization.
// Verified from the asm: destinations +0x00..+0x0b (helper, 3 floats), +0x10..+0x1c (4 dwords), +0x20 and +0x2c
// (helper, 3 floats each) and +0x38 (2 bytes). The member types (FVector_NetQuantize100/10) are not used for the wire.
// CONFIDENCE: HIGH
Ar.Serialize(&Translation.X, 4);
Ar.Serialize(&Translation.Y, 4);
Ar.Serialize(&Translation.Z, 4);
Ar.Serialize(&Rotation.X, 4);
Ar.Serialize(&Rotation.Y, 4);
Ar.Serialize(&Rotation.Z, 4);
Ar.Serialize(&Rotation.W, 4);
Ar.Serialize(&LinearVelocity.X, 4);
Ar.Serialize(&LinearVelocity.Y, 4);
Ar.Serialize(&LinearVelocity.Z, 4);
Ar.Serialize(&AngularVelocity.X, 4);
Ar.Serialize(&AngularVelocity.Y, 4);
Ar.Serialize(&AngularVelocity.Z, 4);
Ar.Serialize(&SyncKey, 2);
bOutSuccess = true;
return true;
