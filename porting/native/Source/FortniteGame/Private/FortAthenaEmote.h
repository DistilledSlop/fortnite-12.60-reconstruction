// Server-side emote playback (ServerPlayEmoteItem) for the original 12.60 client.
#pragma once

namespace FortAthenaEmote
{
	// Rebinds AFortPlayerController::ServerPlayEmoteItem (original body 0x1406801c0 is a bare RET: the server code was
	// compiled out of the client exe). Idempotent; only active with -FortniteOriginalClient.
	void InstallRpcOverrides();
}
