// Original address: 0x1420e5920
// CONFIDENCE: high
JumpOffJumpZFactor = (MovementBaseActor && MovementBaseActor->IsA<AFortPawn>()) ? 0.3f : 0.5f;
Super::JumpOff(MovementBaseActor);
