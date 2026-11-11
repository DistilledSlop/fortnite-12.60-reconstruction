// CONFIDENCE: MEDIUM. Original exec 0x14296e2a0; body 0x141ef4690. Resolves the world through the context object's GetWorld (virtual slot 0x150) and returns UWorld::IsPlayingReplay (demo net driver present and playing; body 0x144693a30).
// Returns false when the context object or its world is null.
if (WorldContextObject == nullptr)
{
	return false;
}

UWorld* World = WorldContextObject->GetWorld();
if (World == nullptr)
{
	return false;
}

return World->IsPlayingReplay();
