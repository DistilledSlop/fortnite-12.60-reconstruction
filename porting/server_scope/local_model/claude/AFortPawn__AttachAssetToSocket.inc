// CONFIDENCE: MEDIUM
// Original address: 0x142a1b310 (exec thunk). Reads one struct parameter InAttachment. The local copy starts with bFlag18 = true (raw 2-byte write of 1 at +0x18), zeroed otherwise. If the FFrame reports a by-reference property, that referenced struct is used instead of the local copy. Forwards to the native body 0x142305f70 (deps AFortPawn__AttachAssetToSocketImpl).
FAttachmentAsset* AttachmentPtr = &InAttachment;
AttachAssetToSocketImpl(AttachmentPtr);
