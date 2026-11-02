"""Fail-closed, bounded source/model hooks for the Slate lifecycle candidate."""
import hashlib
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'original-windows.json').read_text())
NAMES = ('NativeBox', 'NativeIcon', 'NativeProgress', 'NativeRim', 'NativeCachedBrush',
         'RebuildWidget', 'OnWidgetRebuilt', 'ReleaseSlateResources')
MEMBERS = {'ProgressMaterialBrush': 'FSlateBrush ProgressMaterialBrush;',
           'IconRimBrush': 'FSlateBrush IconRimBrush;',
           'ProgressMaterialParam': 'FName ProgressMaterialParam;',
           'ProgressDynamicMaterial': 'UMaterialInstanceDynamic* ProgressDynamicMaterial;',
           'InputActions': 'TArray<FDataTableRowHandle> InputActions;'}


def validate_source(image_path):
    with Path(image_path).open('rb') as image:
        for w in CONTRACT['windows']:
            image.seek(int(w['va'], 16) - 0x140000000)
            data = image.read(w['size'])
            if len(data) != w['size'] or hashlib.sha256(data).hexdigest() != w['sha256']:
                raise ValueError('original widget Slate code differs: ' + w['name'])


def apply(model, block, owner_fns):
    if model['name'] != 'UCommonActionWidget':
        return model
    if (model.get('package') != '/Script/CommonUI' or model.get('parent') != 'UWidget'
            or block.get('block_va') != CONTRACT['class_block']
            or block.get('class_noregister_fn') != CONTRACT['class_noregister_fn']
            or block.get('properties') != CONTRACT['properties']
            or '0x140807790' not in owner_fns):
        raise ValueError('original widget owner or reflected properties differ')
    for name, decl in MEMBERS.items():
        if [m.get('decl') for m in model.get('members_decl_order', []) if m.get('name') == name] != [decl]:
            raise ValueError('widget reflected declaration differs: ' + name)
    if not model.get('common_action_widget_receivers_runtime'):
        raise ValueError('widget Slate lifecycle requires complete receiver policy and aliases')
    if model.get('common_action_widget_slate_runtime') or any(
            m.get('name') in NAMES for group in ('functions', 'members_table_order',
            'members_decl_order', 'fields', 'helpers', 'static_helpers') for m in model.get(group, [])):
        raise ValueError('widget Slate lifecycle declaration collision')
    model['common_action_widget_slate_runtime'] = True
    return model


def declarations(model):
    return (HERE / 'declarations.inc').read_text() if model.get('common_action_widget_slate_runtime') else ''


def definitions(model):
    return (HERE / 'definitions.inc').read_text() if model.get('common_action_widget_slate_runtime') else ''


def validate_rendered(model, header, source):
    if model.get('common_action_widget_slate_runtime'):
        if declarations(model) not in header or definitions(model) not in source:
            raise ValueError('widget Slate fragments absent from generated output')
