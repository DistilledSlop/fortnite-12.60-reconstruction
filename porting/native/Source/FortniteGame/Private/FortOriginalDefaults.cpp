#include "FortOriginalDefaults.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/Class.h"
#include "UObject/Package.h"
#include "UObject/UObjectIterator.h"
#include "UObject/UnrealType.h"
#include <cfloat>

DEFINE_LOG_CATEGORY_STATIC(LogFortOriginalDefaults, Log, All);

namespace
{
    bool IsFortniteNativeClass(const UClass* Class)
    {
        return Class && Class->HasAnyClassFlags(CLASS_Native) && Class->GetOutermost() &&
            Class->GetOutermost()->GetName().StartsWith(TEXT("/Script/Fortnite"));
    }

    // Parse decimal integer tokens without routing through double or accepting overflow.
    bool ParseMagnitude(const FString& Text, bool& bNegative, uint64& Magnitude)
    {
        int32 Index = 0;
        bNegative = Text.StartsWith(TEXT("-"));
        if (bNegative) ++Index;
        if (Index == Text.Len()) return false;
        Magnitude = 0;
        for (; Index < Text.Len(); ++Index)
        {
            const TCHAR Char = Text[Index];
            if (Char < TEXT('0') || Char > TEXT('9')) return false;
            const uint64 Digit = static_cast<uint64>(Char - TEXT('0'));
            if (Magnitude > (MAX_uint64 - Digit) / 10) return false;
            Magnitude = Magnitude * 10 + Digit;
        }
        return true;
    }

    enum class EApplyResult { Applied, Nonzero, Unknown, Invalid };

    EApplyResult ApplyValue(FProperty* Property, UObject* CDO, const TSharedPtr<FJsonValue>& Value)
    {
        if (!Property || Property->ArrayDim != 1 || !Value.IsValid()) return EApplyResult::Unknown;
        void* Data = Property->ContainerPtrToValuePtr<void>(CDO);
        if (FBoolProperty* Bool = CastField<FBoolProperty>(Property))
        {
            if (Bool->GetPropertyValue(Data)) return EApplyResult::Nonzero;
            bool bValue = false;
            if (Value->Type == EJson::Boolean)
            {
                if (!Value->TryGetBool(bValue)) return EApplyResult::Invalid;
            }
            else if (Value->Type == EJson::Number)
            {
                FString Number;
                if (!Value->TryGetString(Number) || (Number != TEXT("0") && Number != TEXT("1"))) return EApplyResult::Invalid;
                bValue = Number == TEXT("1");
            }
            else return EApplyResult::Invalid;
            Bool->SetPropertyValue(Data, bValue);
            return EApplyResult::Applied;
        }
        FNumericProperty* Numeric = CastField<FNumericProperty>(Property);
        if (FEnumProperty* Enum = CastField<FEnumProperty>(Property)) Numeric = Enum->GetUnderlyingProperty();
        if (!Numeric || Value->Type != EJson::Number) return Numeric ? EApplyResult::Invalid : EApplyResult::Unknown;
        if (Numeric->IsFloatingPoint())
        {
            if (Numeric->GetFloatingPointPropertyValue(Data) != 0.0) return EApplyResult::Nonzero;
            double Number;
            if (!Value->TryGetNumber(Number) || !FMath::IsFinite(Number) ||
                (CastField<FFloatProperty>(Numeric) && (Number > FLT_MAX || Number < -FLT_MAX))) return EApplyResult::Invalid;
            Numeric->SetFloatingPointPropertyValue(Data, Number);
            return EApplyResult::Applied;
        }
        if (!Numeric->IsInteger()) return EApplyResult::Unknown;
        if (Numeric->GetUnsignedIntPropertyValue(Data) != 0) return EApplyResult::Nonzero;
        FString Text;
        bool bNegative;
        uint64 Magnitude;
        if (!Value->TryGetString(Text) || !ParseMagnitude(Text, bNegative, Magnitude)) return EApplyResult::Invalid;
        const bool bSigned = CastField<FInt8Property>(Numeric) || CastField<FInt16Property>(Numeric) ||
            CastField<FIntProperty>(Numeric) || CastField<FInt64Property>(Numeric);
        const int32 Bits = Numeric->ElementSize * 8;
        if (Bits <= 0 || Bits > 64) return EApplyResult::Invalid;
        if (bSigned)
        {
            const uint64 Limit = uint64(1) << (Bits - 1);
            if (Magnitude > (bNegative ? Limit : Limit - 1)) return EApplyResult::Invalid;
            // Unsigned subtraction preserves INT64_MIN without signed overflow.
            Numeric->SetIntPropertyValue(Data, bNegative ? uint64(0) - Magnitude : Magnitude);
        }
        else
        {
            const uint64 Limit = Bits == 64 ? MAX_uint64 : (uint64(1) << Bits) - 1;
            if ((bNegative && Magnitude != 0) || Magnitude > Limit) return EApplyResult::Invalid;
            Numeric->SetIntPropertyValue(Data, Magnitude);
        }
        return EApplyResult::Applied;
    }
}

void FortOriginalDefaults::Apply()
{
    if (!FParse::Param(FCommandLine::Get(), TEXT("FortniteOriginalClient")) ||
        FParse::Param(FCommandLine::Get(), TEXT("FortniteNoOriginalDefaults"))) return;

    const FString Path = FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("OriginalDefaults.json"));
    FString Text;
    TSharedPtr<FJsonObject> Root;
    if (!FFileHelper::LoadFileToString(Text, *Path) ||
        !FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text), Root, FJsonSerializer::EFlags::StoreNumbersAsStrings) || !Root.IsValid())
    {
        UE_LOG(LogFortOriginalDefaults, Warning, TEXT("OriginalDefaults: cannot read or parse %s"), *Path);
        return;
    }

    TArray<UClass*> NativeClasses;
    for (TObjectIterator<UClass> It; It; ++It)
    {
        if (IsFortniteNativeClass(*It)) NativeClasses.Add(*It);
    }
    TMap<UClass*, TSharedPtr<FJsonObject>> Entries;
    int32 Unknown = 0, Invalid = 0, Applied = 0, Nonzero = 0, Classes = 0;
    for (const auto& Entry : Root->Values)
    {
        UClass* Class = FindObject<UClass>(ANY_PACKAGE, *Entry.Key);
        const TSharedPtr<FJsonObject>* Object = nullptr;
        if (!IsFortniteNativeClass(Class) || !Entry.Value.IsValid() || !Entry.Value->TryGetObject(Object) || !Object || !Object->IsValid())
        {
            ++Unknown;
            continue;
        }
        Entries.Add(Class, *Object);
    }
    const bool bVerbose = FParse::Param(FCommandLine::Get(), TEXT("FortniteOriginalDefaultsVerbose"));
    for (UClass* Class : NativeClasses)
    {
        TArray<UClass*> Ancestors;
        for (UClass* Current = Class; Current; Current = Current->GetSuperClass())
            if (Entries.Contains(Current)) Ancestors.Add(Current);
        if (Ancestors.Num() == 0) continue;
        UObject* CDO = Class->GetDefaultObject();
        if (!CDO) continue;
        ++Classes;
        for (int32 Index = Ancestors.Num() - 1; Index >= 0; --Index)
        {
            for (const auto& Entry : Entries.FindChecked(Ancestors[Index])->Values)
            {
                FProperty* Property = FindFProperty<FProperty>(Class, FName(*Entry.Key));
                const EApplyResult Result = ApplyValue(Property, CDO, Entry.Value);
                if (Result == EApplyResult::Applied)
                {
                    ++Applied;
                    if (bVerbose)
                    {
                        FString ValueText;
                        Property->ExportTextItem(ValueText, Property->ContainerPtrToValuePtr<void>(CDO), nullptr, CDO, PPF_None);
                        UE_LOG(LogFortOriginalDefaults, Log, TEXT("OriginalDefaults: %s.%s = %s"), *Class->GetName(), *Entry.Key, *ValueText);
                    }
                }
                else if (Result == EApplyResult::Nonzero) ++Nonzero;
                else if (Result == EApplyResult::Unknown) ++Unknown;
                else ++Invalid;
            }
        }
    }
    UE_LOG(LogFortOriginalDefaults, Log, TEXT("OriginalDefaults: applied %d values on %d classes (%d skipped nonzero, %d unknown)"), Applied, Classes, Nonzero, Unknown);
    if (Unknown || Invalid) UE_LOG(LogFortOriginalDefaults, Warning, TEXT("OriginalDefaults: %d unknown classes/properties and %d invalid values skipped"), Unknown, Invalid);
}
