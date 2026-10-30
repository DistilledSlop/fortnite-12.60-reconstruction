#include "FortniteMapClasses.h"

AFortWorldManager::AFortWorldManager()
{
    MySceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("MySceneComponent"));
    RootComponent = MySceneComponent;
    WorldTileManager = CreateDefaultSubobject<UWorldTileManager>(TEXT("WorldTileManager_0"));
}

void AFortSkydivingShadowProxy::ShowShadowProxy_Implementation() {}
void AFortSkydivingShadowProxy::HideShadowProxy_Implementation() {}

AFortSkydivingShadowProxy::AFortSkydivingShadowProxy()
{
    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = DefaultSceneRoot;
}

AFortAthenaMapInfo::AFortAthenaMapInfo()
{
    RootComponent0 = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent0"));
    RootComponent = RootComponent0;
}

bool UFortKismetLibrary::IsMobilePlatform()
{
#if PLATFORM_ANDROID || PLATFORM_IOS
    return true;
#else
    return false;
#endif
}
