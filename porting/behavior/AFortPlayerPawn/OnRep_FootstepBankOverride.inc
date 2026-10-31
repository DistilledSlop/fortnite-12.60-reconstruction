// Behaviour snippet: AFortPlayerPawn::OnRep_FootstepBankOverride (body only; spliced into the generated member function).
// Original address: 0x142a7dd50
// Decompile source: porting/generated/decomp/AFortPlayerPawn/OnRep_FootstepBankOverride.c
// Proxy score: 70.3 (portability.json)
// Inferred: Members: FootstepBankOverride at +0x1c50, OriginalFootstepBank at +0x1c58 (both AFortPlayerPawn), FootstepBank at +0x998 (AFortPawn). Names come from reflection offsets; the three fields are pointers (8 bytes).
// Declared in generated header: yes
// CONFIDENCE: MEDIUM
UFortFootstepAudioBank* Override = FootstepBankOverride;
UFortFootstepAudioBank* Original = OriginalFootstepBank;
if (Override == nullptr)
{
    if (Original != nullptr)
    {
        FootstepBank = Original;
        OriginalFootstepBank = nullptr;
    }
}
else
{
    if (Original == nullptr)
    {
        OriginalFootstepBank = FootstepBank;
    }
    if (FootstepBank != Override)
    {
        FootstepBank = Override;
    }
}
