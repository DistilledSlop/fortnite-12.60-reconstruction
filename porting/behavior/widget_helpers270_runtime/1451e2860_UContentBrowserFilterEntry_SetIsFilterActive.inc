// CONFIDENCE: high; native implementation 0x144cacab0, event dispatch 0x1451e0ea0, delegate dispatch 0x1406a9dc0.
bFilterActive = bInFilterActive;
struct FUpdateCheckMarkStateParameters
{
    bool bIsChecked;
};
FUpdateCheckMarkStateParameters Parameters = {bInFilterActive};
ProcessEvent(FindFunctionChecked(TEXT("UpdateCheckMarkState")), &Parameters);
UContentBrowserTag* CurrentTag = Tag;
OnFilterEnabled.Broadcast(bFilterActive, CurrentTag);
