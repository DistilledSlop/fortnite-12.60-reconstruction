// Behaviour snippet: UFortCollectionBookProgressionRewardDetailInspectWidget::SetAssociatedLevel (wrapper, body only).
// Original body address: 0x1450cb690 (thunk 0x14520d810), this_delta_total 0.
// Offset evidence: MOV RCX,[RCX+0x258] loads LevelTextWidget (reflected Object, exact offset 0x258; declared UCommonNumericTextBlock* in FortniteUI/Generated/UFortCollectionBookProgressionRewardDetailInspectWidget.h).
// Callee: 0x1407f6320 -> UCommonNumericTextBlock::SetCurrentValue(const float NewValue) (CommonUI engine declaration, CommonNumericTextBlock.h); XMM1 = float(Level + 1) (LEA EAX,[RDX+1]; CVTDQ2PS); JMP is a tail call.
// Null test on LevelTextWidget skips the call and returns (JZ to RET).
// CONFIDENCE: MEDIUM
if (LevelTextWidget != nullptr)
{
    LevelTextWidget->SetCurrentValue(static_cast<float>(Level + 1));
}
