#pragma once

#include "GameFramework/OnlineReplStructs.h"

class UFortGameInstance;
class UFortRegisteredPlayerInfo;

// Native lookup 0x141e62c80. The caller supplies the team member's resolved ID.
UFortRegisteredPlayerInfo* FindToastRegisteredPlayer(
    const UFortGameInstance* GameInstance, const FUniqueNetIdRepl& PlayerId);

class ALobbyBeaconPlayerState;
class APlayerState;
class UFortSocialUser;

// Native campaign team-member identity precedence at 0x142292160.
FUniqueNetIdRepl ResolveToastTeamMemberId(
    TWeakObjectPtr<ALobbyBeaconPlayerState> LobbyPlayer,
    UFortSocialUser* SocialUser,
    TWeakObjectPtr<APlayerState> PlayerState);
