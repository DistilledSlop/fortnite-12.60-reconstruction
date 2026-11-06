# Ability data recovery

`GetAllowedPropClassByIndex` is complete. Wrapper 0x1428fd500 parses the index and calls 0x141626d10. The callee checks 0 <= index < Num, retrieves the soft object at 40-byte stride, validates UClass and its ABuildingActor subclass, and otherwise returns null. Reflection identifies the return meta-class as ABuildingActor and property `AllowedPropClasses` as TArray<TSoftClassPtr<ABuildingActor>>. `Get()` supplies the same validations; it does not load the class. Const is proposed from read-only behavior.

Batch stopped below requested count: AIPortal GetPortalInPoint, GetPortalOutPoint, and HasPortalOutPoint use unreflected instance fields at +0xa6c/+0xa70/+0xa84, +0xa78, and bit 1 of +0xaa0 respectively, with no verified source member names or storage declarations. UFortAttributeListItem_NUI HasPreviewAttribute similarly operates on unreflected +0x330 through 0x142c2e460; its type and corresponding reflected member are unproven. PossessProp SetAllowedPropTable calls 0x141612450 and iterates its results to repopulate the soft class array; exact row extraction and row type remain unproven. These were deliberately not ported.

No builds or tests run, per instruction. No canonical manifest was edited.
