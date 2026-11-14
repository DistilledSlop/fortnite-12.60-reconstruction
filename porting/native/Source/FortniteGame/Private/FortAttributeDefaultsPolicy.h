#pragma once

#include "CoreMinimal.h"

namespace FortAttributeDefaultsPolicy
{
// Original cached FortGlobals BROnly policy, 0x141ed8a30. Used by the globals
// loader before invoking UAbilitySystemGlobals::InitAttributeDefaults.
bool IsBROnly();
}
