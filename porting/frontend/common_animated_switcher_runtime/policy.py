"""Original container, queue-clear and animated Slate switcher source policy."""
import hashlib
import json
import os
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'original-policy.json').read_text())
WINDOWS = json.loads((HERE / 'original-windows.json').read_text())['windows']
FLAG = 'common_animated_switcher_runtime'
CONTAINER = 'UCommonActivatableWidgetContainerBase'
QUEUE = 'UCommonActivatableWidgetQueue'
ACTIVATABLE = 'UCommonActivatableWidget'
# Non-reflected declarations are inserted in original relative field order.
FIELDS = (
    ('NativeContainerOverlay1a8', 'TSharedPtr<SOverlay> NativeContainerOverlay1a8;', 0x1a8),
    ('NativeTransitionSpacer1b8', 'TSharedPtr<SSpacer> NativeTransitionSpacer1b8;', 0x1b8),
    ('NativeAnimatedSwitcher1c8', 'TSharedPtr<SCommonAnimatedSwitcher> NativeAnimatedSwitcher1c8;', 0x1c8),
    ('NativeDeferredSlateWidgets1d8', 'TArray<TSharedPtr<SWidget>> NativeDeferredSlateWidgets1d8;', 0x1d8),
    ('NativePendingDeactivation1e8', 'bool NativePendingDeactivation1e8 = false;', 0x1e8),
    ('NativeDisplayedWidgetChanged1f0', 'FNativeDisplayedWidgetChanged NativeDisplayedWidgetChanged1f0;', 0x1f0),
)
MEMBERS = {
    'TransitionType': 'ECommonSwitcherTransition TransitionType;',
    'TransitionCurveType': 'ETransitionCurve TransitionCurveType;',
    'TransitionDuration': 'float TransitionDuration;',
    'WidgetList': 'TArray<UCommonActivatableWidget*> WidgetList;',
    'DisplayedWidget': 'UCommonActivatableWidget* DisplayedWidget;',
    'GeneratedWidgetsPool': 'FUserWidgetPool GeneratedWidgetsPool;',
}
RESERVED = {v[0] for v in FIELDS} | {'ClearWidgets', 'NativeCreateAnimatedSwitcher1407e7d70',
    'NativeHasSwitcher1c8', 'NativeSwitcherIndex1407c19a0', 'NativeSwitcherTransition140800440',
    'NativeSwitcherIndexChanged1407c61f0', 'NativeSwitcherTransitioningChanged1407cbc30',
    'NativeSetActiveIndex1407f9d70', 'NativeRemoveSlateWidget1407eefa0',
    'NativeReleaseDeferredSlate1407bc5a0', 'NativeDisplayedWidgetDeactivated1407c65f0',
    'ReleaseSlateResources', 'RebuildWidget'}
INCLUDES = {'SCommonAnimatedSwitcher.h', 'Widgets/Layout/SBox.h', 'Widgets/Layout/SSpacer.h',
    'Widgets/SNullWidget.h', 'Widgets/SOverlay.h', 'Containers/Ticker.h', 'Slate/SObjectWidget.h',
    'UCommonActivatableWidget.h', 'UCommonActivatableWidgetQueue.h', 'CommonAnimatedSwitcherNative.h'}
# The caller must explicitly assert these real, already-generated collaborators.
REQUIRED_DEPENDENCIES = frozenset({'activatable_widget_activation_and_deactivation',
                                   'activatable_widget_native_deactivation_delegate'})


def validate_source(image_path):
    fd = os.open(image_path, os.O_RDONLY)
    try:
        for w in WINDOWS:
            data = os.pread(fd, w['size'], int(w['va'], 16) - 0x140000000)
            if len(data) != w['size'] or hashlib.sha256(data).hexdigest() != w['sha256']:
                raise ValueError('animated switcher original bounded window differs: ' + w['va'])
    finally:
        os.close(fd)


def validate_classes(blocks, names, native_execs):
    for name, record in CONTRACT['classes'].items():
        ident = record['identity']
        block = blocks.get(ident['block_va'])
        if (names.get(name) != ident['block_va'] or block is None
                or any(block.get(k) != v for k, v in ident.items())
                or block.get('properties') != record['properties']):
            raise ValueError('animated switcher class/fields differ: ' + name)
    record = CONTRACT['classes'][CONTAINER]
    block = blocks[record['identity']['block_va']]
    if [f for f in block.get('functions', []) if f.get('name') == 'ClearWidgets'] != [record['clear_function']]:
        raise ValueError('ClearWidgets original function metadata differs')
    if not isinstance(native_execs, dict):
        raise ValueError('ClearWidgets original native exec table missing')
    owner = native_execs.get(CONTAINER, native_execs)
    if owner.get('ClearWidgets') != record['clear_exec']:
        raise ValueError('ClearWidgets original native exec differs')


def validate_types(gate):
    for cpp, fn in (('ECommonSwitcherTransition', '0x140817f80'), ('ETransitionCurve', '0x140817fe0')):
        info = gate.info(fn)
        if (info is None or info.fn != fn or info.kind != 'enum' or info.cpp_name != cpp
                or info.package != '/Script/CommonUI' or info.rec != CONTRACT['enums'][cpp]):
            raise ValueError('animated switcher type identity differs: ' + cpp)
        reason = gate.reason(info)
        if reason is not None:
            raise ValueError('animated switcher type unavailable: ' + cpp + ': ' + reason)


def apply(model, block, owner_fns, native_execs=None, proven_dependencies=None):
    name = model.get('name')
    if name not in (CONTAINER, QUEUE):
        return model
    record = CONTRACT['classes'][name]
    if (model.get('package') != '/Script/CommonUI'
            or model.get('parent') != ('UWidget' if name == CONTAINER else CONTAINER)
            or any(block.get(k) != v for k, v in record['identity'].items())
            or block.get('properties') != record['properties']):
        raise ValueError('animated switcher owner model differs: ' + name)
    if name == QUEUE:
        model[FLAG + '_queue'] = True
        return model
    if '0x140806650' not in owner_fns:
        raise ValueError('container original owner join missing')
    if frozenset(proven_dependencies or ()) != REQUIRED_DEPENDENCIES:
        raise ValueError('animated switcher requires the real activation/deactivation bodies and native deactivation delegate')
    if [f for f in block.get('functions', []) if f.get('name') == 'ClearWidgets'] != [record['clear_function']]:
        raise ValueError('ClearWidgets reflected function differs')
    if not isinstance(native_execs, dict) or native_execs.get('ClearWidgets') != record['clear_exec']:
        raise ValueError('ClearWidgets native exec differs')
    for member, decl in MEMBERS.items():
        if [x.get('decl') for x in model.get('members_decl_order', []) if x.get('name') == member] != [decl]:
            raise ValueError('container emitted field differs: ' + member)
    if model.get(FLAG) or any(x.get('name') in RESERVED for group in
            ('functions', 'fields', 'helpers', 'static_helpers', 'members_decl_order')
            for x in model.get(group, [])):
        raise ValueError('animated switcher declaration collision')
    for name, decl, offset in FIELDS:
        model.setdefault('fields', []).append({'name': name, 'decl': decl, 'access': 'public', 'offset': offset})
    model['functions'].append(dict(name='ClearWidgets', ret='void', args=[], const=False,
        rpc=False, service=None, notify=False, splice=False, blueprint_event=False,
        runtime_implementation=True, access='public', specifiers=['BlueprintCallable'], used={}))
    model['skipped'] = [x for x in model.get('skipped', []) if not
        (x.get('item') == 'function' and x.get('name') == 'ClearWidgets')]
    model[FLAG] = True
    return model


def configure_model(model):
    if model.get(FLAG):
        model['includes'] = sorted(set(model.get('includes', [])) | {'CommonAnimatedSwitcherNative.h'})
        model['cpp_includes'] = sorted(set(model.get('cpp_includes', [])) | INCLUDES)
        model['forward_decls'] = sorted(set(model.get('forward_decls', [])) |
                                        {'SOverlay', 'SSpacer', 'SCommonAnimatedSwitcher'})
    return model


def declarations(model):
    if not model.get(FLAG): return ''
    return (HERE / 'container.constructor.declarations.inc').read_text() + (HERE / 'container.declarations.inc').read_text()


def definitions(model):
    if not model.get(FLAG): return ''
    return ('\n'.join((HERE / name).read_text() for name in
        ('container.constructor.definitions.inc', 'container.definitions.inc', 'modal-queue.definitions.inc')))


def sources(blocks, names, native_execs):
    validate_classes(blocks, names, native_execs)
    return {name: (HERE / name).read_text() for name in
            ('CommonAnimatedSwitcherNative.h', 'SCommonAnimatedSwitcher.h', 'SCommonAnimatedSwitcher.cpp')}


def validate_output(model, header, cpp):
    if not model.get(FLAG): return
    for output, fragment in ((header, declarations(model)), (cpp, definitions(model))):
        if output.count(fragment.strip()) != 1:
            raise ValueError('animated switcher declaration or definition missing/duplicated')
    for name, decl, _ in FIELDS:
        if header.count(decl) != 1:
            raise ValueError('animated switcher field missing or duplicated: ' + name)
    for name in ('ClearWidgets', 'NativeSetActiveIndex1407f9d70', 'RebuildWidget', 'ReleaseSlateResources'):
        if cpp.count(CONTAINER + '::' + name + '(') != 1:
            raise ValueError('animated switcher method missing or duplicated: ' + name)
    for inc in INCLUDES:
        if '#include "' + inc + '"' not in cpp:
            raise ValueError('animated switcher include missing: ' + inc)
