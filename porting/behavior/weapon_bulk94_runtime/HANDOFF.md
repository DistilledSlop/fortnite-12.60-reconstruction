# Weapon bulk 94 getter recovery

Eight candidates in manifest-entries.json, reflection flags and exec addresses from generated/reflection/native_funcs.json. Reticle fields +0x9e8/+0xa10/+0x9b0/+0xa40 use 0x141c56510; alteration fields +0x840/+0x868 use 0x142659290. Both resolvers immediately return null for empty paths, otherwise resolve/cache, synchronously load if needed, cast and track the result including null. Actor class +0x740 uses 0x142659660; its returned TSubclassOf retains the raw loaded UClass while tracking its checked value even for an empty path. Equipped abilities +0x930 uses 0x142659870, tracks each checked class including null, appends valid classes in source order to a newly initialized output array, and preserves duplicates. Existing delayed loader and asset owner provide dependencies.

The native soft pointer Get paths perform their typed checks before falling back to delayed loading; array/actor loaded values are cast to UClass before TSubclassOf checking. No generator, build, tests, Ghidra, or canonical manifest changes were run. These candidates remain build pending and require integration review.

Class identity evidence: direct inspection of `porting/generated/dump/FortniteClient-Win64-Shipping.runtime.bin` (runtime image offset = VA - 0x140000000) shows the following class-name UTF-16 pointers passed by each class constructor/getter. `objdump -D -b binary -m i386:x86-64 --adjust-vma=0x140000000 --start-address=<getter> --stop-address=<getter+0xb4> <image>` identifies the RIP-relative LEA; reading UTF-16 from that VA gives:

- 0x1427acc10, LEA at 0x1427acc4d -> 0x1462bcee2 `FortAlterationItemDefinition`.
- 0x1448046d0, LEA at 0x14480470d -> 0x146c395f2 `Texture2D`.
- 0x1428f53b0, LEA at 0x1428f53ed -> 0x1463d7212 `FortGameplayAbility`; also explicit in `porting/generated/callee_probe/gadget_ammo_override_batch_2/0x1428f53b0.c`.
- 0x142d68560, LEA at 0x142d685ab -> 0x1465f661a `Class`.
- 0x142eeab70, LEA at 0x142eeabad -> 0x146621caa `Object`.

Signatures are proposed pending promotion, class includes are inferred by the generator. Array iteration captures source begin/end at loop entry, and checks subclass again after TrackAsset before appending, matching the original order.
