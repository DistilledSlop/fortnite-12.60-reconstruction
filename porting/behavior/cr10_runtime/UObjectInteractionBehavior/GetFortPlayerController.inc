// Original body address: 0x14266d0d0 (thunk 0x1428c26e0)
// Offset evidence: MOV RBX,qword [RCX+0xa0] loads an object pointer at object offset 0xa0. The reflected own members of UObjectInteractionBehavior start at 0xb0 and UActorComponent's reflected UCSModifiedProperties array covers 0x90-0x9f, so 0xa0 is inherited unreflected storage and is read raw. The two inlined class-membership tests (ClassPrivate table indexed by StaticClass()) are expressed as UObject::IsA.
// CONFIDENCE: MEDIUM
static_assert(sizeof(AFortCreativeMoveTool) > 0, "AFortCreativeMoveTool must be complete for its class and helper");
static_assert(sizeof(AFortPlayerController) > 0, "AFortPlayerController must be complete for its class and cast");
const uint8* ObjectBytes = reinterpret_cast<const uint8*>(this);
UObject* MoveToolObject = *reinterpret_cast<UObject* const*>(ObjectBytes + 0xa0);
if (MoveToolObject == nullptr)
{
    return nullptr;
}
if (!MoveToolObject->IsA(AFortCreativeMoveTool::StaticClass()))
{
    return nullptr;
}
AController* InstigatorController = AFortCreativeMoveTool::NativeGetInstigatorController143f76650(static_cast<AFortCreativeMoveTool*>(MoveToolObject));
if (InstigatorController == nullptr)
{
    return nullptr;
}
if (!InstigatorController->IsA(AFortPlayerController::StaticClass()))
{
    return nullptr;
}
return static_cast<AFortPlayerController*>(InstigatorController);
