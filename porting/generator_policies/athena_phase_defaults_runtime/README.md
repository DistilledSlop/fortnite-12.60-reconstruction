# Athena phase-time constructor defaults

The original `AFortGameStateAthena` constructor at `0x14197f7b0` writes
`-1.0f` to six consecutive phase timestamps at original offsets
`0x12d4..0x12e8` (`0x141980d44..0x141980d76`). This policy restores those
stores using reflected names; it does not recover the entire constructor.

It also restores `PlaylistEndTime` and `SafeZonePauseTime` to `-1.0f`, from
stores at `0x14197fa6d` and `0x14197fa78`. In the socket/wallet diagnostic,
both live server values were zero while the class defaults were `-1.0f`.
The match flow treats a nonnegative `SafeZonePauseTime` as paused, so the
missing per-instance sentinel prevented storm progression after its initial
countdown. These values require constructor initialization, not just CDO repair.

`UpdateWarmupPhase` (`0x14197a400`) initializes a countdown only when its
end time is negative. Zero therefore means an already-expired countdown,
causing flight to start immediately when the second required player joins.
The CDO-only defaults loader was insufficient in the recorded original-client
run `20261113-rpc-trace-name-two-profile-2`, whose instance times remained zero.

Generation fails on constructor conflicts or changed field declarations.
Validate actual Blueprint game-state instance timestamps and a configured
countdown in the staged server. Separately measure original-client bus
positions; fixing the countdown alone does not establish synchronized flight.

The hand-written match setup also restores the twelve replicated curve-driven
defaults from original initializer `0x1419653d0`. It evaluates the original
`AthenaGameData` rows at their original inputs, preserving existing values for
absent rows. In particular, the original client's bus auto-deployment policy
reads `DefaultParachuteDeployTraceForGroundDistance` from the game state; zero
silently replaces the pawn's fallback distance. The cooked curve evaluates to
10,000 cm at time zero. This restores the input to that policy, not the full
server custom movement, glider lifecycle, or proof of visible deployment.
