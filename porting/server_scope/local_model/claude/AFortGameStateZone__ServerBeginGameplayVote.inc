// CONFIDENCE: HIGH. Original 0x14292ca30. The exec thunk reads VoteType (byte), PlayerNetId and VoteConfig, then calls Stub_Empty (0x1406801c0). The native body is empty.
// The VoteConfig local is default-initialised by the thunk (values 2, 45.0f, 0, 1, true) before the frame read; nothing consumes it.
