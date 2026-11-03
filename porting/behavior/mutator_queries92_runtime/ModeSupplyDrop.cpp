// Typed behavioral reconstruction; integration requires the bindings below and
// FRecoveredMutatorIterator from ../infinite_ammo_iterator_runtime/Proposal.cpp.in.
// Names are descriptive, not recovered native symbols.
struct FSupplyDropRow;
struct FSupplyDropBindings
{
    // Read game mode +0x278, then native IsA(AFortGameStateAthena); no extra
    // validity checks. Read the phase as the original byte at state +0x1cb0.
    static AFortGameStateAthena* AthenaState(AFortGameModeAthena* Mode);
    static uint8 StatePhaseByte(AFortGameStateAthena* State);
    // UObject +0x10 is its UClass pointer; the caller forwards the drop itself.
    static UClass* SelectorKeyAt10(const AFortAthenaSupplyDrop* Data) { return Data->GetClass(); }
    // 0x1419498c0: unresolved row selection and lifetime; retain its effects.
    static const FSupplyDropRow* SelectRow(AFortGameModeAthena* Mode, uint8 Phase, UClass* Key);
    // 0x1406f73b0: scalable value at the given row byte offset, level 0,
    // context null. Returns signed int32; conversion to float follows the call.
    static int32 RowValue(const FSupplyDropRow* Row, uint32 Offset);
    // Virtual +0x780; exact object/enum names not yet recovered. Preserve pointer
    // identity, aliased outputs, dynamic dispatch, writes and low-byte result.
    static uint8 OverrideSpeed(AFortAthenaMutator* Mutator, float* OutFirst,
                               float* OutSecond, const AFortAthenaSupplyDrop* Data, uint8 Phase);
};

void RecoveredSupplyDropSpawnRange(AFortGameModeAthena* Mode, const AFortAthenaSupplyDrop* Data,
                                   float* OutFirst, float* OutSecond)
{
    if (AFortGameStateAthena* State = FSupplyDropBindings::AthenaState(Mode))
    {
        const uint8 Phase = FSupplyDropBindings::StatePhaseByte(State);
        const FSupplyDropRow* Row = FSupplyDropBindings::SelectRow(
            Mode, Phase, FSupplyDropBindings::SelectorKeyAt10(Data));
        if (Row)
        {
            *OutFirst = static_cast<float>(FSupplyDropBindings::RowValue(Row, 0xc8));
            *OutSecond = static_cast<float>(FSupplyDropBindings::RowValue(Row, 0xe8));
            return;
        }
    }
    *OutFirst = 28000.0f;
    *OutSecond = 32000.0f;
}

void RecoveredSupplyDropSpeedRange(AFortGameModeAthena* Mode, const AFortAthenaSupplyDrop* Data,
                                   float* OutFirst, float* OutSecond)
{
    if (AFortGameStateAthena* State = FSupplyDropBindings::AthenaState(Mode))
    {
        const uint8 Phase = FSupplyDropBindings::StatePhaseByte(State);
        {
            FRecoveredMutatorIterator It(nullptr, Mode, true);
            while (AFortAthenaMutator* Mutator = It.Get())
            {
                if (FSupplyDropBindings::OverrideSpeed(Mutator, OutFirst, OutSecond, Data, Phase))
                    return; // iterator destruction precedes return
                It.Advance();
            }
        } // destruction, including its virtual context clear, precedes selection
        const FSupplyDropRow* Row = FSupplyDropBindings::SelectRow(
            Mode, Phase, FSupplyDropBindings::SelectorKeyAt10(Data));
        if (Row)
        {
            *OutFirst = static_cast<float>(FSupplyDropBindings::RowValue(Row, 0x108));
            *OutSecond = static_cast<float>(FSupplyDropBindings::RowValue(Row, 0x128));
            return;
        }
    }
    *OutFirst = 280.0f;
    *OutSecond = 500.0f;
}
