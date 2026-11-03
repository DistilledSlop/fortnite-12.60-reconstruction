// CONFIDENCE: HIGH
Item.NativeUpdateValue = UpdateValue;
Item.TemplateId = Json.GetStringField(TEXT("templateId"));
const double Quantity = FMath::TruncToDouble(Json.GetNumberField(TEXT("quantity")));
Item.Quantity = Quantity >= -2147483648.0 && Quantity < 2147483648.0
    ? static_cast<int32>(Quantity) : MIN_int32;
{
    TMap<FString, TSharedPtr<FJsonValue>> NewAttributes;
    {
        TSharedPtr<FJsonObject> AttributesObject = Json.GetObjectField(TEXT("attributes"));
        NativeCopyProfileAttributes(NewAttributes, *AttributesObject);
    }
    Item.Attributes = MoveTemp(NewAttributes);
}
Item.Profile = Profile;
NativeUpdateItemTemplateKey(Item);
Profile->NotifyProfileItemUpdated(Item);
