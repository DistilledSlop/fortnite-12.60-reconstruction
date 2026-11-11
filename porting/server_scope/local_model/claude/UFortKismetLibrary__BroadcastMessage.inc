// CONFIDENCE: LOW. Copies Message and sends it through the world-context message dispatch. The native presentation fields are fixed constants (two 32.0 scale values, a 6.0 duration, priority 3, channel 0) and are set the same way here.
// Original address: 0x142967320 (exec). Native helpers: 0x141ee7cc0 (message build), 0x142c0c240 (text copy), 0x141881700 (dispatch).
FFortBroadcastMessage BroadcastData;
BroadcastData.Text = Message;
BroadcastData.Scale = FVector2D(32.0f, 32.0f);
BroadcastData.Duration = 6.0f;
BroadcastData.Priority = 3;
BroadcastData.Channel = 0;
BroadcastFortMessage(WorldContextObject, BroadcastData);
