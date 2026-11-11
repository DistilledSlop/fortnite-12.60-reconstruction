// CONFIDENCE: HIGH
// Native 0x14296bfe0 -> 0x140738030: the native body only default-constructs the result array. The Controller argument
// is consumed by the thunk but never read, so the result is always empty.
return {};
