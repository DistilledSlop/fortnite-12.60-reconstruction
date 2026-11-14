// CONFIDENCE: high; preserve instigator cast, current-weapon identity, and virtual equip-state query.
const AFortPawn* OwningPawn = Cast<AFortPawn>(GetInstigator());
return OwningPawn && OwningPawn->CurrentWeapon == this && !NativeIsEquippingWeapon();
