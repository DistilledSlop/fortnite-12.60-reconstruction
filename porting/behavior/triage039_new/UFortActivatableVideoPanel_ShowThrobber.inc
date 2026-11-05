// CONFIDENCE: HIGH; captured body calls SetVisibility on both image fields.
if (Image_ThrobberBackground)
{
    Image_ThrobberBackground->SetVisibility(ESlateVisibility::Visible);
}
if (Image_Throbber)
{
    Image_Throbber->SetVisibility(ESlateVisibility::Visible);
}
