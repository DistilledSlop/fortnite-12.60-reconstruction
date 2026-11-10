// CONFIDENCE: MEDIUM - the touch-input sub-object lookup (0x14081b480) is fully read; the world flag check and the class used for Player casting are read from the decompile without names.
// Original address: 0x142a48520 (exec thunk, bool return). Helper: 0x1423909c0. Player cast class: 0x144786700 (unnamed). Sub-object lookup: 0x14081b480.
// Only evaluated when the controller is not the authority (Role != ROLE_Authority). Touch input is reported when the touch sub-object's input type byte (+0x79) equals 2.
if (Role == ENetRole::ROLE_Authority)
{
	return false;
}
UWorld* World = GetWorld();
if (World != nullptr && (World->UnknownFlags_11D & 0x20) != 0)
{
	return false;
}
UPlayer* PlayerObj = Cast<UUnnamed_144786700>(Player);
if (PlayerObj == nullptr)
{
	return false;
}
UTouchInputSubobject* Touch = FindSubobjectOfClass_0x14081b480(PlayerObj);
if (Touch == nullptr)
{
	return false;
}
return Touch->InputType_0x79 == 2;
