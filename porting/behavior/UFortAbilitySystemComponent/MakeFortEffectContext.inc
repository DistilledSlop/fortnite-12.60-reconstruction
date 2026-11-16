// Original address: 0x0 (retail native not located; the original BP-callable body is unknown).
// CONFIDENCE: low
// stand-in: declares the native so cooked Blueprints resolve it; Private/FortBlueprintNatives.cpp rebinds the exec at runtime.
// Default result if the rebind is not installed:
return {};
