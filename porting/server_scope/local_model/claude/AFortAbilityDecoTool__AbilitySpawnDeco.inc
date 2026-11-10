// CONFIDENCE: LOW. Spawns the ability-deco actor described by SpawnParameters and returns it. Field roles inside the parameter struct are inferred from the call pattern.
// Original address: 0x142780380 (exec). Native body: 0x14265e3d0 forwards to virtual slot 0xac8 of AFortAbilityDecoTool, resolved to 0x1426b5370 (SpawnAbilityDeco, see deps).
return SpawnAbilityDeco(SpawnParameters.Tool, SpawnParameters.DecoClass, SpawnParameters.Location, SpawnParameters.Rotation, SpawnParameters.Instigator, 8, 5);
