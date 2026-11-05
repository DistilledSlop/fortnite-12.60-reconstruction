#pragma once

#include "CoreMinimal.h"

// Current service fields, after the caller's configuration and discovery passes.
// No deployed values are implied by this type.
struct FLegacyMcpServiceUrlComponents
{
    FString Protocol;
    FString Domain;
    FString BasePath;
    FString ServiceName;
};

// Required integration with the SSL certificate manager, MCP log category and
// platform exit request. IsDomainPinned is called even for a bad protocol.
struct ILegacyMcpServiceUrlContext
{
    virtual ~ILegacyMcpServiceUrlContext() = default;
    virtual bool IsDomainPinned(const FString& Domain) = 0;
    virtual bool ShouldLogSeverity2() = 0;
    virtual void LogUnpinnedConnection(bool bBadProtocol, const FString& Protocol,
        bool bBadDomain, const FString& Domain) = 0;
    virtual void RequestExit(bool bForce) = 0;
};

// Reads the supplied fields after the gate as well as during it: context calls
// can have side effects. A returning exit request does not stop assembly.
FString MakeLegacyMcpServiceUrl(const FLegacyMcpServiceUrlComponents& Components,
    ILegacyMcpServiceUrlContext& Context);

// Engine integration: FSslModule certificate manager, Error logging and
// FPlatformMisc::RequestExit(false). This does not register a discovery provider.
FString MakeLegacyMcpServiceUrl(const FLegacyMcpServiceUrlComponents& Components);
