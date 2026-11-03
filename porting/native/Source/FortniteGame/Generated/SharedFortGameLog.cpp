#include "SharedFortGameLog.h"
DEFINE_LOG_CATEGORY(LogFort);
void NativeLogClientEventWorldFailure141eece7a()
{
    UE_LOG(LogFort, Error, TEXT("UFortKismetLibrary::ExecuteClientEvent: No WorldContext!"));
}
