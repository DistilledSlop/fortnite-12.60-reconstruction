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
  -project=/opt/fortnite/FortniteClient-12.60-CL-13477524/porting/native/FortniteLinux.uproject \
  -NoHotReload
```

The project disables unused SteamVR and UdpMessaging plugins. Build the editor
and shader worker, then cook the tiny engine Entry map:

```sh
cd /opt/fortnite/UnrealEngine
Engine/Build/BatchFiles/Linux/Build.sh FortniteLinuxEditor Linux Development \
  -project=/opt/fortnite/FortniteClient-12.60-CL-13477524/porting/native/FortniteLinux.uproject \
  -NoHotReload
Engine/Build/BatchFiles/Linux/Build.sh ShaderCompileWorker Linux Development -NoHotReload
Engine/Binaries/Linux/UE4Editor-Cmd \
  /opt/fortnite/FortniteClient-12.60-CL-13477524/porting/native/FortniteLinux.uproject \
  -run=Cook -TargetPlatform=LinuxNoEditor -Map=/Engine/Maps/Entry -unattended -nop4
```

Stage the cooked project with `RunUAT.sh BuildCookRun` using `-skipbuild
-skipcook -stage -archive -platform=Linux -clientconfig=Development`. A staged
headless run with `-nullrhi -nosound -unattended -ExecCmds=Quit` loaded
`/Engine/Maps/Entry` and exited with code 0. The staged output stays ignored
by Git. This validates only the empty engine map; Fortnite gameplay, content,
and server integration remain to be implemented.
