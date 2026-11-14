#include "FortAttributeSetInitter.h"

#include "AbilitySystemComponent.h"
#include "Engine/CurveTable.h"
#include "UObject/UnrealType.h"

DEFINE_LOG_CATEGORY_STATIC(LogFortAttributeSetInitter, Log, All);

// 0x141bc1730. Preloading accumulates entries; a later duplicate replaces its
// subcategory's curve. It does not clear previously loaded tables' entries.
void FFortAttributeSetInitter::PreloadAttributeSetData(const TArray<UCurveTable*>& CurveData)
{
    for (UCurveTable* Table : CurveData)
    {
        if (!Table)
        {
            continue;
        }
        for (const auto& Row : Table->GetRowMap())
        {
            TArray<FString> Tokens;
            Row.Key.ToString().ParseIntoArray(Tokens, TEXT("."), true);
            if (Tokens.Num() < 3 || Tokens.Num() > 4)
            {
                UE_LOG(LogFortAttributeSetInitter, Warning,
                    TEXT("FFortAttributeSetInitter::PreloadAttributeSetData Unable to parse row %s in %s; Expected %d to %d tokens, parsed %d"),
                    *Row.Key.ToString(), *Table->GetName(), 3, 4, Tokens.Num());
                continue;
            }
            const bool bHasSubCategory = Tokens.Num() == 4;
            const FName Category(*Tokens[0]);
            const FName SubCategory = bHasSubCategory ? FName(*Tokens[1]) : NAME_None;
            const FName ClassName(*Tokens[bHasSubCategory ? 2 : 1]);
            const FName PropertyName(*Tokens[bHasSubCategory ? 3 : 2]);
            Defaults.FindOrAdd(ClassName).FindOrAdd(PropertyName)
                .FindOrAdd(Category).Add(SubCategory, Row.Value);
        }
    }
}

// +0x10/+0x18 both point to 0x1406801c0 in the original Fort vtable.
void FFortAttributeSetInitter::InitAttributeSetDefaults(UAbilitySystemComponent*, FName, int32, bool) const
{
}

void FFortAttributeSetInitter::ApplyAttributeDefault(UAbilitySystemComponent*, FGameplayAttribute&, FName, int32) const
{
}

// +0x20 = 0x140693ec0, which returns an empty TArray<float>.
TArray<float> FFortAttributeSetInitter::GetAttributeSetValues(UClass*, FProperty*, FName) const
{
    return TArray<float>();
}

// 0x141baa550. Only a missing subcategory permits fallback, and only to None
// within the already-selected category. A present null curve does not fall back.
bool FFortAttributeSetInitter::FindValue(const FFortAttributeInitializationKey& Key,
    const FPropertyCurves& Properties, FName PropertyName, float Level, float& OutValue) const
{
    const FCategoryCurves* Categories = Properties.Find(PropertyName);
    if (!Categories)
    {
        return false;
    }
    const FSubCategoryCurves* SubCategories = Categories->Find(Key.AttributeInitCategory);
    if (!SubCategories)
    {
        return false;
    }
    FRealCurve* const* Curve = SubCategories->Find(Key.AttributeInitSubCategory);
    if (!Curve && !Key.AttributeInitSubCategory.IsNone())
    {
        Curve = SubCategories->Find(NAME_None);
    }
    if (!Curve || !*Curve)
    {
        return false;
    }
    OutValue = (*Curve)->Eval(Level, 0.f);
    return true;
}

// 0x141badc20. A map for a parent class is applied to that parent's own
// properties, rather than treating all inherited fields as derived-class rows.
void FFortAttributeSetInitter::InitAttributeSet(UAbilitySystemComponent* AbilitySystemComponent,
    UAttributeSet* Set, const FFortAttributeInitializationKey& Key, float Level,
    bool bInitialInit, TSet<FProperty*>& OutInitializedAttributes) const
{
    if (!AbilitySystemComponent || !Set || Key.AttributeInitCategory.IsNone())
    {
        return;
    }
    for (UClass* Class = Set->GetClass(); Class && Class != UAttributeSet::StaticClass();
         Class = Class->GetSuperClass())
    {
        const FPropertyCurves* Properties = Defaults.Find(Class->GetFName());
        if (!Properties)
        {
            continue;
        }
        for (TFieldIterator<FProperty> It(Class, EFieldIteratorFlags::ExcludeSuper,
                 EFieldIteratorFlags::IncludeDeprecated, EFieldIteratorFlags::ExcludeInterfaces); It; ++It)
        {
            FProperty* Property = *It;
            if ((CastField<FNumericProperty>(Property) ||
                 FGameplayAttribute::IsGameplayAttributeDataProperty(Property)) &&
                Set->ShouldInitProperty(bInitialInit, Property))
            {
                float Value;
                if (FindValue(Key, *Properties, Property->GetFName(), Level, Value))
                {
                    AbilitySystemComponent->SetNumericAttributeBase(FGameplayAttribute(Property), Value);
                    OutInitializedAttributes.Add(Property);
                }
            }
        }
    }
}

// 0x141badad0. The output is emptied even when the ASC/key is unusable.
void FFortAttributeSetInitter::InitAttributeSetDefaults(UAbilitySystemComponent* AbilitySystemComponent,
    const FFortAttributeInitializationKey& Key, float Level, bool bInitialInit,
    bool bForceReplication, TSet<FProperty*>& OutInitializedAttributes) const
{
    OutInitializedAttributes.Empty();
    if (!AbilitySystemComponent || Key.AttributeInitCategory.IsNone())
    {
        return;
    }
    for (UAttributeSet* Set : AbilitySystemComponent->SpawnedAttributes)
    {
        InitAttributeSet(AbilitySystemComponent, Set, Key, Level, bInitialInit, OutInitializedAttributes);
    }
    if (bForceReplication)
    {
        AbilitySystemComponent->ForceReplication();
    }
}

// 0x141b9b170. No superclass map search and no ShouldInitProperty gate here.
bool FFortAttributeSetInitter::GetAttributeDefault(const FFortAttributeInitializationKey& Key,
    const FGameplayAttribute& Attribute, int32 Level, float& OutValue) const
{
    if (!Attribute.IsValid())
    {
        return false;
    }
    FProperty* Property = Attribute.GetUProperty();
    UClass* Class = Cast<UClass>(Property->GetOwner<UObject>());
    if (!Class || !Class->IsChildOf(UAttributeSet::StaticClass()))
    {
        return false;
    }
    const FPropertyCurves* Properties = Defaults.Find(Class->GetFName());
    return Properties && FindValue(Key, *Properties, Property->GetFName(), float(Level), OutValue);
}
