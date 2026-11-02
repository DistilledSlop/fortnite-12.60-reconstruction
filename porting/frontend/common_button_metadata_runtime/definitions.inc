void UCommonButton::OnWidgetRebuilt()
{
    Super::OnWidgetRebuilt();
    const TSharedRef<FCommonButtonMetaData> MetaData = MakeShared<FCommonButtonMetaData>(this);
    GetCachedWidget()->AddMetadata(MetaData);
}
