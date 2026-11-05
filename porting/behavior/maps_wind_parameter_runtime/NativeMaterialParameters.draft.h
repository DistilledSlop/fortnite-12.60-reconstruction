// Typed behavior of material parameter helpers; review draft, not integrated.
#pragma once
#include "Materials/Material.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceDynamic.h"

namespace FortWindParameterRuntimeDraft
{
// 0x141dccf40: the original is an AFortWindManager member, but does not use
// its receiver. Only MI overrides count; a non-MI parent ends the search.
// Unlike the generic material parameter lookup, association/index are ignored.
inline FScalarParameterValue* FindScalarOverride(
    UMaterialInstance* Instance, const FName& Name)
{
    while (Instance)
    {
        for (FScalarParameterValue& Value : Instance->ScalarParameterValues)
            if (Value.ParameterInfo.Name == Name)
                return &Value;
        Instance = Cast<UMaterialInstance>(Instance->Parent);
    }
    return nullptr;
}

// 0x144333900 and 0x1443339a0. The UE methods construct global parameter
// info (association 2, index -1), set the initial value, and return its index.
inline bool InitializeScalar(UMaterialInstanceDynamic& Instance, const FName& Name,
                             float Value, int32& Index)
{
    return Instance.InitializeScalarParameterAndGetIndex(Name, Value, Index);
}
inline bool InitializeVector(UMaterialInstanceDynamic& Instance, const FName& Name,
                             const FLinearColor& Value, int32& Index)
{
    return Instance.InitializeVectorParameterAndGetIndex(Name, Value, Index);
}

// 0x14433ac70 and 0x14433b1f0. A valid index succeeds even if the value
// is unchanged; a changed value updates the render parameter and uniform cache.
inline bool SetScalar(UMaterialInstanceDynamic& Instance, int32 Index, float Value)
{
    return Instance.SetScalarParameterByIndex(Index, Value);
}
inline bool SetVector(UMaterialInstanceDynamic& Instance, int32 Index,
                      const FLinearColor& Value)
{
    return Instance.SetVectorParameterByIndex(Index, Value);
}
// Setup's base-material vtable slots 0x3e0/0x3f8. The caller ignores the
// boolean and consumes the output initialized to zero, including on failure.
// The local UE checkout takes FHashedMaterialParameterInfo; the captured client
// takes its older un-hashed FMaterialParameterInfo ABI. Both calls are global.
inline float BaseScalarValue(const UMaterial& Base, const FName& Name)
{
    float Value = 0.0f;
    Base.GetScalarParameterValue(FHashedMaterialParameterInfo(Name), Value, false);
    return Value;
}
inline FLinearColor BaseVectorValue(const UMaterial& Base, const FName& Name)
{
    FLinearColor Value(0.0f, 0.0f, 0.0f, 0.0f);
    Base.GetVectorParameterValue(FHashedMaterialParameterInfo(Name), Value, false);
    return Value;
}

}
