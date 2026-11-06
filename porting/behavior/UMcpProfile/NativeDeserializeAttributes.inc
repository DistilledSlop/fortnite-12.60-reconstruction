// CONFIDENCE: HIGH
TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
for (const auto& Attribute : Attributes)
{
    Object->Values.Add(Attribute.Key, Attribute.Value);
}
return FJsonObjectConverter::JsonAttributesToUStruct(Object->Values, StructDefinition, OutStruct, 0, 0);
