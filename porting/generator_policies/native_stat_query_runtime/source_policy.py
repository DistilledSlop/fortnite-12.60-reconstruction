"""Restore the original non-reflected stat storage and native query (1424aebb0)."""
import re


def _add(header, text):
    end = header.rfind('\n};')
    if end < 0:
        raise ValueError('class terminator not found')
    return header[:end] + '\n' + text + header[end:]


def finalize(model, header, cpp):
    name = model['name']
    if name in ('UStat', 'UBaseStatManager', 'AFortPlayerController') and model.get('package') != '/Script/FortniteGame':
        raise ValueError(name + ': native stat package changed')
    if name == 'UBaseStatManager':
        if model.get('parent') != 'UObject' or 'NativeStatsByName' in header:
            raise ValueError('unexpected UBaseStatManager layout')
        header = _add(header, '\t// Original non-reflected map. Its values are not GC references.\n'
                      '\tTMap<FName, UStat*> NativeStatsByName;\n'
                      '\tint32 GetNativeStatValue(FName Name, EStatRecordingPeriod Period) const;')
        header = header.replace('#include "UObject/Object.h"', '#include "UObject/Object.h"\n#include "EStatRecordingPeriod.h"', 1)
        cpp += '''\n#include "UStat.h"\n#include <xmmintrin.h>\n\nint32 UBaseStatManager::GetNativeStatValue(FName Name, EStatRecordingPeriod Period) const\n{\n    UStat* const* Entry = NativeStatsByName.Find(Name);\n    if (!Entry || !*Entry)\n        return 0;\n    const float Value = static_cast<float>((*Entry)->NativePeriodValues.GetData()[static_cast<uint8>(Period)]);\n    const __m128 V = _mm_set_ss(Value);\n    const int32 Rounded = _mm_cvtss_si32(_mm_sub_ss(_mm_set_ss(-0.5f), _mm_add_ss(V, V)));\n    return static_cast<int32>(0u - static_cast<uint32>(Rounded >> 1));\n}\n'''
    elif name == 'UStat':
        if model.get('parent') != 'UObject' or 'NativePeriodValues' in header or re.search(r'\bUStat\s*\(', header):
            raise ValueError('unexpected UStat constructor or layout')
        header = _add(header, '\tUStat(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());\n'
                      '\t// Original non-reflected period values; initial seven entries are zero.\n'
                      '\tTArray<double> NativePeriodValues;')
        cpp += '''\nUStat::UStat(const FObjectInitializer& ObjectInitializer)\n    : Super(ObjectInitializer)\n{\n    NativePeriodValues.AddZeroed(7);\n}\n'''
    elif name == 'AFortPlayerController':
        if 'GetNativeStatValue(' in header:
            raise ValueError('unexpected controller query declaration')
        header = _add(header, '\tint32 GetNativeStatValue(FName Name, EStatRecordingPeriod Period) const;')
        header = header.replace('#include "AFortPlayerController.generated.h"', '#include "EStatRecordingPeriod.h"\n#include "AFortPlayerController.generated.h"', 1)
        cpp += '''\n#include "UStatManager.h"\n\nint32 AFortPlayerController::GetNativeStatValue(FName Name, EStatRecordingPeriod Period) const\n{\n    return StatManager ? StatManager->GetNativeStatValue(Name, Period) : 0;\n}\n'''
    return header, cpp
