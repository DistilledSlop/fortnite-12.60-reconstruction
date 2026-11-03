"""Proposed native interface and owned provider fields; generator hooks required."""

BLOCKS = {
    'AFortPlayerController': '0x14649da30',
    'UFortLocalPlayer': '0x14642adf0',
}


def apply(model, block, owner_fns):
    validate_evidence()
    name = model['name']
    if name not in BLOCKS:
        return model
    if model.get('block_va') != BLOCKS[name] or block.get('block_va') != BLOCKS[name]:
        raise ValueError('analytics provider original class identity changed')
    if name == 'AFortPlayerController':
        bases = model.setdefault('native_interface_bases', [])
        if 'IFortAnalyticsControllerInterface' not in bases:
            bases.append('IFortAnalyticsControllerInterface')
        model['splice'].setdefault('header_includes', set()).add('UFortAnalyticsControllerInterface.h')
    return model


def interface_declarations(name):
    if name == 'UFortAnalyticsControllerInterface':
        return ['public:', '    virtual TSharedPtr<class FFortNativeAnalyticsProvider, ESPMode::NotThreadSafe>& NativeGetAnalyticsProvider() = 0;']
    return []


def class_declarations(name):
    if name in BLOCKS:
        return ['public:', '    TSharedPtr<class FFortNativeAnalyticsProvider, ESPMode::NotThreadSafe> NativeAnalyticsProvider;']
    return []


_VALIDATED = False
def validate_evidence():
    global _VALIDATED
    if _VALIDATED:
        return
    import hashlib, json
    from pathlib import Path
    evidence = json.loads((Path(__file__).parent / 'native-evidence.json').read_text())
    with Path(evidence['image']).open('rb') as stream:
        for window in evidence['windows']:
            stream.seek(int(window['begin'], 16) - 0x140000000)
            if hashlib.sha256(stream.read(window['length'])).hexdigest() != window['sha256']:
                raise ValueError('controller analytics original capture changed: ' + window['begin'])
    _VALIDATED = True
