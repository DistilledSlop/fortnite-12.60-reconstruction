// CONFIDENCE: LOW for member and engine-slot names; control flow is HIGH. Revives a pawn in DBNO state: clears DBNO, records the revive for the defender class, notifies controllers, removes the DBNO effects and sends the revive gameplay event.
// Original address: 0x1427a5800 (exec). Native targets: SetDBNO slot 0xa50 -> 0x14234d8e0; GetWorld slot 0x150 -> 0x143f793a0; embedded tag query slot 0x18 -> 0x14232c9f0; attribute refresh slot 0xcf8 -> 0x1422ea200; ASC HandleGameplayEvent slot 0x620 -> 0x140711e80.
if ((ProtectedFlags53A & 0x02) == 0)
{
    if (ReviveLogVerbosity > 2)
    {
        UE_LOG(LogFortAI, Warning, TEXT("ReviveFromDBNO called on pawn that is NOT DBNO!  Pawn: %s"), *GetName());
    }
    return;
}

if (IsPendingKill())
{
    if (ReviveLogVerbosity > 2)
    {
        UE_LOG(LogFortAI, Warning, TEXT("ReviveFromDBNO called on pawn that is already pending kill!  Cannot revive.  Pawn: %s"), *GetName());
    }
    return;
}

if (DBNOInteractionType != 0)
{
    AIFlags18DC &= ~0x10000000;
}

SetDBNO(false); // vtable slot 0xa50 with 0 (0x14234d8e0)
AIFlags1344 &= ~0x04;

const FGameplayTag ReviveCheckTag = GetFortGlobals()->DBNOReviveCheckTag; // FFortGlobals+0xea8
if (DBNOTagQuery.QueryTag(ReviveCheckTag)) // embedded object at +0x4b0, slot 0x18 (0x14232c9f0)
{
    // Record the revive against the defender class in the DBNO stats table.
    UWorld* World = GetWorld();
    UObject* DBNOStatsSubsystem = GetDBNOStatsSubsystem(World);   // 0x141f88460
    UDBNOStatsTable* StatsTable = DBNOStatsSubsystem ? GetDBNOStatsTable(DBNOStatsSubsystem) : nullptr; // 0x141f887c0
    if (StatsTable)
    {
        FName DefenderKey;
        bool bHaveDefenderName = false;
        if (DefenderItem)
        {
            if (UObject* DefenderDefinition = DefenderItem->GetDefinitionObject()) // slot 0x3e8 (1000)
            {
                DefenderKey = DefenderDefinition->GetFName();
                bHaveDefenderName = true;
            }
        }
        if (!bHaveDefenderName || DefenderKey.IsNone())
        {
            DefenderKey = ClassPrivate->GetFName(); // fallback: this pawn's class name
        }
        StatsTable->IncrementReviveCount(DefenderKey); // 0x141bcc800 (FindOrAdd), count at +8
    }
}

RefreshDBNOAttributeStats(); // vtable slot 0xcf8 -> 0x1422ea200
NotifyControllersOfRevive(this); // 0x1416954e0

AFortAIController* Controller = Cast<AFortAIController>(this->Controller); // +0x250, class 0x142792260
if (Controller)
{
    if (Controller->ControllerComponentA != nullptr) // +0x2d8
    {
        UpdateControllerFlags(Controller); // 0x144a007f0
    }
    UObject* ControllerComponent = GetControllerComponent(Controller); // 0x141702e20
    if (ControllerComponent)
    {
        ControllerComponent->Release(); // slot 0x1000 (1000)
    }
    ClearControllerReviveTasks(Controller); // 0x1416578f0
}

if (AbilitySystemComponent != nullptr) // +0xbb8 (param_1[0x177])
{
    FGameplayTagContainer RemoveTags;
    RemoveTags.AddTag(GetFortGlobals()->DBNORemoveEffectsTag); // FFortGlobals+0x780 (0x143e8b3b0)
    AbilitySystemComponent->RemoveActiveEffectsWithTags(RemoveTags, nullptr, 0); // 0x1406fd020

    FGameplayEventData EventData;
    EventData.Instigator = this;
    EventData.EventMagnitude = 1.0f;
    AbilitySystemComponent->HandleGameplayEvent(GetFortGlobals()->DBNOReviveEventTag, &EventData); // slot 0x620 -> 0x140711e80
}
