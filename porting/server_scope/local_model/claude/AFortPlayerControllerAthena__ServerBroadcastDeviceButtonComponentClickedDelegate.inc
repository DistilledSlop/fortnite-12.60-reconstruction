// CONFIDENCE: HIGH
// Original address: 0x142a5abf0 (RPC thunk). Single parameter DeviceButtonComponent (object).
if (!ServerBroadcastDeviceButtonComponentClickedDelegate_Validate(DeviceButtonComponent))
{
	RPC_ValidateFailed(TEXT("ServerBroadcastDeviceButtonComponentClickedDelegate_Validate"));
	return;
}
ServerBroadcastDeviceButtonComponentClickedDelegate_Implementation(DeviceButtonComponent);
