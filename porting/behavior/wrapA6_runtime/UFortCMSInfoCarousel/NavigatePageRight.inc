// Behaviour snippet: UFortCMSInfoCarousel::NavigatePageRight (wrapper, body only).
// Original body address: 0x144d8c660 (thunk 0x14520ce70), this_delta_total 0.
// Offset evidence: MOV RCX,[RCX+0x498] loads Carousel (reflected Object, exact offset 0x498; declared UCommonWidgetCarousel* in FortniteUI/Generated/UFortCMSInfoCarousel.h). The body reloads [this+0x498] after the first call.
// Callee 1: 0x1407e2570 -> UCommonWidgetCarousel::PreviousPage() (CommonUI engine declaration, CommonWidgetCarousel.h); the body calls PreviousPage, not NextPage.
// Callee 2: 0x1407bb5b0 -> UCommonWidgetCarousel::EndAutoScrolling() (CommonUI engine declaration, CommonWidgetCarousel.h); JMP is a tail call.
// CONFIDENCE: MEDIUM
Carousel->PreviousPage();
Carousel->EndAutoScrolling();
