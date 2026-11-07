// Behaviour snippet: AFortAIPawn::SetCanInteract (body only; spliced into the generated member function).
// Original address: 0x1422ec1f0 (thunk 0x1427a5ad0)
// Offset evidence: dword at object+0x18dc bit 28 (mask 0x10000000) is the declared bitfield bCanInteract (AFortAIPawn.h line 535; bool_bits byte 0x18df mask 0x10). Byte at object+0xf0 is the inherited AActor Role (raw access; no generated AActor header), compared with 3 (ROLE_Authority).
// Virtual slot +0xe88 is OnRep_CanInteract(bool bOldCanInteract) (AFortAIPawn.h line 853). The asm passes the pre-store bit (R8 = old>>28, then AND 1), which matches the parameter name bOldCanInteract.
// CONFIDENCE: HIGH
const bool bOldCanInteract = bCanInteract;
if (bOldCanInteract == CanInteract)
{
    return;
}
bCanInteract = CanInteract;
if (*reinterpret_cast<const uint8*>(reinterpret_cast<const uint8*>(this) + 0xf0) == 3)
{
    OnRep_CanInteract(bOldCanInteract);
}
