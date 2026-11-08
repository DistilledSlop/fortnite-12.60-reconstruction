// Original body address: 0x143f76650 (helper, no thunk)
// Offset evidence: +0x118 AActor::Instigator (reflected Object, read as APawn*); +0x250 APawn::Controller (reflected Object) read only when Instigator is non-null.
// Asm: load Instigator, return null when it is zero, otherwise return Instigator->Controller.
// Callers (7) include AdjustOptionsBasedOnSelectionProperty, which compares the result against a class before use.
// CONFIDENCE: HIGH
APawn* InstigatorPawn = MoveTool->GetInstigator();
if (InstigatorPawn == nullptr)
{
	return nullptr;
}
return InstigatorPawn->GetController();
