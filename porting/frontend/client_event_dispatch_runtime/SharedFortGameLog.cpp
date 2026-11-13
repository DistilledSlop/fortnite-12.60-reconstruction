#include "SharedFortGameLog.h"
DEFINE_LOG_CATEGORY(LogFort);
// Native category registration1402d0320, default Warning and compile-time All.
DEFINE_LOG_CATEGORY(LogFortWind);
// Native initializer1402cf880: default Warning, compile-time All.
DEFINE_LOG_CATEGORY(LogFortGameDifficulty);
// Native initializer 0x1402d0370: LogFortWorld, default Log, compile-time All.
DEFINE_LOG_CATEGORY(LogFortWorld);
void NativeLogClientEventWorldFailure141eece7a()
{
    UE_LOG(LogFort, Error, TEXT("UFortKismetLibrary::ExecuteClientEvent: No WorldContext!"));
}
