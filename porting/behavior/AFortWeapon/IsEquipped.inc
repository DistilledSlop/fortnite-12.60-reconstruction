// CONFIDENCE: high; preserve owner cast, current-weapon identity, and virtual equip-state query.
const AFortPawn* OwningPawn = Cast<AFortPawn>(GetOwner());
return OwningPawn && OwningPawn->CurrentWeapon == this && !NativeIsEquippingWeapon();
