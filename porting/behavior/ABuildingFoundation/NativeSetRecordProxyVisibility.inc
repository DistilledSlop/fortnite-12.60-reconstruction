// CONFIDENCE: HIGH. Original 0x141bd0cb0.
for (TLazyObjectPtr<AFortHLODSMActor>& LazyProxy : Record.ProxyInfo)
    if (AFortHLODSMActor* Proxy = Cast<AFortHLODSMActor>(LazyProxy.Get()))
        Proxy->NativeSetFoundationProxyVisibility(bVisible);
