// Behaviour snippet: UCommonInputManager::GetTopPanel (body only; spliced into the generated member function).
// Original address: 0x1408100d0
// Source: local-model fn runner; static checks only, not compiled or runtime verified.
// CONFIDENCE: HIGH
if (ActivatablePanelStack.Num() < 1)
{
	return nullptr;
}
return ActivatablePanelStack.Last();
