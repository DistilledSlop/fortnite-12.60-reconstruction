# Native build harness

This is a newly authored UE project used to test Linux compilation and to
reconstruct the game module incrementally. Its module is named `FortniteGame`
so that future classes can use the `/Script/FortniteGame` path found in cooked
assets. It contains no Fortnite gameplay code, no supplied assets, and no EAC
integration. It is not playable yet.

After the UE 4.25 source checkout is set up, build a Linux Development target:

```sh
/opt/fortnite/UnrealEngine/Engine/Build/BatchFiles/Linux/Build.sh \
  FortniteLinux Linux Development \
  /opt/fortnite/FortniteClient-12.60-CL-13477524/porting/native/FortniteLinux.uproject
```

The editor target is needed later to create and cook Linux-compatible assets.
