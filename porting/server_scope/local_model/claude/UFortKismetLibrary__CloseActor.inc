// CONFIDENCE: MEDIUM. Closes OpenableInterfaceActor when its class implements the openable interface, passing OptionalControllerInstigator. Returns true when the close was dispatched. Returns false, with no effect, for a null actor or an actor whose class does not implement the interface.
// Original address: 0x142967f40 (exec). Native body: 0x141ee8de0. Helpers: 0x142b78be0 (IOpenableInterface class), 0x142d6a820 (class implements interface), 0x142b7c430 (close dispatch).
if (OpenableInterfaceActor == nullptr || !OpenableInterfaceActor->GetClass()->ImplementsInterface(UOpenableInterface::StaticClass()))
{
    return false;
}
IOpenableInterface::Execute_Close(OpenableInterfaceActor, OptionalControllerInstigator);
return true;
