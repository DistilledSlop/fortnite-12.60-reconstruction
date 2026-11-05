#pragma once

#if PLATFORM_LINUX && !UE_SERVER
#include "CoreMinimal.h"
#include "LegacyOriginalMaterialReader.h"
#include "ShaderParameterMetadata.h"

class FJsonObject;
class FLegacyOriginalUniformLayout;

// Offsets refer to the renamed module, never the conversion receipt's module.
struct FLegacySpirvDescriptor
{
    uint32 SetOffset = UINT32_MAX, BindingOffset = UINT32_MAX;
    uint32 Set = UINT32_MAX, Binding = UINT32_MAX;
};
bool LegacyRewriteConvertedSpirv(const FString& Path, const FString& ExpectedSha1,
    const FString& ExpectedSha256, uint32 ExpectedModel, TArray<uint32>& Words,
    uint32& Crc, TMap<uint32, FLegacySpirvDescriptor>& Descriptors, FString& Error,
    bool bAllowMissingSha1 = false);

/** Independently recovered ORIGINAL member metadata, including nested structs.
 * Never populate these fields by copying current-engine metadata: doing so would
 * turn a layout comparison into a tautology. Block span alone proves no member
 * semantics. Array strides follow the original UE shader-parameter ABI.
 */
struct FLegacyLocalMemberContract
{
    FString Name, ShaderType, StructTypeName;
    uint32 Offset = 0, Rows = 0, Columns = 0, Elements = 0, StructSize = 0;
    EUniformBufferBaseType BaseType = UBMT_INVALID;
    EShaderPrecisionModifier::Type Precision = EShaderPrecisionModifier::Float;
    TArray<FLegacyLocalMemberContract> Members;
};
struct FLegacyLocalUniformContract
{
    uint16 SourceRegister = 0;
    // The original pre-static-slot ABI requires OriginalStaticSlotName empty.
    FString ParameterName, OriginalStructTypeName, OriginalStaticSlotName;
    uint32 OriginalSize = 0;
    // Original D3D resource-table hash, when present. Not a semantic-layout proof.
    uint32 OriginalResourceLayoutHash = 0;
    TArray<FLegacyLocalMemberContract> OriginalMembers;
    TArray<FLegacyMaterialResource> OriginalResources;
    const FShaderParametersMetadata* CurrentMetadata = nullptr;
};
struct FLegacyLocalLooseContract
{
    FString ParameterName;
    uint16 SourceBuffer = 0, ByteOffset = 0, ByteSize = 0;
};

// Explicit shader-output ABI target; CurrentMetadata remains the registered
// input used by the data repacker. Material continues to use factory metadata.
struct FLegacyLocalOriginalUniformTarget
{
    uint16 SourceRegister = 0;
    TSharedPtr<FLegacyOriginalUniformLayout> Layout;
};

/** Produce the independently recovered 64-byte/eight-member original LocalVF
 * contract, after global uniform metadata has been initialized. Current metadata
 * is compared against fixed original values; it is never copied as source proof.
 * Source hash is 0 for a constant-only use or 105252376 for an active table.
 * BuildLegacyLocalVFOutput additionally verifies that use against D3D tablebits.
 * Out is unchanged on failure. No runtime provider is installed.
 */
bool BuildLegacyLocalVFUniformContract(uint16 SourceRegister, uint32 SourceResourceLayoutHash,
    FLegacyLocalUniformContract& Out, FString& Error);

/** The two observed Paper2D expression shapes share an independently recovered
 * 80-byte Material ABI. Metadata must be the factory's actual expression-set
 * metadata. This validates its complete layout; it does not evaluate expressions
 * or establish texture ownership. Out is unchanged on failure.
 */
bool BuildLegacyPaper2DMaterialUniformContract(const FLegacyOriginalMaterial& Material,
    uint16 SourceRegister, uint32 SourceResourceLayoutHash,
    const FShaderParametersMetadata& Metadata, FLegacyLocalUniformContract& Out, FString& Error);

/** Complete output-construction kernel for an original inventory occurrence.
 * ConvertedShader is one shader-manifest.json shaders[] entry, not an owner
 * occurrence, from a digest-pinned, offline-verified manifest. This kernel checks
 * internal receipt consistency; it does not authenticate an arbitrary manifest.
 * All original UB and loose allocations must have explicit contracts;
 * all reflected resources must be consumed once. No implicit View/pass/VF data,
 * shader substitution, or registration is performed. Out remains unchanged on
 * failure. The caller must retain all occurrences and ordered pipeline stages.
 *
 * Deliberately NOT connected to the runtime provider: original complete member
 * contracts and compatible bindings are not yet established for every View and pass buffer.
 */
bool BuildLegacyLocalVFOutput(const FLegacyOriginalRecord& Original,
    const TSharedPtr<FJsonObject>& ConvertedShader,
    const TArray<FLegacyLocalUniformContract>& UniformContracts,
    const TArray<FLegacyLocalLooseContract>& LooseContracts,
    FLegacyMaterialShaderInput& Out, FString& Error);

/** Construct outputs for explicit original-layout targets. Each target must
 * match the complete independently recovered member tree, size and resources.
 * Original table indices are validated before mapping to the target RHI's
 * resource order. Untargeted uniforms retain the strict existing validation.
 * This writes shader headers only; it neither creates nor binds uniform buffers.
 * The renderer must bind original-ABI bytes/buffers for every explicit target.
 * Out remains unchanged on failure. No runtime provider is installed.
 */
bool BuildLegacyLocalVFOutputWithOriginalUniforms(const FLegacyOriginalRecord& Original,
    const TSharedPtr<FJsonObject>& ConvertedShader,
    const TArray<FLegacyLocalUniformContract>& UniformContracts,
    const TArray<FLegacyLocalLooseContract>& LooseContracts,
    const TArray<FLegacyLocalOriginalUniformTarget>& OriginalTargets,
    FLegacyMaterialShaderInput& Out, FString& Error);
/** Landscape-only variant for the three recovered landscape vertex factories.
 * Bound serialized landscape VF slots require independently supplied loose
 * contracts under their original names. All ordinary uniform, descriptor and
 * complete loose-allocation checks still apply, including unsupported VT feedback.
 * This constructs shader output only; it does not provide or bind runtime data.
 */
bool BuildLegacyLandscapeVFOutputWithOriginalUniforms(const FLegacyOriginalRecord& Original,
    const TSharedPtr<FJsonObject>& ConvertedShader,
    const TArray<FLegacyLocalUniformContract>& UniformContracts,
    const TArray<FLegacyLocalLooseContract>& LooseContracts,
    const TArray<FLegacyLocalOriginalUniformTarget>& OriginalTargets,
    FLegacyMaterialShaderInput& Out, FString& Error);
#endif
