// Server-side building RPC re-creation for -FortniteOriginalClient.
#pragma once

class AFortGameStateAthena;

namespace FortAthenaBuilding
{
	void InstallRpcOverrides();
	// Checks whether the complete original-client class table is available.
	void PrepareBuildableClasses(AFortGameStateAthena* GameState);
}
