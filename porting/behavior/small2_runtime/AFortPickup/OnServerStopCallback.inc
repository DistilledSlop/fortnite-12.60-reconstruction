// Behaviour snippet: AFortPickup::OnServerStopCallback (body only; spliced into the generated member function).
// Original address: 0x142036a80
// Evidence: Role (0xf0) compared to 3.
// Evidence: Byte [RCX+0x49d] = reflected bitfield bServerStoppedSimulation (mask 1).
// Evidence: Qword [RCX+0x480] = MovementComponent (Object) null-checked.
// Evidence: Dword store 0 at [RCX+0x5c8] = StartSimulatingTime (Float).
// CONFIDENCE: MEDIUM
(void)Hit;
if (static_cast<uint8>(Role) != 3)
{
    return;
}
if (bServerStoppedSimulation || MovementComponent == nullptr)
{
    return;
}
bServerStoppedSimulation = true;
StartSimulatingTime = 0.0f;
