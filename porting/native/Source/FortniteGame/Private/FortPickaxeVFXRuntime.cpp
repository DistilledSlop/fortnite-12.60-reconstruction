#include "FortPickaxeVFXRuntime.h"
#include "FMarshalledVFXRuntimeData.h"
#include "FortNativeGameplayTagsRuntime.h"
#include "UFortVFXDataOwner.h"
#include "Particles/ParticleSystemComponent.h"
#include "UObject/UnrealType.h"

namespace
{
const FMarshalledVFXRuntimeData* FindRuntimeData(const UObject* Context)
{
    if (!Context) return nullptr;
    // Original 0x141dcce60 uses the native interface address, never Blueprint dispatch.
    UObject* Mutable = const_cast<UObject*>(Context);
    if (void* Address = Mutable->GetInterfaceAddress(UFortVFXDataOwner::StaticClass()))
        return static_cast<IFortVFXDataOwner*>(Address)->GetMarshalledVFXRuntimeData();

    // 0x141dcce60 passes recursion=0 and deprecated=1 to 0x141dbd550.
    for (TPropertyValueIterator<FStructProperty> It(Context->GetClass(), Context,
            EPropertyValueIteratorFlags::NoRecursion, EFieldIteratorFlags::IncludeDeprecated); It; ++It)
    {
        if (It->Key->Struct == FMarshalledVFXRuntimeData::StaticStruct())
            return static_cast<const FMarshalledVFXRuntimeData*>(It->Value);
    }
    return nullptr;
}
}

UFXSystemComponent* FortPickaxeVFXRuntime::GetFirstOrFallback(
    const UObject* Context, uint32 TagOffset, UFXSystemComponent* Fallback)
{
    const FMarshalledVFXRuntimeData* Data = FindRuntimeData(Context);
    if (!Data) return Fallback;
    const FGameplayTag Tag = FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(TagOffset);
    FGameplayTagContainer Required;
    if (Tag.IsValid()) Required.AddTag(Tag); // original invalid tag maps to an empty container
    for (UFXSystemComponent* Component : Data->DynamicSystems)
    {
        if (!Component) continue;
        const FGameplayTagContainer* Tags = Data->ComponentTags.Find(TWeakObjectPtr<UFXSystemComponent>(Component));
        if (Tags && Tags->HasAll(Required)) return Component;
    }
    return Fallback;
}
