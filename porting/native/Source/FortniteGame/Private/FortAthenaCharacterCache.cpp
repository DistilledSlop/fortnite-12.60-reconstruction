#include "FortAthenaCharacterCache.h"
#include "FortAthenaRegisteredSession.h"
#include "FortAssets.h"
#include "AFortPlayerController.h"
#include "AFortPlayerPawn.h"
#include "AFortPlayerState.h"
#include "EFortCustomBodyType.h"
#include "EFortCustomGender.h"
#include "EFortCustomPartType.h"
#include "FCustomCharacterData.h"
#include "FFortAthenaLoadout.h"
#include "FMcpVariantChannelInfo.h"
#include "UAthenaCharacterItemDefinition.h"
#include "UCustomCharacterPart.h"
#include "UFortCosmeticCharacterPartVariant.h"
#include "UFortCosmeticMaterialVariant.h"
#include "UFortCosmeticVariant.h"
#include "UFortHeroSpecialization.h"
#include "UFortHeroType.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Engine/NetConnection.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagsManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/SecureHash.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace FortAthenaCharacterCache
{
    namespace
    {
        // Pinned SHA-1 of Config/AthenaCharacterRegistry.json produced from AssetRegistry.bin
        // (SHA-256: 8cdd9e893b9e043d45702fb285d312476105b448330493cbb872ba3d58fe47b0, 952 rows).
        constexpr TCHAR CharacterManifestSha1[] = TEXT("AAFB2F7788864605068D85FCCD1DE7E627F18682");
        constexpr int32 ExpectedManifestCount = 952;

        // Match-independent FString-only manifest cache (no cached UObject* pointers).
        bool bManifestPrepared = false;
        FString ManifestLoadError;
        TMap<FString, FString> PrimaryIdToObjectPath;
        TMap<FString, FString> PrimaryIdRedirects;

        FString FoldAscii(const FString& Value)
        {
            FString Out = Value;
            for (int32 i = 0; i < Out.Len(); ++i)
            {
                if (Out[i] >= TEXT('A') && Out[i] <= TEXT('Z'))
                {
                    Out[i] = static_cast<TCHAR>(Out[i] + (TEXT('a') - TEXT('A')));
                }
            }
            return Out;
        }

        bool IsAllowedManifestObjectPath(const FString& ObjectPath)
        {
            return ObjectPath.StartsWith(TEXT("/Game/Athena/Items/Cosmetics/Characters/"), ESearchCase::CaseSensitive) ||
                   ObjectPath.StartsWith(TEXT("/Game/Heroes/Commando/CosmeticCharacterItemDefinitions/"), ESearchCase::CaseSensitive) ||
                   ObjectPath.StartsWith(TEXT("/Game/Heroes/Constructor/CosmeticCharacterItemDefinitions/"), ESearchCase::CaseSensitive) ||
                   ObjectPath.StartsWith(TEXT("/Game/Heroes/Ninja/CosmeticCharacterItemDefinitions/"), ESearchCase::CaseSensitive) ||
                   ObjectPath.StartsWith(TEXT("/Game/Heroes/Outlander/CosmeticCharacterItemDefinitions/"), ESearchCase::CaseSensitive);
        }

        bool EnsureManifestLoaded(FString& OutError)
        {
            if (bManifestPrepared)
                return true;

            PrimaryIdToObjectPath.Reset();
            PrimaryIdRedirects.Reset();

            const FString FilePath = FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("AthenaCharacterRegistry.json"));
            TArray<uint8> Bytes;
            if (!FFileHelper::LoadFileToArray(Bytes, *FilePath))
            {
                ManifestLoadError = TEXT("missing AthenaCharacterRegistry.json");
                OutError = ManifestLoadError;
                return false;
            }

            FSHAHash Digest;
            FSHA1::HashBuffer(Bytes.GetData(), Bytes.Num(), Digest.Hash);
            if (!Digest.ToString().Equals(CharacterManifestSha1, ESearchCase::IgnoreCase))
            {
                ManifestLoadError = TEXT("AthenaCharacterRegistry.json digest mismatch");
                OutError = ManifestLoadError;
                return false;
            }

            FString Json;
            FFileHelper::BufferToString(Json, Bytes.GetData(), Bytes.Num());
            TSharedPtr<FJsonObject> Root;
            if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root) || !Root.IsValid())
            {
                ManifestLoadError = TEXT("invalid AthenaCharacterRegistry.json");
                OutError = ManifestLoadError;
                return false;
            }

            const TArray<TSharedPtr<FJsonValue>>* Rows = nullptr;
            if (!Root->TryGetArrayField(TEXT("rows"), Rows) || !Rows || Rows->Num() != ExpectedManifestCount)
            {
                ManifestLoadError = TEXT("unexpected AthenaCharacterRegistry.json record count");
                OutError = ManifestLoadError;
                return false;
            }

            for (const TSharedPtr<FJsonValue>& Val : *Rows)
            {
                const TSharedPtr<FJsonObject>* RowObj = nullptr;
                FString PrimaryId, ObjectPath;
                if (!Val.IsValid() || !Val->TryGetObject(RowObj) || !RowObj || !RowObj->IsValid() ||
                    !(*RowObj)->TryGetStringField(TEXT("primaryAssetId"), PrimaryId) ||
                    !(*RowObj)->TryGetStringField(TEXT("objectPath"), ObjectPath))
                {
                    PrimaryIdToObjectPath.Reset();
                    ManifestLoadError = TEXT("malformed AthenaCharacterRegistry row");
                    OutError = ManifestLoadError;
                    return false;
                }
                if (!IsAllowedManifestObjectPath(ObjectPath))
                {
                    PrimaryIdToObjectPath.Reset();
                    ManifestLoadError = TEXT("unexpected AthenaCharacter objectPath directory");
                    OutError = ManifestLoadError;
                    return false;
                }
                const FString Key = FoldAscii(PrimaryId);
                if (PrimaryIdToObjectPath.Contains(Key))
                {
                    PrimaryIdToObjectPath.Reset();
                    ManifestLoadError = TEXT("duplicate primaryAssetId in AthenaCharacterRegistry");
                    OutError = ManifestLoadError;
                    return false;
                }
                PrimaryIdToObjectPath.Add(Key, ObjectPath);
            }

            const TArray<TSharedPtr<FJsonValue>>* Redirects = nullptr;
            if (Root->TryGetArrayField(TEXT("redirects"), Redirects) && Redirects)
            {
                for (const TSharedPtr<FJsonValue>& Val : *Redirects)
                {
                    const TSharedPtr<FJsonObject>* RedirObj = nullptr;
                    FString OldId, NewId;
                    if (!Val.IsValid() || !Val->TryGetObject(RedirObj) || !RedirObj || !RedirObj->IsValid() ||
                        !(*RedirObj)->TryGetStringField(TEXT("old"), OldId) ||
                        !(*RedirObj)->TryGetStringField(TEXT("new"), NewId))
                    {
                        PrimaryIdToObjectPath.Reset();
                        PrimaryIdRedirects.Reset();
                        ManifestLoadError = TEXT("malformed redirect entry in AthenaCharacterRegistry");
                        OutError = ManifestLoadError;
                        return false;
                    }
                    const FString NewKey = FoldAscii(NewId);
                    if (!PrimaryIdToObjectPath.Contains(NewKey))
                    {
                        PrimaryIdToObjectPath.Reset();
                        PrimaryIdRedirects.Reset();
                        ManifestLoadError = TEXT("redirect target missing in AthenaCharacterRegistry");
                        OutError = ManifestLoadError;
                        return false;
                    }
                    PrimaryIdRedirects.Add(FoldAscii(OldId), NewKey);
                }
            }

            bManifestPrepared = true;
            ManifestLoadError.Empty();
            UE_LOG(LogTemp, Display, TEXT("Character cache ready: %d AthenaCharacter entries"), PrimaryIdToObjectPath.Num());
            return true;
        }

        bool ResolveCharacterObjectPath(const FString& TemplateId, FString& OutObjectPath, FString& OutError)
        {
            if (!EnsureManifestLoaded(OutError))
                return false;
            FString Key = FoldAscii(TemplateId);
            if (const FString* Redirected = PrimaryIdRedirects.Find(Key))
                Key = *Redirected;
            const FString* FoundPath = PrimaryIdToObjectPath.Find(Key);
            if (!FoundPath)
            {
                OutError = FString::Printf(TEXT("unknown AthenaCharacter templateId '%s'"), *TemplateId);
                return false;
            }
            OutObjectPath = *FoundPath;
            return true;
        }

        bool TagMatchesPrefixed(const FGameplayTag& Tag, const FString& Input, const TCHAR* DefaultPrefix)
        {
            if (!Tag.IsValid() || Input.IsEmpty())
                return false;
            const FString TagStr = Tag.GetTagName().ToString();
            if (TagStr.Equals(Input, ESearchCase::IgnoreCase))
                return true;
            if (!Input.Contains(TEXT(".")))
            {
                const FString Prefixed = FString(DefaultPrefix) + Input;
                if (TagStr.Equals(Prefixed, ESearchCase::IgnoreCase))
                    return true;
            }
            return false;
        }

        bool DeriveGenderAndBodyType(
            const UAthenaCharacterItemDefinition* CharacterDef,
            const UFortHeroType* HeroDef,
            UCustomCharacterPart* const (&Parts)[EFortCustomPartType::NumTypes],
            EFortCustomGender::Type& OutGender,
            EFortCustomBodyType::Type& OutBodyType,
            FString& OutError)
        {
            OutGender = EFortCustomGender::Invalid;
            if (CharacterDef->Gender == EFortCustomGender::Male || CharacterDef->Gender == EFortCustomGender::Female)
                OutGender = CharacterDef->Gender;
            if (OutGender == EFortCustomGender::Invalid && Parts[EFortCustomPartType::Body])
            {
                const EFortCustomGender::Type G = Parts[EFortCustomPartType::Body]->GenderPermitted.GetValue();
                if (G == EFortCustomGender::Male || G == EFortCustomGender::Female)
                    OutGender = G;
            }
            if (OutGender == EFortCustomGender::Invalid && Parts[EFortCustomPartType::Head])
            {
                const EFortCustomGender::Type G = Parts[EFortCustomPartType::Head]->GenderPermitted.GetValue();
                if (G == EFortCustomGender::Male || G == EFortCustomGender::Female)
                    OutGender = G;
            }
            if (OutGender == EFortCustomGender::Invalid &&
                (HeroDef->Gender == EFortCustomGender::Male || HeroDef->Gender == EFortCustomGender::Female))
            {
                OutGender = HeroDef->Gender;
            }
            if (OutGender == EFortCustomGender::Invalid)
            {
                for (int32 i = 0; i < EFortCustomPartType::NumTypes; ++i)
                {
                    if (!Parts[i]) continue;
                    const EFortCustomGender::Type G = Parts[i]->GenderPermitted.GetValue();
                    if (G == EFortCustomGender::Male || G == EFortCustomGender::Female)
                    {
                        OutGender = G;
                        break;
                    }
                }
            }
            if (OutGender == EFortCustomGender::Invalid)
            {
                OutError = TEXT("unable to derive CharacterGender from loaded character definition or parts");
                return false;
            }

            // BodyTypesPermitted uses bitmask values: Small=1, Medium=2, Large=4, combinations 3/5/6/7.
            OutBodyType = EFortCustomBodyType::NONE;
            const int32 PrioritySlots[] = {
                EFortCustomPartType::Body,
                EFortCustomPartType::Head,
                EFortCustomPartType::Face,
                EFortCustomPartType::Hat,
                EFortCustomPartType::Backpack,
                EFortCustomPartType::Charm
            };
            int32 CombinedMask = static_cast<int32>(EFortCustomBodyType::All);
            bool bSawPart = false;
            for (int32 Slot : PrioritySlots)
            {
                if (!Parts[Slot]) continue;
                bSawPart = true;
                const EFortCustomBodyType::Type Perm = Parts[Slot]->BodyTypesPermitted.GetValue();
                if (Perm == EFortCustomBodyType::NONE || Perm >= EFortCustomBodyType::Deprecated)
                {
                    OutError = FString::Printf(TEXT("character part %s has invalid BodyTypesPermitted=%d"),
                        *Parts[Slot]->GetPathName(), static_cast<int32>(Perm));
                    return false;
                }
                const int32 PermInt = static_cast<int32>(Perm);
                CombinedMask &= PermInt;
                if (OutBodyType == EFortCustomBodyType::NONE &&
                    (Perm == EFortCustomBodyType::Small || Perm == EFortCustomBodyType::Medium || Perm == EFortCustomBodyType::Large))
                {
                    OutBodyType = Perm;
                }
            }
            if (!bSawPart || CombinedMask == 0)
            {
                OutError = TEXT("character parts have empty or conflicting BodyTypesPermitted");
                return false;
            }
            if (OutBodyType == EFortCustomBodyType::NONE)
            {
                if (CombinedMask & static_cast<int32>(EFortCustomBodyType::Medium))
                    OutBodyType = EFortCustomBodyType::Medium;
                else if (CombinedMask & static_cast<int32>(EFortCustomBodyType::Small))
                    OutBodyType = EFortCustomBodyType::Small;
                else if (CombinedMask & static_cast<int32>(EFortCustomBodyType::Large))
                    OutBodyType = EFortCustomBodyType::Large;
                else
                {
                    OutError = TEXT("unable to resolve CharacterBodyType");
                    return false;
                }
            }
            return true;
        }

        void FailClosedController(AController* Controller, const FString& Reason)
        {
            UE_LOG(LogTemp, Error, TEXT("Registered character apply failed closed: %s"), *Reason);
            FortAthenaRegisteredSession::MarkControllerLoadoutFailed(Controller, Reason);
            if (APlayerController* PC = Cast<APlayerController>(Controller))
            {
                if (UNetConnection* Conn = PC->GetNetConnection())
                {
                    Conn->Close();
                }
            }
        }
    }

    bool Prepare()
    {
        FString IgnoredError;
        const bool bOk = EnsureManifestLoaded(IgnoredError);
        if (!bOk)
        {
            UE_LOG(LogTemp, Error, TEXT("Character cache prepare failed: %s"), *IgnoredError);
        }
        return bOk;
    }

    bool ApplyForController(AController* Controller, bool bAfterPawnSpawn)
    {
        if (!Controller || !FortAthenaRegisteredSession::Enabled())
            return false;

        FortAthenaRegisteredSession::FResolvedPeerLoadout Loadout;
        FString SessionError;
        const FortAthenaRegisteredSession::ELoadoutStatus Status =
            FortAthenaRegisteredSession::QueryControllerLoadout(Controller, Loadout, SessionError);

        if (Status == FortAthenaRegisteredSession::ELoadoutStatus::NotRegisteredMode)
            return true;
        if (Status == FortAthenaRegisteredSession::ELoadoutStatus::Pending)
        {
            // Async /player/<account> request is still in flight; leave HeroType null until it finishes.
            return false;
        }
        if (Status == FortAthenaRegisteredSession::ELoadoutStatus::Failed)
        {
            FailClosedController(Controller, SessionError);
            return false;
        }

        AFortPlayerState* PlayerState = Cast<AFortPlayerState>(Controller->PlayerState);
        if (!PlayerState)
        {
            FailClosedController(Controller, TEXT("controller missing AFortPlayerState"));
            return false;
        }

        FString Error;
        FString ObjectPath;
        if (!ResolveCharacterObjectPath(Loadout.CharacterTemplateId, ObjectPath, Error))
        {
            FailClosedController(Controller, Error);
            return false;
        }

        UAthenaCharacterItemDefinition* CharacterDef =
            LoadObject<UAthenaCharacterItemDefinition>(nullptr, *ObjectPath);
        if (!CharacterDef || !CharacterDef->IsA(UAthenaCharacterItemDefinition::StaticClass()))
        {
            FailClosedController(Controller, FString::Printf(TEXT("failed to load character definition %s"), *ObjectPath));
            return false;
        }
        FFortAssets::Get().TrackAsset(CharacterDef);

        UFortHeroType* HeroDef = CharacterDef->HeroDefinition;
        if (!HeroDef)
        {
            FailClosedController(Controller, FString::Printf(TEXT("%s missing HeroDefinition"), *Loadout.CharacterTemplateId));
            return false;
        }
        FFortAssets::Get().TrackAsset(HeroDef);

        UCustomCharacterPart* ResolvedParts[EFortCustomPartType::NumTypes] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
        if (HeroDef->Specializations.Num() == 0)
        {
            FailClosedController(Controller, FString::Printf(TEXT("%s HeroDefinition has no Specializations"), *Loadout.CharacterTemplateId));
            return false;
        }

        for (const TSoftObjectPtr<UFortHeroSpecialization>& SpecPtr : HeroDef->Specializations)
        {
            if (!SpecPtr.ToSoftObjectPath().IsValid())
            {
                FailClosedController(Controller, FString::Printf(TEXT("%s has invalid Specialization soft pointer"), *Loadout.CharacterTemplateId));
                return false;
            }
            UFortHeroSpecialization* Spec = SpecPtr.LoadSynchronous();
            if (!Spec)
            {
                FailClosedController(Controller, FString::Printf(TEXT("failed to load HeroSpecialization for %s"), *Loadout.CharacterTemplateId));
                return false;
            }
            FFortAssets::Get().TrackAsset(Spec);

            for (const TSoftObjectPtr<UCustomCharacterPart>& PartPtr : Spec->CharacterParts)
            {
                if (!PartPtr.ToSoftObjectPath().IsValid())
                {
                    FailClosedController(Controller, FString::Printf(TEXT("%s specialization has invalid CharacterPart soft pointer"), *Loadout.CharacterTemplateId));
                    return false;
                }
                UCustomCharacterPart* Part = PartPtr.LoadSynchronous();
                if (!Part)
                {
                    FailClosedController(Controller, FString::Printf(TEXT("failed to load CustomCharacterPart %s"), *PartPtr.ToSoftObjectPath().ToString()));
                    return false;
                }
                FFortAssets::Get().TrackAsset(Part);

                const int32 Slot = static_cast<int32>(Part->CharacterPartType.GetValue());
                if (Slot < 0 || Slot >= EFortCustomPartType::NumTypes)
                {
                    FailClosedController(Controller, FString::Printf(TEXT("unsupported CharacterPartType %d on %s"), Slot, *Part->GetName()));
                    return false;
                }
                ResolvedParts[Slot] = Part;
            }
        }

        bool bHasAnyResolvedPart = false;
        for (int32 Slot = 0; Slot < EFortCustomPartType::NumTypes; ++Slot)
        {
            if (ResolvedParts[Slot] != nullptr)
            {
                bHasAnyResolvedPart = true;
                break;
            }
        }
        if (!bHasAnyResolvedPart)
        {
            FailClosedController(Controller, FString::Printf(TEXT("%s resolved zero CharacterParts from Specializations"), *Loadout.CharacterTemplateId));
            return false;
        }

        TArray<FMcpVariantChannelInfo> ResolvedChannels;
        for (const FortAthenaRegisteredSession::FVariantSelection& Sel : Loadout.Variants)
        {
            UFortCosmeticVariant* MatchedVariant = nullptr;
            for (UFortCosmeticVariant* VariantObj : CharacterDef->ItemVariants)
            {
                if (VariantObj && TagMatchesPrefixed(VariantObj->VariantChannelTag, Sel.Channel, TEXT("Cosmetics.Variant.Channel.")))
                {
                    MatchedVariant = VariantObj;
                    break;
                }
            }
            if (!MatchedVariant)
            {
                FailClosedController(Controller, FString::Printf(TEXT("unsupported variant channel '%s' on %s"), *Sel.Channel, *Loadout.CharacterTemplateId));
                return false;
            }

            FGameplayTag ActiveTag;
            if (UFortCosmeticCharacterPartVariant* PartVar = Cast<UFortCosmeticCharacterPartVariant>(MatchedVariant))
            {
                const FPartVariantDef* MatchedOption = nullptr;
                for (const FPartVariantDef& Opt : PartVar->PartOptions)
                {
                    if (TagMatchesPrefixed(Opt.CustomizationVariantTag, Sel.Active, TEXT("Cosmetics.Variant.Property.")))
                    {
                        MatchedOption = &Opt;
                        break;
                    }
                }
                if (!MatchedOption)
                {
                    FailClosedController(Controller, FString::Printf(TEXT("unsupported variant active tag '%s' on %s"), *Sel.Active, *Loadout.CharacterTemplateId));
                    return false;
                }
                ActiveTag = MatchedOption->CustomizationVariantTag;
                for (const TSoftObjectPtr<UCustomCharacterPart>& VPartPtr : MatchedOption->VariantParts)
                {
                    if (!VPartPtr.ToSoftObjectPath().IsValid())
                    {
                        FailClosedController(Controller, TEXT("selected variant contains an invalid character part path"));
                        return false;
                    }
                    UCustomCharacterPart* VPart = VPartPtr.LoadSynchronous();
                    if (!VPart)
                    {
                        FailClosedController(Controller, FString::Printf(TEXT("failed to load VariantPart %s"), *VPartPtr.ToSoftObjectPath().ToString()));
                        return false;
                    }
                    FFortAssets::Get().TrackAsset(VPart);
                    const int32 Slot = static_cast<int32>(VPart->CharacterPartType.GetValue());
                    if (Slot < 0 || Slot >= EFortCustomPartType::NumTypes)
                    {
                        FailClosedController(Controller, FString::Printf(TEXT("unsupported variant CharacterPartType %d"), Slot));
                        return false;
                    }
                    ResolvedParts[Slot] = VPart;
                }
            }
            else if (UFortCosmeticMaterialVariant* MatVar = Cast<UFortCosmeticMaterialVariant>(MatchedVariant))
            {
                const FMaterialVariantDef* MatchedOption = nullptr;
                for (const FMaterialVariantDef& Opt : MatVar->MaterialOptions)
                {
                    if (TagMatchesPrefixed(Opt.CustomizationVariantTag, Sel.Active, TEXT("Cosmetics.Variant.Property.")))
                    {
                        MatchedOption = &Opt;
                        break;
                    }
                }
                if (!MatchedOption)
                {
                    FailClosedController(Controller, FString::Printf(TEXT("unsupported material variant tag '%s' on %s"), *Sel.Active, *Loadout.CharacterTemplateId));
                    return false;
                }
                ActiveTag = MatchedOption->CustomizationVariantTag;
            }
            else
            {
                FailClosedController(Controller, FString::Printf(TEXT("unsupported cosmetic variant type %s on %s"), *MatchedVariant->GetClass()->GetName(), *Loadout.CharacterTemplateId));
                return false;
            }

            FMcpVariantChannelInfo ChannelInfo{};
            ChannelInfo.VariantChannelTag = MatchedVariant->VariantChannelTag;
            ChannelInfo.ActiveVariantTag = ActiveTag;
            ChannelInfo.ItemVariantIsUsedFor = CharacterDef;
            for (const FString& OwnedStr : Sel.Owned)
            {
                const FString FullOwned = OwnedStr.Contains(TEXT(".")) ? OwnedStr : FString(TEXT("Cosmetics.Variant.Property.")) + OwnedStr;
                FGameplayTag OwnedTag = UGameplayTagsManager::Get().RequestGameplayTag(FName(*FullOwned), false);
                if (OwnedTag.IsValid())
                {
                    ChannelInfo.OwnedVariantTags.AddTag(OwnedTag);
                }
            }
            ResolvedChannels.Add(MoveTemp(ChannelInfo));
        }

        EFortCustomGender::Type Gender = EFortCustomGender::Invalid;
        EFortCustomBodyType::Type BodyType = EFortCustomBodyType::NONE;
        if (!DeriveGenderAndBodyType(CharacterDef, HeroDef, ResolvedParts, Gender, BodyType, Error))
        {
            FailClosedController(Controller, Error);
            return false;
        }

        if (AFortPlayerController* FortPC = Cast<AFortPlayerController>(Controller))
        {
            FortPC->CosmeticLoadoutPC.Character = CharacterDef;
            FortPC->CosmeticLoadoutPC.CharacterVariantChannels = ResolvedChannels;
            FortPC->CosmeticLoadoutPC.bIsDefaultCharacter = false;
        }

        if (!bAfterPawnSpawn || PlayerState->HeroType != HeroDef)
        {
            PlayerState->HeroType = HeroDef;
            PlayerState->CharacterData.WasPartReplicatedFlags = 0;
            PlayerState->CharacterData.RequiredVariantPartFlags = 0;
            PlayerState->CharacterData.VariantRequiredCharacterParts.Reset();
            PlayerState->CharacterData.bReplicationFailed = false;
            for (int32 Slot = 0; Slot < EFortCustomPartType::NumTypes; ++Slot)
            {
                PlayerState->CharacterData.Parts[Slot] = ResolvedParts[Slot];
                if (ResolvedParts[Slot])
                {
                    PlayerState->CharacterData.WasPartReplicatedFlags |= static_cast<uint8>(1u << Slot);
                }
            }
            PlayerState->CharacterGender = Gender;
            PlayerState->CharacterBodyType = BodyType;
            PlayerState->ForceNetUpdate();
        }

        if (AFortPlayerPawn* FortPawn = Cast<AFortPlayerPawn>(Controller->GetPawn()))
        {
            FortPawn->CosmeticLoadout.Character = CharacterDef;
            FortPawn->CosmeticLoadout.CharacterVariantChannels = ResolvedChannels;
            FortPawn->CosmeticLoadout.bIsDefaultCharacter = false;
            FortPawn->CharacterGender = Gender;
            FortPawn->CharacterBodyType = BodyType;
            FortPawn->ForceNetUpdate();
        }

        FortAthenaRegisteredSession::MarkControllerLoadoutApplied(Controller);
        UE_LOG(LogTemp, Display,
            TEXT("Applied registered character %s (hero=%s flags=0x%02x gender=%d body=%d variants=%d afterPawn=%d) to %s"),
            *Loadout.CharacterTemplateId, *HeroDef->GetName(),
            static_cast<int32>(PlayerState->CharacterData.WasPartReplicatedFlags),
            static_cast<int32>(Gender), static_cast<int32>(BodyType),
            ResolvedChannels.Num(), bAfterPawnSpawn ? 1 : 0, *PlayerState->GetName());
        return true;
    }
}
