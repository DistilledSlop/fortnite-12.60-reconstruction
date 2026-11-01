#!/usr/bin/env python3
"""Freeze original UI startup code, typed metadata and bounded source package evidence."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import struct

ROOT = Path(__file__).resolve().parents[2]
BASE = 0x140000000
EXPECTED = '604900985a724144b6e0fc6e033b0cef2adb74a295a65a4fe01aceebb52df93d'


def sha(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--output', required=True, type=Path)
    ap.add_argument('--last-batch', type=int, default=9)
    ap.add_argument('--packages', type=Path, default=ROOT / 'porting/generated/frontend-native-evidence/native-ui-startup-packages-2')
    args = ap.parse_args()
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    dump = ROOT / 'porting/generated/dump/FortniteClient-Win64-Shipping.runtime.bin'
    assert sha(dump) == EXPECTED
    spec = importlib.util.spec_from_file_location('reflection', ROOT / 'porting/extract_reflection.py')
    reflection = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(reflection)

    class MemoryImage(reflection.Image):
        def off(self, address):
            offset = address - BASE if address else -1
            return offset if 0 <= offset < len(self.m) else None

    image = MemoryImage(dump)
    assert image.imagebase == BASE
    data = image.m
    decoded = {}
    for name, address in [('FortUIDirector', 0x14700d9a0), ('FortUIManager', 0x14700e250),
                          ('FortUIDataConfiguration', 0x14700a130),
                          ('FortRootViewportLayout', 0x146fc4ea0),
                          ('FortRootViewportLayout_Athena', 0x146fc4f50),
                          ('FortGameInstance', 0x1463cbc30),
                          ('FortLocalPlayer', 0x14642adf0)]:
        offset = address - BASE
        decoded[name] = {'class_block': hex(address), 'properties': reflection.decode_props(image, struct.unpack_from('<Q', data, offset + 40)[0], struct.unpack_from('<I', data, offset + 64)[0])}
    for name, address, slots in [('Director', 0x14700fc30, [0x268, 0x270, 0x280, 0x290]), ('StartupManager', 0x147011440, [0x268, 0x280, 0x290, 0x298]),
                                 ('RootBase', 0x146fc7bf0, [0x260, 0x2a8, 0x490, 0x498]),
                                 ('RootAthena', 0x146fc80c8, [0x260, 0x2a8, 0x490, 0x498])]:
        decoded[name + '_vtable'] = {'address': hex(address), 'slots': {hex(slot): hex(image.u64(address + slot)) for slot in slots}}
    enum_fields = struct.unpack_from('<5QII', data, 0x147bd80e8 - BASE)
    decoded['EFortStartupPhase'] = [{'name': image.cstr(image.u64(enum_fields[4] + i * 16)), 'value': struct.unpack_from('<q', data, enum_fields[4] - BASE + i * 16 + 8)[0]} for i in range(enum_fields[5])]
    assert decoded['EFortStartupPhase'][3] == {'name': 'EFortStartupPhase::GameStartupLoading', 'value': 3}
    game_state_fields = struct.unpack_from('<5QII', data, 0x147bd8078 - BASE)
    assert image.cstr(game_state_fields[2]) == 'EFortGameFeatureState'
    decoded['EFortGameFeatureState'] = [{'name': image.cstr(image.u64(game_state_fields[4] + i * 16)), 'value': struct.unpack_from('<q', data, game_state_fields[4] - BASE + i * 16 + 8)[0]} for i in range(game_state_fields[5])]
    assert decoded['EFortGameFeatureState'][8] == {'name': 'EFortGameFeatureState::Loaded', 'value': 8}
    (out / 'metadata.json').write_text(json.dumps(decoded, indent=2) + '\n')
    windows = {0x141e6c120: 0x400, 0x141856620: 0x100, 0x145312730: 0x100, 0x14700fc30: 0x2a8, 0x147011440: 0x2c0, 0x14515a550: 0x300, 0x145176990: 0x300, 0x1451746d0: 0x400, 0x145146bc0: 0x200, 0x14514ab50: 0x100, 0x145146470: 0x200, 0x14514b080: 0x200, 0x14515aac0: 0x80, 0x14515a790: 0x400}
    windows.update({0x146fc7bf0: 0x4a0, 0x146fc80c8: 0x4a0,
                    0x1452c6390: 0x60, 0x1452c69a0: 0x100,
                    0x145169f10: 0xd00, 0x14516c7c0: 0x140,
                    0x143e93680: 0x200, 0x14513f920: 0x800,
                    0x1405ff6a0: 0x40, 0x146e430e0: 8,
                    0x145167a40: 0x700, 0x145174c00: 0x100,
                    0x144db86e0: 0x480, 0x14530d680: 0xc0,
                    0x142b1bc60: 0x40, 0x141ed7750: 0x50,
                    0x141df8a70: 0x60, 0x141dfb870: 0x10,
                    0x140822100: 0x220, 0x147bd80e8: 56,
                    0x142b24950: 0x40})
    # Checkpoint 4: actual phase transitions and process-global context lifecycle.
    windows.update({0x141e17270: 0x1e0, 0x141df4af0: 0x280,
                    0x141df80d0: 0xb0, 0x14081fca0: 0x400,
                    0x140820790: 0x700, 0x140820f00: 0x260,
                    0x1408200c0: 0x360, 0x140821ca0: 0x460,
                    0x140822910: 0xe0, 0x140822a30: 0x1d0,
                    0x140822c00: 0x120, 0x1408206a0: 0x20,
                    0x1408206c0: 0x20, 0x1408234d0: 0x40,
                    0x1408232c0: 0x110, 0x1408143c0: 0x10,
                    0x144786700: 0xc0, 0x145a0f930: 0x2a8,
                    0x1408229f0: 10, 0x140822a00: 0x30,
                    0x145a0f798: 0x58, 0x14081f190: 0x140,
                    0x14081f2c0: 0x140, 0x14081f3f0: 0x90,
                    0x141e4f680: 0x220, 0x1402c0cd0: 0x20,
                    0x144691ab0: 0x990, 0x144687a40: 0x570,
                    0x144640340: 0xa0, 0x144627d90: 0x270,
                    0x142b24410: 0x30, 0x147bd8078: 8})
    # Checkpoint 5: real existing-player startup, teardown and strong value traversal.
    windows.update({0x14238a5c0: 0x70, 0x1423a2490: 0x150,
                    0x142989f60: 0xc0, 0x142988640: 0x30,
                    0x142366a90: 0x200, 0x14612ead8: 0x2c0,
                    0x1442eb960: 0x170, 0x14642c9d0: 0x2c0,
                    0x1442fa570: 0x10, 0x1442fa540: 0x30,
                    0x1423a2410: 0x80, 0x14238bde0: 0x4b0,
                    0x142a07490: 0xc0, 0x145a0fbd8: 0x20,
                    0x140820420: 0x40, 0x140820460: 0x210,
                    0x14081f5d0: 0x230, 0x141dedeb0: 0x90,
                    0x141dec440: 0x3a0, 0x142a02c30: 0x30})
    assert image.u64(0x14612ead8 + 0x280) == 0x14238bde0
    assert image.u64(0x14612ead8 + 0x288) == 0x1423a2490
    assert image.u64(0x14642c9d0 + 0x288) == 0x1442fa570
    assert image.u64(0x145a0fbd8 + 8) == 0x140820460
    assert next(x for x in decoded['FortLocalPlayer']['properties'] if x['name'] == 'FortOnlineAccount')['offset'] == 0x288
    for offset, target in [(0x278, 0x1408229f0), (0x290, 0x1408229f0)]:
        assert image.u64(0x145a0f930 + offset) == target
    windows[0x147bd8078] = 48
    windows[game_state_fields[4]] = game_state_fields[5] * 16
    for ptr in [game_state_fields[2]] + [image.u64(game_state_fields[4] + i * 16) for i in range(game_state_fields[5])]:
        windows[ptr] = len(image.cstr(ptr).encode()) + 1
    windows[enum_fields[4]] = enum_fields[5] * 16
    for address, size in windows.items():
        (out / (hex(address) + '.bin')).write_bytes(data[address - BASE:address - BASE + size])
    config = Path('/mnt/vibrato/fortnite-extracted-full/FortniteGame/Config/DefaultGame.ini')
    section = None
    entries = []
    for number, line in enumerate(config.read_text(encoding='utf-8-sig').splitlines(), 1):
        if line.startswith('['):
            section = line
        if line.startswith(('EntrywayManagerClassPath=', 'AthenaManagerClassPath=', 'CampaignManagerClassPath=')):
            assert section == '[/Script/FortniteUI.FortUIDirector]'
            entries.append({'line': number, 'text': line})
    assert len(entries) == 3
    (out / 'source-config-selection.json').write_text(json.dumps({'path': str(config), 'sha256': sha(config), 'section': '[/Script/FortniteUI.FortUIDirector]', 'entries': entries}, indent=2) + '\n')
    ui_config = [line for line in config.read_text(encoding='utf-8-sig').splitlines() if line.startswith(('DefaultUIDataConfigurationPath=', 'BROnlyUIDataConfigurationPath=', 'ErebusUIDataConfigurationPath='))]
    assert len(ui_config) == 3
    engine_config = config.with_name('DefaultEngine.ini')
    asset_manager = [line for line in engine_config.read_text(encoding='utf-8-sig').splitlines() if line.startswith('AssetManagerClassName=')]
    assert asset_manager == ['AssetManagerClassName=/Script/FortniteGame.FortAssetManager']
    (out / 'source-ui-data-selection.json').write_text(json.dumps({'game_ini': {'path': str(config), 'sha256': sha(config), 'selected_lines': ui_config}, 'engine_ini': {'path': str(engine_config), 'sha256': sha(engine_config), 'selected_lines': asset_manager}}, indent=2) + '\n')
    sources = [Path(__file__).resolve(), ROOT / 'porting/extract_reflection.py', ROOT / 'porting/material_pipeline/view_original_probe.py', ROOT / 'porting/ghidra/decompile_targets.py']
    for report_path in sorted(args.packages.glob('*.json')):
        report = json.loads(report_path.read_text())
        sources.append(report_path)
        for item in report['inputs']:
            path = Path(item['path'])
            assert sha(path) == item['sha256']
            sources.append(path)
        if report_path.name == 'UIManager_Startup.json':
            entry = next(x for x in report['export_map'] if x['name'] == 'Default__UIManager_Startup_C')
            raw = Path(report['inputs'][1]['path']).read_bytes()
            first = entry['serial_offset'] - report['total_header_size']
            body = raw[first:first + entry['serial_size']]
            assert len(body) == 45
            name, number, kind, kind_number, size, array_index = struct.unpack_from('<6I', body)
            assert (report['name_map'][name], number, report['name_map'][kind], kind_number, size, array_index, body[24]) == ('LayoutClass', 0, 'SoftObjectProperty', 0, 12, 0, 0)
            path_name, path_number, sub_length, terminator, terminator_number = struct.unpack_from('<5I', body, 25)
            assert report['name_map'][path_name] == '/Game/UI/Foundation/RootLayout_Athena.RootLayout_Athena_C'
            assert (path_number, sub_length, report['name_map'][terminator], terminator_number) == (0, 0, 'None', 0)
            (out / 'startup-cdo-frame.json').write_text(json.dumps({'start_in_uexp': first, 'export_size': 45, 'exact_eof': True, 'property': 'LayoutClass', 'value': report['name_map'][path_name], 'no_other_tagged_property': True, 'raw_hex': body.hex()}, indent=2) + '\n')
        if report_path.name == 'DefaultUIDataConfiguration.json':
            cdo = next(x for x in report['objects'] if x['name'] == 'Default__DefaultUIDataConfiguration_C')
            assert cdo['value']['Properties']['StartupStateWidget_Login']['AssetPathName'] == '/Game/UI/Foundation/StateLayouts/LoginScreen.LoginScreen_C'
    for batch in range(1, args.last_batch + 1):
        directory = ROOT / f'porting/generated/frontend-native-evidence/native-ui-startup-{batch}'
        assert directory.is_dir()
        sources.extend(sorted(directory.rglob('*')))
    for batch in [15, 17, 19, 20, 21]:
        sources.extend(sorted((ROOT / f'porting/generated/frontend-native-evidence/common-input-manager-queue-{batch}').rglob('*')))
    probe = ROOT / 'porting/frontend/native_ui_startup_probe'
    sources += [probe / 'Program.cs', probe / 'NativeUiStartupProbe.csproj']
    sources.extend(sorted((probe / 'bin/Release/net10.0').rglob('*')))
    frozen = out / 'inputs'
    frozen.mkdir()
    bindings = []
    for source in sources:
        if not source.is_file():
            continue
        digest = sha(source)
        target = frozen / digest
        if not target.exists():
            target.write_bytes(source.read_bytes())
        assert sha(target) == digest
        bindings.append({'source': str(source.resolve()), 'snapshot': str(target), 'sha256': digest})
    receipt = {'schema': 'original-native-ui-startup-v1', 'original': {'path': str(dump), 'sha256': EXPECTED, 'layout': 'memory; VA=0x140000000+fileoffset'}, 'inputs': bindings, 'outputs': [{'path': str(path), 'sha256': sha(path)} for path in sorted(out.glob('*')) if path.is_file()], 'runtime_claim': False, 'raw_code_is_bounded_windows_not_inferred_function_extents': True}
    (out / 'receipt.json').write_text(json.dumps(receipt, indent=2) + '\n')
    print(json.dumps({'receipt': str(out / 'receipt.json'), 'sha256': sha(out / 'receipt.json'), 'inputs': len(bindings)}))


if __name__ == '__main__':
    main()
