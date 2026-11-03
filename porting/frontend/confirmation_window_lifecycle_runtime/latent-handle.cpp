#include "FFortDialogExternalLatentActionHandle.h"
#include "CoreMinimal.h"

// One process-wide counter; every port of original 141f87900 must use this
// function, including factories which do not pass through the window.
void NativeAllocateDialogLatentHandle141f87900(FFortDialogExternalLatentActionHandle& Handle)
{
    static int32 LastHandle = INDEX_NONE;
    LastHandle = LastHandle == MAX_int32 ? 0 : LastHandle + 1;
    Handle.Handle = LastHandle;
}
