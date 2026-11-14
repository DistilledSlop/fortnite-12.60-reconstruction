#pragma once

#include "CoreMinimal.h"
#include "AudioDevice.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Sound/SoundBase.h"

namespace FortWeaponNative
{
// Synthetic call-site adapter for the complete playback effect used by 1426b60f0.
// It receives no original body/address credit for the general 144224f50 function.
inline void PlayStopSound(UObject* WorldContext, USoundBase* Sound, const FVector& Location)
{
    if (!Sound || !GEngine || !GEngine->UseSound())
        return;
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull);
    if (!World || !World->bAllowAudioPlayback)
        return;
    if (FAudioDeviceHandle Device = World->GetAudioDevice())
        Device->PlaySoundAtLocation(Sound, World, 1.0f, 1.0f, 0.0f, Location,
            FRotator::ZeroRotator, nullptr, nullptr, nullptr, nullptr);
}
}
