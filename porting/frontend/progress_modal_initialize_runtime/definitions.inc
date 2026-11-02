void UFortProgressModal::InitializeModal()
{
    FText InitialTitle = Title;
    FText InitialDescription = Description;
    FSlateBrush InitialIcon = Icon;
    UFortProgressWidget* Widget = ProgressWidget_Main;
    Widget->Text_Title->SetText(InitialTitle);
    Widget->Text_Description->SetText(InitialDescription);
    Widget->Image_MainIcon->SetBrush(InitialIcon);
    Widget->Image_MainIcon->SetVisibility(IsValid(InitialIcon.GetResourceObject())
        ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}
