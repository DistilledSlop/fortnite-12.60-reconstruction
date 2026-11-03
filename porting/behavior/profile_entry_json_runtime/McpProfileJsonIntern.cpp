#include "McpProfileJsonIntern.h"
#include "Dom/JsonObject.h"

namespace
{
// Original process-wide shared values; preserve identity across all entry updates.
const FMcpProfileJsonInternPool Pool{
    MakeShared<FJsonValueArray>(TArray<TSharedPtr<FJsonValue>>()),
    MakeShared<FJsonValueObject>(MakeShared<FJsonObject>()),
    MakeShared<FJsonValueString>(FString()),
    MakeShared<FJsonValueNumber>(0.0),
    MakeShared<FJsonValueNumber>(1.0),
    MakeShared<FJsonValueBoolean>(true),
    MakeShared<FJsonValueBoolean>(false)
};
}

const FMcpProfileJsonInternPool& NativeGetProfileJsonInternPool()
{
    return Pool;
}
