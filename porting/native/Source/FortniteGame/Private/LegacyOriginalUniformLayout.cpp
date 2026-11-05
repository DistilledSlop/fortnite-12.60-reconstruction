#include "LegacyOriginalUniformLayout.h"

#if PLATFORM_LINUX && !UE_SERVER
namespace
{
constexpr uint32 MaxBytes = 65536, MaxNodes = 65536, MaxDepth = 16;
static_assert(sizeof(void*) == 8 && SHADER_PARAMETER_POINTER_ALIGNMENT == 8, "Unexpected resource ABI");
static_assert(SHADER_PARAMETER_ARRAY_ELEMENT_ALIGNMENT == 16, "Unexpected numeric array ABI");
bool Fail(FString& Error, const TCHAR* Why) { Error = Why; return false; }
bool Identifier(const FString& S)
{
    if (S.IsEmpty() || S.Len() > 256) return false;
    for (int32 I = 0; I < S.Len(); ++I)
        if (!((S[I] >= TEXT('a') && S[I] <= TEXT('z')) || (S[I] >= TEXT('A') && S[I] <= TEXT('Z')) ||
              S[I] == TEXT('_') || (I && S[I] >= TEXT('0') && S[I] <= TEXT('9')))) return false;
    return true;
}
bool Resource(EUniformBufferBaseType T)
{
    return T == UBMT_TEXTURE || T == UBMT_SRV || T == UBMT_UAV || T == UBMT_SAMPLER;
}
bool Struct(EUniformBufferBaseType T) { return T == UBMT_NESTED_STRUCT || T == UBMT_INCLUDED_STRUCT; }
bool ValidString(const FString& S) { return S.Len() <= 4096 && FCString::Strlen(*S) == S.Len(); }

// Expand struct arrays as the engine does. This also bounds the total amount
// of metadata traversal and verifies every absolute resource offset before any
// metadata constructor can invoke InitializeLayout's checks.
bool Validate(const TArray<FLegacyLocalMemberContract>& Members, uint32 Size, uint32 Base,
    uint32 Depth, uint32& Nodes, TArray<FLegacyMaterialResource>& Resources, FString& Error)
{
    if (Depth > MaxDepth || !Size || Size > MaxBytes || !Members.Num() || Members.Num() > int32(MaxNodes))
        return Fail(Error, TEXT("invalid original uniform member tree bounds"));
    TArray<uint8> Occupied; Occupied.SetNumZeroed(Size);
    TSet<FString> Names;
    for (const auto& M : Members)
    {
        if (++Nodes > MaxNodes || !Identifier(M.Name) || Names.Contains(M.Name) || !ValidString(M.ShaderType) ||
            M.Elements > MaxNodes || (M.Precision != EShaderPrecisionModifier::Float &&
            M.Precision != EShaderPrecisionModifier::Half && M.Precision != EShaderPrecisionModifier::Fixed))
            return Fail(Error, TEXT("invalid or duplicate original uniform member"));
        Names.Add(M.Name);
        uint32 Stride, Alignment;
        if (Struct(M.BaseType))
        {
            if (!Identifier(M.StructTypeName) || !M.StructSize || M.StructSize > MaxBytes || M.StructSize % 16 ||
                M.Rows != 1 || M.Columns != 1 || (M.BaseType == UBMT_INCLUDED_STRUCT && M.Elements))
                return Fail(Error, TEXT("invalid original inline struct"));
            Stride = M.StructSize; Alignment = 16;
        }
        else
        {
            if (!M.StructTypeName.IsEmpty() || M.StructSize || M.Members.Num())
                return Fail(Error, TEXT("non-struct original member has child metadata"));
            if (Resource(M.BaseType))
            {
                if (M.Elements || M.Rows != 1 || M.Columns != 1 || M.ShaderType.IsEmpty())
                    return Fail(Error, TEXT("invalid or unsupported original resource shape"));
                Stride = Alignment = 8;
            }
            else
            {
                if ((M.BaseType != UBMT_FLOAT32 && M.BaseType != UBMT_INT32 && M.BaseType != UBMT_UINT32) ||
                    M.Rows < 1 || M.Rows > 4 || M.Columns < 1 || M.Columns > 4)
                    return Fail(Error, TEXT("invalid or unsupported original numeric member"));
                const uint32 ElementBytes = 4 * M.Rows * M.Columns;
                Stride = M.Elements ? Align(ElementBytes, 16u) : ElementBytes;
                Alignment = M.Elements ? 16 : 4;
            }
        }
        const uint32 Count = M.Elements ? M.Elements : 1;
        const uint64 Span = uint64(Stride) * Count;
        if (M.Offset % Alignment || M.Offset > Size || Span > Size - M.Offset)
            return Fail(Error, TEXT("original member extent exceeds uniform bounds"));
        for (uint32 I = M.Offset; I < M.Offset + uint32(Span); ++I)
        {
            if (Occupied[I]) return Fail(Error, TEXT("overlapping original member extents"));
            Occupied[I] = 1;
        }
        if (Struct(M.BaseType))
        {
            for (uint32 I = 0; I < Count; ++I)
                if (!Validate(M.Members, M.StructSize, Base + M.Offset + I * Stride, Depth + 1, Nodes, Resources, Error)) return false;
        }
        else if (Resource(M.BaseType))
        {
            if (Base + M.Offset >= 65536) return Fail(Error, TEXT("original resource offset exceeds RHI limit"));
            FLegacyMaterialResource R; R.Offset = uint16(Base + M.Offset); R.BaseType = M.BaseType;
            Resources.Add(R);
        }
    }
    return true;
}
bool VerifyTree(const TArray<FLegacyLocalMemberContract>& Original, const FShaderParametersMetadata& Metadata)
{
    if (Metadata.GetMembers().Num() != Original.Num()) return false;
    for (int32 I = 0; I < Original.Num(); ++I)
    {
        const auto& A = Original[I]; const auto& B = Metadata.GetMembers()[I];
        if (A.Name != B.GetName() || A.ShaderType != (B.GetShaderType() ? B.GetShaderType() : TEXT("")) ||
            A.Offset != B.GetOffset() || A.BaseType != B.GetBaseType() || A.Precision != B.GetPrecision() ||
            A.Rows != B.GetNumRows() || A.Columns != B.GetNumColumns() || A.Elements != B.GetNumElements()) return false;
        const auto* Child = B.GetStructMetadata();
        if (Struct(A.BaseType))
        {
            if (!Child || Child->GetUseCase() != FShaderParametersMetadata::EUseCase::ShaderParameterStruct ||
                Child->GetShaderVariableName() || Child->HasStaticSlot() || A.StructTypeName != Child->GetStructTypeName() ||
                A.StructSize != Child->GetSize() || !VerifyTree(A.Members, *Child)) return false;
        }
        else if (Child) return false;
    }
    return true;
}
}

FLegacyOriginalUniformLayout::~FLegacyOriginalUniformLayout()
{
    Metadata.Reset();
    // Children were added postorder: destroy each parent before its children.
    while (Children.Num()) Children.Pop();
}
const TCHAR* FLegacyOriginalUniformLayout::Retain(const FString& Value)
{
    TUniquePtr<FString> Copy = MakeUnique<FString>(Value);
    const TCHAR* Result = **Copy;
    Strings.Add(MoveTemp(Copy));
    return Result;
}
const FShaderParametersMetadata* FLegacyOriginalUniformLayout::BuildChild(const FLegacyLocalMemberContract& Member)
{
    TArray<FShaderParametersMetadata::FMember> Members;
    for (const auto& M : Member.Members)
    {
        const auto* Child = Struct(M.BaseType) ? BuildChild(M) : nullptr;
        Members.Emplace(Retain(M.Name), M.ShaderType.IsEmpty() ? nullptr : Retain(M.ShaderType),
            M.Offset, M.BaseType, M.Precision, M.Rows, M.Columns, M.Elements, Child);
    }
    const TCHAR* TypeName = Retain(Member.StructTypeName);
    auto Child = MakeUnique<FShaderParametersMetadata>(FShaderParametersMetadata::EUseCase::ShaderParameterStruct,
        TypeName, TypeName, nullptr, nullptr, Member.StructSize, Members);
    const auto* Result = Child.Get();
    Children.Add(MoveTemp(Child));
    return Result;
}

bool BuildLegacyOriginalUniformLayout(const FLegacyLocalUniformContract& Contract,
    TSharedPtr<FLegacyOriginalUniformLayout>& Out, FString& Error)
{
    if (!Identifier(Contract.ParameterName) || !Identifier(Contract.OriginalStructTypeName) ||
        !Contract.OriginalStaticSlotName.IsEmpty() || !Contract.OriginalSize ||
        Contract.OriginalSize > MaxBytes || Contract.OriginalSize % 16)
        return Fail(Error, TEXT("invalid original uniform root"));
    TArray<FLegacyMaterialResource> Resources;
    uint32 Nodes = 0;
    if (!Validate(Contract.OriginalMembers, Contract.OriginalSize, 0, 0, Nodes, Resources, Error)) return false;
    if (Resources.Num() != Contract.OriginalResources.Num())
        return Fail(Error, TEXT("original resource count differs from member tree"));
    for (int32 I = 0; I < Resources.Num(); ++I)
        if (Resources[I].Offset != Contract.OriginalResources[I].Offset || Resources[I].BaseType != Contract.OriginalResources[I].BaseType)
            return Fail(Error, TEXT("original resource order or type differs from member tree"));

    TSharedPtr<FLegacyOriginalUniformLayout> Result = MakeShareable(new FLegacyOriginalUniformLayout());
    TArray<FShaderParametersMetadata::FMember> Members;
    for (const auto& M : Contract.OriginalMembers)
    {
        const auto* Child = Struct(M.BaseType) ? Result->BuildChild(M) : nullptr;
        Members.Emplace(Result->Retain(M.Name), M.ShaderType.IsEmpty() ? nullptr : Result->Retain(M.ShaderType),
            M.Offset, M.BaseType, M.Precision, M.Rows, M.Columns, M.Elements, Child);
    }
    const TCHAR* TypeName = Result->Retain(Contract.OriginalStructTypeName);
    const TCHAR* ParameterName = Result->Retain(Contract.ParameterName);
    Result->Metadata = MakeUnique<FShaderParametersMetadata>(FShaderParametersMetadata::EUseCase::DataDrivenUniformBuffer,
        TypeName, TypeName, ParameterName, nullptr, Contract.OriginalSize, Members);
    const auto& Metadata = *Result->Metadata;
    if (Metadata.GetSize() != Contract.OriginalSize || Metadata.GetUseCase() != FShaderParametersMetadata::EUseCase::DataDrivenUniformBuffer ||
        Metadata.HasStaticSlot() || Contract.OriginalStructTypeName != Metadata.GetStructTypeName() ||
        Contract.ParameterName != Metadata.GetShaderVariableName() || !VerifyTree(Contract.OriginalMembers, Metadata))
        return Fail(Error, TEXT("constructed original metadata differs from contract"));
    Resources.Sort([](const FLegacyMaterialResource& A, const FLegacyMaterialResource& B) { return A.Offset < B.Offset; });
    const auto& RHIResources = Metadata.GetLayout().Resources;
    if (Metadata.GetLayout().ConstantBufferSize != Contract.OriginalSize || RHIResources.Num() != Resources.Num())
        return Fail(Error, TEXT("constructed RHI layout size or resource count differs"));
    for (int32 I = 0; I < Resources.Num(); ++I)
        if (RHIResources[I].MemberOffset != Resources[I].Offset || RHIResources[I].MemberType != Resources[I].BaseType)
            return Fail(Error, TEXT("constructed RHI resource layout differs"));
    Out = MoveTemp(Result); Error.Reset(); return true;
}
#endif
