// Proposed method bodies for integration with FMcpProfileItem and UMcpProfile.
// The callback represents UMcpProfile's native virtual slot +0x2a8.
#include "McpProfileItem.h"
#include "Dom/JsonObject.h"
#include <cmath>
#include <limits>

namespace
{
int32 ConvertProfileQuantity(double Number)
{
    const double Truncated = std::trunc(Number);
    if (!(Truncated >= -2147483648.0 && Truncated < 2147483648.0))
        return std::numeric_limits<int32>::min();
    return static_cast<int32>(Truncated);
}

TSharedPtr<FJsonValue> InternProfileJsonValue(const TSharedPtr<FJsonValue>& Value)
{
    // Native 0x1411100d0 requires a valid value; As* calls retain engine error logging.
    static const TSharedPtr<FJsonValue> EmptyString = MakeShared<FJsonValueString>(FString());
    static const TSharedPtr<FJsonValue> Zero = MakeShared<FJsonValueNumber>(0.0);
    static const TSharedPtr<FJsonValue> One = MakeShared<FJsonValueNumber>(1.0);
    static const TSharedPtr<FJsonValue> False = MakeShared<FJsonValueBoolean>(false);
    static const TSharedPtr<FJsonValue> True = MakeShared<FJsonValueBoolean>(true);
    static const TSharedPtr<FJsonValue> EmptyArray = MakeShared<FJsonValueArray>(TArray<TSharedPtr<FJsonValue>>());
    static const TSharedPtr<FJsonValue> EmptyObject = MakeShared<FJsonValueObject>(MakeShared<FJsonObject>());

    switch (Value->Type)
    {
    case EJson::String:
        if (Value->AsString().IsEmpty()) return EmptyString;
        break;
    case EJson::Number:
    {
        const double Number = Value->AsNumber();
        if (Number == 0.0) return Zero;
        if (Number == 1.0) return One;
        break;
    }
    case EJson::Boolean:
        return Value->AsBool() ? True : False;
    case EJson::Array:
        if (Value->AsArray().Num() == 0) return EmptyArray;
        break;
    case EJson::Object:
        if (Value->AsObject()->Values.Num() == 0) return EmptyObject;
        break;
    default:
        break;
    }
    return Value;
}
}

void UpdateProfileItemFromJson(FMcpProfileItem& Item, const FJsonObject& Json,
    int32 UpdateValue, UMcpProfile* Profile, TFunctionRef<void(FMcpProfileItem&)> NotifyProfile)
{
    Item.NativeUpdateValue = UpdateValue;
    Item.TemplateId = Json.GetStringField(TEXT("templateId"));
    Item.Quantity = ConvertProfileQuantity(Json.GetNumberField(TEXT("quantity")));

    const TSharedPtr<FJsonObject>& AttributesObject = Json.GetObjectField(TEXT("attributes"));
    TMap<FString, TSharedPtr<FJsonValue>> NewAttributes;
    NewAttributes.Reserve(AttributesObject->Values.Num());
    for (const auto& Entry : AttributesObject->Values)
    {
        NewAttributes.Add(Entry.Key, InternProfileJsonValue(Entry.Value));
    }
    Item.Attributes = MoveTemp(NewAttributes);
    Item.Profile = Profile;

    int32 Separator = INDEX_NONE;
    if (!Item.TemplateId.FindChar(TEXT('.'), Separator))
        Item.TemplateId.FindChar(TEXT(':'), Separator);
    Item.AlternateDefinitionKey = Separator == INDEX_NONE
        ? Item.TemplateId : Item.TemplateId.Left(Separator);
    NotifyProfile(Item);
}
