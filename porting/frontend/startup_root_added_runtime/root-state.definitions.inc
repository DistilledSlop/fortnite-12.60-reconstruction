// Base virtual490=1451679d0. State is intentionally unused by the base body.
void UFortRootViewportLayout::NativeStateChanged490(const FGameplayTag& State)
{
	if (UFortGlobalUIContext* Context = NativeGetGlobalUIContext144ade110(this))
	{
		if (Context->NativePredicate1406801d0())
		{
			ProgressModal_ControllerDisconnected->SetVisibility(ESlateVisibility::Collapsed);
			ProgressModal_ControllerDisconnected->DeactivateWidget();
		}
		else
		{
			ProgressModal_ControllerDisconnected->SetVisibility(ESlateVisibility::Visible);
			ProgressModal_ControllerDisconnected->ActivateWidget();
		}
	}
}
