// Original address: 0x1428c3000.
// CONFIDENCE: high
// The reflected exec entry is a virtual-dispatch thunk, not a self-contained body.
// It (1) performs a non-zero-guarded 64-bit increment of the script-frame field at
// +0x20 (FFrame* passed as the native Parms argument in RDX): value 0 stays 0, non-zero
// n becomes n+1, and (2) TAIL-CALLS (JMP, not CALL) the class vtable entry at byte
// offset 0xdc8 (index 440). The vtable pointer is loaded from [this] (RCX); the
// dispatch is virtual and must be preserved as such (a derived override is not
// substituted here).
//
// Resolved concrete target: primary vtable 0x146204508 slot 0xdc8 (0x1462052d0) =
// 0x142676290, which is the exact mirror of the sibling HandleScaleDownReleased
// (slot 0xdd8 = 0x142675f50, see work/q27-11100515):
//   (a) sets bIsScaleUpPressed (byte 0xf00) = 0;
//   (b) TAIL-CALLS the shared RPC helper 0x1428c0580 with
//       (bNewIsScaleUpPressed = 0, bNewIsScaleDownPressed = bIsScaleDownPressed byte 0xf35),
//       which builds param struct {bool@0, bool@1}, resolves the UFunction via
//       FindFunctionChecked(FName 0x148191ec8) and issues this->ProcessEvent (vtable
//       +0x210) = ServerUpdateScalePressed(bNewIsScaleUpPressed, bNewIsScaleDownPressed);
//   (c) reads the FVector ScaleModifier (bytes 0xe38..0xe43) into a local struct and
//       CALLS 0x1406a9dc0(&this->0xd78, &struct) = OnMoveToolScaleModified.Broadcast(ScaleModifier),
//       where 0x1406a9dc0 is the standard UE dynamic-multicast-delegate Broadcast helper.
// 23.20 SDK confirms: FVector ScaleModifier; FOnMoveToolScaleModified
// OnMoveToolScaleModified (DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVector currentScale));
// ServerUpdateScalePressed(bool bNewIsScaleUpPressed, bool bNewIsScaleDownPressed).
// See the evidence JSON for the full proof chain.
*(uint8**)((uint8*)&Stack + 0x20) += (*(uint8**)((uint8*)&Stack + 0x20) != nullptr);
(*(void (**)(void*))(*(uint8**)this + 0xdc8))(this);
