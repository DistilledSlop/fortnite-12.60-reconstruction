// Behaviour snippet: UFortCreativeCostComponent::SetItemResourceCost (body only; spliced into the generated member function).
// Original address: 0x141d90cd0
// Evidence: MOV RAX,[RCX+0xb8] = reflected ItemCosts (TArray<int32>, UFortCreativeCostComponent.h) data pointer; MOV dword [RAX+RDX*4],R8D stores Cost at index ResourceType.
// CONFIDENCE: HIGH
ItemCosts[static_cast<int32>(ResourceType)] = Cost;
