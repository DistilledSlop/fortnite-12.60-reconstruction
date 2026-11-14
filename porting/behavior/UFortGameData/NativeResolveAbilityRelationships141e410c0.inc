// CONFIDENCE: HIGH. Original141e410c0 specialized to shared GameData soft field44d0.
// Typed soft Get retains weak/path-cache resolution; nonempty attempts track even null.
if (AbilityTagRelationshipMapping.IsNull()) return nullptr;
UFortAbilityTagRelationshipMapping* Mapping = AbilityTagRelationshipMapping.Get();
if (!Mapping)
{
    Mapping = Cast<UFortAbilityTagRelationshipMapping>(FortLoadDelayedObject(
        AbilityTagRelationshipMapping.ToSoftObjectPath(), bLogLoad));
}
FFortAssets::Get().TrackAsset(Mapping);
return Mapping;
