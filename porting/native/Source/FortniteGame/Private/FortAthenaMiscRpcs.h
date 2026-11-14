// Small re-creations of stripped server RPC bodies (original body 0x1406801c0 = bare RET) for the original 12.60 client.
#pragma once

namespace FortAthenaMiscRpcs
{
	// Rebinds AFortPlayerPawn::ServerSendZiplineState. Idempotent; only with -FortniteOriginalClient.
	void InstallRpcOverrides();
}
