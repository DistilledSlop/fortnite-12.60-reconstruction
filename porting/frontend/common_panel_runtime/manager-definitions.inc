
bool UCommonInputManager::IsInputSuspended() const
{
    return bNativeOperationInputSuspended || bNativeExternalInputSuspended;
}

void UCommonInputManager::NativeSetExternalInputSuspended(bool bSuspended)
{
    bNativeExternalInputSuspended = bSuspended;
}
