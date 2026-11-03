#include "FortniteMapClasses.h"
#include "UFortGameInstance.h"
#include "SharedFortGameLog.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

void AFortSkydivingShadowProxy::ShowShadowProxy_Implementation() {}
void AFortSkydivingShadowProxy::HideShadowProxy_Implementation() {}

AFortSkydivingShadowProxy::AFortSkydivingShadowProxy()
{
    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = DefaultSceneRoot;
}

bool UFortKismetLibrary::IsMobilePlatform()
{
#if PLATFORM_ANDROID || PLATFORM_IOS
    return true;
#else
    return false;
#endif
}

bool UFortKismetLibrary::IsMobileGame(const UObject* WorldContextObject)
{
    return false;
}

void UFortKismetLibrary::ExecuteClientEvent(UObject* WorldContextObject, FFortClientEventName EventType,
    UObject* EventSource, UObject* EventFocus)
{
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    UFortGameInstance* Instance = World ? Cast<UFortGameInstance>(World->GetGameInstance()) : nullptr;
    if (Instance)
        Instance->NativeExecuteClientEvent141e59c80(EventType, EventSource, EventFocus);
    else
        NativeLogClientEventWorldFailure141eece7a();
}

float UFortKismetLibrary::GetAngleDegrees(const FVector& Source, const FVector& Target)
{
    // CONFIDENCE: HIGH
    // Native 0x141ef0300: the initial unordered comparison retains NaN vectors.
    auto Normalize = [](const FVector& V) -> FVector
    {
        const float SquareSum = V.X * V.X + V.Y * V.Y + V.Z * V.Z;
        if (!(SquareSum < 1.0f) && !(SquareSum > 1.0f)) return V;
        if (SquareSum >= 1.0e-8f)
        {
            const float Scale = FMath::InvSqrt(SquareSum);
            return FVector(V.X * Scale, V.Y * Scale, V.Z * Scale);
        }
        return FVector::ZeroVector;
    };
    const FVector A = Normalize(Source);
    const FVector B = Normalize(Target);
    const float Dot = B.Y * A.Y + B.X * A.X + B.Z * A.Z;
    const float Clamped = !(Dot >= -1.0f) ? -1.0f : (Dot >= 1.0f ? 1.0f : Dot);
    return FMath::Acos(Clamped) * 57.295776f;
}

FVector UFortKismetLibrary::K2_GetClosestAxisXY(FVector InVector)
{
    // CONFIDENCE: HIGH
    // Native 0x141efa100: four successive quarter-turns, with a strict dot threshold.
    FVector Axis(1.0f, 0.0f, 0.0f);
    for (int32 I = 0; I < 4; ++I)
    {
        if (InVector.X * Axis.X + InVector.Y * Axis.Y + InVector.Z * Axis.Z > 0.7071)
            return Axis;
        const float X = Axis.X;
        const float Y = Axis.Y;
        const float Z = Axis.Z;
        // Preserve the original rotation matrix operations, including zero products.
        Axis.X = -1.1920929e-7f * X + (0.0f - 1.0f) * Y + (0.0f + 0.0f) * Z;
        Axis.Y = -1.1920929e-7f * Y + (0.0f + 1.0f) * X + (0.0f - 0.0f) * Z;
        Axis.Z = (1.0000001f - 1.1920929e-7f) * Z + (0.0f - 0.0f) * X + (0.0f + 0.0f) * Y;
    }
    return InVector;
}

FVector UFortKismetLibrary::NormalizedAngleInDegreesToVector(float A)
{
    // CONFIDENCE: HIGH
    // Native 0x141efb390 uses the engine's scalar SinCos approximation.
    float Sine, Cosine;
    FMath::SinCos(&Sine, &Cosine, A * 0.017453292f);
    return FVector(Cosine, Sine, 0.0f);
}

float UFortKismetLibrary::VectorToNormalizedAngleInDegrees(FVector V)
{
    // CONFIDENCE: HIGH
    // Native 0x141f0ac50 uses Unreal's Atan2 and NormalizeAxis implementations.
    return FRotator::NormalizeAxis(FMath::Atan2(V.Y, V.X) * 57.295776f);
}

// Native 0x140df1140; capture evidence in utility_leaf357_runtime.
bool UFortKismetLibrary::IsValid_UniqueNetIdRepl(const FUniqueNetIdRepl& InUniqueNetIdRepl)
{
    return InUniqueNetIdRepl.IsValid();
}


// Native 0x141f8f0a0; capture evidence in utility_leaf357_runtime.
void UFortKismetLibrary::InvalidateLatentActionHandle(FFortDialogExternalLatentActionHandle& Handle)
{
    Handle.Handle = -1;
}


// Native 0x141f8f570; capture evidence in utility_leaf357_runtime.
bool UFortKismetLibrary::IsLatentActionHandleValid(const FFortDialogExternalLatentActionHandle& Handle)
{
    return Handle.Handle >= 0;
}


// Native 0x141ef9370; capture evidence in utility_leaf357_runtime.
bool UFortKismetLibrary::IsAssetNull(TSoftObjectPtr<UObject> Asset)
{
    return Asset.IsNull();
}


// Native 0x141ef9370: the engine path null check tests the complete None name.
bool UFortKismetLibrary::IsAssetNull(TSoftObjectPtr<UObject> Asset)
{
    return Asset.IsNull();
}
