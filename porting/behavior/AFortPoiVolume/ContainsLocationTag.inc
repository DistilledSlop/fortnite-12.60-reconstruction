// CONFIDENCE: high
// Native body at 0x141a3eb60 checks Tag validity, then checks LocationTags GameplayTags and ParentTags arrays.
return Tag.IsValid() && LocationTags.HasTag(Tag);
