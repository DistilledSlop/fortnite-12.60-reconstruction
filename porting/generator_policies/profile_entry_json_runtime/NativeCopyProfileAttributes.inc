// CONFIDENCE: HIGH
OutAttributes.Reset();
OutAttributes.Reserve(Object.Values.Num());
for (const auto& Attribute : Object.Values)
{
    OutAttributes.Add(Attribute.Key, NativeInternProfileJsonValue(Attribute.Value));
}
