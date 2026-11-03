#include "ToastRegisteredPlayerLookup.h"

#include "UFortGameInstance.h"
#include "UFortRegisteredPlayerInfo.h"

UFortRegisteredPlayerInfo* FindToastRegisteredPlayer(
    const UFortGameInstance* GameInstance, const FUniqueNetIdRepl& PlayerId)
{
    if (!GameInstance || !PlayerId.IsValid())
    {
        return nullptr;
    }
    for (UFortRegisteredPlayerInfo* Player : GameInstance->RegisteredPlayers)
    {
        if (IsValid(Player) && Player->PlayerId == PlayerId)
        {
            return Player;
        }
    }
    return nullptr;
}

#include "LobbyBeaconPlayerState.h"
#include "GameFramework/PlayerState.h"
#include "UFortSocialUser.h"

FUniqueNetIdRepl ResolveToastTeamMemberId(
    TWeakObjectPtr<ALobbyBeaconPlayerState> LobbyPlayer,
    UFortSocialUser* SocialUser,
    TWeakObjectPtr<APlayerState> PlayerState)
{
    if (ALobbyBeaconPlayerState* Lobby = LobbyPlayer.Get())
    {
        return Lobby->UniqueId;
    }
    if (SocialUser)
    {
        return SocialUser->GetUserId(ESocialSubsystem::Primary);
    }
    if (APlayerState* Player = PlayerState.Get())
    {
        return Player->GetUniqueId();
    }
    return FUniqueNetIdRepl();
}
