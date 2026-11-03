// CONFIDENCE: high
// Reflected native wrapper 0x14080fd90.
for (int32 Index = 0; Index < NativeButtonGroupWidgets.Num(); ++Index)
{
    if (NativeButtonGroupWidgets[Index].Get() == ButtonToFind)
    {
        return Index;
    }
}
return INDEX_NONE;
