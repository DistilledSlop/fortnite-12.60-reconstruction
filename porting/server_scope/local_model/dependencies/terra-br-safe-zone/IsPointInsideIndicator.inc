// CONFIDENCE: HIGH for body; original 0x141a4a920.
const FVector Center = SafeZoneMesh->GetComponentLocation();
const float DeltaY = Center.Y - Position.Y;
const float DeltaX = Center.X - Position.X;
return DeltaY * DeltaY + DeltaX * DeltaX <= Radius * Radius;
