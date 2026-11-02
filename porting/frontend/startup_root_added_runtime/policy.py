"""Unregistered source inputs for the real Startup root-added boundary."""
import hashlib
import json
import struct
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'contract.json').read_text())


def validate_original(image_path):
    with Path(image_path).open('rb') as image:
        for item in CONTRACT['windows']:
            image.seek(int(item['va'], 0) - 0x140000000)
            data = image.read(item['size'])
            if len(data) != item['size'] or hashlib.sha256(data).hexdigest() != item['sha256']:
                raise ValueError('Startup root-added source differs: ' + item['label'])
        for item in CONTRACT['slots']:
            image.seek(int(item['table'], 0) + int(item['slot'], 0) - 0x140000000)
            if image.read(8) != struct.pack('<Q', int(item['body'], 0)):
                raise ValueError('Startup root-added virtual target differs')


def validate_classes(blocks, names):
    for name, expected in CONTRACT['classes'].items():
        va = expected['block_va']
        if names.get(name) != va or blocks.get(va) != expected:
            raise ValueError('Startup root-added reflected type differs: ' + name)


def fragments():
    return {
        'declarations': json.loads((HERE / 'declarations.json').read_text()),
        'definitions': {
            'UFortUIManager_Startup': (HERE / 'startup.definitions.inc').read_text(),
            'UFortRootViewportLayout': (HERE / 'root.definitions.inc').read_text() +
                                      (HERE / 'root-state.definitions.inc').read_text(),
            'UFortRootViewportLayout_Athena': (HERE / 'athena.definitions.inc').read_text(),
            'UFortGlobalUIContext': (HERE / 'context.definitions.inc').read_text(),
            'UCommonUIActionRouter': (HERE / 'router.definitions.inc').read_text(),
        },
        'support_header': (HERE / 'StartupRootNativeTags.h').read_text(),
        'provider_header': (HERE / 'RootStartupProviders.h').read_text(),
        'support_definitions': (HERE / 'native-tags.definitions.inc').read_text(),
        'source_includes': {
            'UFortUIManager_Startup': ['UFortLocalPlayer.h', 'UFortRootViewportLayout.h',
                                     'UFortRootViewportLayout_Athena.h', 'UFortGameInstance.h',
                                     'StartupRootNativeTags.h'],
            'UFortRootViewportLayout': ['UFortLocalPlayer.h', 'UCommonUIActionRouter.h',
                                       'Engine/LocalPlayer.h', 'Widgets/Layout/SBox.h',
                                       'Widgets/Layout/SScaleBox.h', 'RootLayoutSlateSettings.h',
                                       'UFortGlobalUIContext.h', 'UFortProgressModal.h',
                                       'RootStartupProviders.h'],
            'UFortRootViewportLayout_Athena': ['UFortLocalPlayer.h', 'UFortUIManager.h',
                                             'UFortUIDataConfiguration.h', 'UFortUIStateWidget_Login.h',
                                             'UStartUpUIStateWidget_SubgameSelect.h',
                                             'UAthenaUIStateWidget_Frontend.h', 'UAthenaHUDBase.h',
                                             'UAthenaBroadcastHUDBase.h', 'UFortUIStateWidgetBase.h',
                                             'UFortUINotificationManager.h', 'UFortToastDisplayArea.h',
                                             'UFortUINotificationQueue.h', 'Engine/GameInstance.h',
                                             'StartupRootNativeTags.h', 'RootStartupProviders.h'],
        },
    }


def apply(*args, **kwargs):
    raise ValueError('Startup root-added promotion is not closed: ' +
                     '; '.join(CONTRACT['required_real_dependencies']))
