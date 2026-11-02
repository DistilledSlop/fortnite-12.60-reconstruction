#include "CommonInputControllerBrushLookup.h"
#include "UCommonInputControllerData.h"
#include "FCommonInputPlatformData.h"

bool FindCommonInputControllerBrush(const UCommonInputControllerData& ControllerData,
    const TArray<FKey>& Keys, FSlateBrush& OutBrush)
{
    if (Keys.Num() == 1)
    {
        for (const FCommonInputKeyBrushConfiguration& Entry : ControllerData.InputBrushDataMap)
        {
            if (Entry.Key.GetFName() == Keys[0].GetFName())
            {
                OutBrush = Entry.KeyBrush;
                return true;
            }
        }
    }
    else if (Keys.Num() >= 2)
    {
        for (const FCommonInputKeySetBrushConfiguration& Entry : ControllerData.InputBrushKeySets)
        {
            if (Entry.Keys.Num() != Keys.Num())
            {
                continue;
            }
            bool bAllFound = true;
            for (const FKey& Key : Keys)
            {
                bool bFound = false;
                for (const FKey& Candidate : Entry.Keys)
                {
                    if (Candidate.GetFName() == Key.GetFName())
                    {
                        bFound = true;
                        break;
                    }
                }
                if (!bFound)
                {
                    bAllFound = false;
                    break;
                }
            }
            if (bAllFound)
            {
                OutBrush = Entry.KeyBrush;
                return true;
            }
        }
    }
    return false;
}

bool FindCommonInputPlatformBrush(const FCommonInputPlatformData& PlatformData,
    const TArray<FKey>& Keys, ECommonInputType InputType,
    ECommonGamepadType GamepadType, FSlateBrush& OutBrush)
{
    for (const TSubclassOf<UCommonInputControllerData>& ControllerClass : PlatformData.ControllerDataClasses)
    {
        UClass* Class = ControllerClass.Get();
        if (Class == nullptr)
        {
            continue;
        }
        const UCommonInputControllerData* ControllerData =
            static_cast<UCommonInputControllerData*>(Class->GetDefaultObject(true));
        if (ControllerData != nullptr && ControllerData->InputType == InputType &&
            (InputType != ECommonInputType::Gamepad || ControllerData->GamepadType == GamepadType))
        {
            return FindCommonInputControllerBrush(*ControllerData, Keys, OutBrush);
        }
    }
    return false;
}
