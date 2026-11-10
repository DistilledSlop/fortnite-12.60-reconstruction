// CONFIDENCE: HIGH - exec 0x14277cfb0 calls body 0x141da9b90. GetWorld is the class virtual (slot 0x150 -> 0x143f793a0).
// Original addresses: 0x14277cfb0 (exec thunk) ; 0x141da9b90 (direct body)
UWorld* World = GetWorld();
if (World == nullptr || World->GameState == nullptr)
{
    return false;
}
return World->GameState->IsA(AFortGameStateFrontEnd::StaticClass());
