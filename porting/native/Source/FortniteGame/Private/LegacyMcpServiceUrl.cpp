#include "LegacyMcpServiceUrl.h"
#include "HAL/PlatformMisc.h"
#include "Interfaces/ISslCertificateManager.h"
#include "SslModule.h"

DEFINE_LOG_CATEGORY_STATIC(LogLegacyMcp, Log, All);

// Provider 0x140a468e0 and gate 0x140d4a810; see
// porting/behavior/mcp_url_provider_body_runtime/evidence.json.
namespace
{
TCHAR FoldAscii(TCHAR Character)
{
    return Character >= TEXT('A') && Character <= TEXT('Z')
        ? Character + (TEXT('a') - TEXT('A')) : Character;
}

bool EqualAsciiInsensitive(const TCHAR* Left, const TCHAR* Right)
{
    while (*Left && *Right && FoldAscii(*Left) == FoldAscii(*Right))
    {
        ++Left;
        ++Right;
    }
    return FoldAscii(*Left) == FoldAscii(*Right);
}

class FEngineServiceUrlContext final : public ILegacyMcpServiceUrlContext
{
public:
    bool IsDomainPinned(const FString& Domain) override
    {
        return FSslModule::Get().GetCertificateManager().IsDomainPinned(Domain);
    }
    bool ShouldLogSeverity2() override
    {
        return UE_LOG_ACTIVE(LogLegacyMcp, Error);
    }
    void LogUnpinnedConnection(bool bBadProtocol, const FString& Protocol,
        bool bBadDomain, const FString& Domain) override
    {
        UE_LOG(LogLegacyMcp, Error,
            TEXT("Exiting due to OSS connection attempt to unpinned protocol (%d %s) or domain (%d %s)"),
            bBadProtocol, *Protocol, bBadDomain, *Domain);
    }
    void RequestExit(bool bForce) override
    {
        FPlatformMisc::RequestExit(bForce);
    }
};
}

FString MakeLegacyMcpServiceUrl(const FLegacyMcpServiceUrlComponents& Components,
    ILegacyMcpServiceUrlContext& Context)
{
    const bool bBadDomain = !Context.IsDomainPinned(Components.Domain);
    const bool bBadProtocol = !EqualAsciiInsensitive(*Components.Protocol, TEXT("https"))
        && !EqualAsciiInsensitive(*Components.Protocol, TEXT("wss"));
    if (bBadProtocol || bBadDomain)
    {
        if (Context.ShouldLogSeverity2())
        {
            Context.LogUnpinnedConnection(bBadProtocol, Components.Protocol,
                bBadDomain, Components.Domain);
        }
        Context.RequestExit(false);
    }

    FString Result = Components.Protocol + TEXT("://") + Components.Domain;
    if (!Components.BasePath.IsEmpty())
    {
        if (Components.BasePath.Find(TEXT("/"), ESearchCase::CaseSensitive) == INDEX_NONE)
        {
            Result += TEXT("/");
        }
        Result += Components.BasePath;
    }
    if (!Components.ServiceName.IsEmpty())
    {
        Result += TEXT("/");
        Result += Components.ServiceName;
    }
    return Result;
}

FString MakeLegacyMcpServiceUrl(const FLegacyMcpServiceUrlComponents& Components)
{
    FEngineServiceUrlContext Context;
    return MakeLegacyMcpServiceUrl(Components, Context);
}
