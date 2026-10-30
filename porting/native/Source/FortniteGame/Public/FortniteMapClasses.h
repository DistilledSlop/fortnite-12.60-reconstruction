#pragma once

#include "Engine/LevelScriptActor.h"
#include "GameFramework/WorldSettings.h"
#include "NavAreas/NavAreaMeta.h"
#include "FortniteMapClasses.generated.h"

// Initial reflection-compatible bases for loading a recovered map. Gameplay
// behavior and serialized Fortnite properties still need reconstruction.
UCLASS()
class FORTNITEGAME_API AFortLevelScriptActor : public ALevelScriptActor
{
    GENERATED_BODY()
};

UCLASS()
class FORTNITEGAME_API AFortWorldSettings : public AWorldSettings
{
    GENERATED_BODY()
};

UCLASS()
class FORTNITEGAME_API UFortMetaNavArea_Obstacles : public UNavAreaMeta
{
    GENERATED_BODY()
};
