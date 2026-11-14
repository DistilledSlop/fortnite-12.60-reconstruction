# UFortCharacterAttrSet constructor

Original 0x141b886d0 constructs Fortitude, Offense, Tech, Resistance, and MaxCharacterLevel in that order, each at 0x30 + 0x28*n. The five inlined FFortGameplayAttributeData constructors set BaseValue, CurrentValue, UnclampedBaseValue and UnclampedCurrentValue to zero, Minimum to -FLT_MAX, Maximum to FLT_MAX, and the three clamp flags false. Then MaxCharacterLevel at 0xd0 receives SetBaseValue followed by virtual SetCurrentValue, each with float bits 0x46fffe00 (32767). All four resulting MaxCharacterLevel values are 32767 and clamp flags remain false.

Evidence: `porting/generated/native129-constructors/0x141b886d0.c` and `.asm`; see `porting/generator_policies/fort_attribute_data_runtime/README.md` for the member constructor and setter behavior. The candidate include assumes that member lifecycle policy is active and is not integrated automatically.
