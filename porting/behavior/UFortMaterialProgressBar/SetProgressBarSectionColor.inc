// CONFIDENCE: high; section bounds, gradient guard, stored color, and MID parameter guard match native.
{
    const int32 SectionIndex = static_cast<int32>(BarSection);
    if (SectionIndex >= 0 && SectionIndex < 4)
    {
        FFortMaterialProgressBarSectionStyle& SectionStyle = BarSectionInfo[SectionIndex].SectionStyle;
        FName ParameterName;
        if (static_cast<uint8>(ColorNumber) == 0)
        {
            SectionStyle.BarColorOne = Color;
            ParameterName = SectionStyle.BarColorOneParamName;
        }
        else
        {
            if (!SectionStyle.bGradientBar)
            {
                return;
            }
            SectionStyle.BarColorTwo = Color;
            ParameterName = SectionStyle.BarColorTwoParamName;
        }
        if (ProgressBarMID && !ParameterName.IsNone())
        {
            ProgressBarMID->SetVectorParameterValue(ParameterName, Color);
        }
    }
}
