// Behaviour snippet: UFortGameStateComponent_EventLevel::GetLastNavObject (body only; spliced into the generated member function).
// Original address: 0x141a799c0 (thunk 0x142924460). Asm: porting/generated/native108-impl/0x141a799c0.asm
// Offset evidence: ClickableActors = TArray<TWeakObjectPtr<AFortEventLevelNavigationActor>> at +0x218 (Num at +0x220, element stride 8).
// Offset evidence: LastSelectedNavObj = FGameplayTag at +0x298 (8-byte key); compared against AFortEventLevelNavigationActor::NavigationActorTag (FGameplayTag, +0x2a0) lo/hi dwords at +0x2a0/+0x2a4.
// Weak pointer helpers: 0x142ee7880 = IsValid, 0x142ee5dd0 = Get (known helpers).
// CONFIDENCE: HIGH
for (int32 Index = 0; Index < ClickableActors.Num(); ++Index)
{
    const TWeakObjectPtr<AFortEventLevelNavigationActor> Candidate = ClickableActors[Index];
    if (Candidate.IsValid())
    {
        AFortEventLevelNavigationActor* NavObj = Candidate.Get();
        if (NavObj->NavigationActorTag == LastSelectedNavObj)
        {
            return NavObj;
        }
    }
}
return nullptr;
