// Evidence lift, not integration-ready source. Names beginning Native are unresolved
// operations or fields, not implementations. Virtual calls must remain virtual.
// 0x141b94070 -- CreateAbilitySystemComponent(bool bFromLazyGetter)
void ABuildingActor::CreateAbilitySystemComponent_Lift(bool bFromLazyGetter)
{
    if (!GetWorld()) return;

    EAbilitySystemComponentCreationPolicy EffectivePolicy = AbilitySystemComponentCreationPolicy;
    if (EffectivePolicy == EAbilitySystemComponentCreationPolicy::Lazy &&
        !NativeFortGlobalsChecked()->bEnableLazyASC) // helper 0x141ed2350
    {
        EffectivePolicy = EAbilitySystemComponentCreationPolicy::Always;
    }
    else if (EffectivePolicy == EAbilitySystemComponentCreationPolicy::Lazy && bFromLazyGetter)
    {
        FlushNetDormancy();
    }

    AbilitySystemComponent = NewObject<UFortAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
    AbilitySystemComponent->PrimaryComponentTick.bStartWithTickEnabled = false;
    AbilitySystemComponent->PrimaryComponentTick.bCanEverTick = false;
    AbilitySystemComponent->bAutoActivate = false;
    AbilitySystemComponent->RegisterComponent();
    NativeLogASCCreatedIfVerbose6(); // actor, sublevel, component

    // Native validates BuildingAttributeSetClass against its expected base; preserve
    // its null/invalid-class behavior when recovering the exact NewObject overload.
    BuildingAttributeSet = NativeNewBuildingAttributeSet(BuildingAttributeSetClass,
                                                          this, TEXT("BuildingAttributeSet"));
    NativeVirtual_7a0(nullptr, BuildingAttributeSet);
    if (EffectivePolicy == EAbilitySystemComponentCreationPolicy::Lazy)
        AbilitySystemComponent->SpawnedAttributes.AddUnique(BuildingAttributeSet);
    else
    {
        AbilitySystemComponent->SetNetAddressable();
        BuildingAttributeSet->SetNetAddressable();
    }
    NativeVirtual_790();
    NativeVirtual_798(); // base body below, but subclasses may override
}

// 0x141b8f710 -- base virtual +0x798
void ABuildingActor::InitializeAbilitySystemComponent_Lift()
{
    if (NativeASCInitializedBit_3e7_02 || !AbilitySystemComponent || !BuildingAttributeSet)
        return;

    NativeVirtual_758(); // base: add StaticGameplayTags as individual loose tags
    NativeVirtual_760(); // base: bind seven UFortHealthSet delegates; see handoff
    UWorld* World = GetWorld();
    if (AbilitySystemComponentCreationPolicy == EAbilitySystemComponentCreationPolicy::Lazy &&
        NativeFortGlobalsChecked()->bEnableLazyASC &&
        AbilitySystemComponentCreationPolicy == EAbilitySystemComponentCreationPolicy::Lazy)
    {
        NativeVirtual_780(true);
        if (NativeByte_34c == 1 && World)
        {
            auto* Registry = NativeWorldLookup_1416413a0(World);
            if (Registry) NativeWorldRegister_141ea8c00(Registry, this);
        }
        NativeVirtual_770();
    }
    NativeASCInitializedBit_3e7_02 = true;
    NativeLogASCInitializedIfVerbose6(); // includes bPersistToWorld
    if (bPersistToWorld && World)
    {
        auto* Service = NativeWorldLookup_1416412b0(World);
        if (Service && NativeServiceVirtual_9b0(Service)) NativeVirtual_700();
    }
}

// 0x141bc5470 -- base virtual +0x758. The getter and each notification may
// dispatch virtually. Reacquire the iterator because a notification may mutate tags.
void ABuildingActor::InitializeStaticAbilityTags_Lift()
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    if (ASC)
        for (int32 Index = 0; StaticGameplayTags.IsValidIndex(Index); ++Index)
        {
            auto Tag = StaticGameplayTags.CreateConstIterator();
            for (int32 Skip = 0; Skip < Index; ++Skip) ++Tag;
            ASC->AddLooseGameplayTag(*Tag, 1);
        }
}
