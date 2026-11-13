# Pawn aim rotation

`AFortPlayerPawn::GetBaseAimRotation` restores original `0x1423198d0`, proved
through the reflected `APawn::GetBaseAimRotation` thunk's virtual dispatch at
slot `0x6d8`. All five reflected Fort pawn subclasses use this override.
Controllerless pawns use packed remote view data and the replay pitch value;
the vehicle-dependent yaw decision calls the recovered native interface slot.
The exact packed-angle multiplier and unordered replay-pitch behavior are kept.

The interface slot `0xc8` is unreflected, so its name remains neutral. The
original native interface registration scan found two roots and 23 implementers.
Athena vehicles return false except Octopus and Spaghetti; character vehicles
and their Ostrich descendant return true. Both bodies are recovered constant
machine-code targets, not fallback guesses. Every native implementer inherits
one of the declared implementations. The interface has no default body.

This restores the aim-rotation dependency; it does not implement full airborne
physics, establish an attachment-pose fix, or prove runtime synchronization.
