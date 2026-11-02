#pragma once
#include "UCommonTextBlock.h"
#include "Widgets/Text/STextBlock.h"
#include "CommonTextTransformProbe.generated.h"

UCLASS()
class UTextTransformProbeBlock : public UCommonTextBlock
{
    GENERATED_BODY()
public:
    int32 SyncCount = 0;
    virtual void SynchronizeProperties() override { ++SyncCount; Super::SynchronizeProperties(); }
    bool HasSlate() const { return MyTextBlock.IsValid(); }
    FText SlateText() const { return MyTextBlock.IsValid() ? MyTextBlock->GetText() : FText::GetEmpty(); }
    FVector2D SlateSize() const { MyTextBlock->SlatePrepass(1); return MyTextBlock->GetDesiredSize(); }
};
void RegisterCommonTextTransformProbeDriver();
