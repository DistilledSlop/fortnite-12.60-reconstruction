// Behaviour snippet: UFortMediaSubtitlesPlayer::SetSubtitles (body only; spliced into the generated member function).
// Original address: 0x141226a70
// Evidence: asm MOV qword [RCX+0x30],RDX; RET. Member SourceSubtitles (Object) at +0x30 (reflection offset 48). No header in porting/native for this class, so the name is from reflection only.
// CONFIDENCE: MEDIUM
SourceSubtitles = Subtitles;
