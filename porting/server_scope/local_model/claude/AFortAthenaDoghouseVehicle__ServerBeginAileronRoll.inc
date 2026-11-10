// CONFIDENCE: HIGH. Original exec 0x1427d83e0 (Server RPC thunk). Parameter: Direction (EAileronRollDirection).
// RPC pattern: validate first, reject with RPC_ValidateFailed when validation fails, then run the implementation.
// Validate is vtable slot 0xec0, resolved to 0x1406801d0, which always returns true.
// Implementation is vtable slot 0xec8, resolved to 0x1418f71d0, which dispatches the Blueprint event through ProcessEvent.
if (!ServerBeginAileronRoll_Validate(Direction))
{
    RPC_ValidateFailed(TEXT("ServerBeginAileronRoll_Validate"));
    return;
}

ServerBeginAileronRoll_Implementation(Direction);
