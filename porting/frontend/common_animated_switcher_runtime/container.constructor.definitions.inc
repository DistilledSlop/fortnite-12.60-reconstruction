// Original container constructor 0x140796700.
UCommonActivatableWidgetContainerBase::UCommonActivatableWidgetContainerBase(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , GeneratedWidgetsPool(*this)
{
    TransitionDuration = 0.4f;
    Visibility = ESlateVisibility::Collapsed;
}
