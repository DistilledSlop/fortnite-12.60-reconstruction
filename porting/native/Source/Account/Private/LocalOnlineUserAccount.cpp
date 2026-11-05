#include "LocalAccountSubsystem.h"

#include "OnlineSubsystemTypes.h"

namespace
{
// Local username policy: no email or real-name data is collected. This supplies
// the FUserOnlineAccount consumed by original ProcessUserLogin, not an Epic
// provider implementation or a replacement for the original login state machine.
class FLocalOnlineUserAccount final : public FUserOnlineAccount
{
public:
    FLocalOnlineUserAccount(const TSharedRef<const FUniqueNetId>& InUserId,
        const FString& InDisplayName, const FString& InAccessToken)
        : UserId(InUserId), DisplayName(InDisplayName), AccessToken(InAccessToken)
    {
    }

    virtual TSharedRef<const FUniqueNetId> GetUserId() const override { return UserId; }
    virtual FString GetRealName() const override { return FString(); }
    virtual FString GetDisplayName(const FString& Platform = FString()) const override { return DisplayName; }
    virtual FString GetAccessToken() const override { return AccessToken; }

    virtual bool GetUserAttribute(const FString& AttrName, FString& OutAttrValue) const override
    {
        if (AttrName == TEXT("id")) OutAttrValue = UserId->ToString();
        else if (AttrName == TEXT("displayName")) OutAttrValue = DisplayName;
        else if (AttrName == TEXT("email")) OutAttrValue.Empty();
        else return false;
        return true;
    }

    virtual bool GetAuthAttribute(const FString& AttrName, FString& OutAttrValue) const override
    {
        // The selection bridge currently retains this token only; it must not
        // fabricate refresh tokens or other authentication attributes.
        if (AttrName != TEXT("access_token")) return false;
        OutAttrValue = AccessToken;
        return true;
    }

    virtual bool SetUserAttribute(const FString& AttrName, const FString& AttrValue) override
    {
        // Original ProcessUserLogin can update its cached displayName. This
        // changes this snapshot only, never the persistent username/account key.
        if (AttrName != TEXT("displayName") || DisplayName == AttrValue) return false;
        DisplayName = AttrValue;
        return true;
    }

private:
    TSharedRef<const FUniqueNetId> UserId;
    FString DisplayName;
    FString AccessToken;
};
}

TSharedPtr<FUserOnlineAccount> ULocalAccountSubsystem::CreateSelectedOnlineUserAccount(
    const TSharedRef<const FUniqueNetId>& UserId, FString& Error) const
{
    if (AccountId.IsEmpty() || DisplayName.IsEmpty() || AccessToken.IsEmpty() ||
        AthenaProfileJson.IsEmpty() || !UserId->IsValid() || UserId->ToString() != AccountId)
    {
        Error = TEXT("Selected local account and provider user ID do not match or are incomplete.");
        return nullptr;
    }
    Error.Empty();
    return MakeShared<FLocalOnlineUserAccount>(UserId, DisplayName, AccessToken);
}
