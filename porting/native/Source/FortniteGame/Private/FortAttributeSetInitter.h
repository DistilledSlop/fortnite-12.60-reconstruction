#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "FFortAttributeInitializationKey.h"

struct FRealCurve;

// Native, non-reflected initter allocated by UFortAbilitySystemGlobals. The
// engine's one-part-key entry points are intentionally inert in the original.
struct FORTNITEGAME_API FFortAttributeSetInitter : public FAttributeSetInitter
{
    virtual void PreloadAttributeSetData(const TArray<UCurveTable*>& CurveData) override;
    virtual void InitAttributeSetDefaults(UAbilitySystemComponent* AbilitySystemComponent,
        FName GroupName, int32 Level, bool bInitialInit) const override;
    virtual void ApplyAttributeDefault(UAbilitySystemComponent* AbilitySystemComponent,
        FGameplayAttribute& InAttribute, FName GroupName, int32 Level) const override;
    virtual TArray<float> GetAttributeSetValues(UClass* AttributeSetClass,
        FProperty* AttributeProperty, FName GroupName) const override;

    // Original additional virtual +0x28, 0x141badad0.
    virtual void InitAttributeSetDefaults(UAbilitySystemComponent* AbilitySystemComponent,
        const FFortAttributeInitializationKey& Key, float Level, bool bInitialInit,
        bool bForceReplication, TSet<FProperty*>& OutInitializedAttributes) const;

    // Original additional virtual +0x30, 0x141b9b170. Unlike initialization,
    // this query uses the property's declaring class and an integer level.
    virtual bool GetAttributeDefault(const FFortAttributeInitializationKey& Key,
        const FGameplayAttribute& Attribute, int32 Level, float& OutValue) const;

private:
    using FSubCategoryCurves = TMap<FName, FRealCurve*>;
    using FCategoryCurves = TMap<FName, FSubCategoryCurves>;
    using FPropertyCurves = TMap<FName, FCategoryCurves>;
    // Curve pointers are owned by the globals' loaded UCurveTables, as in the
    // original; those tables must outlive this native cache.
    TMap<FName, FPropertyCurves> Defaults;

    bool FindValue(const FFortAttributeInitializationKey& Key,
        const FPropertyCurves& Properties, FName PropertyName,
        float Level, float& OutValue) const;
    void InitAttributeSet(UAbilitySystemComponent* AbilitySystemComponent,
        UAttributeSet* Set, const FFortAttributeInitializationKey& Key,
        float Level, bool bInitialInit, TSet<FProperty*>& OutInitializedAttributes) const;
};
