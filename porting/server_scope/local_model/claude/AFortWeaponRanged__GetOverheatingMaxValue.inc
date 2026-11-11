// CONFIDENCE: LOW - attribute name, 1.0f fallback and WeaponData +0x2a4 default are exact; the attribute-set and attribute-value accessors are LOW names.
// Original addresses: exec 0x142b4f7d0 -> native 0x1426ced60.
const UFortWeaponItemDefinition* Definition = GetWeaponData();
AFortPlayerController* OwnerController = Cast<AFortPlayerController>(GetOwner());
UAbilitySystemComponent* ASC = OwnerController ? OwnerController->GetAbilitySystemComponentForWeapon() : nullptr;
UAttributeSet* AttrSet = ASC ? ASC->FindAttributeSetOfClass(UFortWeaponAttrSet::StaticClass()) : nullptr;
if (!AttrSet)
{
    return 1.0f;
}
static const FGameplayAttribute OverheatMaxMultiplierAttr =
    FindAttributeByName(UFortWeaponAttrSet::StaticClass(), TEXT("WeaponOverheatMaxMultiplier"));
const float DefaultOverheatMax = Definition ? Definition->OverheatMaxDefault : 0.0f;
return GetWeaponAttributeValueOrDefault(OverheatMaxMultiplierAttr, DefaultOverheatMax);
