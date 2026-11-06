// CONFIDENCE: HIGH
// 0x144ccc560 uses 0x142568f80 to resolve the theater map viewer.
UWorld* World = GetWorld();
const AFortGameModeFrontEnd* Mode = World ? Cast<AFortGameModeFrontEnd>(World->GetAuthGameMode()) : nullptr;
const AFortTheaterMapViewer* Viewer = Mode ? Mode->TheaterMapViewer : nullptr;
if (Viewer && Viewer->CurrentFocusedTile)
{
    return Viewer->CurrentFocusedTile;
}
World = GetWorld();
Mode = World ? Cast<AFortGameModeFrontEnd>(World->GetAuthGameMode()) : nullptr;
Viewer = Mode ? Mode->TheaterMapViewer : nullptr;
return Viewer ? Viewer->CurrentSelectedTile : nullptr;
