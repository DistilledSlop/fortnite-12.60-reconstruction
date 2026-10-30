#include "FortniteBackgroundMaterialsCommandlet.h"

#include "AssetRegistryModule.h"
#include "Dom/JsonObject.h"
#include "Engine/Texture2D.h"
#include "HAL/FileManager.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionAbs.h"
#include "Materials/MaterialExpressionAdd.h"
#include "Materials/MaterialExpressionAppendVector.h"
#include "Materials/MaterialExpressionCameraVectorWS.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionDivide.h"
#include "Materials/MaterialExpressionDotProduct.h"
#include "Materials/MaterialExpressionLinearInterpolate.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionNormalize.h"
#include "Materials/MaterialExpressionOneMinus.h"
#include "Materials/MaterialExpressionPixelDepth.h"
#include "Materials/MaterialExpressionPower.h"
#include "Materials/MaterialExpressionSaturate.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionSubtract.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionVertexNormalWS.h"
#include "Materials/MaterialExpressionWorldPosition.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/Package.h"

namespace
{
// Direction toward the sun for AApolloSunLight at rotation (pitch -50, yaw -35).
const FLinearColor SunToward(-0.5266f, 0.3688f, 0.7660f);

struct FGraph
{
    UMaterial* Material;

    template <class T>
    T* Make()
    {
        T* Expression = NewObject<T>(Material);
        Material->Expressions.Add(Expression);
        return Expression;
    }
    UMaterialExpression* Scalar(const TCHAR* Name, float Default)
    {
        auto* E = Make<UMaterialExpressionScalarParameter>();
        E->ParameterName = Name;
        E->DefaultValue = Default;
        return E;
    }
    UMaterialExpression* Vector(const TCHAR* Name, const FLinearColor& Default)
    {
        auto* E = Make<UMaterialExpressionVectorParameter>();
        E->ParameterName = Name;
        E->DefaultValue = Default;
        return E;
    }
    UMaterialExpression* Const(float Value)
    {
        auto* E = Make<UMaterialExpressionConstant>();
        E->R = Value;
        return E;
    }
    UMaterialExpression* Const3(const FLinearColor& Value)
    {
        auto* E = Make<UMaterialExpressionConstant3Vector>();
        E->Constant = Value;
        return E;
    }
    UMaterialExpression* Mul(UMaterialExpression* A, UMaterialExpression* B, int32 OutA = 0, int32 OutB = 0)
    {
        auto* E = Make<UMaterialExpressionMultiply>();
        E->A.Connect(OutA, A);
        E->B.Connect(OutB, B);
        return E;
    }
    UMaterialExpression* Mul(UMaterialExpression* A, float B)
    {
        auto* E = Make<UMaterialExpressionMultiply>();
        E->A.Connect(0, A);
        E->ConstB = B;
        return E;
    }
    UMaterialExpression* Add(UMaterialExpression* A, UMaterialExpression* B)
    {
        auto* E = Make<UMaterialExpressionAdd>();
        E->A.Connect(0, A);
        E->B.Connect(0, B);
        return E;
    }
    UMaterialExpression* Add(UMaterialExpression* A, float B)
    {
        auto* E = Make<UMaterialExpressionAdd>();
        E->A.Connect(0, A);
        E->ConstB = B;
        return E;
    }
    UMaterialExpression* Sub(UMaterialExpression* A, UMaterialExpression* B)
    {
        auto* E = Make<UMaterialExpressionSubtract>();
        E->A.Connect(0, A);
        E->B.Connect(0, B);
        return E;
    }
    UMaterialExpression* Div(UMaterialExpression* A, UMaterialExpression* B)
    {
        auto* E = Make<UMaterialExpressionDivide>();
        E->A.Connect(0, A);
        E->B.Connect(0, B);
        return E;
    }
    UMaterialExpression* Pow(UMaterialExpression* Base, UMaterialExpression* Exponent)
    {
        auto* E = Make<UMaterialExpressionPower>();
        E->Base.Connect(0, Base);
        E->Exponent.Connect(0, Exponent);
        return E;
    }
    UMaterialExpression* Sat(UMaterialExpression* A)
    {
        auto* E = Make<UMaterialExpressionSaturate>();
        E->Input.Connect(0, A);
        return E;
    }
    UMaterialExpression* Lerp(UMaterialExpression* A, UMaterialExpression* B, UMaterialExpression* Alpha, int32 OutA = 0)
    {
        auto* E = Make<UMaterialExpressionLinearInterpolate>();
        E->A.Connect(OutA, A);
        E->B.Connect(0, B);
        E->Alpha.Connect(0, Alpha);
        return E;
    }
    UMaterialExpression* Dot(UMaterialExpression* A, UMaterialExpression* B)
    {
        auto* E = Make<UMaterialExpressionDotProduct>();
        E->A.Connect(0, A);
        E->B.Connect(0, B);
        return E;
    }
    UMaterialExpression* Normalize(UMaterialExpression* A)
    {
        auto* E = Make<UMaterialExpressionNormalize>();
        E->VectorInput.Connect(0, A);
        return E;
    }
    UMaterialExpression* Append(UMaterialExpression* A, UMaterialExpression* B, int32 OutA = 0, int32 OutB = 0)
    {
        auto* E = Make<UMaterialExpressionAppendVector>();
        E->A.Connect(OutA, A);
        E->B.Connect(OutB, B);
        return E;
    }
    UMaterialExpression* Mask(UMaterialExpression* A, bool R, bool G, bool B)
    {
        auto* E = Make<UMaterialExpressionComponentMask>();
        E->Input.Connect(0, A);
        E->R = R;
        E->G = G;
        E->B = B;
        E->A = false;
        return E;
    }
    UMaterialExpression* Sample(UTexture2D* Texture, UMaterialExpression* UV)
    {
        auto* E = Make<UMaterialExpressionTextureSample>();
        E->Texture = Texture;
        E->SamplerType = Texture && !Texture->SRGB ? SAMPLERTYPE_LinearColor : SAMPLERTYPE_Color;
        if (UV)
            E->Coordinates.Connect(0, UV);
        return E;
    }
};

float ScalarValue(const TSharedPtr<FJsonObject>& Values, const TCHAR* Name, float Default)
{
    double Value = Default;
    if (Values.IsValid())
        Values->TryGetNumberField(Name, Value);
    return Value;
}

FLinearColor VectorValue(const TSharedPtr<FJsonObject>& Values, const TCHAR* Name, const FLinearColor& Default)
{
    const TArray<TSharedPtr<FJsonValue>>* Array = nullptr;
    if (Values.IsValid() && Values->TryGetArrayField(Name, Array) && Array->Num() >= 3)
        return FLinearColor((*Array)[0]->AsNumber(), (*Array)[1]->AsNumber(), (*Array)[2]->AsNumber(),
            Array->Num() > 3 ? (*Array)[3]->AsNumber() : 1.0f);
    return Default;
}

bool SavePackageAsset(UObject* Asset)
{
    UPackage* Package = Asset->GetOutermost();
    Package->MarkPackageDirty();
    const FString Filename = FPackageName::LongPackageNameToFilename(
        Package->GetName(), FPackageName::GetAssetPackageExtension());
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename), true);
    if (!UPackage::SavePackage(Package, Asset, RF_Standalone, *Filename, GError))
    {
        UE_LOG(LogTemp, Error, TEXT("Could not save %s"), *Filename);
        return false;
    }
    UE_LOG(LogTemp, Display, TEXT("Saved reconstructed asset: %s"), *Filename);
    return true;
}

UMaterial* NewMaterial(const FString& PackageName, EBlendMode Blend)
{
    UPackage* Package = CreatePackage(nullptr, *PackageName);
    UMaterial* Material = NewObject<UMaterial>(Package, *FPackageName::GetShortName(PackageName),
        RF_Public | RF_Standalone);
    Material->BlendMode = Blend;
    Material->SetShadingModel(MSM_Unlit);
    return Material;
}

UTexture2D* LoadLinearTexture(const FString& PackageName, bool bFlagLinear)
{
    const FString ObjectPath = PackageName + TEXT(".") + FPackageName::GetShortName(PackageName);
    UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, *ObjectPath);
    if (!Texture)
    {
        UE_LOG(LogTemp, Error, TEXT("Texture did not load: %s"), *ObjectPath);
        return nullptr;
    }
    if (bFlagLinear && (Texture->SRGB || Texture->CompressionSettings != TC_VectorDisplacementmap))
    {
        // Keep the detail normal and noise alpha exact: linear data, uncompressed BGRA8.
        Texture->SRGB = false;
        Texture->CompressionSettings = TC_VectorDisplacementmap;
        Texture->MipGenSettings = TMGS_FromTextureGroup;
        Texture->PostEditChange();
        SavePackageAsset(Texture);
    }
    return Texture;
}

// Simplified reconstruction of M_Volcano_Cloud: unlit translucent cloud shell.
// The cooked graph keeps 46 of 111 expressions, so the shading below is built
// from the surviving parameters and texture, not translated from the original.
UMaterial* BuildCloud(const TSharedPtr<FJsonObject>& Spec)
{
    const FString PackageName = Spec->GetStringField(TEXT("package"));
    UTexture2D* Detail = LoadLinearTexture(Spec->GetStringField(TEXT("texture")), true);
    if (!Detail)
        return nullptr;
    const TSharedPtr<FJsonObject> Scalars = Spec->GetObjectField(TEXT("scalars"));
    const TSharedPtr<FJsonObject> Vectors = Spec->GetObjectField(TEXT("vectors"));
    UMaterial* Material = NewMaterial(PackageName, BLEND_Translucent);
    FGraph G{Material};

    auto* TexCoord = G.Make<UMaterialExpressionTextureCoordinate>();
    UMaterialExpression* Tile = G.Append(G.Scalar(TEXT("Tile_N_X"), ScalarValue(Scalars, TEXT("Tile_N_X"), 2)),
        G.Scalar(TEXT("Tile_N_Y"), ScalarValue(Scalars, TEXT("Tile_N_Y"), 6)));
    UMaterialExpression* Sample = G.Sample(Detail, G.Mul(TexCoord, Tile));

    // Detail normal xy from the texture's red and green, flattened by FlattenDetailNormal.
    UMaterialExpression* Rg = G.Append(Sample, Sample, 1, 2);
    UMaterialExpression* Xy = G.Add(G.Mul(Rg, 2.0f), -1.0f);
    UMaterialExpression* Flatten = G.Scalar(TEXT("FlattenDetailNormal"), ScalarValue(Scalars, TEXT("FlattenDetailNormal"), 0.5f));
    UMaterialExpression* Perturb = G.Append(G.Mul(Xy, Flatten), G.Const(0.0f));
    UMaterialExpression* Normal = G.Normalize(G.Add(G.Make<UMaterialExpressionVertexNormalWS>(), Perturb));

    // Shadow to light color by wrapped sun lighting, softened by SoftenShadow.
    UMaterialExpression* Wrap = G.Sat(G.Add(G.Mul(G.Dot(Normal, G.Const3(SunToward)), 0.5f), 0.5f));
    UMaterialExpression* Soften = G.Scalar(TEXT("SoftenShadow"), ScalarValue(Scalars, TEXT("SoftenShadow"), 2.4f));
    UMaterialExpression* Lit = G.Pow(Wrap, G.Div(G.Const(1.0f), Soften));
    UMaterialExpression* Light = G.Vector(TEXT("LightColorDay"), VectorValue(Vectors, TEXT("LightColorDay"), FLinearColor::White));
    UMaterialExpression* Shadow = G.Vector(TEXT("ShadowColorDay"), VectorValue(Vectors, TEXT("ShadowColorDay"), FLinearColor::Gray));
    Material->EmissiveColor.Connect(0, G.Lerp(Shadow, Light, Lit));

    // Opacity: silhouette fade by view angle. Viewed from below, the "Under" values
    // apply (AngleFadeSensitivityU, OpacityUnder); from above, Opacity applies.
    auto* CameraVector = G.Make<UMaterialExpressionCameraVectorWS>();
    UMaterialExpression* Under = G.Sat(G.Mul(G.Mask(CameraVector, false, false, true), -4.0f));
    UMaterialExpression* ViewDot = G.Make<UMaterialExpressionAbs>();
    Cast<UMaterialExpressionAbs>(ViewDot)->Input.Connect(0, G.Dot(Normal, CameraVector));
    UMaterialExpression* Sensitivity = G.Lerp(
        G.Scalar(TEXT("AngleFadeSensitivity"), ScalarValue(Scalars, TEXT("AngleFadeSensitivity"), 2.4f)),
        G.Scalar(TEXT("AngleFadeSensitivityU"), ScalarValue(Scalars, TEXT("AngleFadeSensitivityU"), 3.0f)), Under);
    UMaterialExpression* AngleFade = G.Pow(G.Sat(ViewDot), Sensitivity);
    UMaterialExpression* Gain = G.Mul(AngleFade, G.Lerp(
        G.Scalar(TEXT("Opacity"), ScalarValue(Scalars, TEXT("Opacity"), 0.3f)),
        G.Scalar(TEXT("OpacityUnder"), ScalarValue(Scalars, TEXT("OpacityUnder"), 2.0f)), Under));
    UMaterialExpression* NearFade = G.Sat(G.Div(G.Make<UMaterialExpressionPixelDepth>(),
        G.Scalar(TEXT("CameraDistanceFade"), ScalarValue(Scalars, TEXT("CameraDistanceFade"), 1200.0f))));
    UMaterialExpression* Puff = G.Lerp(G.Const(0.7f), G.Const(1.0f), Sample, 0);
    Cast<UMaterialExpressionLinearInterpolate>(Puff)->Alpha.Connect(4, Sample);
    Material->Opacity.Connect(0, G.Sat(G.Mul(G.Mul(Gain, NearFade), Puff)));

    Material->PostEditChange();
    FAssetRegistryModule::AssetCreated(Material);
    return SavePackageAsset(Material) ? Material : nullptr;
}

// Simplified reconstruction of M_BackDrop_P: opaque unlit terrain backdrop.
UMaterial* BuildBackdrop(const TSharedPtr<FJsonObject>& Spec)
{
    const FString PackageName = Spec->GetStringField(TEXT("package"));
    UTexture2D* Cliff = LoadObject<UTexture2D>(nullptr,
        *(Spec->GetStringField(TEXT("texture")) + TEXT(".") + FPackageName::GetShortName(Spec->GetStringField(TEXT("texture")))));
    if (!Cliff)
    {
        UE_LOG(LogTemp, Error, TEXT("Cliff texture did not load"));
        return nullptr;
    }
    UMaterial* Material = NewMaterial(PackageName, BLEND_Opaque);
    FGraph G{Material};
    // Defaults are assumptions for values the M_BackDrop_P cook did not report;
    // every child instance overrides the grass colors it uses.
    UMaterialExpression* Grass = G.Vector(TEXT("Grass"), FLinearColor(0.08f, 0.14f, 0.03f));
    UMaterialExpression* GrassLow = G.Vector(TEXT("Grass Low"), FLinearColor(0.06f, 0.21f, 0.0f));
    UMaterialExpression* Grass2 = G.Vector(TEXT("Grass2"), FLinearColor(0.08f, 0.07f, 0.04f));
    UMaterialExpression* Waterline = G.Vector(TEXT("Cliffs Waterline"), FLinearColor(0.78f, 0.78f, 0.78f));

    auto* Position = G.Make<UMaterialExpressionWorldPosition>();
    UMaterialExpression* Normal = G.Make<UMaterialExpressionVertexNormalWS>();
    UMaterialExpression* Up = G.Mask(Normal, false, false, true);

    // Grass wherever the surface is flat enough; contrast and transition are recovered parameters.
    UMaterialExpression* Transition = G.Scalar(TEXT("Transition Grass"), 0.5f);
    UMaterialExpression* ContrastGrass = G.Scalar(TEXT("Contrast Grass"), 4.0f);
    auto* OneMinus = G.Make<UMaterialExpressionOneMinus>();
    OneMinus->Input.Connect(0, Transition);
    UMaterialExpression* GrassMask = G.Sat(G.Mul(G.Sub(G.Sat(Up), OneMinus), ContrastGrass));

    // Tone by height between Grass Low and Grass, then a Grass2 patch variation.
    UMaterialExpression* Height = G.Mask(Position, false, false, true);
    UMaterialExpression* ToneHeight = G.Scalar(TEXT("GrassTone Height"), -30000.0f);
    UMaterialExpression* Tone = G.Sat(G.Div(G.Sub(Height, ToneHeight), G.Const(40000.0f)));
    UMaterialExpression* Xy = G.Mask(Position, true, true, false);
    UMaterialExpression* Tile = G.Scalar(TEXT("DetailTile"), 130000.0f);
    UMaterialExpression* NoiseSample = G.Sample(Cliff, G.Mul(G.Div(Xy, Tile), 16.0f));
    UMaterialExpression* Patch = G.Sat(G.Mul(NoiseSample, G.Scalar(TEXT("Detail Value"), 0.5f), 1, 0));
    UMaterialExpression* GrassColor = G.Lerp(G.Lerp(GrassLow, Grass, Tone), Grass2, Patch);

    // Cliff color from the recovered original cliff diffuse, with a waterline band.
    UMaterialExpression* RockSample = G.Sample(Cliff, G.Div(Xy, G.Const(16384.0f)));
    UMaterialExpression* WaterlineHeight = G.Scalar(TEXT("Cliffs Waterline Height"), -100000.0f);
    UMaterialExpression* WaterlineBlur = G.Scalar(TEXT("Cliffs Waterline Blur"), 10000.0f);
    UMaterialExpression* Band = G.Sat(G.Div(G.Sub(WaterlineHeight, Height), WaterlineBlur));
    UMaterialExpression* RockColor = G.Lerp(RockSample, Waterline, Band);

    UMaterialExpression* Albedo = G.Lerp(RockColor, GrassColor, GrassMask);
    // Wrapped sun shading stands in for the original's material-computed lighting.
    UMaterialExpression* Wrap = G.Sat(G.Add(G.Mul(G.Dot(Normal, G.Const3(SunToward)), 0.5f), 0.5f));
    UMaterialExpression* Shade = G.Add(G.Mul(Wrap, 0.55f), 0.45f);
    Material->EmissiveColor.Connect(0, G.Mul(Albedo, Shade));

    Material->PostEditChange();
    FAssetRegistryModule::AssetCreated(Material);
    return SavePackageAsset(Material) ? Material : nullptr;
}
}

UFortniteBackgroundMaterialsCommandlet::UFortniteBackgroundMaterialsCommandlet()
{
    IsClient = false;
    IsEditor = true;
    IsServer = false;
    LogToConsole = true;
}

int32 UFortniteBackgroundMaterialsCommandlet::Main(const FString& Params)
{
    FString SpecPath;
    FString Text;
    if (!FParse::Value(*Params, TEXT("Spec="), SpecPath) || !FFileHelper::LoadFileToString(Text, *SpecPath))
    {
        UE_LOG(LogTemp, Error, TEXT("Use -Spec=/path/apollo-background-materials.json"));
        return 2;
    }
    TSharedPtr<FJsonObject> Spec;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text), Spec) || !Spec.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid spec: %s"), *SpecPath);
        return 2;
    }

    TMap<FString, UMaterialInterface*> Built;
    const bool bSkipCloud = FParse::Param(*Params, TEXT("SkipCloud"));
    const bool bSkipBackdrop = FParse::Param(*Params, TEXT("SkipBackdrop"));
    if (!bSkipCloud)
    {
        const TSharedPtr<FJsonObject> Cloud = Spec->GetObjectField(TEXT("cloud"));
        UMaterial* Material = BuildCloud(Cloud);
        if (!Material)
            return 3;
        Built.Add(Cloud->GetStringField(TEXT("package")), Material);
    }
    if (!bSkipBackdrop)
    {
        const TSharedPtr<FJsonObject> Backdrop = Spec->GetObjectField(TEXT("backdrop"));
        UMaterial* Material = BuildBackdrop(Backdrop);
        if (!Material)
            return 4;
        Built.Add(Backdrop->GetStringField(TEXT("package")), Material);
    }

    int32 Applied = 0;
    int32 Ignored = 0;
    for (const TSharedPtr<FJsonValue>& Value : Spec->GetArrayField(TEXT("instances")))
    {
        const TSharedPtr<FJsonObject> Instance = Value->AsObject();
        const FString PackageName = Instance->GetStringField(TEXT("package"));
        const FString ParentName = Instance->GetStringField(TEXT("parent"));
        UMaterialInterface** Parent = Built.Find(ParentName);
        if (!Parent)
            continue; // Its family was skipped on this run.
        UPackage* Package = CreatePackage(nullptr, *PackageName);
        UMaterialInstanceConstant* Constant = NewObject<UMaterialInstanceConstant>(Package,
            *FPackageName::GetShortName(PackageName), RF_Public | RF_Standalone);
        Constant->SetParentEditorOnly(*Parent);
        TArray<FMaterialParameterInfo> ScalarInfos;
        TArray<FMaterialParameterInfo> VectorInfos;
        TArray<FGuid> Ids;
        (*Parent)->GetAllScalarParameterInfo(ScalarInfos, Ids);
        (*Parent)->GetAllVectorParameterInfo(VectorInfos, Ids);
        auto Has = [](const TArray<FMaterialParameterInfo>& Infos, const FString& Name)
        {
            return Infos.ContainsByPredicate([&](const FMaterialParameterInfo& Info) { return Info.Name.ToString() == Name; });
        };
        for (const auto& Pair : Instance->GetObjectField(TEXT("scalars"))->Values)
        {
            if (Has(ScalarInfos, Pair.Key))
            {
                Constant->SetScalarParameterValueEditorOnly(FMaterialParameterInfo(*Pair.Key), Pair.Value->AsNumber());
                Applied++;
            }
            else
            {
                UE_LOG(LogTemp, Display, TEXT("%s: scalar %s has no counterpart in the rebuilt parent"),
                    *PackageName, *Pair.Key);
                Ignored++;
            }
        }
        for (const auto& Pair : Instance->GetObjectField(TEXT("vectors"))->Values)
        {
            if (Has(VectorInfos, Pair.Key))
            {
                Constant->SetVectorParameterValueEditorOnly(FMaterialParameterInfo(*Pair.Key),
                    VectorValue(Instance->GetObjectField(TEXT("vectors")), *Pair.Key, FLinearColor::White));
                Applied++;
            }
            else
            {
                UE_LOG(LogTemp, Display, TEXT("%s: vector %s has no counterpart in the rebuilt parent"),
                    *PackageName, *Pair.Key);
                Ignored++;
            }
        }
        Constant->PostEditChange();
        FAssetRegistryModule::AssetCreated(Constant);
        if (!SavePackageAsset(Constant))
            return 5;
        Built.Add(PackageName, Constant);
    }
    UE_LOG(LogTemp, Display, TEXT("Background materials: %d assets, %d instance parameters applied, %d without counterpart"),
        Built.Num(), Applied, Ignored);
    return 0;
}
