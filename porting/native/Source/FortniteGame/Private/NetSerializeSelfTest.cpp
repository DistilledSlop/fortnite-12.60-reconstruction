// Runtime self-test for the recovered struct_net NetSerialize bodies (porting/behavior/README.md, "Native NetSerialize").
// Hand-written, not generated. It runs only when the process starts with -NetSerializeSelfTest: FFortniteGameModule::
// StartupModule (FortniteGame.cpp) calls RunNetSerializeSelfTest, which logs one line per case and exits with 0 when
// every case passes and 1 otherwise. No network, level or RHI is needed.
//
// Each struct is serialized with FBitWriter, read back with FBitReader on the written bits, and compared with the wire
// rules of porting/decompilation_notes.md (Eleventh pass) and with the engine's FRepMovement, FRotator and packed vector
// code. A failing case is reported as it is. Never adjust a body or a check here to make a case pass.

#include "CoreMinimal.h"
#include "HAL/PlatformMisc.h"
#include "Serialization/BitReader.h"
#include "Serialization/BitWriter.h"

#include "FCreateBuildingActorData.h"
#include "FFortAIAttributeReplicationProxy.h"
#include "FFortAISharedRepMovement.h"
#include "FReplicatedPhysicsPawnState.h"
#include "FSharedRepMovement.h"

DEFINE_LOG_CATEGORY_STATIC(LogNetSerializeSelfTest, Log, All);

namespace
{
    // SyncKey step of the 13x quantizer (the sync-key writer in the Eleventh pass notes).
    const float SyncKeyStep = 0.07692308f;
    // Largest and smallest SyncKey the 24-bit field holds: (value - 0x800000) / 13.
    const float SyncKeyMax = 8388607.0f / 13.0f;
    const float SyncKeyMin = -8388608.0f / 13.0f;

    // Quantized vector and rotator inputs. Position values cover the packed-vector range at every level.
    const float PositionValues[] = { 0.0f, 1000.5f, -250000.25f };
    const float RotationValues[] = { 0.0f, 90.0f, -90.0f, 180.0f };

    struct FSelfTestState
    {
        int32 Cases = 0;
        int32 Failures = 0;
    };

    // Logs one line per case. The detail is only written on FAIL.
    void ReportCase(FSelfTestState& State, const TCHAR* StructName, const TCHAR* CaseName, const FString& Failure)
    {
        State.Cases++;
        if (Failure.IsEmpty())
        {
            UE_LOG(LogNetSerializeSelfTest, Display, TEXT("NetSerializeSelfTest: %s %s PASS"), StructName, CaseName);
        }
        else
        {
            State.Failures++;
            UE_LOG(LogNetSerializeSelfTest, Error, TEXT("NetSerializeSelfTest: %s %s FAIL %s"), StructName, CaseName, *Failure);
        }
    }

    FString FirstFailure(const FString& Earlier, const FString& Next)
    {
        return Earlier.IsEmpty() ? Next : Earlier;
    }

    FString CheckFloat(const FString& Field, float Expected, float Loaded, float Tolerance)
    {
        if (FMath::Abs(Expected - Loaded) <= Tolerance)
        {
            return FString();
        }
        return FString::Printf(TEXT("%s expected %.6f got %.6f (tolerance %.6f)"), *Field, Expected, Loaded, Tolerance);
    }

    FString CheckInt(const FString& Field, int64 Expected, int64 Loaded)
    {
        if (Expected == Loaded)
        {
            return FString();
        }
        return FString::Printf(TEXT("%s expected %lld got %lld"), *Field, Expected, Loaded);
    }

    // Angles are equal modulo 360 (a rotator component of 180 or -90 comes back as 180 or 270).
    FString CheckAngle(const FString& Field, float Expected, float Loaded, float Tolerance)
    {
        float Distance = FMath::Fmod(FMath::Abs(Expected - Loaded), 360.0f);
        Distance = FMath::Min(Distance, 360.0f - Distance);
        if (Distance <= Tolerance)
        {
            return FString();
        }
        return FString::Printf(TEXT("%s expected %.6f got %.6f (tolerance %.6f, angle distance %.6f)"),
            *Field, Expected, Loaded, Tolerance, Distance);
    }

    FString CheckVector(const TCHAR* Field, const FVector& Expected, const FVector& Loaded, float Tolerance)
    {
        FString Failure = CheckFloat(FString::Printf(TEXT("%s.X"), Field), Expected.X, Loaded.X, Tolerance);
        Failure = FirstFailure(Failure, CheckFloat(FString::Printf(TEXT("%s.Y"), Field), Expected.Y, Loaded.Y, Tolerance));
        Failure = FirstFailure(Failure, CheckFloat(FString::Printf(TEXT("%s.Z"), Field), Expected.Z, Loaded.Z, Tolerance));
        return Failure;
    }

    // Float resolution (2^-23, relative). The packed-vector decode is (float)(int) / ScaleFactor: above 2^24 quantization
    // units a float cannot hold every integer, so a decoded value can sit one float resolution beyond the step.
    const float FloatResolution = 1.1920929e-7f;

    // Packed vector: each component within one quantization step of the input, plus the float resolution of its magnitude.
    FString CheckPackedVector(const TCHAR* Field, const FVector& Expected, const FVector& Loaded, float Step)
    {
        FString Failure = CheckFloat(FString::Printf(TEXT("%s.X"), Field), Expected.X, Loaded.X,
            Step + FMath::Abs(Expected.X) * FloatResolution);
        Failure = FirstFailure(Failure, CheckFloat(FString::Printf(TEXT("%s.Y"), Field), Expected.Y, Loaded.Y,
            Step + FMath::Abs(Expected.Y) * FloatResolution));
        Failure = FirstFailure(Failure, CheckFloat(FString::Printf(TEXT("%s.Z"), Field), Expected.Z, Loaded.Z,
            Step + FMath::Abs(Expected.Z) * FloatResolution));
        return Failure;
    }

    // One quantization step of a packed vector. The engine rounds each component to the nearest multiple of
    // 1/ScaleFactor, so the error is at most half a step; the check allows one full step (see CheckPackedVector).
    float VectorStep(EVectorQuantization Level)
    {
        switch (Level)
        {
        case EVectorQuantization::RoundTwoDecimals:
            return 0.01f;
        case EVectorQuantization::RoundOneDecimal:
            return 0.1f;
        default:
            return 1.0f;
        }
    }

    // One step of the rotator compression: 8 or 16 bits per component over a full turn.
    float RotatorStep(ERotatorQuantization Level)
    {
        return Level == ERotatorQuantization::ShortComponents ? 360.0f / 65536.0f : 360.0f / 256.0f;
    }

    // FRepMovement::NetSerialize (engine), as called by the recovered bodies. AngularVelocity is on the wire only
    // when bRepPhysics is set; without it the receiver keeps zero.
    FString CheckRepMovement(const FRepMovement& Expected, const FRepMovement& Loaded)
    {
        FString Failure = CheckInt(TEXT("RepMovement.bSimulatedPhysicSleep"), Expected.bSimulatedPhysicSleep, Loaded.bSimulatedPhysicSleep);
        Failure = FirstFailure(Failure, CheckInt(TEXT("RepMovement.bRepPhysics"), Expected.bRepPhysics, Loaded.bRepPhysics));

        const float LocationTolerance = VectorStep(Expected.LocationQuantizationLevel);
        const float VelocityTolerance = VectorStep(Expected.VelocityQuantizationLevel);
        const float RotationTolerance = RotatorStep(Expected.RotationQuantizationLevel);

        Failure = FirstFailure(Failure, CheckPackedVector(TEXT("RepMovement.Location"), Expected.Location, Loaded.Location, LocationTolerance));
        Failure = FirstFailure(Failure, CheckAngle(TEXT("RepMovement.Rotation.Pitch"), Expected.Rotation.Pitch, Loaded.Rotation.Pitch, RotationTolerance));
        Failure = FirstFailure(Failure, CheckAngle(TEXT("RepMovement.Rotation.Yaw"), Expected.Rotation.Yaw, Loaded.Rotation.Yaw, RotationTolerance));
        Failure = FirstFailure(Failure, CheckAngle(TEXT("RepMovement.Rotation.Roll"), Expected.Rotation.Roll, Loaded.Rotation.Roll, RotationTolerance));
        Failure = FirstFailure(Failure, CheckPackedVector(TEXT("RepMovement.LinearVelocity"), Expected.LinearVelocity, Loaded.LinearVelocity, VelocityTolerance));

        const FVector ExpectedAngular = Expected.bRepPhysics ? Expected.AngularVelocity : FVector::ZeroVector;
        Failure = FirstFailure(Failure, CheckPackedVector(TEXT("RepMovement.AngularVelocity"), ExpectedAngular, Loaded.AngularVelocity, VelocityTolerance));
        return Failure;
    }

    // The quantization levels are not on the wire, so the receiving struct holds the same levels as the sender.
    void ConfigureReceiver(FRepMovement& Receiver, const FRepMovement& Sender)
    {
        Receiver.LocationQuantizationLevel = Sender.LocationQuantizationLevel;
        Receiver.VelocityQuantizationLevel = Sender.VelocityQuantizationLevel;
        Receiver.RotationQuantizationLevel = Sender.RotationQuantizationLevel;
    }

    void ConfigureReceiver(FSharedRepMovement& Receiver, const FSharedRepMovement& Sender)
    {
        Receiver = FSharedRepMovement();
        ConfigureReceiver(Receiver.RepMovement, Sender.RepMovement);
    }

    void ConfigureReceiver(FFortAISharedRepMovement& Receiver, const FFortAISharedRepMovement& Sender)
    {
        Receiver = FFortAISharedRepMovement();
        ConfigureReceiver(Receiver.RepMovement, Sender.RepMovement);
    }

    // The other structs have no quantization levels; a fresh receiver is enough.
    template <typename TStruct>
    void ConfigureReceiver(TStruct& Receiver, const TStruct& Sender)
    {
        (void)Sender;
        Receiver = TStruct();
    }

    // Serializes Source with FBitWriter, then reads the written bits back with FBitReader into Loaded.
    // Fails when either direction reports failure or when the reader does not consume exactly the written bits.
    template <typename TStruct>
    bool RoundTrip(const TStruct& Source, TStruct& Loaded, int64& OutBits, FString& OutFailure)
    {
        FBitWriter Writer(8192, true);
        TStruct Written = Source;
        bool bSaveSuccess = true;
        const bool bSaveReturn = Written.NetSerialize(Writer, nullptr, bSaveSuccess);
        if (!Writer.IsSaving() || Writer.IsError() || !bSaveReturn || !bSaveSuccess)
        {
            OutFailure = TEXT("save returned failure");
            return false;
        }
        OutBits = Writer.GetNumBits();
        TArray<uint8> Bytes = *Writer.GetBuffer();

        FBitReader Reader(Bytes.GetData(), OutBits);
        bool bLoadSuccess = true;
        const bool bLoadReturn = Loaded.NetSerialize(Reader, nullptr, bLoadSuccess);
        if (!Reader.IsLoading() || Reader.IsError() || !bLoadReturn || !bLoadSuccess)
        {
            OutFailure = TEXT("load returned failure");
            return false;
        }
        if (Reader.GetPosBits() != OutBits)
        {
            OutFailure = FString::Printf(TEXT("load consumed %lld of %lld written bits"), Reader.GetPosBits(), OutBits);
            return false;
        }
        return true;
    }

    // Round trip of Sent, then Check(Sent, Loaded). Returns an empty string on success. FixedBits >= 0 is the bit count
    // the layout requires; it must match exactly.
    template <typename TStruct, typename TCheck>
    FString ExpectRoundTrip(const TStruct& Sent, TCheck Check, int64 FixedBits)
    {
        TStruct Loaded = TStruct();
        ConfigureReceiver(Loaded, Sent);
        int64 Bits = 0;
        FString Failure;
        if (!RoundTrip(Sent, Loaded, Bits, Failure))
        {
            return Failure;
        }
        if (FixedBits >= 0 && Bits != FixedBits)
        {
            return FString::Printf(TEXT("wrote %lld bits, the layout has %lld"), Bits, FixedBits);
        }
        return Check(Sent, Loaded);
    }

    // FSharedRepMovement: the ten bools in ascending offset order (+0x4b .. +0x54), each a 32-bit UBOOL.
    struct FSharedBoolField
    {
        const TCHAR* Name;
        bool FSharedRepMovement::* Member;
    };

    const FSharedBoolField SharedBoolFields[] = {
        { TEXT("bProxyIsJumpForceApplied"), &FSharedRepMovement::bProxyIsJumpForceApplied },
        { TEXT("bIsCrouched"), &FSharedRepMovement::bIsCrouched },
        { TEXT("bIsSkydiving"), &FSharedRepMovement::bIsSkydiving },
        { TEXT("bIsParachuteOpen"), &FSharedRepMovement::bIsParachuteOpen },
        { TEXT("bIsSlopeSliding"), &FSharedRepMovement::bIsSlopeSliding },
        { TEXT("bIsProxySimulationTimedOut"), &FSharedRepMovement::bIsProxySimulationTimedOut },
        { TEXT("bIsTargeting"), &FSharedRepMovement::bIsTargeting },
        { TEXT("bIsWaterJump"), &FSharedRepMovement::bIsWaterJump },
        { TEXT("bIsWaterSprintBoost"), &FSharedRepMovement::bIsWaterSprintBoost },
        { TEXT("bIsWaterSprintBoostPending"), &FSharedRepMovement::bIsWaterSprintBoostPending },
    };

    void SetSharedBools(FSharedRepMovement& Sent, uint32 Mask)
    {
        for (int32 Bit = 0; Bit < static_cast<int32>(UE_ARRAY_COUNT(SharedBoolFields)); ++Bit)
        {
            Sent.*(SharedBoolFields[Bit].Member) = ((Mask >> Bit) & 1) != 0;
        }
    }

    FString CheckSharedRepMovement(const FSharedRepMovement& Expected, const FSharedRepMovement& Loaded)
    {
        FString Failure = CheckRepMovement(Expected.RepMovement, Loaded.RepMovement);
        Failure = FirstFailure(Failure, CheckInt(TEXT("RepMovementMode"), Expected.RepMovementMode, Loaded.RepMovementMode));
        Failure = FirstFailure(Failure, CheckInt(TEXT("JumpFlashCountPacked"), Expected.JumpFlashCountPacked, Loaded.JumpFlashCountPacked));
        Failure = FirstFailure(Failure, CheckInt(TEXT("LandingFlashCountPacked"), Expected.LandingFlashCountPacked, Loaded.LandingFlashCountPacked));
        Failure = FirstFailure(Failure, CheckInt(TEXT("CurrentMovementStyle"),
            static_cast<int64>(Expected.CurrentMovementStyle.GetValue()), static_cast<int64>(Loaded.CurrentMovementStyle.GetValue())));
        Failure = FirstFailure(Failure, CheckFloat(TEXT("RepTimeStamp"), Expected.RepTimeStamp, Loaded.RepTimeStamp, 0.0f));
        Failure = FirstFailure(Failure, CheckFloat(TEXT("TurretYaw"), Expected.TurretYaw, Loaded.TurretYaw, 0.0f));
        Failure = FirstFailure(Failure, CheckFloat(TEXT("TurretPitch"), Expected.TurretPitch, Loaded.TurretPitch, 0.0f));
        Failure = FirstFailure(Failure, CheckInt(TEXT("RemoteViewData32"), Expected.RemoteViewData32, Loaded.RemoteViewData32));
        Failure = FirstFailure(Failure, CheckInt(TEXT("AccelerationPack"), Expected.AccelerationPack, Loaded.AccelerationPack));
        Failure = FirstFailure(Failure, CheckInt(TEXT("AccelerationZPack"), Expected.AccelerationZPack, Loaded.AccelerationZPack));
        for (int32 Index = 0; Index < static_cast<int32>(UE_ARRAY_COUNT(SharedBoolFields)); ++Index)
        {
            const FSharedBoolField& Field = SharedBoolFields[Index];
            Failure = FirstFailure(Failure, CheckInt(Field.Name, Expected.*(Field.Member), Loaded.*(Field.Member)));
        }
        return Failure;
    }

    FString CheckFortAISharedRepMovement(const FFortAISharedRepMovement& Expected, const FFortAISharedRepMovement& Loaded)
    {
        FString Failure = CheckRepMovement(Expected.RepMovement, Loaded.RepMovement);
        Failure = FirstFailure(Failure, CheckInt(TEXT("RepMovementMode"), Expected.RepMovementMode, Loaded.RepMovementMode));
        Failure = FirstFailure(Failure, CheckInt(TEXT("RepCurrentFortAILODLevel"),
            static_cast<int64>(Expected.RepCurrentFortAILODLevel), static_cast<int64>(Loaded.RepCurrentFortAILODLevel)));
        Failure = FirstFailure(Failure, CheckFloat(TEXT("RepTimeStamp"), Expected.RepTimeStamp, Loaded.RepTimeStamp, 0.0f));
        Failure = FirstFailure(Failure, CheckInt(TEXT("RepSharedAnimInfo.AnimState"),
            static_cast<int64>(Expected.RepSharedAnimInfo.AnimState), static_cast<int64>(Loaded.RepSharedAnimInfo.AnimState)));
        Failure = FirstFailure(Failure, CheckInt(TEXT("RepSharedAnimInfo.MontageSectionToPlay"),
            Expected.RepSharedAnimInfo.MontageSectionToPlay, Loaded.RepSharedAnimInfo.MontageSectionToPlay));
        Failure = FirstFailure(Failure, CheckInt(TEXT("RepSharedAnimInfo.MontageIndex"),
            Expected.RepSharedAnimInfo.MontageIndex, Loaded.RepSharedAnimInfo.MontageIndex));
        return Failure;
    }

    // With MontageIndex -1 the bit is clear and the body loads the defaults (asm: 0xffffffff0000000b over +0x3c..+0x43).
    FString CheckNoMontageDefaults(const FFortAISharedRepMovement& Expected, const FFortAISharedRepMovement& Loaded)
    {
        FFortAISharedRepMovement Defaults = Expected;
        Defaults.RepSharedAnimInfo.AnimState = EFortSharedAnimationState::Anim_MAX;
        Defaults.RepSharedAnimInfo.MontageSectionToPlay = 0;
        Defaults.RepSharedAnimInfo.MontageIndex = -1;
        return CheckFortAISharedRepMovement(Defaults, Loaded);
    }

    FString CheckPhysicsState(const FReplicatedPhysicsPawnState& Expected, const FReplicatedPhysicsPawnState& Loaded)
    {
        // Raw floats and a raw uint16: every field must be exact.
        FString Failure = CheckVector(TEXT("Translation"), Expected.Translation, Loaded.Translation, 0.0f);
        Failure = FirstFailure(Failure, CheckFloat(TEXT("Rotation.X"), Expected.Rotation.X, Loaded.Rotation.X, 0.0f));
        Failure = FirstFailure(Failure, CheckFloat(TEXT("Rotation.Y"), Expected.Rotation.Y, Loaded.Rotation.Y, 0.0f));
        Failure = FirstFailure(Failure, CheckFloat(TEXT("Rotation.Z"), Expected.Rotation.Z, Loaded.Rotation.Z, 0.0f));
        Failure = FirstFailure(Failure, CheckFloat(TEXT("Rotation.W"), Expected.Rotation.W, Loaded.Rotation.W, 0.0f));
        Failure = FirstFailure(Failure, CheckVector(TEXT("LinearVelocity"), Expected.LinearVelocity, Loaded.LinearVelocity, 0.0f));
        Failure = FirstFailure(Failure, CheckVector(TEXT("AngularVelocity"), Expected.AngularVelocity, Loaded.AngularVelocity, 0.0f));
        Failure = FirstFailure(Failure, CheckInt(TEXT("SyncKey"), Expected.SyncKey, Loaded.SyncKey));
        return Failure;
    }

    FString CheckAttributeProxy(const FFortAIAttributeReplicationProxy& Expected, const FFortAIAttributeReplicationProxy& Loaded)
    {
        FString Failure = CheckInt(TEXT("Health"), Expected.Health, Loaded.Health);
        return FirstFailure(Failure, CheckInt(TEXT("MaxHealth"), Expected.MaxHealth, Loaded.MaxHealth));
    }

    // Layout: BuildingClassHandle 9 bits, bMirrored 32 bits, BuildLoc 3 raw floats, UpgradeLevel 8 bits, SyncKey 24 bits,
    // yaw bin 8 bits. BuildRot.Pitch and Roll are not on the wire, so they are not compared.
    FString CheckBuilding(const FCreateBuildingActorData& Expected, const FCreateBuildingActorData& Loaded,
        float ExpectedSync, float ExpectedYaw)
    {
        FString Failure = CheckInt(TEXT("BuildingClassHandle"), Expected.BuildingClassHandle, Loaded.BuildingClassHandle);
        Failure = FirstFailure(Failure, CheckInt(TEXT("bMirrored"), Expected.bMirrored, Loaded.bMirrored));
        Failure = FirstFailure(Failure, CheckVector(TEXT("BuildLoc"), Expected.BuildLoc, Loaded.BuildLoc, 0.0f));
        Failure = FirstFailure(Failure, CheckInt(TEXT("BuildingClassData.UpgradeLevel"),
            Expected.BuildingClassData.UpgradeLevel, Loaded.BuildingClassData.UpgradeLevel));
        Failure = FirstFailure(Failure, CheckFloat(TEXT("SyncKey"), ExpectedSync, Loaded.SyncKey, SyncKeyStep));
        Failure = FirstFailure(Failure, CheckFloat(TEXT("BuildRot.Yaw"), ExpectedYaw, Loaded.BuildRot.Yaw, 0.0f));
        return Failure;
    }

    // Yaw is compared to its bin exactly: the body loads one of 180, 90, 0 or -90.
    // Bits: 177 for the layout. SerializeInt(handle, 0x1ff) takes nine bits, except for 255: FBitWriter::SerializeInt stops
    // before the top bit when the lower eight bits are all ones (255 + 256 == 0x1ff), so eight bits are written.
    FString BuildingCase(const FCreateBuildingActorData& Sent, float ExpectedSync, float ExpectedYaw)
    {
        const int64 ExpectedBits = Sent.BuildingClassHandle == 255 ? 176 : 177;
        return ExpectRoundTrip(Sent, [ExpectedSync, ExpectedYaw](const FCreateBuildingActorData& Expected, const FCreateBuildingActorData& Loaded)
        {
            return CheckBuilding(Expected, Loaded, ExpectedSync, ExpectedYaw);
        }, ExpectedBits);
    }

    // Rotation grid: every combination of 0, 90, -90 and 180 for pitch, yaw and roll, with the byte and the short level.
    template <typename TStruct, typename TCheck>
    FString RepMovementRotationGrid(TCheck Check)
    {
        const ERotatorQuantization Levels[] = { ERotatorQuantization::ByteComponents, ERotatorQuantization::ShortComponents };
        for (const ERotatorQuantization Level : Levels)
        {
            for (const float Pitch : RotationValues)
            {
                for (const float Yaw : RotationValues)
                {
                    for (const float Roll : RotationValues)
                    {
                        TStruct Sent = TStruct();
                        Sent.RepMovement.RotationQuantizationLevel = Level;
                        Sent.RepMovement.Rotation = FRotator(Pitch, Yaw, Roll);
                        const FString Failure = ExpectRoundTrip(Sent, Check, -1);
                        if (!Failure.IsEmpty())
                        {
                            return FString::Printf(TEXT("%s rotation (%g, %g, %g): %s"),
                                Level == ERotatorQuantization::ByteComponents ? TEXT("byte") : TEXT("short"), Pitch, Yaw, Roll, *Failure);
                        }
                    }
                }
            }
        }
        return FString();
    }

    // Location and velocity grid: every combination of PositionValues at each vector level, bRepPhysics set so that the
    // angular velocity is on the wire too.
    template <typename TStruct, typename TCheck>
    FString RepMovementVectorGrid(TCheck Check)
    {
        const EVectorQuantization Levels[] = { EVectorQuantization::RoundWholeNumber, EVectorQuantization::RoundOneDecimal, EVectorQuantization::RoundTwoDecimals };
        for (const EVectorQuantization Level : Levels)
        {
            for (const float X : PositionValues)
            {
                for (const float Y : PositionValues)
                {
                    for (const float Z : PositionValues)
                    {
                        TStruct Sent = TStruct();
                        Sent.RepMovement.LocationQuantizationLevel = Level;
                        Sent.RepMovement.VelocityQuantizationLevel = Level;
                        Sent.RepMovement.bRepPhysics = true;
                        Sent.RepMovement.Location = FVector(X, Y, Z);
                        Sent.RepMovement.LinearVelocity = FVector(X, Y, Z);
                        Sent.RepMovement.AngularVelocity = FVector(Z, Y, X);
                        const FString Failure = ExpectRoundTrip(Sent, Check, -1);
                        if (!Failure.IsEmpty())
                        {
                            return FString::Printf(TEXT("level %d value (%g, %g, %g): %s"),
                                static_cast<int32>(Level), X, Y, Z, *Failure);
                        }
                    }
                }
            }
        }
        return FString();
    }

    // All 1024 combinations of the ten bools.
    FString SharedBoolGrid()
    {
        for (uint32 Mask = 0; Mask < 1024; ++Mask)
        {
            FSharedRepMovement Sent = FSharedRepMovement();
            SetSharedBools(Sent, Mask);
            const FString Failure = ExpectRoundTrip(Sent, &CheckSharedRepMovement, -1);
            if (!Failure.IsEmpty())
            {
                return FString::Printf(TEXT("bool mask 0x%03X: %s"), Mask, *Failure);
            }
        }
        return FString();
    }

    // Each optional field both zero and nonzero, and the two RepMovement flags. Turret yaw and pitch are set separately,
    // so a partly zero pair (one of the two set) also round trips.
    FString SharedOptionalFieldGrid()
    {
        for (uint32 Mask = 0; Mask < 128; ++Mask)
        {
            FSharedRepMovement Sent = FSharedRepMovement();
            Sent.RepMovement.bSimulatedPhysicSleep = (Mask & 1) != 0;
            Sent.RepMovement.bRepPhysics = (Mask & 2) != 0;
            Sent.RepMovement.AngularVelocity = FVector(3.0f, 4.0f, 5.0f);
            Sent.RepTimeStamp = (Mask & 4) ? 12.5f : 0.0f;
            Sent.AccelerationPack = (Mask & 8) ? 0x1234 : 0;
            Sent.AccelerationZPack = (Mask & 16) ? -5 : 0;
            Sent.TurretYaw = (Mask & 32) ? 45.0f : 0.0f;
            Sent.TurretPitch = (Mask & 64) ? -10.0f : 0.0f;
            const FString Failure = ExpectRoundTrip(Sent, &CheckSharedRepMovement, -1);
            if (!Failure.IsEmpty())
            {
                return FString::Printf(TEXT("optional mask 0x%02X: %s"), Mask, *Failure);
            }
        }
        return FString();
    }

    // EFortMovementStyle (unscoped, 0..5) with the byte fields at their extremes.
    FString SharedStyleGrid()
    {
        for (int32 Style = 0; Style <= 5; ++Style)
        {
            FSharedRepMovement Sent = FSharedRepMovement();
            Sent.CurrentMovementStyle = static_cast<EFortMovementStyle::Type>(Style);
            Sent.RepMovementMode = 255;
            Sent.JumpFlashCountPacked = 0;
            Sent.LandingFlashCountPacked = 255;
            const FString Failure = ExpectRoundTrip(Sent, &CheckSharedRepMovement, -1);
            if (!Failure.IsEmpty())
            {
                return FString::Printf(TEXT("style %d: %s"), Style, *Failure);
            }
        }
        return FString();
    }

    // Montage index edges: -2 is not -1, so the bit is set and the index is sent in full.
    FString FortAIMontageEdgeGrid()
    {
        const int32 Indexes[] = { 0, -2, 1, 0x7FFFFFFF };
        const EFortSharedAnimationState States[] = { EFortSharedAnimationState::Anim_Walk, EFortSharedAnimationState::Anim_RangedAttack };
        const uint8 Sections[] = { 0, 255 };
        for (const int32 Index : Indexes)
        {
            for (const EFortSharedAnimationState State : States)
            {
                for (const uint8 Section : Sections)
                {
                    FFortAISharedRepMovement Sent = FFortAISharedRepMovement();
                    Sent.RepSharedAnimInfo.MontageIndex = Index;
                    Sent.RepSharedAnimInfo.AnimState = State;
                    Sent.RepSharedAnimInfo.MontageSectionToPlay = Section;
                    const FString Failure = ExpectRoundTrip(Sent, &CheckFortAISharedRepMovement, -1);
                    if (!Failure.IsEmpty())
                    {
                        return FString::Printf(TEXT("MontageIndex %d AnimState %d Section %d: %s"),
                            Index, static_cast<int32>(State), static_cast<int32>(Section), *Failure);
                    }
                }
            }
        }
        return FString();
    }

    // Every byte of RepCurrentFortAILODLevel and RepMovementMode, paired so each value of one meets each of the other.
    FString FortAILodModeGrid()
    {
        for (int32 Value = 0; Value < 256; ++Value)
        {
            FFortAISharedRepMovement Sent = FFortAISharedRepMovement();
            Sent.RepCurrentFortAILODLevel = static_cast<EFortAILODLevel>(Value);
            Sent.RepMovementMode = static_cast<uint8>(255 - Value);
            const FString Failure = ExpectRoundTrip(Sent, &CheckFortAISharedRepMovement, -1);
            if (!Failure.IsEmpty())
            {
                return FString::Printf(TEXT("value %d: %s"), Value, *Failure);
            }
        }
        return FString();
    }

    // Raw translation, linear and angular velocity: every combination of PositionValues on the three axes.
    FString PhysicsAxisGrid()
    {
        for (const float X : PositionValues)
        {
            for (const float Y : PositionValues)
            {
                for (const float Z : PositionValues)
                {
                    FReplicatedPhysicsPawnState Sent = FReplicatedPhysicsPawnState();
                    Sent.Translation.X = X;
                    Sent.Translation.Y = Y;
                    Sent.Translation.Z = Z;
                    Sent.LinearVelocity.X = Z;
                    Sent.LinearVelocity.Y = Y;
                    Sent.LinearVelocity.Z = X;
                    Sent.AngularVelocity.X = X;
                    Sent.AngularVelocity.Y = Z;
                    Sent.AngularVelocity.Z = Y;
                    Sent.SyncKey = 4321;
                    const FString Failure = ExpectRoundTrip(Sent, &CheckPhysicsState, 432);
                    if (!Failure.IsEmpty())
                    {
                        return FString::Printf(TEXT("translation (%g, %g, %g): %s"), X, Y, Z, *Failure);
                    }
                }
            }
        }
        return FString();
    }

    // Two SerializeIntPacked values per pair. Each boundary sits at a 7-bit group edge of the packed encoding.
    FString AttributeProxyPackedGrid()
    {
        const uint32 Pairs[][2] = {
            { 0, 0xFFFFFFFF }, { 0xFFFFFFFF, 0 }, { 1, 127 }, { 128, 16383 },
            { 16384, 2097151 }, { 2097152, 268435455 }, { 268435456, 0xFFFFFFFE }, { 0xFFFFFFFF, 0xFFFFFFFF },
        };
        for (const uint32(&Pair)[2] : Pairs)
        {
            FFortAIAttributeReplicationProxy Sent = FFortAIAttributeReplicationProxy();
            Sent.Health = Pair[0];
            Sent.MaxHealth = Pair[1];
            const FString Failure = ExpectRoundTrip(Sent, &CheckAttributeProxy, -1);
            if (!Failure.IsEmpty())
            {
                return FString::Printf(TEXT("Health %u MaxHealth %u: %s"), Pair[0], Pair[1], *Failure);
            }
        }
        return FString();
    }

    // Every handle below SerializeInt(v, 0x1ff): the engine requires v < Max, so the largest handle is 510.
    FString BuildingHandleGrid()
    {
        const uint8 Upgrades[] = { 0, 255 };
        const bool Mirrors[] = { false, true };
        for (uint32 Handle = 0; Handle <= 510; ++Handle)
        {
            for (const uint8 Upgrade : Upgrades)
            {
                for (const bool bMirror : Mirrors)
                {
                    FCreateBuildingActorData Sent = FCreateBuildingActorData();
                    Sent.BuildingClassHandle = Handle;
                    Sent.bMirrored = bMirror;
                    Sent.BuildingClassData.UpgradeLevel = Upgrade;
                    const FString Failure = BuildingCase(Sent, 0.0f, 0.0f);
                    if (!Failure.IsEmpty())
                    {
                        return FString::Printf(TEXT("handle %u upgrade %u mirrored %d: %s"), Handle, static_cast<uint32>(Upgrade), bMirror ? 1 : 0, *Failure);
                    }
                }
            }
        }
        return FString();
    }

    // SyncKey values inside the range: the loaded value is within one step of the input.
    FString BuildingSyncGrid()
    {
        const float Values[] = { 0.0f, 0.04f, -0.04f, 1.0f / 13.0f, -1.0f / 13.0f, 12.5f, -7.25f, 645277.0f, -645277.0f };
        for (const float Value : Values)
        {
            FCreateBuildingActorData Sent = FCreateBuildingActorData();
            Sent.SyncKey = Value;
            const FString Failure = BuildingCase(Sent, Value, 0.0f);
            if (!Failure.IsEmpty())
            {
                return FString::Printf(TEXT("SyncKey %g: %s"), Value, *Failure);
            }
        }
        return FString();
    }

    // Yaw values that are not bin representatives. Each maps to the bin that holds its angle after the modulo-360 wrap.
    FString BuildingYawInteriorGrid()
    {
        struct FYawEntry
        {
            float Input;
            float Bin;
        };
        const FYawEntry Entries[] = {
            { 135.0f, 180.0f }, { 45.0f, 90.0f }, { -45.0f, 0.0f }, { -135.0f, -90.0f }, { -180.0f, 180.0f },
            { 450.0f, 90.0f }, { -270.0f, 90.0f }, { 540.0f, 180.0f }, { -360.0f, 0.0f },
        };
        for (const FYawEntry& Entry : Entries)
        {
            FCreateBuildingActorData Sent = FCreateBuildingActorData();
            Sent.BuildRot.Yaw = Entry.Input;
            const FString Failure = BuildingCase(Sent, 0.0f, Entry.Bin);
            if (!Failure.IsEmpty())
            {
                return FString::Printf(TEXT("yaw %g: %s"), Entry.Input, *Failure);
            }
        }
        return FString();
    }

    void TestSharedRepMovement(FSelfTestState& State)
    {
        const TCHAR* Struct = TEXT("FSharedRepMovement");

        {
            const FSharedRepMovement Sent = FSharedRepMovement();
            ReportCase(State, Struct, TEXT("zero_defaults"), ExpectRoundTrip(Sent, &CheckSharedRepMovement, -1));
        }
        {
            FSharedRepMovement Sent = FSharedRepMovement();
            Sent.RepMovement.LocationQuantizationLevel = EVectorQuantization::RoundOneDecimal;
            Sent.RepMovement.VelocityQuantizationLevel = EVectorQuantization::RoundOneDecimal;
            Sent.RepMovement.RotationQuantizationLevel = ERotatorQuantization::ByteComponents;
            Sent.RepMovement.bRepPhysics = true;
            Sent.RepMovement.Location = FVector(1000.5f, -2500.5f, 40.0f);
            Sent.RepMovement.LinearVelocity = FVector(600.0f, -12.5f, 0.0f);
            Sent.RepMovement.AngularVelocity = FVector(0.5f, -1.5f, 2.0f);
            Sent.RepMovement.Rotation = FRotator(0.0f, 90.0f, -90.0f);
            Sent.RepTimeStamp = 12.5f;
            Sent.TurretYaw = 45.0f;
            Sent.TurretPitch = -10.0f;
            Sent.RemoteViewData32 = 0x12345678;
            Sent.AccelerationPack = 0x1234;
            Sent.AccelerationZPack = -5;
            Sent.RepMovementMode = 2;
            Sent.JumpFlashCountPacked = 3;
            Sent.LandingFlashCountPacked = 4;
            Sent.CurrentMovementStyle = EFortMovementStyle::Sprinting;
            Sent.bIsCrouched = true;
            ReportCase(State, Struct, TEXT("typical_level1_byte_rotation"), ExpectRoundTrip(Sent, &CheckSharedRepMovement, -1));
        }
        {
            FSharedRepMovement Sent = FSharedRepMovement();
            Sent.RepMovement.LocationQuantizationLevel = EVectorQuantization::RoundTwoDecimals;
            Sent.RepMovement.VelocityQuantizationLevel = EVectorQuantization::RoundTwoDecimals;
            Sent.RepMovement.RotationQuantizationLevel = ERotatorQuantization::ShortComponents;
            Sent.RepMovement.bSimulatedPhysicSleep = true;
            Sent.RepMovement.bRepPhysics = true;
            Sent.RepMovement.Location = FVector(-250000.25f, 1000.5f, 0.25f);
            Sent.RepMovement.LinearVelocity = FVector(-1234.56f, 0.01f, 99999.99f);
            Sent.RepMovement.AngularVelocity = FVector(-3.5f, 0.0f, 7.25f);
            Sent.RepMovement.Rotation = FRotator(90.0f, 180.0f, -90.0f);
            Sent.RepTimeStamp = 65535.5f;
            Sent.TurretYaw = 359.5f;
            Sent.TurretPitch = -0.5f;
            Sent.RemoteViewData32 = 0xFFFFFFFF;
            Sent.AccelerationPack = 0xFFFF;
            Sent.AccelerationZPack = -128;
            Sent.RepMovementMode = 255;
            Sent.JumpFlashCountPacked = 255;
            Sent.LandingFlashCountPacked = 255;
            Sent.CurrentMovementStyle = EFortMovementStyle::Flying;
            SetSharedBools(Sent, 0x3FFu);
            ReportCase(State, Struct, TEXT("extreme_level2_short_rotation"), ExpectRoundTrip(Sent, &CheckSharedRepMovement, -1));
        }
        {
            FSharedRepMovement Sent = FSharedRepMovement();
            Sent.RepMovement.LocationQuantizationLevel = EVectorQuantization::RoundWholeNumber;
            Sent.RepMovement.VelocityQuantizationLevel = EVectorQuantization::RoundWholeNumber;
            Sent.RepMovement.RotationQuantizationLevel = ERotatorQuantization::ByteComponents;
            Sent.RepMovement.bRepPhysics = false;
            Sent.RepMovement.bSimulatedPhysicSleep = true;
            Sent.RepMovement.Location = FVector(1000.5f, -250000.25f, 0.0f);
            Sent.RepMovement.LinearVelocity = FVector(1.5f, -2.5f, 3.25f);
            Sent.RepMovement.AngularVelocity = FVector(9.0f, 9.0f, 9.0f);
            Sent.RepMovement.Rotation = FRotator(-90.0f, 90.0f, 180.0f);
            ReportCase(State, Struct, TEXT("level0_no_physics_angular_not_sent"), ExpectRoundTrip(Sent, &CheckSharedRepMovement, -1));
        }
        ReportCase(State, Struct, TEXT("rotation_grid_0_90_m90_180_byte_and_short"),
            RepMovementRotationGrid<FSharedRepMovement>(&CheckSharedRepMovement));
        ReportCase(State, Struct, TEXT("location_velocity_grid_3_levels_27_values"),
            RepMovementVectorGrid<FSharedRepMovement>(&CheckSharedRepMovement));
        ReportCase(State, Struct, TEXT("bools_all_1024_combinations"), SharedBoolGrid());
        ReportCase(State, Struct, TEXT("optional_fields_and_flags_128_combinations"), SharedOptionalFieldGrid());
        ReportCase(State, Struct, TEXT("movement_style_and_byte_extremes"), SharedStyleGrid());
    }

    void TestFortAISharedRepMovement(FSelfTestState& State)
    {
        const TCHAR* Struct = TEXT("FFortAISharedRepMovement");

        {
            FFortAISharedRepMovement Sent = FFortAISharedRepMovement();
            Sent.RepSharedAnimInfo.MontageIndex = -1;
            Sent.RepSharedAnimInfo.AnimState = EFortSharedAnimationState::Anim_MAX;
            Sent.RepSharedAnimInfo.MontageSectionToPlay = 0;
            ReportCase(State, Struct, TEXT("zero_no_montage"), ExpectRoundTrip(Sent, &CheckFortAISharedRepMovement, -1));
        }
        {
            FFortAISharedRepMovement Sent = FFortAISharedRepMovement();
            Sent.RepMovement.LocationQuantizationLevel = EVectorQuantization::RoundOneDecimal;
            Sent.RepMovement.VelocityQuantizationLevel = EVectorQuantization::RoundOneDecimal;
            Sent.RepMovement.Location = FVector(1000.5f, 2.5f, -40.0f);
            Sent.RepMovement.Rotation = FRotator(0.0f, 180.0f, -90.0f);
            Sent.RepTimeStamp = 3.25f;
            Sent.RepMovementMode = 1;
            Sent.RepCurrentFortAILODLevel = EFortAILODLevel::Normal;
            Sent.RepSharedAnimInfo.MontageIndex = 7;
            Sent.RepSharedAnimInfo.AnimState = EFortSharedAnimationState::Anim_Attack;
            Sent.RepSharedAnimInfo.MontageSectionToPlay = 2;
            ReportCase(State, Struct, TEXT("montage_typical"), ExpectRoundTrip(Sent, &CheckFortAISharedRepMovement, -1));
        }
        {
            FFortAISharedRepMovement Sent = FFortAISharedRepMovement();
            Sent.RepMovement.Rotation = FRotator(0.0f, 0.0f, 0.0f);
            Sent.RepSharedAnimInfo.MontageIndex = -1;
            Sent.RepSharedAnimInfo.AnimState = EFortSharedAnimationState::Anim_Walk;
            Sent.RepSharedAnimInfo.MontageSectionToPlay = 5;
            ReportCase(State, Struct, TEXT("no_montage_defaults_applied"), ExpectRoundTrip(Sent, &CheckNoMontageDefaults, -1));
        }
        ReportCase(State, Struct, TEXT("montage_index_edges_16_combinations"), FortAIMontageEdgeGrid());
        ReportCase(State, Struct, TEXT("lod_level_and_mode_all_bytes"), FortAILodModeGrid());
        ReportCase(State, Struct, TEXT("rotation_grid_0_90_m90_180_byte_and_short"),
            RepMovementRotationGrid<FFortAISharedRepMovement>(&CheckFortAISharedRepMovement));
        ReportCase(State, Struct, TEXT("location_velocity_grid_3_levels_27_values"),
            RepMovementVectorGrid<FFortAISharedRepMovement>(&CheckFortAISharedRepMovement));
    }

    void TestReplicatedPhysicsPawnState(FSelfTestState& State)
    {
        const TCHAR* Struct = TEXT("FReplicatedPhysicsPawnState");

        {
            const FReplicatedPhysicsPawnState Sent = FReplicatedPhysicsPawnState();
            ReportCase(State, Struct, TEXT("zero"), ExpectRoundTrip(Sent, &CheckPhysicsState, 432));
        }
        {
            FReplicatedPhysicsPawnState Sent = FReplicatedPhysicsPawnState();
            Sent.Translation.X = 1000.5f;
            Sent.Translation.Y = -250.25f;
            Sent.Translation.Z = 40.125f;
            Sent.Rotation = FQuat(0.0f, 0.0f, 0.0f, 1.0f);
            Sent.LinearVelocity.X = 10.5f;
            Sent.LinearVelocity.Y = -3.25f;
            Sent.LinearVelocity.Z = 0.0f;
            Sent.AngularVelocity.X = 0.1f;
            Sent.AngularVelocity.Y = 0.2f;
            Sent.AngularVelocity.Z = 0.3f;
            Sent.SyncKey = 1000;
            ReportCase(State, Struct, TEXT("typical"), ExpectRoundTrip(Sent, &CheckPhysicsState, 432));
        }
        {
            FReplicatedPhysicsPawnState Sent = FReplicatedPhysicsPawnState();
            Sent.Translation.X = -250000.25f;
            Sent.Translation.Y = 1000.5f;
            Sent.Translation.Z = 0.0f;
            Sent.Rotation = FQuat(0.70710677f, 0.0f, 0.0f, 0.70710677f);
            Sent.LinearVelocity.X = -250000.25f;
            Sent.LinearVelocity.Y = 0.0f;
            Sent.LinearVelocity.Z = 1.0e4f;
            Sent.AngularVelocity.X = 1.0e6f;
            Sent.AngularVelocity.Y = -1.0e6f;
            Sent.AngularVelocity.Z = 0.5f;
            Sent.SyncKey = 65535;
            ReportCase(State, Struct, TEXT("extreme_translation_and_quat"), ExpectRoundTrip(Sent, &CheckPhysicsState, 432));
        }
        {
            FReplicatedPhysicsPawnState Sent = FReplicatedPhysicsPawnState();
            Sent.SyncKey = 0;
            ReportCase(State, Struct, TEXT("sync_key_zero"), ExpectRoundTrip(Sent, &CheckPhysicsState, 432));
        }
        {
            FReplicatedPhysicsPawnState Sent = FReplicatedPhysicsPawnState();
            Sent.SyncKey = 65535;
            ReportCase(State, Struct, TEXT("sync_key_65535"), ExpectRoundTrip(Sent, &CheckPhysicsState, 432));
        }
        ReportCase(State, Struct, TEXT("translation_grid_27_values"), PhysicsAxisGrid());
    }

    void TestFortAIAttributeReplicationProxy(FSelfTestState& State)
    {
        const TCHAR* Struct = TEXT("FFortAIAttributeReplicationProxy");

        {
            const FFortAIAttributeReplicationProxy Sent = FFortAIAttributeReplicationProxy();
            ReportCase(State, Struct, TEXT("zero_zero"), ExpectRoundTrip(Sent, &CheckAttributeProxy, -1));
        }
        {
            FFortAIAttributeReplicationProxy Sent = FFortAIAttributeReplicationProxy();
            Sent.Health = 0xFFFFFFFF;
            Sent.MaxHealth = 0xFFFFFFFF;
            ReportCase(State, Struct, TEXT("max_max"), ExpectRoundTrip(Sent, &CheckAttributeProxy, -1));
        }
        {
            FFortAIAttributeReplicationProxy Sent = FFortAIAttributeReplicationProxy();
            Sent.Health = 150;
            Sent.MaxHealth = 200;
            ReportCase(State, Struct, TEXT("typical_150_200"), ExpectRoundTrip(Sent, &CheckAttributeProxy, -1));
        }
        ReportCase(State, Struct, TEXT("packed_boundaries_8_pairs"), AttributeProxyPackedGrid());
    }

    void TestCreateBuildingActorData(FSelfTestState& State)
    {
        const TCHAR* Struct = TEXT("FCreateBuildingActorData");

        {
            const FCreateBuildingActorData Sent = FCreateBuildingActorData();
            ReportCase(State, Struct, TEXT("zero"), BuildingCase(Sent, 0.0f, 0.0f));
        }
        {
            FCreateBuildingActorData Sent = FCreateBuildingActorData();
            Sent.BuildingClassHandle = 137;
            Sent.bMirrored = true;
            Sent.BuildLoc.X = 1000.5f;
            Sent.BuildLoc.Y = -250000.25f;
            Sent.BuildLoc.Z = 40.0f;
            Sent.BuildingClassData.UpgradeLevel = 3;
            Sent.SyncKey = 12.5f;
            Sent.BuildRot.Yaw = 90.0f;
            ReportCase(State, Struct, TEXT("typical"), BuildingCase(Sent, 12.5f, 90.0f));
        }
        ReportCase(State, Struct, TEXT("handle_0_to_510_upgrade_mirror_grid"), BuildingHandleGrid());
        {
            FCreateBuildingActorData Sent = FCreateBuildingActorData();
            Sent.SyncKey = SyncKeyMax;
            ReportCase(State, Struct, TEXT("synckey_max_representable"), BuildingCase(Sent, SyncKeyMax, 0.0f));
        }
        {
            FCreateBuildingActorData Sent = FCreateBuildingActorData();
            Sent.SyncKey = SyncKeyMin;
            ReportCase(State, Struct, TEXT("synckey_min_representable"), BuildingCase(Sent, SyncKeyMin, 0.0f));
        }
        {
            FCreateBuildingActorData Sent = FCreateBuildingActorData();
            Sent.SyncKey = 700000.0f;
            ReportCase(State, Struct, TEXT("synckey_above_range_saturates"), BuildingCase(Sent, SyncKeyMax, 0.0f));
        }
        {
            FCreateBuildingActorData Sent = FCreateBuildingActorData();
            Sent.SyncKey = -700000.0f;
            ReportCase(State, Struct, TEXT("synckey_below_range_saturates"), BuildingCase(Sent, SyncKeyMin, 0.0f));
        }
        ReportCase(State, Struct, TEXT("synckey_values_in_range"), BuildingSyncGrid());
        {
            FCreateBuildingActorData Sent = FCreateBuildingActorData();
            Sent.BuildRot.Yaw = 180.0f;
            ReportCase(State, Struct, TEXT("yaw_bin_180"), BuildingCase(Sent, 0.0f, 180.0f));
        }
        {
            FCreateBuildingActorData Sent = FCreateBuildingActorData();
            Sent.BuildRot.Yaw = 90.0f;
            ReportCase(State, Struct, TEXT("yaw_bin_90"), BuildingCase(Sent, 0.0f, 90.0f));
        }
        {
            FCreateBuildingActorData Sent = FCreateBuildingActorData();
            Sent.BuildRot.Yaw = 0.0f;
            ReportCase(State, Struct, TEXT("yaw_bin_0"), BuildingCase(Sent, 0.0f, 0.0f));
        }
        {
            FCreateBuildingActorData Sent = FCreateBuildingActorData();
            Sent.BuildRot.Yaw = -90.0f;
            ReportCase(State, Struct, TEXT("yaw_bin_minus90"), BuildingCase(Sent, 0.0f, -90.0f));
        }
        ReportCase(State, Struct, TEXT("yaw_interior_and_wrap"), BuildingYawInteriorGrid());
    }
}

// Entry point, called from FFortniteGameModule::StartupModule when -NetSerializeSelfTest is on the command line.
void RunNetSerializeSelfTest()
{
    FSelfTestState State;
    TestSharedRepMovement(State);
    TestFortAISharedRepMovement(State);
    TestReplicatedPhysicsPawnState(State);
    TestFortAIAttributeReplicationProxy(State);
    TestCreateBuildingActorData(State);

    UE_LOG(LogNetSerializeSelfTest, Display, TEXT("NetSerializeSelfTest: %d cases run"), State.Cases);
    if (State.Failures == 0)
    {
        UE_LOG(LogNetSerializeSelfTest, Display, TEXT("NetSerializeSelfTest: ALL PASS"));
    }
    else
    {
        UE_LOG(LogNetSerializeSelfTest, Error, TEXT("NetSerializeSelfTest: FAILED %d"), State.Failures);
    }
    FPlatformMisc::RequestExitWithStatus(false, State.Failures == 0 ? 0 : 1);
}
