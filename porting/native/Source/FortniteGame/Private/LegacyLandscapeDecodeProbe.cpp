#include "LegacyLandscapeDecodeProbe.h"
#if PLATFORM_LINUX && !UE_SERVER
#include "LegacyOriginalMaterialReader.h"
#include "LegacyOriginalUniformContracts.h"
#include "LegacyOriginalMaterialLayout.h"
#include "LegacyLocalVFOutputBatch.h"
#include "LegacyLandscapeVFOutputBatch.h"
#include "LegacyTerrainRuntimeInventory.h"
#include "LegacySlateMaterial.h"
#include "MaterialShared.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Guid.h"
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
    FString BatchPath, BatchHash, MapPins[2];
    const bool bProbeBatch=FParse::Value(FCommandLine::Get(),TEXT("FortniteProbeLandscapeManifest="),BatchPath);
    if (bProbeBatch && (!FParse::Value(FCommandLine::Get(),TEXT("FortniteProbeLandscapeManifestSHA256="),BatchHash) ||
        !FParse::Value(FCommandLine::Get(),TEXT("FortniteProbeLandscapeMap0SHA256="),MapPins[0]) ||
        !FParse::Value(FCommandLine::Get(),TEXT("FortniteProbeLandscapeMap1SHA256="),MapPins[1])))
    { Error=TEXT("batch probe requires manifest and both map SHA-256 pins"); return false; }
    FString InventoryPath, InventoryPin;
    const bool bProbeInventory=FParse::Value(FCommandLine::Get(),TEXT("FortniteProbeLandscapeInventory="),InventoryPath);
    if (bProbeInventory && (!bProbeBatch || !FParse::Value(FCommandLine::Get(),TEXT("FortniteProbeLandscapeInventorySHA256="),InventoryPin) || InventoryPin.Len()!=64))
    { Error=TEXT("inventory probe requires a uniform bundle pin and pinned batch manifest/maps"); return false; }
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
        if (bProbeInventory)
        {
            TArray<FLegacyTerrainRuntimeInventoryRecord> Inventory;
            if (!BuildLegacyTerrainRuntimeInventory(Native,BatchPath,BatchHash,MapPins[MapIndex],MapIndex,Start,InventoryPath,InventoryPin,Inventory,Error) || Inventory.Num()!=Count)
            { if (Error.IsEmpty()) Error=TEXT("terrain inventory count mismatch"); return false; }
            int32 UniformCount=0, LooseCount=0;
            for (const auto& Row:Inventory)
            {
                UniformCount+=Row.Uniforms.Num(); LooseCount+=Row.VFLoose.Num();
                for (const auto& Binding:Row.Uniforms)
                    if (Binding.bOriginalStaticSlotKnown) { Error=TEXT("unexpected resolved landscape static slot"); return false; }
            }
            TArray<FLegacyTerrainRuntimeInventoryRecord> Sentinel; Sentinel.AddDefaulted(); Sentinel[0].ShaderIndex=123456;
            FString Rejected, BadPin=InventoryPin; BadPin[0]=BadPin[0]==TEXT('0') ? TEXT('1') : TEXT('0');
            if (BuildLegacyTerrainRuntimeInventory(Native,BatchPath,BatchHash,MapPins[MapIndex],MapIndex,Start,InventoryPath,BadPin,Sentinel,Rejected) ||
                Sentinel.Num()!=1 || Sentinel[0].ShaderIndex!=123456)
            { Error=TEXT("terrain inventory digest rejection failed"); return false; }
            UE_LOG(LogTemp,Display,TEXT("Legacy terrain inventory: map=%d records=%d uniform occurrences=%d VF loose=%d; static slots unresolved"),MapIndex,Inventory.Num(),UniformCount,LooseCount);
        }
        FLegacyMaterialIR Prepared;
        if (!PrepareLegacyOriginalMaterialLayout(Native,Prepared,Error) || Prepared.UniformBufferSize!=1376 || Prepared.Resources.Num()!=77)
        { if (Error.IsEmpty()) Error=TEXT("unexpected landscape Material layout"); return false; }
        FLegacyOriginalMaterial ContractMaterial=Native; ContractMaterial.IR=Prepared;
        bool bReachedFactoryOutputs=false; FString FactoryError;
        TRefCountPtr<FMaterialShaderMap> FactoryMap=FMaterialShaderMap::CreateLegacyLandscapeVertexFactoryMap(
            Prepared, [&](const FUniformExpressionSet&, const FShaderParametersMetadata& Metadata,
                TArray<FLegacyMaterialShaderInput>&, TArray<FLegacyMaterialPipelineInput>&, FString& OutError) -> bool
            {
                FLegacyLocalUniformContract Contract;
                if (!BuildLegacyLandscapeMaterialUniformContract(ContractMaterial,0,224723792,Metadata,Contract,OutError))
                { OutError+=FString::Printf(TEXT(" (engine struct: %s)"),Metadata.GetStructTypeName()); return false; }
                if (bProbeBatch)
                {
                    TArray<FLegacyLocalVFRecordContracts> BatchContracts;
                    BatchContracts.SetNum(Native.Records.Num());
                    TArray<FLegacyMaterialShaderInput> BatchOutputs;
                    TArray<FLegacyMaterialPipelineInput> BatchPipelines;
                    FString BatchError;
                    const bool bBuilt=BuildLegacyLandscapeVFOutputBatch(ContractMaterial,BatchPath,BatchHash,MapPins[MapIndex],
                        MapIndex,Start,Metadata,BatchContracts,BatchOutputs,BatchPipelines,BatchError);
                    if (bBuilt || BatchOutputs.Num() || BatchPipelines.Num() || BatchError!=TEXT("incomplete original uniform contracts"))
                    { OutError=FString::Printf(TEXT("unexpected landscape batch result: %s"),*BatchError); return false; }
                    UE_LOG(LogTemp,Display,TEXT("Legacy landscape batch: map=%d incomplete-contract rejection: %s"),MapIndex,*BatchError);
                }
                bReachedFactoryOutputs=true;
                OutError=TEXT("probe stopped before shader construction");
                return false;
            }, FactoryError);
        if (FactoryMap.IsValid() || !bReachedFactoryOutputs || FactoryError!=TEXT("probe stopped before shader construction"))
        { Error=FString::Printf(TEXT("map %d material factory: %s"),MapIndex,*FactoryError); return false; }
        UE_LOG(LogTemp,Display,TEXT("Legacy landscape material factory: map=%d expression metadata validated; shader construction not attempted"),MapIndex);
        auto BadLayout=Native; BadLayout.Records[0].LayoutOffsets[0]++;
        FLegacyMaterialIR Sentinel; Sentinel.UniformBufferSize=7; FString Reject;
        if (PrepareLegacyOriginalMaterialLayout(BadLayout,Sentinel,Reject) || Sentinel.UniformBufferSize!=7)
        { Error=TEXT("invalid Material layout was not rejected transactionally"); return false; }
        FString Friendly, PathExpected, MapHash;
        if (!Expected->TryGetStringField(TEXT("friendly_name"),Friendly) || !Expected->TryGetStringField(TEXT("material_path"),PathExpected) ||
            !Expected->TryGetStringField(TEXT("cooked_shader_map_id_hash"),MapHash) || Native.FriendlyName!=Friendly || Native.MaterialPath!=PathExpected ||
            !Native.IR.OriginalMapHash.ToString().Equals(MapHash,ESearchCase::IgnoreCase) || int32(Native.IR.QualityLevel)!=Quality ||
            int32(Native.IR.FeatureLevel)!=Feature || Native.Records.Num()!=Count)
        { Error=TEXT("map identity mismatch"); return false; }
        int32 VFCount=0; bool bNegativeChecked=false;
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
            if (R.Frequency==SF_Vertex && !R.VertexFactory.IsNone())
            {
                FObj VF, Params;
                if (!Object(E,TEXT("vertex_factory_parameters"),VF) || !Object(VF,TEXT("landscape_parameters"),Params))
                { Error=TEXT("missing landscape VF receipt"); return false; }
                const struct { const TCHAR* Name; const TCHAR* Key; } Slots[]={
                    {TEXT("LodValues"),TEXT("lod_values")}, {TEXT("ForcedLod"),TEXT("forced_lod")},
                    {TEXT("LodTessellationParams"),TEXT("lod_tessellation")}, {TEXT("NeighborSectionLod"),TEXT("neighbor_section_lod")},
                    {TEXT("LodBias"),TEXT("lod_bias")}, {TEXT("SectionLods"),TEXT("section_lods")},
                };
                TArray<FLegacyLocalLooseContract> Contracts;
                if (!BuildLegacyLandscapeVFLooseContracts(R,Contracts,Error)) return false;
                int32 Bound=0;
                for (const auto& Slot:Slots)
                {
                    FObj P; int32 Base, SlotSize, Buffer;
                    if (!Object(Params,Slot.Key,P) || !Number(P,TEXT("base_index"),Base) ||
                        !Number(P,TEXT("byte_count"),SlotSize) || !Number(P,TEXT("buffer_index"),Buffer))
                    { Error=TEXT("invalid landscape VF receipt"); return false; }
                    if (!SlotSize) continue;
                    ++Bound; int32 Matches=0;
                    for (const auto& C:Contracts)
                        if (C.ParameterName==Slot.Name && C.SourceBuffer==Buffer && C.ByteOffset==Base && C.ByteSize==SlotSize) ++Matches;
                    if (Matches!=1) { Error=TEXT("landscape VF name/allocation mismatch"); return false; }
                }
                if (Bound!=Contracts.Num()) { Error=TEXT("unexpected landscape VF contract"); return false; }
                if (!bNegativeChecked && R.LandscapeLodValues.Size)
                {
                    auto Bad=R; Bad.LandscapeLodValues.BaseIndex=UINT16_MAX;
                    TArray<FLegacyLocalLooseContract> VFTestSentinel; VFTestSentinel.AddDefaulted_GetRef().ParameterName=TEXT("sentinel"); FString VFReject;
                    if (BuildLegacyLandscapeVFLooseContracts(Bad,VFTestSentinel,VFReject) || VFTestSentinel.Num()!=1 || VFTestSentinel[0].ParameterName!=TEXT("sentinel"))
                    { Error=TEXT("invalid VF allocation was not rejected transactionally"); return false; }
                    bNegativeChecked=true;
                }
                ++VFCount;
            }
            for (int32 J=0; J<Offsets->Num(); ++J)
                if (!(*Offsets)[J].IsValid() || (*Offsets)[J]->Type!=EJson::Number || !(*Types)[J].IsValid() || (*Types)[J]->Type!=EJson::Number ||
                    (*Offsets)[J]->AsNumber()!=R.LayoutOffsets[J] || (*Types)[J]->AsNumber()!=R.LayoutTypes[J])
                { Error=TEXT("layout member mismatch"); return false; }
        }
        if (VFCount!=35 || !bNegativeChecked) { Error=TEXT("landscape VF inventory mismatch"); return false; }
        UE_LOG(LogTemp,Display,TEXT("Legacy landscape decode: map=%d quality=%d feature=%d records=%d VF contracts=%d hash=%s"),MapIndex,Quality,Feature,Count,VFCount,*MapHash);
        PreviousEnd=End;
    }
    if (PreviousEnd!=Raw.Num()) { Error=TEXT("resource has trailing bytes"); return false; }
    return true;
}
// Exercises the real loader with caller-pinned generated bytes, without installing
// metadata or claiming that these contracts are ready for renderer binding.
bool CheckUniformBindings(const FString& Path, const FString& Pin, FString& Error)
{
    if (FPaths::IsRelative(Path)) { Error=TEXT("uniform bundle path must be absolute"); return false; }
    static const TCHAR* Names[]={TEXT("LandscapeParameters"),TEXT("LandscapeContinuousLODParameters"),
        TEXT("Primitive"),TEXT("PrecomputedLightingBuffer"),TEXT("View"),TEXT("Material"),
        TEXT("ShadowDepthPass"),TEXT("LightmapDensityPass")};
    static const int32 Expected[]={33,24,30,2,37,9,6,2};
    int32 Counts[8]={}; TArray<FLegacyOriginalUniformContractBinding> Bindings;
    if (!LoadLegacyLandscapeOriginalUniformBindings(Path,Pin,Bindings,Error)) return false;
    if (Bindings.Num()!=143) { Error=TEXT("uniform binding inventory size mismatch"); return false; }
    for (const auto& Binding:Bindings)
    {
        int32 I=0; while (I<8 && Binding.ParameterName!=Names[I]) ++I;
        if (I==8) { Error=TEXT("unexpected uniform role"); return false; }
        ++Counts[I];
        const bool Decoded=I!=5;
        const auto& Contract=Binding.Contract;
        if (Binding.bOriginalLayoutResolved!=Decoded || Binding.bOriginalStaticSlotKnown || Contract.CurrentMetadata ||
            (Decoded && (Contract.ParameterName!=Binding.ParameterName || Contract.SourceRegister!=Binding.SourceRegister ||
                Contract.OriginalResourceLayoutHash!=Binding.SourceResourceLayoutHash || !Contract.OriginalSize ||
                !Contract.OriginalMembers.Num() || !Contract.OriginalStaticSlotName.IsEmpty())) ||
            (!Decoded && (!Contract.ParameterName.IsEmpty() || Contract.OriginalSize || Contract.OriginalMembers.Num() ||
                Contract.OriginalResources.Num())))
        { Error=FString::Printf(TEXT("uniform role %s has incorrect decoded or unresolved state"),*Binding.ParameterName); return false; }
    }
    for (int32 I=0; I<8; ++I)
        if (Counts[I]!=Expected[I]) { Error=FString::Printf(TEXT("uniform role %s count mismatch"),Names[I]); return false; }

    TArray<uint8> Bytes;
    if (!FFileHelper::LoadFileToArray(Bytes,*Path) || !Bytes.Num()) { Error=TEXT("cannot read bundle for corruption check"); return false; }
    Bytes[Bytes.Num()/2]^=1;
    const FString Temp=FPaths::Combine(FPlatformProcess::UserTempDir(),
        FString::Printf(TEXT("legacy-landscape-uniform-%s.json"),*FGuid::NewGuid().ToString()));
    if (!FFileHelper::SaveArrayToFile(Bytes,*Temp)) { Error=TEXT("cannot write corrupted bundle copy"); return false; }
    TArray<FLegacyOriginalUniformContractBinding> Sentinel;
    Sentinel.AddDefaulted(); Sentinel[0].ParameterName=TEXT("sentinel");
    FString CorruptError;
    const bool Accepted=LoadLegacyLandscapeOriginalUniformBindings(Temp,Pin,Sentinel,CorruptError);
    IFileManager::Get().Delete(*Temp);
    if (Accepted || !CorruptError.Contains(TEXT("digest mismatch")) || Sentinel.Num()!=1 || Sentinel[0].ParameterName!=TEXT("sentinel"))
    { Error=TEXT("corrupted bundle was not rejected by digest pin with output unchanged"); return false; }
    UE_LOG(LogTemp,Display,TEXT("Legacy landscape uniform loader: 143 bindings, 134 decoded, 9 unresolved; static slots unknown; corrupted bundle rejected"));
    return true;
}

}
bool ProbeLegacyLandscapeDecode()
{
    FString UniformPath;
    if (FParse::Value(FCommandLine::Get(),TEXT("FortniteProbeLegacyLandscapeUniforms="),UniformPath))
    {
        FString Pin, Error;
        const bool Ok=FParse::Value(FCommandLine::Get(),TEXT("FortniteLegacyLandscapeUniformSHA256="),Pin) &&
            CheckUniformBindings(UniformPath,Pin,Error);
        if (Ok) { UE_LOG(LogTemp,Display,TEXT("Legacy landscape uniform loader: SUCCESS (loader only)")); }
        else { UE_LOG(LogTemp,Error,TEXT("Legacy landscape uniform loader: FAILURE: %s"),Error.IsEmpty() ? TEXT("missing SHA-256 pin") : *Error); }
        FPlatformMisc::RequestExitWithStatus(false,Ok ? 0 : 1); return true;
    }
    if (FParse::Param(FCommandLine::Get(),TEXT("FortniteProbeLegacyLandscapeUniforms")))
    {
        UE_LOG(LogTemp,Error,TEXT("Legacy landscape uniform loader: missing bundle path"));
        FPlatformMisc::RequestExitWithStatus(false,1); return true;
    }
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
