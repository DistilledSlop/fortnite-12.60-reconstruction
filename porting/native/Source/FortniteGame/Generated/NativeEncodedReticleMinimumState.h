#pragma once
#include "CoreMinimal.h"

// Semantic portable representation of original +0xe08..+0xe0d.
// Place in a generator-supported shared header, not as a reflected USTRUCT.
struct FNativeEncodedReticleMinimumState
{
    uint8 Slots[4] = {0, 0, 0, 0};
    uint8 Index = 0;
    uint8 Key = 0x4e;

    FNativeEncodedReticleMinimumState() { Set(true); }

    bool Get() const { return Slots[Index] != static_cast<uint8>(Key | 9); }

    void Set(bool Value)
    {
        Key = static_cast<uint8>(Key + 11);
        Index = static_cast<uint8>((Index + 1) & 3);
        Slots[Index] = static_cast<uint8>((Key | 9) ^ static_cast<uint8>(Value));
    }

    void CopyFrom(const FNativeEncodedReticleMinimumState& Source)
    {
        // Read before advancing this instance, including the self-copy case.
        const uint8 Encoded = Source.Slots[Source.Index];
        const uint8 SourceKey = Source.Key;
        Key = static_cast<uint8>(Key + 11);
        Index = static_cast<uint8>((Index + 1) & 3);
        Slots[Index] = static_cast<uint8>((Key | 9) ^ (SourceKey | 9) ^ Encoded);
    }
};
