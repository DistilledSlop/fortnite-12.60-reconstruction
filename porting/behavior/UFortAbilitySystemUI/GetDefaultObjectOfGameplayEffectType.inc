// CONFIDENCE: high; captured native branches, fields and engine contracts.
UClass* Class = EffectType.Get();
return Class && Class->IsChildOf(UGameplayEffect::StaticClass())
    ? Class->GetDefaultObject<UGameplayEffect>() : nullptr;
