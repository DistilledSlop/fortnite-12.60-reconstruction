#pragma once
#include "CoreMinimal.h"
#include "UCommonButton.h"
#include "MainButtonStateProbe.generated.h"

// Controlled query fixture; these writes do not test the original state producers.
UCLASS(Transient, NotBlueprintable)
class UMainButtonStateProbeWidget : public UCommonButton
{
    GENERATED_BODY()
public:
    bool NativeDefaults() const { return bNativeButtonEnabled && bNativeInteractionEnabled; }
    void Configure(ESlateVisibility Visibility, bool WidgetEnabled, bool NativeEnabled, bool InteractionEnabled)
    {
        UWidget::SetIsEnabled(WidgetEnabled);
        SetVisibility(Visibility);
        bNativeButtonEnabled = NativeEnabled;
        bNativeInteractionEnabled = InteractionEnabled;
    }
};
void RegisterMainButtonStateProbeDriver();
