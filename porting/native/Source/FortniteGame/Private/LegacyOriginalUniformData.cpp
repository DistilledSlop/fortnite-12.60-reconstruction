#include "LegacyOriginalUniformData.h"

#if PLATFORM_LINUX && !UE_SERVER
namespace
{
constexpr uint32 UniformDataMaxBytes = 65536, UniformDataMaxNodes = 65536, UniformDataMaxDepth = 16;
constexpr uint32 NumericArrayAlignment = 16, ResourceBytes = 8;
static_assert(sizeof(void*) == ResourceBytes, "Legacy resource slots require 64-bit pointers");
static_assert(SHADER_PARAMETER_ARRAY_ELEMENT_ALIGNMENT == NumericArrayAlignment, "Unexpected numeric array ABI");
static_assert(SHADER_PARAMETER_POINTER_ALIGNMENT == ResourceBytes, "Unexpected resource ABI");

bool UniformDataFail(FString& Error, const FString& Why)
{
    Error = Why;
    return false;
}

bool UniformDataResource(EUniformBufferBaseType Type)
{
    return Type == UBMT_TEXTURE || Type == UBMT_SRV || Type == UBMT_UAV || Type == UBMT_SAMPLER;
}

bool SameType(const FLegacyLocalMemberContract& A, const FLegacyLocalMemberContract& B)
{
    // Storage offsets/sizes are intentionally excluded: members can move.
    return A.Name == B.Name && A.ShaderType == B.ShaderType && A.StructTypeName == B.StructTypeName &&
        A.BaseType == B.BaseType && A.Precision == B.Precision && A.Rows == B.Rows &&
        A.Columns == B.Columns && A.Elements == B.Elements;
}

struct FUniformDataLeaf
{
    FString Path;
    FLegacyLocalMemberContract Type;
    TArray<FLegacyLocalMemberContract> Ancestors;
    uint32 Offset = 0, ElementBytes = 0, Count = 0, Stride = 0;
};

bool UniformDataIdentifier(const FString& Name)
{
    if (Name.IsEmpty() || Name.Len() > 256) return false;
    for (int32 I = 0; I < Name.Len(); ++I)
        if (!((Name[I] >= TEXT('a') && Name[I] <= TEXT('z')) ||
            (Name[I] >= TEXT('A') && Name[I] <= TEXT('Z')) || Name[I] == TEXT('_') ||
            (I > 0 && Name[I] >= TEXT('0') && Name[I] <= TEXT('9')))) return false;
    return true;
}

// Validate each storage extent at its containing level, then enumerate only
// semantic bytes. Rejecting overlapping extents also protects nested padding.
bool Flatten(const TArray<FLegacyLocalMemberContract>& Members, uint32 Size, uint32 Base,
    const FString& Prefix, const TArray<FLegacyLocalMemberContract>& Ancestors,
    uint32 Depth, uint32& Nodes, TArray<FUniformDataLeaf>& Leaves, FString& Error)
{
    if (Depth > UniformDataMaxDepth || !Size || Size > UniformDataMaxBytes || Members.Num() == 0 || Members.Num() > int32(UniformDataMaxNodes))
        return UniformDataFail(Error, TEXT("invalid uniform member tree bounds"));
    TArray<uint8> Occupied;
    Occupied.SetNumZeroed(Size);
    TSet<FString> Names;
    for (const auto& M : Members)
    {
        if (++Nodes > UniformDataMaxNodes || !UniformDataIdentifier(M.Name) || Names.Contains(M.Name) || M.Elements > UniformDataMaxNodes ||
            (M.Precision != EShaderPrecisionModifier::Float && M.Precision != EShaderPrecisionModifier::Half &&
             M.Precision != EShaderPrecisionModifier::Fixed))
            return UniformDataFail(Error, TEXT("invalid or duplicate uniform member"));
        Names.Add(M.Name);
        const bool bStruct = M.BaseType == UBMT_NESTED_STRUCT || M.BaseType == UBMT_INCLUDED_STRUCT;
        const bool bResource = UniformDataResource(M.BaseType);
        const uint32 Count = M.Elements ? M.Elements : 1;
        uint32 ElementBytes = 0, Stride = 0, Alignment = 4;
        if (bStruct)
        {
            if (!UniformDataIdentifier(M.StructTypeName) || !M.StructSize || M.StructSize > UniformDataMaxBytes ||
                M.StructSize % 16 || M.Rows != 1 || M.Columns != 1 ||
                (M.BaseType == UBMT_INCLUDED_STRUCT && M.Elements))
                return UniformDataFail(Error, TEXT("invalid inline uniform struct"));
            ElementBytes = Stride = M.StructSize;
            Alignment = 16;
        }
        else
        {
            if (!M.StructTypeName.IsEmpty() || M.StructSize || M.Members.Num())
                return UniformDataFail(Error, TEXT("non-struct uniform member has child metadata"));
            if (bResource)
            {
                if (M.Rows != 1 || M.Columns != 1 || M.ShaderType.IsEmpty())
                    return UniformDataFail(Error, TEXT("invalid resource member shape"));
                ElementBytes = Stride = Alignment = ResourceBytes;
            }
            else
            {
                if ((M.BaseType != UBMT_FLOAT32 && M.BaseType != UBMT_INT32 && M.BaseType != UBMT_UINT32) ||
                    M.Rows < 1 || M.Rows > 4 || M.Columns < 1 || M.Columns > 4)
                    return UniformDataFail(Error, TEXT("unsupported uniform numeric member"));
                ElementBytes = 4 * M.Rows * M.Columns;
                Stride = M.Elements ? Align(ElementBytes, NumericArrayAlignment) : ElementBytes;
                Alignment = M.Elements ? NumericArrayAlignment : 4;
            }
        }
        const uint64 Span = uint64(Stride) * Count;
        if (M.Offset % Alignment || M.Offset > Size || Span > Size - M.Offset)
            return UniformDataFail(Error, TEXT("uniform member extent exceeds its containing struct"));
        for (uint32 I = M.Offset; I < M.Offset + uint32(Span); ++I)
        {
            if (Occupied[I]) return UniformDataFail(Error, TEXT("overlapping uniform member extents"));
            Occupied[I] = 1;
        }
        const FString Path = Prefix + TEXT(".") + M.Name;
        if (bStruct)
        {
            TArray<FLegacyLocalMemberContract> Parents = Ancestors;
            FLegacyLocalMemberContract Parent = M;
            Parent.Members.Reset();
            Parents.Add(MoveTemp(Parent));
            for (uint32 I = 0; I < Count; ++I)
            {
                const FString ChildPath = M.Elements ? Path + FString::Printf(TEXT("[%u]"), I) : Path;
                if (!Flatten(M.Members, M.StructSize, Base + M.Offset + I * Stride,
                    ChildPath, Parents, Depth + 1, Nodes, Leaves, Error)) return false;
            }
        }
        else
        {
            FUniformDataLeaf Leaf;
            Leaf.Path = Path; Leaf.Type = M; Leaf.Ancestors = Ancestors;
            Leaf.Offset = Base + M.Offset; Leaf.Count = Count;
            Leaf.ElementBytes = ElementBytes; Leaf.Stride = Stride;
            Leaves.Add(MoveTemp(Leaf));
        }
    }
    return true;
}

bool ReadCurrent(const FShaderParametersMetadata& Metadata, uint32 Depth, uint32& Nodes,
    TArray<FLegacyLocalMemberContract>& Out, FString& Error)
{
    if (Depth > UniformDataMaxDepth || Metadata.GetMembers().Num() > int32(UniformDataMaxNodes))
        return UniformDataFail(Error, TEXT("current metadata recursion/member limit exceeded"));
    for (const auto& M : Metadata.GetMembers())
    {
        if (++Nodes > UniformDataMaxNodes) return UniformDataFail(Error, TEXT("current metadata node limit exceeded"));
        FLegacyLocalMemberContract C;
        C.Name = M.GetName(); C.ShaderType = M.GetShaderType() ? M.GetShaderType() : TEXT("");
        C.Offset = M.GetOffset(); C.BaseType = M.GetBaseType(); C.Precision = M.GetPrecision();
        C.Rows = M.GetNumRows(); C.Columns = M.GetNumColumns(); C.Elements = M.GetNumElements();
        if (const auto* Child = M.GetStructMetadata())
        {
            C.StructTypeName = Child->GetStructTypeName(); C.StructSize = Child->GetSize();
            if (!ReadCurrent(*Child, Depth + 1, Nodes, C.Members, Error)) return false;
        }
        Out.Add(MoveTemp(C));
    }
    return true;
}

bool ResourcesMatch(const TArray<FUniformDataLeaf>& Leaves, const TArray<FLegacyMaterialResource>& Resources,
    bool bCurrentLayout, FString& Error)
{
    TArray<FLegacyMaterialResource> Expected;
    for (const auto& Leaf : Leaves)
        if (UniformDataResource(Leaf.Type.BaseType))
            for (uint32 I = 0; I < Leaf.Count; ++I)
            {
                FLegacyMaterialResource Entry;
                Entry.Offset = uint16(Leaf.Offset + I * Leaf.Stride);
                Entry.BaseType = Leaf.Type.BaseType;
                Expected.Add(Entry);
            }
    // Original resource indices follow recovered member traversal/array order.
    // Current InitializeLayout explicitly sorts its final list by MemberOffset.
    if (bCurrentLayout)
        Expected.Sort([](const FLegacyMaterialResource& A, const FLegacyMaterialResource& B)
            { return A.Offset < B.Offset; });
    if (Expected.Num() != Resources.Num()) return UniformDataFail(Error, TEXT("resource enumeration count differs from member tree"));
    for (int32 I = 0; I < Resources.Num(); ++I)
    {
        if (Resources[I].Offset != Expected[I].Offset || Resources[I].BaseType != Expected[I].BaseType)
            return UniformDataFail(Error, TEXT("resource enumeration order/offset/type differs from member tree"));
    }
    return true;
}

bool Compatible(const FUniformDataLeaf& A, const FUniformDataLeaf& B)
{
    if (!SameType(A.Type, B.Type) || A.Ancestors.Num() != B.Ancestors.Num()) return false;
    for (int32 I = 0; I < A.Ancestors.Num(); ++I)
        if (!SameType(A.Ancestors[I], B.Ancestors[I])) return false;
    return true;
}

}

bool BuildLegacyOriginalUniformData(const FLegacyLocalUniformContract& Contract,
    const void* CurrentContents, uint32 CurrentContentsSize,
    const TArray<FLegacyOriginalUniformMemberValue>& Values,
    TArray<uint8>& OutBytes, FString& Error)
{
    const auto* Metadata = Contract.CurrentMetadata;
    if (!Metadata || !UniformDataIdentifier(Contract.ParameterName) || !UniformDataIdentifier(Contract.OriginalStructTypeName) ||
        !Contract.OriginalStaticSlotName.IsEmpty() || !CurrentContents || !Contract.OriginalSize ||
        Contract.OriginalSize > UniformDataMaxBytes || CurrentContentsSize == 0 || CurrentContentsSize > UniformDataMaxBytes ||
        CurrentContentsSize != Metadata->GetSize() || Values.Num() > int32(UniformDataMaxNodes))
        return UniformDataFail(Error, TEXT("invalid uniform data inputs or bounded current contents"));
    if (Metadata->GetUseCase() != FShaderParametersMetadata::EUseCase::UniformBuffer ||
        !Metadata->GetShaderVariableName() || Contract.ParameterName != Metadata->GetShaderVariableName() ||
        FShaderParametersMetadata::GetNameStructMap().FindRef(Metadata->GetShaderVariableHashedName()) != Metadata)
        return UniformDataFail(Error, TEXT("current uniform metadata is not the actual registered named uniform"));

    TArray<FLegacyLocalMemberContract> CurrentMembers;
    uint32 Nodes = 0;
    if (!ReadCurrent(*Metadata, 0, Nodes, CurrentMembers, Error)) return false;
    TArray<FUniformDataLeaf> Original, Current;
    TArray<FLegacyLocalMemberContract> Parents;
    Nodes = 0;
    if (!Flatten(Contract.OriginalMembers, Contract.OriginalSize, 0, Contract.ParameterName,
        Parents, 0, Nodes, Original, Error)) return false;
    Nodes = 0;
    if (!Flatten(CurrentMembers, CurrentContentsSize, 0, Contract.ParameterName,
        Parents, 0, Nodes, Current, Error)) return false;
    if (!ResourcesMatch(Original, Contract.OriginalResources, false, Error)) return false;
    const auto& Layout = Metadata->GetLayout();
    if (!Layout.GetHash() || Layout.ConstantBufferSize != CurrentContentsSize || Layout.Resources.Num() > int32(UniformDataMaxNodes))
        return UniformDataFail(Error, TEXT("current registered layout size/hash is invalid"));
    TArray<FLegacyMaterialResource> CurrentResources;
    for (const auto& R : Layout.Resources)
    {
        FLegacyMaterialResource Entry;
        Entry.Offset = R.MemberOffset; Entry.BaseType = R.MemberType;
        CurrentResources.Add(Entry);
    }
    if (!ResourcesMatch(Current, CurrentResources, true, Error)) return false;

    TMap<FString, int32> SourceByPath, ValueByPath;
    for (int32 I = 0; I < Current.Num(); ++I) SourceByPath.Add(Current[I].Path, I);
    for (int32 I = 0; I < Values.Num(); ++I)
    {
        if (Values[I].Path.IsEmpty() || ValueByPath.Contains(Values[I].Path))
            return UniformDataFail(Error, TEXT("duplicate or empty original member override path"));
        ValueByPath.Add(Values[I].Path, I);
    }
    const auto* Source = static_cast<const uint8*>(CurrentContents);
    TArray<uint8> Result;
    Result.SetNumZeroed(Contract.OriginalSize);
    for (const auto& Leaf : Original)
    {
        const uint8* Packed = nullptr;
        const FUniformDataLeaf* From = nullptr;
        if (const int32* ValueIndex = ValueByPath.Find(Leaf.Path))
        {
            const auto& Value = Values[*ValueIndex];
            if (!SameType(Value.Type, Leaf.Type) || Value.Type.Offset != Leaf.Type.Offset ||
                Value.Type.StructSize != Leaf.Type.StructSize || Value.Type.Members.Num() ||
                uint32(Value.Bytes.Num()) != Leaf.Count * Leaf.ElementBytes)
                return UniformDataFail(Error, TEXT("original member override type or complete payload mismatch: ") + Leaf.Path);
            Packed = Value.Bytes.GetData();
            ValueByPath.Remove(Leaf.Path);
        }
        else
        {
            const int32* Index = SourceByPath.Find(Leaf.Path);
            if (!Index || Contract.OriginalStructTypeName != Metadata->GetStructTypeName() ||
                !Compatible(Leaf, Current[*Index]))
                return UniformDataFail(Error, TEXT("original member needs an explicit complete typed value: ") + Leaf.Path);
            From = &Current[*Index];
        }
        for (uint32 I = 0; I < Leaf.Count; ++I)
        {
            const uint8* Bytes = Packed ? Packed + I * Leaf.ElementBytes : Source + From->Offset + I * From->Stride;
            FMemory::Memcpy(Result.GetData() + Leaf.Offset + I * Leaf.Stride, Bytes, Leaf.ElementBytes);
        }
    }
    if (ValueByPath.Num()) return UniformDataFail(Error, TEXT("unused original member override"));
    OutBytes = MoveTemp(Result);
    Error.Reset();
    return true;
}
#endif
