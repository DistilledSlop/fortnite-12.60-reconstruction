// CONFIDENCE: high; the native body updates the stored color only when the MID exists.
{
    if (ProgressBarMID)
    {
        BackgroundColor = Color;
        if (!BackgroundColorParamName.IsNone())
        {
            ProgressBarMID->SetVectorParameterValue(BackgroundColorParamName, Color);
        }
    }
}
