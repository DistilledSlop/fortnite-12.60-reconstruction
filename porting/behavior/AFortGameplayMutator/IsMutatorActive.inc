// CONFIDENCE: high
// Native body at 0x1420e5530 checks bMutatorActive and bMutatorActiveInternal bitfields; returns true if either bit is set.
return (bMutatorActive != 0) || (bMutatorActiveInternal != 0);
