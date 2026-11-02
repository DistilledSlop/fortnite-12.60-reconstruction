// Candidate only. Register only with the complete production Initialize/lifecycle.
// Original receiver 0x1407f9c70; NativeRebuildStyle is 0x1407acd30.
void UCommonButton::SetStyle(TSubclassOf<UCommonButtonStyle> InStyle)
{
    UClass* Incoming = InStyle.Get();
    if (!Incoming) return;

    // TSubclassOf::Get validates ancestry; the old class can be invalid even
    // when its raw stored pointer is non-null. The original checks both again.
    if (Style.Get() == InStyle.Get()) return;
    Style = Incoming;
    NativeRebuildStyle();
}
