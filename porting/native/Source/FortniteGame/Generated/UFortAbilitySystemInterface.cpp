// Generated native ability interface default implementation.
#include "UFortAbilitySystemInterface.h"
#include "UFortAbilitySystemComponent.h"

UFortAbilitySystemComponent* IFortAbilitySystemInterface::NativeGetFortAbilitySystemComponent() const
{
    return Cast<UFortAbilitySystemComponent>(GetAbilitySystemComponent());
}
