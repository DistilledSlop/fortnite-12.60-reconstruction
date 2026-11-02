#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ActionBindingDataProbe.generated.h"

// Controlled delegate receiver, not an original reflected game type.
UCLASS(Transient)
class UActionBindingDataProbeReceiver : public UObject
{
    GENERATED_BODY()
public:
    void Click();
    void Progress(float Value);
};

void RegisterActionBindingDataProbeDriver();
