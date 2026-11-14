# Shared pawn action gate

Original base pawn virtual +ce0 is `14230c260`, including AFortPawn itself; the recovered method therefore belongs on AFortPawn rather than AFortPlayerPawn. Athena retains override `141a0d7c0`. Character vehicle and Ostrich dispatch to `141937350`, which uses reflected OverrideAbilitySystemComponent. Base uses reflected AbilitySystemComponent. Both return false for null, then tail-call the same helper `1415ee070`.

That helper resets/adds the action tag to one function-static container (original `1480fa6e0`) and returns inverse virtual AreAbilityTagsBlocked. Keeping one helper preserves shared storage across pawn/vehicle calls and reentry. The previous player-only body had the container locally. The move retains one original-body count and adds two actual original bodies; it does not duplicate the moved body.

Constructor table inventory and original-image evidence are recorded in ignored `generated/agents/reload-gates208/`. Seven indexed tables use base, two Athena, and two vehicle; unindexed AFortAIBuddy and runtime Blueprint tables are not claimed. The existing weapon-definition ammo virtuals are reused. Broad ASC producer lifecycle and end-to-end reload/movement acceptance remain open. This change is build/runtime pending.
