// Helper 0x1426a5ae0: virtual slot 0x658 of AFortGameplayEffectDeliveryActor, caller AFortProjectileBase::SetGravityScale.
// Asm: MOVZX EAX,byte ptr [RCX+0x3e0]; AND AL,0x1; RET (bool in AL).
// Offset 0x3e0 is not covered by a reflected member (nearest is OverlapRestrictions at 0x3d0), so raw access.
// CONFIDENCE: HIGH
return (*reinterpret_cast<const uint8*>(reinterpret_cast<const uint8*>(this) + 0x3e0) & 0x1) != 0;
