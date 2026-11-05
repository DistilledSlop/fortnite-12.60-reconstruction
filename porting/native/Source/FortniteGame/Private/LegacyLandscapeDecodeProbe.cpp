#include "LegacyLandscapeDecodeProbe.h"
#if PLATFORM_LINUX && !UE_SERVER
#include "LegacyOriginalMaterialReader.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"

namespace
{
using FObj = TSharedPtr<FJsonObject>;
using FArr = TArray<TSharedPtr<FJsonValue>>;
bool Object(const FObj& O, const TCHAR* Key, FObj& Out)
{
    const TSharedPtr<FJsonObject>* P = nullptr;
    if (!O.IsValid() || !O->TryGetObjectField(Key, P) || !P || !P->IsValid()) return false;
    Out = *P; return true;
}
bool Array(const FObj& O, const TCHAR* Key, const FArr*& Out)
{ return O.IsValid() && O->TryGetArrayField(Key, Out) && Out; }
bool Number(const FObj& O, const TCHAR* Key, int32& Out)
{
    double N = 0;
    if (!O.IsValid() || !O->TryGetNumberField(Key, N) || N < 0 || N > 67108864 || N != double(int32(N))) return false;
    Out = int32(N); return true;
}
bool Item(const TSharedPtr<FJsonValue>& V, FObj& Out)
{
    if (!V.IsValid() || V->Type != EJson::Object) return false;
    Out = V->AsObject(); return Out.IsValid();
}
int32 Nibble(TCHAR C)
{
    if (C >= '0' && C <= '9') return C-'0';
    if (C >= 'a' && C <= 'f') return C-'a'+10;
    if (C >= 'A' && C <= 'F') return C-'A'+10;
    return -1;
}
bool Collect(const FObj& Container, const TCHAR* Key, TArray<FObj>& Records)
{
    const FArr* A = nullptr; if (!Array(Container, Key, A)) return false;
    for (const auto& V : *A) { FObj R; if (!Item(V, R)) return false; Records.Add(R); }
    return true;
}
bool Decode(const FString& Path, FString& Error)
{
    if (FPaths::IsRelative(Path)) { Error=TEXT("ingress path must be absolute"); return false; }
    FString Text; FObj Root;
    if (!FFileHelper::LoadFileToString(Text, *Path) || !FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text), Root) || !Root.IsValid())
    { Error=TEXT("cannot read ingress JSON"); return false; }
    FString Schema, Hex; FObj Framing, Decoded; const FArr *Names=nullptr, *Maps=nullptr;
    if (!Root->TryGetStringField(TEXT("schema"),Schema) || Schema!=TEXT("fortnite.legacy-landscape-ingress.v1") ||
        !Root->TryGetStringField(TEXT("resource_raw_hex"),Hex) || Hex.Len()==0 || Hex.Len()>128*1024*1024 || (Hex.Len()&1) ||
        !Object(Root,TEXT("framing"),Framing) || !Array(Framing,TEXT("names"),Names) || Names->Num()!=100 ||
        !Object(Root,TEXT("decoded"),Decoded) || !Array(Decoded,TEXT("maps"),Maps) || Maps->Num()!=2)
    { Error=TEXT("invalid schema, framing or maps"); return false; }
    TArray<FName> LocalNames;
    for (const auto& V:*Names)
    {
        if (!V.IsValid() || V->Type!=EJson::String || V->AsString().Len()>1024) { Error=TEXT("invalid local name"); return false; }
        LocalNames.Add(FName(*V->AsString()));
    }
    TArray<uint8> Raw; Raw.SetNumUninitialized(Hex.Len()/2);
    for(int32 I=0; I<Raw.Num(); ++I)
    {
        int32 A=Nibble(Hex[2*I]), B=Nibble(Hex[2*I+1]);
        if (A<0 || B<0) { Error=TEXT("invalid resource hex"); return false; }
        Raw[I]=uint8((A<<4)|B);
    }
    int32 PreviousEnd=0;
    for (int32 MapIndex=0; MapIndex<Maps->Num(); ++MapIndex)
    {
        FObj Entry, Expected, Material; const FArr* Meshes=nullptr; int32 Start=0, End=0, Count=0, Quality=0, Feature=0;
        if (!Item((*Maps)[MapIndex],Entry) || !Number(Entry,TEXT("offset"),Start) || !Number(Entry,TEXT("end"),End) ||
            Start!=PreviousEnd || End<=Start || End>Raw.Num() || !Object(Entry,TEXT("decoded"),Expected) ||
            !Number(Expected,TEXT("shader_record_count"),Count) || Count!=68 || !Number(Expected,TEXT("quality_level"),Quality) ||
            !Number(Expected,TEXT("feature_level"),Feature) || !Object(Expected,TEXT("material_shaders"),Material) ||
            !Array(Expected,TEXT("mesh_shader_maps"),Meshes)) { Error=TEXT("invalid map boundaries or receipt"); return false; }
        TArray<FObj> Records; if (!Collect(Material,TEXT("records"),Records)) { Error=TEXT("invalid material records"); return false; }
        for (const auto& V:*Meshes)
        {
            FObj Mesh; if (!Item(V,Mesh) || !Collect(Mesh,TEXT("records"),Records)) { Error=TEXT("invalid mesh records"); return false; }
            const FArr* Pipelines=nullptr;
            if (Array(Mesh,TEXT("pipelines"),Pipelines)) for (const auto& PV:*Pipelines)
            { FObj Pipeline; if (!Item(PV,Pipeline) || !Collect(Pipeline,TEXT("stages"),Records)) { Error=TEXT("invalid stages"); return false; } }
        }
        if (Records.Num()!=Count) { Error=TEXT("record count mismatch"); return false; }
        FLegacyOriginalMaterial Native; Native.Names=LocalNames; Native.Bytes.Append(Raw.GetData()+Start,End-Start);
        Native.SerializedOffset=Start;
        FString DecodeError;
        if (!DecodeLegacyOriginalMaterial(Native,DecodeError)) { Error=FString::Printf(TEXT("map %d: %s"),MapIndex,*DecodeError); return false; }
        FString Friendly, PathExpected, MapHash;
        if (!Expected->TryGetStringField(TEXT("friendly_name"),Friendly) || !Expected->TryGetStringField(TEXT("material_path"),PathExpected) ||
            !Expected->TryGetStringField(TEXT("cooked_shader_map_id_hash"),MapHash) || Native.FriendlyName!=Friendly || Native.MaterialPath!=PathExpected ||
            !Native.IR.OriginalMapHash.ToString().Equals(MapHash,ESearchCase::IgnoreCase) || int32(Native.IR.QualityLevel)!=Quality ||
            int32(Native.IR.FeatureLevel)!=Feature || Native.Records.Num()!=Count)
        { Error=TEXT("map identity mismatch"); return false; }
        for (int32 I=0; I<Count; ++I)
        {
            const auto& R=Native.Records[I]; const FObj& E=Records[I]; FObj Type, Hash, Layout; FString TypeName, HashValue; int32 Offset=0, EndOffset=0, Size=0;
            const FArr *Offsets=nullptr, *Types=nullptr;
            if (!Object(E,TEXT("type"),Type) || !Type->TryGetStringField(TEXT("name"),TypeName) || !Object(E,TEXT("output_hash"),Hash) ||
                !Hash->TryGetStringField(TEXT("hash"),HashValue) || !Number(E,TEXT("offset"),Offset) || !Number(E,TEXT("end"),EndOffset) ||
                !Object(E,TEXT("debug_uniform_buffer_layout"),Layout) || !Number(Layout,TEXT("constant_bytes"),Size) ||
                !Array(Layout,TEXT("resource_offsets"),Offsets) || !Array(Layout,TEXT("resource_types"),Types) ||
                R.Type!=TypeName || R.Begin+Start!=Offset || R.End+Start!=EndOffset || !R.OutputHash.ToString().Equals(HashValue,ESearchCase::IgnoreCase) ||
                R.LayoutSize!=uint32(Size) || R.LayoutOffsets.Num()!=Offsets->Num() || R.LayoutTypes.Num()!=Types->Num())
            { Error=FString::Printf(TEXT("map %d record %d identity/layout mismatch"),MapIndex,I); return false; }
            for (int32 J=0; J<Offsets->Num(); ++J)
                if (!(*Offsets)[J].IsValid() || (*Offsets)[J]->Type!=EJson::Number || !(*Types)[J].IsValid() || (*Types)[J]->Type!=EJson::Number ||
                    (*Offsets)[J]->AsNumber()!=R.LayoutOffsets[J] || (*Types)[J]->AsNumber()!=R.LayoutTypes[J])
                { Error=TEXT("layout member mismatch"); return false; }
        }
        UE_LOG(LogTemp,Display,TEXT("Legacy landscape decode: map=%d quality=%d feature=%d records=%d hash=%s"),MapIndex,Quality,Feature,Count,*MapHash);
        PreviousEnd=End;
    }
    if (PreviousEnd!=Raw.Num()) { Error=TEXT("resource has trailing bytes"); return false; }
    return true;
}
}
bool ProbeLegacyLandscapeDecode()
{
    FString Path;
    if (!FParse::Value(FCommandLine::Get(),TEXT("FortniteProbeLegacyLandscapeDecode="),Path))
    {
        if (!FParse::Param(FCommandLine::Get(),TEXT("FortniteProbeLegacyLandscapeDecode"))) return false;
        UE_LOG(LogTemp,Error,TEXT("Legacy landscape decode: missing ingress path")); FPlatformMisc::RequestExitWithStatus(false,1); return true;
    }
    FString Error; const bool Ok=Decode(Path,Error);
    if (Ok) { UE_LOG(LogTemp,Display,TEXT("Legacy landscape decode: SUCCESS (decoder only)")); }
    else { UE_LOG(LogTemp,Error,TEXT("Legacy landscape decode: FAILURE: %s"),*Error); }
    FPlatformMisc::RequestExitWithStatus(false,Ok ? 0 : 1); return true;
}
#endif
