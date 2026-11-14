// CONFIDENCE: HIGH. Original1423240e0; controller component lookup stays virtual.
if (Controller && NativeNetMode143f7a920() != NM_DedicatedServer)
{
    return static_cast<UFortControllerComponent_SkydiveFeedback*>(
        Controller->FindComponentByClass(UFortControllerComponent_SkydiveFeedback::StaticClass()));
}
return nullptr;
