// Behaviour snippet: UFortStoreFrontOfferDetailsWidgetBase::GetOfferInfoById (body only; spliced into the generated member function).
// Original address: 0x1450b0ab0 (via exec wrapper 0x1452f1f30)
// Source: local-model fn runner; static checks only, not compiled or runtime verified.
// CONFIDENCE: HIGH
for (UFortStoreFrontOfferInfo* Item : PagedItems)
{
	if (Item != nullptr && Item->GetOfferId() == OfferId)
	{
		return Item;
	}
}
if (OfferData != nullptr && OfferData->GetOfferId() == OfferId)
{
	return OfferData;
}
return nullptr;
