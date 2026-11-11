// CONFIDENCE: LOW. Original exec 0x14296d770; body 0x141ef3050 -> 0x141ecd440 (writes the surface byte through the out pointer).
// Surface comes from the hit's physical material when it has one, else from the hit actor's IFortDamageableActorInterface.
// A fallback to 0x17 applies only when neither source supplied a surface and the threshold test passes; a per-level key pair remaps the result to 0xc/0xe/0xf/0x10.
// Field identities for Hit (0x24, 0x64, 0x6c, 0x7c, 0x80) and the settings threshold (0x2cc) are unverified. See deps/FortPhysicalSurfaceLevelRemap.json.
EFortPhysicalSurface Surface = static_cast<EFortPhysicalSurface>(0);
bool bSkipFallback = false;

const UPhysicalMaterial* PhysMaterial = Hit.PhysMaterial.Get();
const IFortDamageableActorInterface* Damageable = Cast<IFortDamageableActorInterface>(Hit.Actor.Get());

if (PhysMaterial != nullptr && PhysMaterial->SurfaceType != EPhysicalSurface::SurfaceType_Default)
{
	Surface = static_cast<EFortPhysicalSurface>(PhysMaterial->SurfaceType);
	bSkipFallback = Damageable != nullptr;
}
else if (Damageable != nullptr)
{
	Surface = static_cast<EFortPhysicalSurface>(Damageable->GetDamageableSurfaceType());
	bSkipFallback = true;
}

if (!bSkipFallback && Surface != static_cast<EFortPhysicalSurface>(8))
{
	const UFortSurfaceSettings* Settings = GetFortSurfaceSettings();
	if (Settings->DistanceThreshold <= Hit.SurfaceDistance)
	{
		Surface = static_cast<EFortPhysicalSurface>(0x17);
	}
}

if (Hit.SurfaceKeyA == FortSurfaceRemapKeys::KeyA && Hit.SurfaceKeyB == FortSurfaceRemapKeys::KeyB)
{
	switch (static_cast<uint8>(Surface))
	{
	case 1:
	case 14:
		Surface = static_cast<EFortPhysicalSurface>(14);
		break;
	case 2:
	case 15:
		Surface = static_cast<EFortPhysicalSurface>(15);
		break;
	case 3:
	case 16:
		Surface = static_cast<EFortPhysicalSurface>(16);
		break;
	default:
		Surface = static_cast<EFortPhysicalSurface>(12);
		break;
	}
}

return Surface;
