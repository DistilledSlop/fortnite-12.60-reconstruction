// CONFIDENCE: LOW. Exec thunk 0x14291fa30 reads one byte (the message type) and fills the remaining fields of the request with defaults: an empty FText (copy of the static default at 0x142c0c240), Delay 0.0, Duration 5.0 (bit pattern 0x40a00000), and zeroed flags. It then calls native handler 0x14199a760 (deps AFortGameStateAthena__HandleGameModeMessageNative).
// Original address: 0x14291fa30 (exec). The struct layout is partly recovered; field names below are proposed.
FGameModeMessageRequest Request;
Request.MessageType = MessageData;
Request.Text = FText::GetEmpty();
Request.Delay = 0.0f;
Request.Duration = 5.0f;
Request.bFlag2C = false;
HandleGameModeMessageNative(Request);
