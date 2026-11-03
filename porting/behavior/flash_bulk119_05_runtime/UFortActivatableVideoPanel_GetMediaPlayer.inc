// CONFIDENCE: high
// Native 0x144f5e210 dereferences the widget without a null guard.
UFortVideoPlayerWidget* Widget = VideoPlayerWidget;
return Widget->GetMediaPlayer();
