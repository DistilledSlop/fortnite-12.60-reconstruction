// Construct at native module initialization, matching the deferred native adder.
static FStartupRootNativeTags GStartupRootNativeTags;
FStartupRootNativeTags& FStartupRootNativeTags::Get()
{
	return GStartupRootNativeTags;
}
