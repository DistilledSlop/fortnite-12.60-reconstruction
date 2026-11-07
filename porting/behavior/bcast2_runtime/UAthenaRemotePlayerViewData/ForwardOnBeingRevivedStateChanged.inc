// Behaviour snippet: UAthenaRemotePlayerViewData::ForwardOnBeingRevivedStateChanged (body only; spliced into the generated member function).
// Original body: 0x144c1fe20 (thunk 0x1451b8160); native body is a broadcast through ProcessMulticastDelegate 0x1406a9dc0.
// Offset evidence:
//   Asm: [RSP+0x20]=RCX (receiver, this) is the first params slot; [RSP+0x28]=R8B (bool) is the second.
//   header_decl lists InPlayerState first, but the body never reads RDX; the delegate slot receives this.
//   Object offset 0xe8 = OnBeingRevivedStateChanged, FBoolDelegate(UAthenaRemotePlayerViewData* PlayerData, bool BoolParam).
// CONFIDENCE: HIGH
OnBeingRevivedStateChanged.Broadcast(this, bInIsBeingRevived);
