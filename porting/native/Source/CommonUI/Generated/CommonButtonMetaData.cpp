#include "CommonButtonMetaData.h"
#include "UCommonButton.h"

FCommonButtonMetaData::FCommonButtonMetaData(UCommonButton* InButton) : Button(InButton)
{
    static_assert(sizeof(FCommonButtonMetaData) == 0x10, "Original metadata vptr and weak UObject");
}
