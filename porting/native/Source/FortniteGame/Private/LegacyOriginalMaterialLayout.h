#pragma once

#if PLATFORM_LINUX && !UE_SERVER
#include "LegacyOriginalMaterialReader.h"

// Copies the original per-record debug resource layout into the decoded IR only
// after every record agrees and the layout matches the expression shape accepted
// by CreateLegacyMaterialMap. Does not establish a shader archive association,
// wrapper hash, original metadata member tree, or renderer binding. Out is
// unchanged on failure.
bool PrepareLegacyOriginalMaterialLayout(const FLegacyOriginalMaterial& Original,
    FLegacyMaterialIR& Out, FString& Error);
#endif
