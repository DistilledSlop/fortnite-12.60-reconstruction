#pragma once

#include "CoreMinimal.h"

// The empty-input default requires context that is not part of this lookup.
enum class ELegacyMcpDeploymentResult : uint8
{
    RequiresDefault,
    Resolved,
    Unknown
};

struct FLegacyMcpDeploymentLookup
{
    ELegacyMcpDeploymentResult Result;
    FString SecondArgument;
    FString ThirdArgument;
};

// Returns independent output values; Input may be taken from an earlier result.
// RequiresDefault and Unknown both return empty strings, but remain distinct.
FLegacyMcpDeploymentLookup LookupLegacyMcpDeployment(const FString& Input);

// Supplies the original external calls. GetBranchAfterPlusToSlash must return
// the singleton branch after replacing '+' with '/', before any searches.
// ModeValue can be called twice, with the same non-null object.
struct ILegacyMcpDefaultContext
{
    virtual ~ILegacyMcpDefaultContext() = default;
    virtual void* ModeObject() = 0;
    virtual int32 ModeValue(void* Object) = 0;
    virtual FString GetBranchAfterPlusToSlash() = 0;
    virtual bool TencentFlag() = 0;
};

enum class ELegacyMcpDefaultDiagnostic : uint8
{
    Undetermined,
    Guessing,
    UsingDefault
};

struct ILegacyMcpDefaultDiagnosticSink
{
    virtual ~ILegacyMcpDefaultDiagnosticSink() = default;
    // Name and Branch are provided for Guessing; Name for UsingDefault.
    virtual void Emit(ELegacyMcpDefaultDiagnostic Event, const FString& Name, const FString& Branch) = 0;
};

// UE4WarningIssued is caller-owned shared state corresponding to the original
// once byte. The sink is called only at the observed verbosity thresholds.
// Context is used only when the normalized override is empty.
FLegacyMcpDeploymentLookup ResolveLegacyMcpDeployment(
    const FString& Input, ILegacyMcpDefaultContext& Context,
    ILegacyMcpDefaultDiagnosticSink& Sink, int32 Verbosity, bool& UE4WarningIssued);
