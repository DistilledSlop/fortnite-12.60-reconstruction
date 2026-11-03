"""Emit shared native diagnostic types and the original CSV category."""
import hashlib
import importlib.util
import json
from pathlib import Path
HERE = Path(__file__).resolve().parent
MODULE = 'FortniteGame'
def sources():
    result = {}
    loading = HERE.parent / 'asset_loading_predicate_source_runtime'
    spec = importlib.util.spec_from_file_location('asset_loading_state_support', loading / 'source_policy.py')
    policy = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(policy)
    result.update(policy.sources())
    for dirname in ('asset_diagnostic_reads_source_runtime', 'asset_diagnostic_support_runtime', 'asset_load_analytics_source_runtime', 'asset_load_callback_runtime', 'independent_native_recovery_runtime', 'asset_owner_materialization_runtime', 'weapon_ammo_source_batch_runtime', 'building_init_mode_helpers_runtime', 'gadget_ammo_override_batch_runtime', 'tracked_soft_class_resolver_runtime'):
        folder = HERE.parent / dirname
        evidence = json.loads((folder / 'native-evidence.json').read_text())
        with Path(evidence['image']).open('rb') as stream:
            for window in evidence['windows']:
                begin = int(window['begin'], 16)
                length = window.get('length', int(window.get('end_exclusive', window['begin']), 16) - begin)
                stream.seek(begin - 0x140000000)
                if hashlib.sha256(stream.read(length)).hexdigest() != window['sha256']:
                    raise ValueError('asset diagnostic original capture changed: ' + hex(begin))
    reads = HERE.parent / 'asset_diagnostic_reads_source_runtime'
    result['FortAssetDiagnosticMap.h'] = (reads / 'FortAssetDiagnosticMap.h').read_text()
    provider = HERE.parent / 'controller_analytics_provider_binding_runtime'
    result['FortNativeAnalyticsProvider.h'] = (provider / 'FortNativeAnalyticsProvider.h').read_text()
    for name in ('SharedFortCsv.h', 'SharedFortCsv.cpp'):
        result[name] = (HERE / name).read_text()
    callback = HERE.parent / 'asset_load_callback_runtime'
    for name in ('FortAssetLoadDiagnosticState.h', 'FortAssetLoadDiagnosticState.cpp'):
        result[name] = (callback / name).read_text()
    owner = HERE.parent / 'asset_owner_materialization_runtime'
    for name in ('FortAssets.h', 'FortAssets.cpp'):
        result[name] = (owner / name).read_text()
    ammo = HERE.parent / 'weapon_ammo_source_batch_runtime'
    result['FortDelayedObjectLoader.h'] = (ammo / 'FortDelayedObjectLoader.h').read_text()
    gadget = HERE.parent / 'gadget_ammo_override_batch_runtime'
    for name in ('FortGadgetAmmo.h', 'FortGadgetAmmo.cpp'):
        result[name] = (gadget / name).read_text()
    classes = HERE.parent / 'tracked_soft_class_resolver_runtime'
    result['FortTrackedTimeOfDayClass.h'] = (classes / 'FortTrackedTimeOfDayClass.h').read_text()
    return result
