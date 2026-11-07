// Original body address: 0x144e7d240 (shared body with UFortRewardNewQuestWidget::IsValidConversation)
// Offset evidence: TEST RDX,RDX (null returns false). CMP dword ptr [RDX+0x38],0 is Sentences.Num() (TArray Sentences at 0x30, Num at +8); JLE returns false.
// CONFIDENCE: HIGH
if (Conversation == nullptr) { return false; }
return Conversation->Sentences.Num() > 0;
