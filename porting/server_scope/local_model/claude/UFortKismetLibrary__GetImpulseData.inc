// CONFIDENCE: LOW. Original exec 0x14296e030; body 0x141ef4220. Reads the Fort effect context attached to the handle (0x1415f4460).
// ImpulseDirection is the normalized trace (End - Start) when the context has a trace. When the instigator is an ABuildingTrap, the direction is replaced by that instigator's root component location (or (0,1,0) when it has no root).
// KnockbackMagnitude and KnockbackZAngle are copied from the context. Outputs are left unchanged when there is no Fort effect context.
const FFortEffectContext* FortContext = GetFortEffectContext(EffectContext);
if (FortContext == nullptr)
{
	return;
}

const AActor* Instigator = Cast<AActor>(EffectContext.GetInstigator());
const bool bInstigatorIsBuildingTrap = Instigator != nullptr && Instigator->IsA(ABuildingTrap::StaticClass());

if (const FFortImpulseTrace* Trace = FortContext->GetImpulseTrace())
{
	ImpulseDirection = (Trace->End - Trace->Start).GetSafeNormal();
}

if (bInstigatorIsBuildingTrap)
{
	const USceneComponent* Root = Instigator->GetRootComponent();
	ImpulseDirection = Root != nullptr ? Root->GetComponentLocation() : FVector(0.0f, 1.0f, 0.0f);
}

KnockbackMagnitude = FortContext->KnockbackMagnitude;
KnockbackZAngle = FortContext->KnockbackZAngle;
