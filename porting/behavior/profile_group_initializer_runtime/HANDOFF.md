# Profile group initializer state

The initializer is not fully closed: the allocated native support object's exact type and destruction are unknown. Do not integrate a substitute support object as complete behavior. Image `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`, base `0x140000000`, SHA-256 `338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680`; byte-window hashes below are identity pins, not function bounds.

Reflection class block `0x145c01250` in `porting/generated/reflection/fortnite_game_reflection.json` identifies group+0x140 as `FString PlayerName` (offset 320). Neither +0x110 nor +0x168 appears as a property. The helper at `0x140b35480` is behaviorally `FUniqueNetIdWrapper::ToDebugString()`: valid ID yields `GetType().ToString() + ":" + ToDebugString()`, otherwise `INVALID`. Local source `UnrealEngine/Engine/Source/Runtime/CoreUObject/Public/UObject/CoreOnline.h:197` implements it exactly. Group constructor `0x1410ecb60` installs wrapper vptrs at +0x168 and +0x180 and clears their two shared pointer words; adjacent 0x18-byte instances and the local declarations support `FUniqueNetIdWrapper` rather than `FUniqueNetIdRepl`, which adds a replication array. Initialization `0x141104600` uses `0x1406af7a0` to copy the incoming wrapper into +0x168. Thus, where the real group field is accessible, formatting uses `*Group->PlayerName` and `*Group->AccountId.ToDebugString()` (AccountId is a provisional source name).

Group constructor zeroes +0x110. Group setup `0x141104600` allocates a 0x58-byte native object when its second argument is non-null and puts that argument in its first pointer field; otherwise the constructor's null remains. Caller `0x1410f6720` obtains the argument from slot +0x50 on the loaded OnlineSubsystem module, matching `FOnlineSubsystemModule::GetOnlineSubsystem` in local `Engine/Plugins/Online/OnlineSubsystem/Source/Public/OnlineSubsystemModule.h:120` with inherited interface slot order. This supports a borrowed `IOnlineSubsystem*` at support object +0, copied without reference-count manipulation to profile+0xa0 by `0x1411044d0`. Neither ownership nor the exact support class is established. Do not flatten it into a fully implemented group member without tracing setup and destruction.

Read-only probes: `/tmp/profile-group-probe{,2,3}`; Ghidra transaction rolled back, though project metadata/index files reported modified. No canonical changes, build, or tests. Relevant windows (address, length, SHA-256):

- `0x1410ecb60`, `0x1c0`, `2f151c47e8bd1da7baa86e20a11a2bb31b28b0f4bb2739280d9294b6590d9497`
- `0x141104600`, `0x200`, `d6ebfffe2ba9e90b00b92ebd46a5beb46f0c8134b1bdacb0601bf33530218ff2`
- `0x1410f6720`, `0x200`, `76d6cc63e7354eab7f4478c01e88de63e087c2ddc3546fdadb99591acd7640ea`
- `0x141111540`, `0xab`, `f832f3ebd56191c4e0651749ecd7451909c453f43c6e18552c16254d3d38920a`
- `0x141111bb0`, `0x20`, `4f3c7c4d219dbb530921edd85791ee9bd1d683dac1e42da4d5e331c2aaffb359`
