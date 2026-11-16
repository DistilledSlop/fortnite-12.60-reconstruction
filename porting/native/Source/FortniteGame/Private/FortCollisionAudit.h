#pragma once

class UWorld;

namespace FortCollisionAudit
{
	// Call regularly from the game thread (cheap until -FortniteCollisionAudit is set and 90 s have passed).
	void Tick(UWorld* World);
}
