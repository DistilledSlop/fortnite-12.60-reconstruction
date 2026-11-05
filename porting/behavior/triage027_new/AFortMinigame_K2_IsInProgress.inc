// CONFIDENCE: HIGH; direct captured wrapper, EFortMinigameExec Yes=0, No=1.
OutExec = static_cast<uint8>(static_cast<uint8>(CurrentState) - 1) < 5
    ? EFortMinigameExec::Yes : EFortMinigameExec::No;
