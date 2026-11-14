# Ability block/cancel effects

`UFortAbilitySystemComponent::ApplyAbilityBlockAndCancelTags`, original
`0x1415e9850`, restores the complete override. Existing original captures in
`porting/generated/agents/asc-block182` and independent assembly review establish:

- Copy both incoming tag containers, expand their relationships, then call Super.
- Read ActorInfo after Super; validate its weak owner and cast the owner's Owner
  to AController. Apply movement and look-input gates before resolving the avatar.
- Match the singleton movement/look/target/crouch tag against the expanded
  container in that direction. The captured `0x143e93400` helper confirms it.
- Call the current weapon's targeting virtual before testing and canceling crouch.
  These effects have no additional `bExecuteCancelTags` gate.

The ranged targeting override is canonical in `c93f9707`; the actual crouch
consumer and tag relationship expansion are existing canonical dependencies.
This admits one complete original body. It does not establish the ability producer
lifecycle or walking acceptance. UE build/runtime verification is pending.
