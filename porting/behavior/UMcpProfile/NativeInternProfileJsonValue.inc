// CONFIDENCE: HIGH
const FMcpProfileJsonInternPool& Pool = NativeGetProfileJsonInternPool();
switch (Value->Type)
{
case EJson::String:
    if (Value->AsString().IsEmpty()) return Pool.EmptyString;
    break;
case EJson::Number:
{
    const double Number = Value->AsNumber();
    if (Number == 0.0) return Pool.Zero;
    if (Number == 1.0) return Pool.One;
    break;
}
case EJson::Boolean:
    return Value->AsBool() ? Pool.True : Pool.False;
case EJson::Array:
    if (Value->AsArray().Num() == 0) return Pool.EmptyArray;
    break;
case EJson::Object:
    if (Value->AsObject()->Values.Num() == 0) return Pool.EmptyObject;
    break;
default:
    break;
}
return Value;
