// Generated from porting/generated/reflection/fortnite_game_reflection.json; native ability_interface302_runtime policy.
#pragma once
#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
class UFortAbilitySystemComponent;
#include "UFortAbilitySystemInterface.generated.h"

UINTERFACE(meta=(CannotImplementInterfaceInBlueprint))
class FORTNITEGAME_API UFortAbilitySystemInterface : public UAbilitySystemInterface
{
    GENERATED_BODY()
};

class FORTNITEGAME_API IFortAbilitySystemInterface : public IAbilitySystemInterface
{
    GENERATED_IINTERFACE_BODY()
public:
    // Captured slot +0x20 (0x14084be30): invoke the inherited getter and cast.
    virtual UFortAbilitySystemComponent* NativeGetFortAbilitySystemComponent() const;
};
