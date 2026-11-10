// dep AFortGameplayEffectDeliveryActor__AddAdditionalExplodeEffectContainerInternal original 0x14268eb50
// CONFIDENCE: MEDIUM. Original 0x14268eb50. Empty containers are ignored; otherwise the container is appended to the additional-container list (element stride 0xb8).
if (ExplodeContainer.Entries0.Num() != 0 || ExplodeContainer.Entries1.Num() != 0 || ExplodeContainer.Entries2.Num() != 0)
{
	AdditionalExplodeEffectContainers.Add(ExplodeContainer);   // count +0x3c0, capacity +0x3c4, data +0x3b8; grow 0x140a671f0, copy 0x141614350
}
