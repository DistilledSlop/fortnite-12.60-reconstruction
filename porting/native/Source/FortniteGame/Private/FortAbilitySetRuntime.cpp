#include "FortAbilitySetRuntime.h"
#include "FFortAbilitySetHandle.h"
#include "AbilitySystemComponent.h"

namespace FortAbilitySetRuntime
{
void RemoveAbilitySet(FFortAbilitySetHandle& Handle)
{
    // Native142ee7880 validity gate precedes BOTH side effects and cleanup.
    // Stale/null target handles are deliberately left intact by141611160.
    if (!Handle.TargetAbilitySystemComponent.IsValid())
        return;

    if (UAbilitySystemComponent* ASC = Handle.TargetAbilitySystemComponent.Get())
    {
        // Native order: every ability first, then every effect, all stacks.
        for (const FGameplayAbilitySpecHandle& Ability : Handle.GrantedAbilityHandles)
            ASC->ClearAbility(Ability);
        for (FActiveGameplayEffectHandle Effect : Handle.AppliedEffectHandles)
            ASC->RemoveActiveGameplayEffect(Effect, -1);
    }

    Handle.TargetAbilitySystemComponent.Reset();
    Handle.GrantedAbilityHandles.Empty();
    Handle.AppliedEffectHandles.Empty();
}
}
