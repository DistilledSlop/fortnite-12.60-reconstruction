// CONFIDENCE: high; complete native capture and reflected data contracts.
// Native 0x1420bfab0 checks each explicit query tag against explicit and parent tags.
if (TagsToCheck.IsEmpty())
{
    return bCountEmptyAsMatch;
}
return bRequireAll ? ContainerToCheck.HasAll(TagsToCheck) : ContainerToCheck.HasAny(TagsToCheck);
