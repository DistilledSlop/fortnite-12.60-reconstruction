#include "FortniteMapClasses.h"

AFortWorldManager::AFortWorldManager()
{
    MySceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("MySceneComponent"));
    RootComponent = MySceneComponent;
    WorldTileManager = CreateDefaultSubobject<UWorldTileManager>(TEXT("WorldTileManager_0"));
}
