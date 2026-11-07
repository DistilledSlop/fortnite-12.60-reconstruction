// Original body 0x141c84120 (thunk 0x142b37d60). Offset evidence: bShowPublishWatermark is bit 0x2 of the byte at +0x2d8 (declared bitfield member); delegate OnShowPublishWatermarkChanged at +0x550 (FOnShowPublishWatermarkChanged(AFortVolume* Volume, bool bShowPublishWatermark)); packed parameters are {this, bool}.
// CONFIDENCE: HIGH
OnShowPublishWatermarkChanged.Broadcast(this, bShowPublishWatermark);
