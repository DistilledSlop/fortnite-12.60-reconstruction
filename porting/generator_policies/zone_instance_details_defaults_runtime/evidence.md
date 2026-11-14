# Zone instance details native defaults

Original constructor `0x141f7dbe0` (`porting/generated/agents/foundation-record-closure/captures/0x141f7dbe0.asm`) initializes `FFortZoneInstanceInfo` and `FFortZoneInstanceDetails` inline with no nested calls except unreachable-on-normal-initialization empty-string allocation branches. Reflection `porting/generated/reflection/struct_blocks.json` identifies the layouts and sizes validated in `source_policy.py`.

- Offsets 0..63: four empty FString fields. Offset 64: ZoneThemeClass null soft class. Offset 104: MissionGenerator null soft class. The writes of -1 and 0 at the start of each soft class correspond to the default FWeakObjectPtr reset, not a user-facing index property. `TPersistentObjectPtr::Reset` also clears its object ID and tag. See engine `CoreUObject/Public/UObject/{WeakObjectPtr,PersistentObjectPtr,SoftObjectPtr,SoftObjectPath}.h`.
- Offset 144: empty `FDataTableRowHandle`; its engine constructor initializes null DataTable and NAME_None RowName (`Engine/Classes/Engine/DataTable.h`).
- Offsets 160, 208 and 240: empty `FMcpLootResult`, each composed of FString and TArray. Offset 192: empty TArray. Offset 272: empty FString. Their typed default constructors create those values; no element constructor is invoked for the empty arrays.
- Offset 288: integer TileIndex = -1, expressed as INDEX_NONE. The generated struct has no scalar initializer, so the policy emits the native constructor with this initializer. Default construction of the parent and other members provides the remaining typed defaults.

The policy fails generation if any of the three reflected models changes layout, type, inheritance or package. Existing serialization may overwrite these instance defaults. This only supplies the native details value; it does not create records or advance the world manager.
