// CONFIDENCE: HIGH; thunk 0x142a7fe40 calls impl 0x14234b520, which broadcasts the global balloon-rope delegate (DAT_1478de5a8) via FMulticastScriptDelegate::Broadcast 0x140aae720 with (pawn, new rope, old rope), then stores the new rope at pawn+0x15d0 (BalloonRope).
OnBalloonRopeChanged.Broadcast(this, InBalloonRope, BalloonRope);
BalloonRope = InBalloonRope;
