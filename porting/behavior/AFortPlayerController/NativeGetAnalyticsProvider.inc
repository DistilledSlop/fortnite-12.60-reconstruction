// CONFIDENCE: HIGH
if (UFortLocalPlayer* LocalPlayer = Cast<UFortLocalPlayer>(Player))
    {
        return LocalPlayer->NativeAnalyticsProvider;
    }
    return NativeAnalyticsProvider;
