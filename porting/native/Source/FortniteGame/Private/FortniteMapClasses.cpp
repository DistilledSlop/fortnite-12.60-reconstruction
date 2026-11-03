#include "FortniteMapClasses.h"
#include "UFortGameInstance.h"
#include "SharedFortGameLog.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

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

void UFortKismetLibrary::ExecuteClientEvent(UObject* WorldContextObject, FFortClientEventName EventType,
    UObject* EventSource, UObject* EventFocus)
{
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    UFortGameInstance* Instance = World ? Cast<UFortGameInstance>(World->GetGameInstance()) : nullptr;
    if (Instance)
        Instance->NativeExecuteClientEvent141e59c80(EventType, EventSource, EventFocus);
    else
        NativeLogClientEventWorldFailure141eece7a();
}
