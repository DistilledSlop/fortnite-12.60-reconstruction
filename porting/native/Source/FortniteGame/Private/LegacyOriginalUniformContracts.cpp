#include "LegacyOriginalUniformContracts.h"

#if PLATFORM_LINUX && !UE_SERVER
#include "LegacyMaterialManifestSHA256.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"

namespace
{
using FObj = TSharedPtr<FJsonObject>;
constexpr uint32 MaxSize = 65536, MaxNodes = 65536;
struct FResource { FString Path; uint32 Offset, Type; };

bool Fail(FString& Error, const TCHAR* Message) { Error = Message; return false; }
bool Num(const FObj& O, const TCHAR* Key, uint32& V)
{
    double N;
    if (!O || !O->TryGetNumberField(Key, N) || !(N >= 0 && N <= 4294967295.0) || N != double(uint32(N))) return false;
    V = uint32(N); return true;
}
bool Str(const FObj& O, const TCHAR* Key, FString& V) { return O && O->TryGetStringField(Key, V); }
const TArray<TSharedPtr<FJsonValue>>* Arr(const FObj& O, const TCHAR* Key)
{
    const TArray<TSharedPtr<FJsonValue>>* A = nullptr;
    return O && O->TryGetArrayField(Key, A) ? A : nullptr;
}
FObj Obj(const FObj& O, const TCHAR* Key)
{
    const FObj* P = nullptr; return O && O->TryGetObjectField(Key, P) ? *P : nullptr;
}
bool Keys(const FObj& O, std::initializer_list<const TCHAR*> Required, std::initializer_list<const TCHAR*> Optional = {})
{
    if (!O) return false;
    TSet<FString> Allowed;
    for (auto K : Required) { Allowed.Add(K); if (!O->HasField(K)) return false; }
    for (auto K : Optional) Allowed.Add(K);
    for (const auto& P : O->Values) if (!Allowed.Contains(P.Key)) return false;
    return true;
}
bool UniqueKeys(const FString& Json)
{
    struct FScope { bool bObject; TSet<FString> Keys; };
    TArray<FScope> Stack; EJsonNotation N;
    auto Reader=TJsonReaderFactory<>::Create(Json);
    while (Reader->ReadNext(N))
    {
        if (N==EJsonNotation::Error) return false;
        if (N==EJsonNotation::ObjectEnd || N==EJsonNotation::ArrayEnd)
        {
            if (!Stack.Num() || Stack.Last().bObject!=(N==EJsonNotation::ObjectEnd)) return false;
            Stack.Pop(); continue;
        }
        if (Stack.Num() && Stack.Last().bObject)
        {
            const FString& Key=Reader->GetIdentifier();
            if (Stack.Last().Keys.Contains(Key)) return false;
            Stack.Last().Keys.Add(Key);
        }
        if (N==EJsonNotation::ObjectStart || N==EJsonNotation::ArrayStart)
        {
            if (Stack.Num()>=32) return false;
            FScope Scope; Scope.bObject=N==EJsonNotation::ObjectStart; Stack.Add(MoveTemp(Scope));
        }
    }
    return !Stack.Num();
}
bool Ident(const FString& S)
{
    if (S.IsEmpty() || S.Len() > 256) return false;
    for (int32 I=0; I<S.Len(); ++I)
        if (!((S[I]>=TEXT('a') && S[I]<=TEXT('z')) || (S[I]>=TEXT('A') && S[I]<=TEXT('Z')) || S[I]==TEXT('_') || (I && S[I]>=TEXT('0') && S[I]<=TEXT('9')))) return false;
    return true;
}
bool Type(uint32 Old, EUniformBufferBaseType& Out)
{
    switch (Old) {
    case 2: Out=UBMT_INT32; return true; case 3: Out=UBMT_UINT32; return true;
    case 4: Out=UBMT_FLOAT32; return true; case 5: Out=UBMT_TEXTURE; return true;
    case 6: Out=UBMT_SRV; return true; case 7: Out=UBMT_UAV; return true;
    case 8: Out=UBMT_SAMPLER; return true; case 17: Out=UBMT_NESTED_STRUCT; return true;
    default: return false; }
}
// Exporter v1 permits only scalar struct members and non-array resources.
bool Members(const TArray<TSharedPtr<FJsonValue>>& Items, uint32 ContainerSize, const FString& Prefix,
    uint32 Base, uint32 Depth, uint32& Nodes, TArray<FLegacyLocalMemberContract>& Out, TArray<FResource>& Resources)
{
    if (Depth>16 || !ContainerSize || ContainerSize>MaxSize || !Items.Num() || Items.Num()>int32(MaxNodes)) return false;
    TSet<FString> Names; TArray<uint8> Occupied; Occupied.SetNumZeroed(ContainerSize); uint32 LocalResourceIndex=0;
    for (const auto& V : Items)
    {
        if (!V.IsValid() || V->Type != EJson::Object || ++Nodes>MaxNodes) return false;
        FObj O=V->AsObject(); FLegacyLocalMemberContract M; uint32 Old, P, Size;
        if (!Keys(O,{TEXT("name"),TEXT("offset"),TEXT("base_type"),TEXT("precision"),TEXT("rows"),TEXT("columns"),TEXT("elements"),TEXT("size"),TEXT("shader_type")},
            {TEXT("helper"),TEXT("metadata_object"),TEXT("members"),TEXT("resource_index")}) ||
            !Str(O,TEXT("name"),M.Name) || !Ident(M.Name) || Names.Contains(M.Name) ||
            !Str(O,TEXT("shader_type"),M.ShaderType) || !Num(O,TEXT("offset"),M.Offset) ||
            !Num(O,TEXT("base_type"),Old) || !Type(Old,M.BaseType) || !Num(O,TEXT("precision"),P) || P>2 ||
            !Num(O,TEXT("rows"),M.Rows) || !Num(O,TEXT("columns"),M.Columns) || !Num(O,TEXT("elements"),M.Elements) ||
            !Num(O,TEXT("size"),Size) || M.Elements>MaxNodes) return false;
        Names.Add(M.Name);
        M.Precision = P==0 ? EShaderPrecisionModifier::Float : P==1 ? EShaderPrecisionModifier::Half : EShaderPrecisionModifier::Fixed;
        const bool Nested=Old==17, Resource=Old>=5 && Old<=8;
        uint64 Expected=0; uint32 Alignment=4;
        if (Nested) {
            if (!Ident(M.ShaderType) || M.Rows!=1 || M.Columns!=1 || M.Elements || !Size || Size%16 || !Arr(O,TEXT("members"))) return false;
            M.StructTypeName=M.ShaderType; M.StructSize=Size; Expected=Size; Alignment=16;
        } else if (Resource) {
            if (M.ShaderType.IsEmpty() || M.Rows!=1 || M.Columns!=1 || M.Elements || O->HasField(TEXT("members"))) return false;
            Expected=8; Alignment=8;
        } else {
            if (!M.ShaderType.IsEmpty() || M.Rows<1 || M.Rows>4 || M.Columns<1 || M.Columns>4 || O->HasField(TEXT("members")) || O->HasField(TEXT("resource_index"))) return false;
            uint32 Element=4*M.Rows*M.Columns; Expected=M.Elements ? uint64(Align(Element,16))*M.Elements : Element; Alignment=M.Elements ? 16 : 4;
        }
        if (Expected!=Size || M.Offset%Alignment || M.Offset>ContainerSize || Size>ContainerSize-M.Offset) return false;
        for (uint32 I=M.Offset; I<M.Offset+Size; ++I) { if (Occupied[I]) return false; Occupied[I]=1; }
        FString Path=Prefix.IsEmpty() ? M.Name : Prefix+TEXT(".")+M.Name;
        if (Nested) {
            if (O->HasField(TEXT("resource_index")) || !Members(*Arr(O,TEXT("members")),Size,Path,Base+M.Offset,Depth+1,Nodes,M.Members,Resources)) return false;
        } else if (Resource) {
            FResource R{Path,Base+M.Offset,Old}; Resources.Add(MoveTemp(R));
            // Member-level indices are local to the source tree, whereas the
            // exported resources array has flattened global indices.
            uint32 Index; if (!Num(O,TEXT("resource_index"),Index) || Index!=LocalResourceIndex++) return false;
        }
        for (auto K : {TEXT("helper"),TEXT("metadata_object")}) if (O->HasField(K)) { FString S; if (!Str(O,K,S) || S.IsEmpty()) return false; }
        Out.Add(MoveTemp(M));
    }
    return true;
}
uint32 Hash(uint32 Size, const TArray<FResource>& R)
{
    uint32 H=Size<<16; for (const auto& E:R) H^=E.Offset;
    int32 N=R.Num(); while (N>=4) for (uint32 Shift : {0u,8u,16u,24u}) H^=R[--N].Type<<Shift;
    while (N>=2) for (uint32 Shift : {0u,16u}) H^=R[--N].Type<<Shift;
    if (N) H^=R[--N].Type; return H;
}
}

static bool LoadBindings(const FString& Path, const FString& ExpectedSHA256,
    bool bResolveAllMetadata, TArray<FLegacyOriginalUniformContractBinding>& Out, FString& Error)
{
    static const TCHAR* Names[]={TEXT("View"),TEXT("LocalVF"),TEXT("TranslucentDepthPass"),TEXT("ShadowDepthPass"),TEXT("LightmapDensityPass"),TEXT("TranslucentBasePass")};
    static const TCHAR* Types[]={TEXT("FViewUniformShaderParameters"),TEXT("FLocalVertexFactoryUniformShaderParameters"),TEXT("FTranslucencyDepthPassUniformParameters"),TEXT("FShadowDepthPassUniformParameters"),TEXT("FLightmapDensityPassUniformParameters"),TEXT("FTranslucentBasePassUniformParameters")};
    static const uint32 ExpectedCounts[]={30,15,3,4,1,2};
    if (ExpectedSHA256.Len()!=64) return Fail(Error,TEXT("invalid pin"));
    TArray<uint8> Bytes;
    if (!FFileHelper::LoadFileToArray(Bytes,*Path) || !Bytes.Num() || Bytes.Num()>16*1024*1024) return Fail(Error,TEXT("contract file unavailable or too large"));
    uint8 Digest[32]; LegacyManifestSHA256(Bytes.GetData(),uint32(Bytes.Num()),Digest);
    if (!BytesToHex(Digest,32).Equals(ExpectedSHA256,ESearchCase::IgnoreCase)) return Fail(Error,TEXT("contract digest mismatch"));
    FUTF8ToTCHAR Decoded(reinterpret_cast<const ANSICHAR*>(Bytes.GetData()),Bytes.Num());
    FString Json(Decoded.Length(),Decoded.Get()); FObj Root;
    if (!UniqueKeys(Json) || !FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json),Root) ||
        !Keys(Root,{TEXT("schema"),TEXT("image"),TEXT("inventory"),TEXT("contracts"),TEXT("bindings"),TEXT("runtime_verified"),TEXT("limits")})) return Fail(Error,TEXT("invalid contract bundle schema"));
    FString Schema, Limits; bool Verified;
    const auto* Contracts=Arr(Root,TEXT("contracts")); const auto* Bindings=Arr(Root,TEXT("bindings"));
    if (!Str(Root,TEXT("schema"),Schema) || Schema!=TEXT("paper2d-original-uniform-contracts-v1") || !Str(Root,TEXT("limits"),Limits) ||
        !Root->TryGetBoolField(TEXT("runtime_verified"),Verified) || Verified || !Contracts || Contracts->Num()!=6 || !Bindings || Bindings->Num()!=69)
        return Fail(Error,TEXT("unexpected contract bundle contents"));
    for (auto Key : {TEXT("image"),TEXT("inventory")}) {
        FObj R=Obj(Root,Key); FString P,H;
        if (!Keys(R,{TEXT("path"),TEXT("sha256")}) || !Str(R,TEXT("path"),P) || P.IsEmpty() || !Str(R,TEXT("sha256"),H) || H.Len()!=64) return Fail(Error,TEXT("invalid evidence receipt"));
    }
    FLegacyLocalUniformContract All[6]; bool Seen[6]={}; uint32 Hashes[6]={};
    for (const auto& V : *Contracts) {
        if (!V.IsValid() || V->Type!=EJson::Object) return Fail(Error,TEXT("invalid contract entry"));
        FObj C=V->AsObject(); FString Name,Struct,Slot,Address,UniformAddress;
        if (!Keys(C,{TEXT("parameter_name"),TEXT("struct_type"),TEXT("static_slot_name"),TEXT("original"),TEXT("original_resource_layout_hash"),TEXT("resources"),TEXT("provenance"),TEXT("identity_literal_address"),TEXT("uniform_literal_address")}) ||
            !Str(C,TEXT("parameter_name"),Name) || !Str(C,TEXT("struct_type"),Struct) || !Str(C,TEXT("static_slot_name"),Slot) || !Slot.IsEmpty() ||
            !Str(C,TEXT("identity_literal_address"),Address) || Address.IsEmpty() || !Str(C,TEXT("uniform_literal_address"),UniformAddress) || UniformAddress.IsEmpty()) return Fail(Error,TEXT("invalid original contract identity"));
        int32 I=0; while (I<6 && Name!=Names[I]) ++I;
        if (I==6 || Seen[I] || Struct!=Types[I]) return Fail(Error,TEXT("duplicate or unknown original contract"));
        Seen[I]=true; auto& Result=All[I]; Result.ParameterName=Name; Result.OriginalStructTypeName=Struct; Result.OriginalStaticSlotName=Slot;
        FObj Original=Obj(C,TEXT("original")); const auto* Tree=Arr(Original,TEXT("members")); const auto* Entries=Arr(C,TEXT("resources")); const auto* Provenance=Arr(C,TEXT("provenance"));
        if (!Keys(Original,{TEXT("struct_size"),TEXT("members")}) || !Num(Original,TEXT("struct_size"),Result.OriginalSize) || !Tree || !Entries || !Provenance || !Provenance->Num() || !Num(C,TEXT("original_resource_layout_hash"),Hashes[I])) return Fail(Error,TEXT("invalid original layout"));
        for (const auto& P:*Provenance) { if (!P.IsValid() || P->Type!=EJson::Object) return Fail(Error,TEXT("invalid provenance")); FObj R=P->AsObject(); FString A,B; if (!Keys(R,{TEXT("path"),TEXT("sha256")}) || !Str(R,TEXT("path"),A) || A.IsEmpty() || !Str(R,TEXT("sha256"),B) || B.Len()!=64) return Fail(Error,TEXT("invalid provenance receipt")); }
        TArray<FResource> Resources; uint32 Nodes=0;
        if (!Members(*Tree,Result.OriginalSize,FString(),0,0,Nodes,Result.OriginalMembers,Resources) || Entries->Num()!=Resources.Num() || Hash(Result.OriginalSize,Resources)!=Hashes[I]) return Fail(Error,TEXT("invalid original member tree or hash"));
        for (int32 J=0; J<Entries->Num(); ++J) {
            const auto& E=(*Entries)[J]; if (!E.IsValid() || E->Type!=EJson::Object) return Fail(Error,TEXT("invalid resource entry"));
            FObj R=E->AsObject(); uint32 Offset,TypeValue,Index; FString PathValue; EUniformBufferBaseType Mapped;
            if (!Keys(R,{TEXT("offset"),TEXT("base_type"),TEXT("path"),TEXT("resource_index")}) || !Num(R,TEXT("offset"),Offset) || !Num(R,TEXT("base_type"),TypeValue) || !Num(R,TEXT("resource_index"),Index) || !Str(R,TEXT("path"),PathValue) || Index!=uint32(J) || Offset!=Resources[J].Offset || TypeValue!=Resources[J].Type || PathValue!=Resources[J].Path || Offset>65535 || !Type(TypeValue,Mapped)) return Fail(Error,TEXT("resource enumeration differs from tree"));
            FLegacyMaterialResource Rsrc; Rsrc.Offset=uint16(Offset); Rsrc.BaseType=Mapped; Result.OriginalResources.Add(Rsrc);
        }
    }
    uint32 Counts[6]={}, Materials=0; TSet<FString> Uses, Slots;
    TArray<FLegacyOriginalUniformContractBinding> Result;
    for (const auto& V:*Bindings) {
        if (!V.IsValid() || V->Type!=EJson::Object) return Fail(Error,TEXT("invalid binding")); FObj B=V->AsObject();
        FString Name,Status; uint32 Shader,Register,Recorded; bool Active;
        if (!Keys(B,{TEXT("shader_index"),TEXT("source_register"),TEXT("parameter_name"),TEXT("resource_table_active"),TEXT("recorded_resource_layout_hash"),TEXT("status")}) || !Num(B,TEXT("shader_index"),Shader) || !Num(B,TEXT("source_register"),Register) || Register>=32 || !Str(B,TEXT("parameter_name"),Name) || !Str(B,TEXT("status"),Status) || !Num(B,TEXT("recorded_resource_layout_hash"),Recorded) || !B->TryGetBoolField(TEXT("resource_table_active"),Active)) return Fail(Error,TEXT("invalid binding fields"));
        FString Use=FString::Printf(TEXT("%u:%s"),Shader,*Name), SlotKey=FString::Printf(TEXT("%u:%u"),Shader,Register);
        if (Uses.Contains(Use) || Slots.Contains(SlotKey)) return Fail(Error,TEXT("duplicate binding")); Uses.Add(Use); Slots.Add(SlotKey);
        int32 I=0; while (I<6 && Name!=Names[I]) ++I;
        if (I==6) { if (Name!=TEXT("Material") || Status!=TEXT("requires_original_material_expression_contract")) return Fail(Error,TEXT("unknown binding")); ++Materials; }
        else { if (Status!=TEXT("original_fixed_layout_decoded") || Recorded!=(Active ? Hashes[I] : 0)) return Fail(Error,TEXT("binding resource hash inconsistent")); ++Counts[I];
        }
        auto& Binding=Result.AddDefaulted_GetRef();
        Binding.ShaderIndex=Shader; Binding.ParameterName=Name; Binding.SourceRegister=uint16(Register); Binding.SourceResourceLayoutHash=Recorded;
        if (I<6) { Binding.Contract=All[I]; Binding.Contract.SourceRegister=uint16(Register); Binding.Contract.OriginalResourceLayoutHash=Recorded; }
    }
    for (int32 I=0; I<6; ++I) if (!Seen[I] || Counts[I]!=ExpectedCounts[I]) return Fail(Error,TEXT("incomplete binding inventory"));
    if (Materials!=14) return Fail(Error,TEXT("incomplete Material binding inventory"));
    for (auto& Binding : Result) if (bResolveAllMetadata && Binding.ParameterName!=TEXT("Material")) {
        auto* Metadata=FShaderParametersMetadata::GetNameStructMap().FindRef(FHashedName(*Binding.ParameterName));
        if (!Metadata || Metadata->GetUseCase()!=FShaderParametersMetadata::EUseCase::UniformBuffer || !Metadata->GetShaderVariableName() || Binding.ParameterName!=Metadata->GetShaderVariableName()) return Fail(Error,TEXT("current named uniform is not registered"));
        Binding.Contract.CurrentMetadata=Metadata;
    }
    Out=MoveTemp(Result); Error.Reset(); return true;
}

bool LoadLegacyOriginalUniformBindings(const FString& Path, const FString& ExpectedSHA256,
    TArray<FLegacyOriginalUniformContractBinding>& Out, FString& Error)
{
    return LoadBindings(Path, ExpectedSHA256, true, Out, Error);
}

bool LoadLegacyOriginalUniformContract(const FString& Path, const FString& ExpectedSHA256,
    uint32 ShaderIndex, const FString& ParameterName, uint16 SourceRegister,
    uint32 SourceResourceLayoutHash, FLegacyLocalUniformContract& Out, FString& Error)
{
    if (SourceRegister>=32 || ParameterName==TEXT("Material")) return Fail(Error,TEXT("invalid register or unsupported Material contract"));
    TArray<FLegacyOriginalUniformContractBinding> Bindings;
    if (!LoadBindings(Path,ExpectedSHA256,false,Bindings,Error)) return false;
    for (auto& Binding : Bindings) if (Binding.ShaderIndex==ShaderIndex && Binding.ParameterName==ParameterName) {
        if (Binding.SourceRegister!=SourceRegister || Binding.SourceResourceLayoutHash!=SourceResourceLayoutHash) return Fail(Error,TEXT("selected binding does not match source"));
        auto* Metadata=FShaderParametersMetadata::GetNameStructMap().FindRef(FHashedName(*ParameterName));
        if (!Metadata || Metadata->GetUseCase()!=FShaderParametersMetadata::EUseCase::UniformBuffer || !Metadata->GetShaderVariableName() || ParameterName!=Metadata->GetShaderVariableName()) return Fail(Error,TEXT("current named uniform is not registered"));
        Binding.Contract.CurrentMetadata=Metadata;
        Out=MoveTemp(Binding.Contract); Error.Reset(); return true;
    }
    return Fail(Error,TEXT("selected binding unavailable"));
}
#endif
