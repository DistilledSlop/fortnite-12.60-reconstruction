#pragma once
#if PLATFORM_LINUX && !UE_SERVER
// Returns true when the command was present and handled, including failure.
bool ProbeLegacyLandscapeDecode();
#endif
