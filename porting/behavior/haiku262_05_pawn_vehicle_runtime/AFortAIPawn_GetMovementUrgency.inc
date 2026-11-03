// CONFIDENCE: high
// Native0x1427a4c40 (exec wrapper) -> native body 0x1422d4430. Static; FortAIPawn is the object parameter.
// Body: null -> 4; else returns the byte at AFortAIPawn::MovementUrgency (layout offset 0x18e0, Byte enum).
if (FortAIPawn == nullptr)
{
    return static_cast<EFortMovementUrgency::Type>(4);
}
return FortAIPawn->MovementUrgency;
