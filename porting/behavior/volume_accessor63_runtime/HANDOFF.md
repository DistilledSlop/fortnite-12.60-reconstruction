# Native volume accessor recovery

Recovered the native slot +0x10 and all three direct native implementers present in the image's static interface-entry tables. No canonical files, manifest, generator, build, tests, or commit were changed. The files here are a proposal, not integrated or counted as ported.

`IFortVolumeAccessor` slot +0x10 takes the interface pointer as its sole argument and returns `AFortVolume*`. The machine code does not establish source spelling, const qualification, or whether the original declaration was pure virtual; `NativeGetVolume()` in the proposed snippets is a source-compatible provisional name. Preserve native interface dispatch (`GetInterfaceAddress` / native `Cast`), not Blueprint execute dispatch. The process-global recursion guard and caller control flow remain the actor lookup's responsibility.

| Native class | Interface metadata entry, subobject offset | Constructor and assigned interface vtable | Slot +0x10 body | Result |
|---|---|---|---|---|
| AFortMinigame | 0x146446eb8, +0x228 | 0x14207d6d0, 0x146085b68 | 0x1420974d0: `mov rax,[rcx+0x6e8]; ret` | `Volume`, object +0x910 |
| AFortMinigameSettingsBuilding | 0x146453598, +0x220 | 0x1426e8e90, 0x14622d920 | 0x142702350: `mov rax,[rcx+0xe8]; ret` | `SettingsVolume`, object +0x308 |
| AFortVolume | 0x146564790, +0x298 | 0x141c58290, 0x145f62390 | 0x14165b340: `lea rax,[rcx-0x298]; ret` | owning `this` |

Metadata entries contain no-register interface getter 0x142b39530, a jump to 0x142b39170. The adjacent class parameter blocks 0x146446ef0, 0x1464535b0, 0x1465647c0 are mapped to these names in `porting/generated/reflection/class_name_map.json`; reflected properties independently identify the two pointer fields and their AFortVolume type. The fourth absolute reference to the no-register getter, 0x1465686b0, is interface registration, not an implementation table. This establishes direct native implementers represented by this static registration data, not all possible dynamically generated subclasses.

Native helper 0x142ec9900 walks class and superclasses' implemented-interface arrays, skips Blueprint-only entries, and returns object plus stored offset; inherited implementations therefore dispatch naturally. The actor lookup 0x141c7c5c0 invokes this slot only for its guarded AInfo fallback. Other independent call sites 0x141b03d52, 0x141f2e5e5, 0x141f354a9 and 0x141f36981 corroborate no explicit method arguments and use the returned volume.

Integration requires generator/source policy support for three interface base declarations and four declarations plus the bodies in `proposed.inc`; currently all three generated classes omit this base. Ensure proper include ordering and Unreal interface metadata generation. Avoid manually assuming binary offsets in source. A default interface implementation is not established; require each concrete override. Source ABI cannot be declared identical solely from this evidence (including const and virtual layout), and no compilation has checked the proposal. The actor lookup additionally remains blocked on the volume membership lifecycle described in `../actor_volume61_runtime/HANDOFF.md`.

Evidence image: `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`, base 0x140000000, SHA-256 338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680. No Ghidra used.
