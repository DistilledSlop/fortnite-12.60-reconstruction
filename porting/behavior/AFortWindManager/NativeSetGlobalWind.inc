// CONFIDENCE: high; original source recovery, build/runtime pending.
if ((Intensity != NativeWindIntensity_0x420 ||
     Direction.X != NativeWindDirection_0x424.X ||
     Direction.Y != NativeWindDirection_0x424.Y ||
     Direction.Z != NativeWindDirection_0x424.Z) && GetNetMode() != NM_DedicatedServer)
{
    NativeWindIntensity_0x420 = Intensity;
    NativeWindDirection_0x424 = Direction;
    NativeWindVector_0x430 = Intensity * Direction;
}
