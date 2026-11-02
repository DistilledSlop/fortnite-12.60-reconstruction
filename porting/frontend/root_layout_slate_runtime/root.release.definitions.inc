void UFortRootViewportLayout::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	NativeRootBox2e8.Reset();
	NativePresentationWidget2d8.Reset();
	NativePresentationBox2b8.Reset();
	NativePresentationScale2c8.Reset();
}
