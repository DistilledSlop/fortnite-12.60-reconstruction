#include "CoreMinimal.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

#if PLATFORM_LINUX
#include "DynamicRHI.h"
#include "Misc/Crc.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/SecureHash.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "VulkanCommon.h"
#include "vulkan.h"
#include "VulkanShaderResources.h"

// The mapped receipt identifies these two geometry shaders as having no descriptors.
// Their SHA-1s are checked here against the exact SPIR-V whose SHA-256 was checked
// by the mapped translation report; this probe never modifies the source artifacts.
static bool ProbeOriginalGeometry(const FString& Directory, const TCHAR* File, const TCHAR* ExpectedHash)
{
    TArray<uint8> Bytes;
    if (!FFileHelper::LoadFileToArray(Bytes, *FPaths::Combine(Directory, File)) || Bytes.Num() < 20 || Bytes.Num() % 4)
        return false;
    FSHAHash Hash;
    FSHA1::HashBuffer(Bytes.GetData(), Bytes.Num(), Hash.Hash);
    if (!Hash.ToString().Equals(ExpectedHash, ESearchCase::IgnoreCase))
        return false;

    TArray<uint32> Spirv;
    Spirv.SetNumUninitialized(Bytes.Num() / 4);
    FMemory::Memcpy(Spirv.GetData(), Bytes.GetData(), Bytes.Num());
    if (Spirv[0] != 0x07230203) return false;

    // SPIR-V OpEntryPoint's name is a NUL-terminated string followed by interface IDs.
    // Rebuild just that instruction so UE's runtime entry point convention matches.
    int32 Entry = INDEX_NONE, OldNameWords = 0;
    for (int32 I = 5; I < Spirv.Num(); )
    {
        const uint32 Count = Spirv[I] >> 16, Op = Spirv[I] & 0xffff;
        if (!Count || Count > uint32(Spirv.Num() - I)) return false;
        if (Op == 15)
        {
            if (Entry != INDEX_NONE || Count < 4 || Spirv[I + 1] != 3) return false;
            Entry = I;
            const ANSICHAR* Name = reinterpret_cast<const ANSICHAR*>(&Spirv[I + 3]);
            const int32 MaxBytes = (Count - 3) * 4;
            int32 Len = 0;
            while (Len < MaxBytes && Name[Len]) ++Len;
            if (Len == MaxBytes || FCStringAnsi::Strncmp(Name, "main", Len) != 0 || Len != 4) return false;
            OldNameWords = (Len + 4) / 4;
        }
        I += Count;
    }
    if (Entry == INDEX_NONE) return false;
    const uint32 CRC = FCrc::MemCrc32(Bytes.GetData(), Bytes.Num());
    constexpr int32 NewNameWords = 6; // 22 ASCII characters and NUL, padded to 24 bytes
    const uint32 NewBytes = (Spirv.Num() + NewNameWords - OldNameWords) * 4;
    ANSICHAR NewName[24] = {};
    FCStringAnsi::Snprintf(NewName, UE_ARRAY_COUNT(NewName), "main_%08x_%08x", NewBytes, CRC);
    Spirv.RemoveAt(Entry + 3, OldNameWords);
    Spirv.InsertZeroed(Entry + 3, NewNameWords);
    FMemory::Memcpy(&Spirv[Entry + 3], NewName, sizeof(NewName));
    Spirv[Entry] += (NewNameWords - OldNameWords) << 16;

    FString OutputDirectory;
    if (FParse::Value(FCommandLine::Get(), TEXT("FortniteOriginalSpirvOutputDir="), OutputDirectory))
    {
        const FString OutputPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(OutputDirectory, File));
        if (FPaths::FileExists(OutputPath)) return false;
        TArray<uint8> Rewritten;
        Rewritten.Append(reinterpret_cast<const uint8*>(Spirv.GetData()), Spirv.Num() * sizeof(uint32));
        if (!FFileHelper::SaveArrayToFile(Rewritten, *OutputPath)) return false;
    }

    FVulkanShaderHeader Header(FVulkanShaderHeader::EZero);
    // Diagnostic identity only; original compiler source hash is not available here.
    Header.SourceHash = Hash;
    Header.SpirvCRC = CRC;
    Header.DebugName = File;
    TArray<uint8> Code;
    FMemoryWriter Writer(Code, true);
    Writer << Header;
    Writer << Spirv;
    if (Writer.IsError()) return false;
    FVulkanShaderHeader ReadHeader(FVulkanShaderHeader::EZero);
    TArray<uint32> ReadSpirv;
    FMemoryReader Reader(Code, true);
    Reader << ReadHeader;
    Reader << ReadSpirv;
    if (Reader.IsError() || Reader.Tell() != Code.Num() || ReadHeader.SpirvCRC != CRC ||
        ReadHeader.SourceHash != Hash || ReadHeader.DebugName != File || ReadSpirv != Spirv ||
        ReadHeader.UniformBuffers.Num() || ReadHeader.Globals.Num() || ReadHeader.PackedUBs.Num()) return false;
    // The Vulkan RHI factory consumes the actual serialized header and original translated code.
    // Shader module creation is deferred until a pipeline requests a layout.
    FGeometryShaderRHIRef Shader = RHICreateGeometryShader(Code, Hash);
    UE_LOG(LogTemp, Display, TEXT("Original Vulkan geometry %s: header round-trip and RHI factory %s; module/pipeline/draw untested"),
        File, Shader.IsValid() ? TEXT("PASS") : TEXT("FAIL"));
    return Shader.IsValid();
}
#endif

void RunOriginalVulkanShaderProbe()
{
    if (!FParse::Param(FCommandLine::Get(), TEXT("FortniteOriginalVulkanShaderProbe"))) return;
#if PLATFORM_LINUX
    FString Directory;
    if (!FParse::Value(FCommandLine::Get(), TEXT("FortniteOriginalSpirvDir="), Directory) ||
        !GDynamicRHI || FCString::Stricmp(GDynamicRHI->GetName(), TEXT("Vulkan")) != 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Original Vulkan probe requires Vulkan RHI and -FortniteOriginalSpirvDir=<mapped directory>"));
        return;
    }
    const bool A = ProbeOriginalGeometry(Directory, TEXT("26-fe496ceeced2287caea1.spv"), TEXT("41e9268d31f79a7e9572cae252841f102d641f46"));
    const bool B = ProbeOriginalGeometry(Directory, TEXT("2459-902588da6ec2c15d140f.spv"), TEXT("9f240c5d61753a1710c860696578dfcccdd8cf4b"));
    UE_LOG(LogTemp, Display, TEXT("Original Vulkan shader probe: %s"), A && B ? TEXT("PASS") : TEXT("FAIL"));
#else
    UE_LOG(LogTemp, Error, TEXT("Original Vulkan shader probe is supported only on Linux"));
#endif
}
