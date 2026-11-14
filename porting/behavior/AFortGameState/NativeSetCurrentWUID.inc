// CONFIDENCE: HIGH
// Original 0x141eb1650.
CurrentWUID = InWUID;
if (GetNetMode() < NM_Client)
{
    OnRep_CurrentWUID();
}
