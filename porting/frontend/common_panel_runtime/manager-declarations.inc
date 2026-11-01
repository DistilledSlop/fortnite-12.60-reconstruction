public:
    // Project-local bridge for the original native +FE store; this is not a UFUNCTION.
    void NativeSetExternalInputSuspended(bool bSuspended);
private:
    bool bNativeOperationInputSuspended = false;
    bool bNativeExternalInputSuspended = false;
