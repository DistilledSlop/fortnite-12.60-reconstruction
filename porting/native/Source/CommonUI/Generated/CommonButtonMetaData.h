#pragma once
#include "CoreMinimal.h"
#include "Types/ISlateMetaData.h"

class UCommonButton;

class COMMONUI_API FCommonButtonMetaData : public ISlateMetaData
{
public:
    SLATE_METADATA_TYPE(FCommonButtonMetaData, ISlateMetaData)
    explicit FCommonButtonMetaData(UCommonButton* InButton);
    TWeakObjectPtr<UCommonButton> Button;
};
