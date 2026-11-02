"""Queue widget acquisition/removal, requiring the integrated animated switcher."""
import hashlib
import os
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTAINER = 'UCommonActivatableWidgetContainerBase'
QUEUE = 'UCommonActivatableWidgetQueue'
FLAG = 'common_queue_widget_lifecycle_runtime'
WINDOWS = (
    (0x1407aa4c0, 0x80, '7c66f4b4c6f7d48a9f358fc15f98efb4287e32b142b11b90a9ca16e6e06d4a01'),
    (0x1407ecbe0, 0x8d, '31e50c012f9505ae94a97c6785d0410ac3ba322db86bbd2a9ea34f93c801f107'),
    (0x1407dfda0, 0xed, 'c2521fbf56e9d7eb99b1ba30229e021c29e0f3d0e1a7b52ef72acaeb06077263'),
    (0x1407f1000, 0x16d, '6214872de857826a591917ad7a8655d0dd18fe0a2eaf2f3043e8ddf151867964'),
    (0x1459fd980, 8, '5b41d4511626513e6bba6e7976d83699d9cd768c9e2bacc5c38a62bf8ac2304e'),
    (0x145a04018, 8, 'abd4fb425f17afbe063127e54656dbf5496c078149a32f77a76ff3c4cc2b92e8'),
)


def validate_source(image_path):
    fd = os.open(image_path, os.O_RDONLY)
    try:
        for va, size, digest in WINDOWS:
            data = os.pread(fd, size, va - 0x140000000)
            if len(data) != size or hashlib.sha256(data).hexdigest() != digest:
                raise ValueError('queue lifecycle original differs at ' + hex(va))
    finally:
        os.close(fd)


def apply(model):
    name = model.get('name')
    if name not in (CONTAINER, QUEUE):
        return model
    expected_parent = 'UWidget' if name == CONTAINER else CONTAINER
    if model.get('package') != '/Script/CommonUI' or model.get('parent') != expected_parent:
        raise ValueError('queue lifecycle owner differs: ' + name)
    switcher_flag = 'common_animated_switcher_runtime' + ('_queue' if name == QUEUE else '')
    if not model.get(switcher_flag):
        raise ValueError('queue lifecycle requires animated switcher integration: ' + name)
    reserved = {'NativeAddWidget1407aa4c0', 'NativeRemoveWidget1407f1000', 'NativeOnWidgetAdded1407ecbe0'}
    if model.get(FLAG) or any(x.get('name') in reserved for group in
            ('functions', 'fields', 'helpers', 'static_helpers', 'members_decl_order')
            for x in model.get(group, [])):
        raise ValueError('queue lifecycle declaration collision: ' + name)
    model[FLAG] = True
    return model


def configure_model(model):
    if model.get(FLAG):
        model['cpp_includes'] = sorted(set(model.get('cpp_includes', [])) | {
            'CommonQueueWidgetLifecycleNative.h', 'SCommonAnimatedSwitcher.h',
            'Slate/SObjectWidget.h', 'Widgets/SNullWidget.h',
            'UCommonActivatableWidget.h', 'UCommonActivatableWidgetQueue.h'})
    return model


def declarations(model):
    if not model.get(FLAG): return ''
    return (HERE / ('container.declarations.inc' if model['name'] == CONTAINER else 'queue.declarations.inc')).read_text()


def definitions(model):
    if not model.get(FLAG): return ''
    return (HERE / ('container.definitions.inc' if model['name'] == CONTAINER else 'queue.definitions.inc')).read_text()


def sources():
    name = 'CommonQueueWidgetLifecycleNative.h'
    return {name: (HERE / name).read_text()}


def validate_output(model, header, cpp):
    if not model.get(FLAG): return
    if header.count(declarations(model).strip()) != 1 or cpp.count(definitions(model).strip()) != 1:
        raise ValueError('queue lifecycle fragments missing or duplicated')
