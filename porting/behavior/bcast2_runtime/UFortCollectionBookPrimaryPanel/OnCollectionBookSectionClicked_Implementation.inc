// Behaviour snippet: UFortCollectionBookPrimaryPanel::OnCollectionBookSectionClicked_Implementation (body only; spliced into the generated member function).
// Original body: 0x1450c3890 (thunk 0x14286c850); native body is a broadcast through ProcessMulticastDelegate 0x1406a9dc0.
// Offset evidence:
//   Asm: MOV qword [RCX+0x478],RDX stores the section into LastClickedSection (reflected Object at +0x478).
//   ADD RCX,0x430 -> OnCollectionBookSectionClickedDelegate (+0x430), FOnBookSectionClicked(UFortCollectionBookSection*).
// CONFIDENCE: HIGH
LastClickedSection = ClickedSection;
OnCollectionBookSectionClickedDelegate.Broadcast(ClickedSection);
