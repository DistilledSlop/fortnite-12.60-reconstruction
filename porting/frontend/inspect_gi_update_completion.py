#!/usr/bin/env python3
"""Bind GI callback 141e70c30 to the original update-completion enum/API."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import struct

BASE = 0x140000000
IMAGE_SHA = '338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'
HEADER_SHA = '921121c944f636ad1674bbc23f38d31693ea6f5fada62568f9b9b905e23ae693'
SOURCE_SHA = '119ff584cfea954b740ee82e28ad48526125bce078dc0e9cfdeacbe5c7d49d79'
CONTROL_SHA = 'cf50350df4936ab335aef77ff35e54f24087f22d3e7cae5418ec4ad8d4cb313a'
ASM_SHA = '27b7476c6cd6dadaff3a1ce9c5675b6df243a9a14d6a9a02faa75cb8f8f84aec'
NAMES = ['UpdateUnknown', 'UpdateSuccess', 'UpdateSuccess_NoChange',
         'UpdateSuccess_NeedsReload', 'UpdateSuccess_NeedsRelaunch',
         'UpdateSuccess_NeedsPatch', 'UpdateFailure_PatchCheck',
         'UpdateFailure_HotfixCheck', 'UpdateFailure_NotLoggedIn']


def need(ok, message):
    if not ok:
        raise ValueError(message)


def pinned(path, expected):
    data = Path(path).read_bytes()
    need(hashlib.sha256(data).hexdigest() == expected, 'input hash: ' + str(path))
    return data


def cstring(image, va):
    start = va - BASE
    need(0 <= start < len(image), 'string address')
    end = image.index(b'\0', start)
    return image[start:end].decode('ascii')


def enum_values(image, va):
    fields = struct.unpack_from('<5QII', image, va - BASE)
    need(cstring(image, fields[2]) == 'EUpdateCompletionStatus', 'completion enum name')
    need(fields[5] == 9, 'completion enum count')
    result = []
    for index in range(fields[5]):
        name, value = struct.unpack_from('<Qq', image, fields[4] - BASE + index * 16)
        result.append((cstring(image, name), value))
    need(result == [('EUpdateCompletionStatus::' + name, i) for i, name in enumerate(NAMES)],
         'original enum names/ordinals')
    return result


def displacement(image, va, prefix, size):
    start = va - BASE
    need(image[start:start + len(prefix)] == prefix, 'instruction prefix ' + hex(va))
    return va + size + struct.unpack_from('<i', image, start + size - 4)[0]


def assembly_offsets(text):
    for name, offset in [('Status', 40), ('Complete', 64)]:
        body = text.split('FortUpdate' + name + 'Control:', 1)[1].split('.Lfunc_end', 1)[0]
        instructions = [line.strip() for line in body.splitlines()
                        if line.strip() and not line.lstrip().startswith(('#', '.'))]
        need(len(instructions) == 2 and instructions[1] == 'retq', 'control body ' + name)
        need(re.fullmatch(r'leaq\s+' + str(offset) + r'\(%rdi\),\s*%rax', instructions[0]),
             'control member offset ' + name)


def inspect(image_path, header_path, source_path, control_path, assembly_path):
    image = pinned(image_path, IMAGE_SHA)
    header = pinned(header_path, HEADER_SHA).decode()
    source = pinned(source_path, SOURCE_SHA).decode()
    pinned(control_path, CONTROL_SHA)
    assembly = pinned(assembly_path, ASM_SHA).decode()
    values = enum_values(image, 0x147881970)
    # The enum singleton references the very FEnumParams decoded above.
    need(displacement(image, 0x140fe3c00, b'\x48\x8d\x15', 7) == 0x147881970,
         'original enum singleton binding')
    # Original Init stores the manager, captures this member callback and selects +40.
    need(displacement(image, 0x1490011ab, b'\x48\x8d\x0d', 7) == 0x141e70c30,
         'Init member callback')
    for va, expected in [(0x141624f14, '48898738070000'), (0x1494a0c66, '488d7040'),
                         (0x141e70c35, '88542410'), (0x141e70c5c, '440fb6f2'),
                         (0x141e70ff1, '410fb6d6')]:
        need(image[va - BASE:va - BASE + len(bytes.fromhex(expected))].hex() == expected,
             'manager/callback argument anchor ' + hex(va))
    need(displacement(image, 0x141e7100c, b'\xe8', 5) == 0x140fde930,
         'callback completion string helper')
    # Original LexToString switches over (uint8(Status)-1), with default Unknown.
    need(image[0xfde936:0xfde947].hex() == '0fb6c2488bd9ffc883f8070f8743020000',
         'completion switch domain')
    blocks = [0x140fdeb8a] + [BASE + struct.unpack_from('<I', image, 0xfdebd4 + i * 4)[0]
                             for i in range(8)]
    for value, block in enumerate(blocks):
        end = blocks[value + 1] if 0 < value < 8 else (0x140fdeb8a if value == 8 else 0x140fdebbb)
        text = image[block - BASE:end - BASE]
        literals = []
        for offset in range(len(text) - 6):
            if text[offset:offset + 3] == b'\x48\x8d\x15':
                target = displacement(image, block + offset, b'\x48\x8d\x15', 7)
                raw = image[target - BASE:target - BASE + 120]
                literals.append(raw.decode('utf-16le').split('\0', 1)[0])
        need(literals == [NAMES[value]], 'original completion string case ' + str(value))
    enum_body = re.search(r'enum class EUpdateCompletionStatus : uint8\s*\{(.*?)\};', header, re.S)[1]
    enum_body = re.sub(r'/\*.*?\*/', '', enum_body, flags=re.S)
    need([part.strip() for part in enum_body.split(',') if part.strip()] == NAMES,
         'public enum values')
    need('FOnUpdateCheckComplete& OnUpdateCheckComplete() { return UpdateCheckCompleteDelegates; }' in header,
         'public completion event getter')
    need('DECLARE_MULTICAST_DELEGATE_OneParam(FOnUpdateCheckComplete, EUpdateCompletionStatus' in header,
         'public completion event type')
    need('OnUpdateCheckComplete().Broadcast(FinalResult);' in source, 'public producer')
    assembly_offsets(assembly)
    return {'passed': True, 'callback': '0x141e70c30', 'manager_field': '0x738',
            'event_offset': '0x40', 'event': 'UUpdateManager::OnUpdateCheckComplete',
            'argument': 'EUpdateCompletionStatus', 'enum_values': values,
            'original_lex_to_string': '0x140fde930', 'compiled_status_event_offset': 40,
            'compiled_completion_event_offset': 64, 'compiled_delegate_size': 24,
            'compiled_platform': 'Linux x86-64; not an original Windows execution',
            'full_callback_body_ported': False, 'runtime_claim': False}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('image', 'header', 'source', 'control', 'assembly', 'output'):
        parser.add_argument('--' + name, type=Path, required=True)
    args = parser.parse_args()
    report = inspect(args.image, args.header, args.source, args.control, args.assembly)
    with args.output.open('x') as out:
        json.dump(report, out, indent=2)
        out.write('\n')
    print(json.dumps(report))


if __name__ == '__main__':
    main()
