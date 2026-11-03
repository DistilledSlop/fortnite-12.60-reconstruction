# Pawn ability interface native closure

Proposed one body: `AFortPawn::GetAbilitySystemComponent() const`, override of the engine `IAbilitySystemInterface` virtual. The included body returns the reflected `UFortAbilitySystemComponent* AbilitySystemComponent`, converted to `UAbilitySystemComponent*`; include the complete Fort component type at the definition. Do not splice until canonical interface inheritance and declaration are wired. The generated header currently inherits only `ACharacter`; change its generator/overrides and canonical regeneration path, not just a generated file. Include `AbilitySystemInterface.h`, inherit `public IAbilitySystemInterface`, and declare `virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;` on AFortPawn, with the corresponding definition owned by AFortPawn. GameplayAbilities is already a module dependency. Confirm UHT-generated interface metadata and other existing native interfaces when integrating: native layout contains multiple secondary interfaces, so do not assert this single addition recreates the complete binary layout. Do not invent an abstract stand-in interface.

At 0x1426952cf the weapon helper forms RCX = pawn + 0x4c8, loads the subobject table and calls [table + 0x10]. This is an indirect virtual call; source callers should use `IAbilitySystemInterface` / `GetAbilitySystemComponent()` on the actual object, preserving derived override dispatch. Do not replace this call with a field read. The interface method is `UAbilitySystemComponent* GetAbilitySystemComponent() const` in the installed engine `AbilitySystemInterface.h`; no arguments other than this, pointer return in RAX under Win64 ABI.

Native constructor proof (constructor `this` comes from incoming RCX):

* AFortPawn constructor 0x1422fce30 saves RCX in R14 at 0x1422fce51 and stores table 0x1461167a8 at [R14+0x4c8] at 0x1422fcef0.
* AFortPlayerPawn constructor 0x1422fe830 saves RCX in RDI at 0x1422fe849, calls the pawn constructor, and stores table 0x146117fa8 at [RDI+0x4c8] at 0x1422fe8b4.
* AFortPlayerPawnAthena constructor 0x141a08ef0 saves RCX in R15 at 0x141a08f0a, calls the player pawn constructor, and stores table 0x145e2e538 at [R15+0x4c8] at 0x141a08fe5. Its unwind record is split; inspect beyond the first .pdata range.

Each table's +0x10 entry is 0x141a128d0. The native body is `mov rax,[rcx+0x6f0]; ret`; RCX is the secondary subobject pointer, yielding complete-object offset 0xbb8. Reflection reports the AFortPawn `AbilitySystemComponent` object property at decimal 3000 (0xbb8), and the canonical generated header declares it as `UFortAbilitySystemComponent*`. The shared implementation here establishes behavior for these three sampled classes only; it does not prove all subclasses use the same slot target.

No canonical files, builds, tests, or runtime execution were changed/run. Evidence windows in native-evidence.json use VA-relative offsets into the runtime image (image base 0x140000000).
