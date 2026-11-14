// CONFIDENCE: HIGH. Original1423cb580; spectating permits ADS before the base gate.
if (ActionTag.MatchesTag(FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x768)))
{
    return true;
}
return AFortPlayerController::NativeCanPerformAction15f0(ActionTag);
