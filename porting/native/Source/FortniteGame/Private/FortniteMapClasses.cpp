#include "FortniteMapClasses.h"

void AFortSkydivingShadowProxy::ShowShadowProxy_Implementation() {}
void AFortSkydivingShadowProxy::HideShadowProxy_Implementation() {}

AFortSkydivingShadowProxy::AFortSkydivingShadowProxy()
{
    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = DefaultSceneRoot;
}

bool UFortKismetLibrary::IsMobilePlatform()
{
#if PLATFORM_ANDROID || PLATFORM_IOS
    return true;
#else
    return false;
#endif
}

bool UFortKismetLibrary::IsMobileGame(const UObject* WorldContextObject)
{
    return false;
}
