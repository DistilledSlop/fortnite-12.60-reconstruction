"""Dedicated-server AFortPlayerState name compatibility, not client recovery."""
import re
from pathlib import Path

HERE = Path(__file__).resolve().parent
NAMES = ('SetPlayerName', 'SetPlayerNameInternal', 'GetPlayerNameCustom', 'GetOldPlayerName')
FIELDS = ('bNativePlayerNameSetterActive', 'bNativePlayerNameOnRepActive')


def declarations(model):
    if model['name'] != 'AFortPlayerState':
        return []
    if model.get('parent') != 'APlayerState':
        raise ValueError('AFortPlayerState: player name policy parent mismatch')
    expected = {
        'SetPlayerName': ('void', ['const FString& S'], False),
        'SetPlayerNameInternal': ('void', ['const FString& S'], False),
        'GetPlayerNameCustom': ('FString', [], True),
        'GetOldPlayerName': ('FString', [], True),
    }
    for name in NAMES:
        helpers = [h for h in model.get('helpers', []) if h['name'] == name]
        rejected = [r for r in model.get('splice', {}).get('rejected', [])
                    if r.get('entry') == 'AFortPlayerState::' + name]
        if len(helpers) != 1 or rejected:
            raise ValueError('AFortPlayerState: missing or competing name helper: %s; accepted=%r; rejected=%r' % (name, helpers, rejected))
        h = helpers[0]
        if (h.get('ret'), h.get('args'), h.get('const')) != expected[name] or not h.get('override') or not h.get('virtual') or h.get('body', '').strip() != (HERE / (name + '.inc')).read_text().strip():
            raise ValueError('AFortPlayerState: name helper contract changed: ' + name)
    if any(item.get('name') in FIELDS for key in ('fields', 'members_decl_order') for item in model.get(key, [])):
        raise ValueError('AFortPlayerState: competing player name state field')
    return ['', 'private:',
            '    // Original +0x89c masks 2 (setter) and 4 (client OnRep), respectively.',
            '    bool bNativePlayerNameSetterActive = false;',
            '    bool bNativePlayerNameOnRepActive = false;']


def finalize(model, header, cpp):
    if model['name'] != 'AFortPlayerState':
        return header, cpp
    for name in NAMES + FIELDS:
        if len(re.findall(r'\b' + re.escape(name) + r'\b', header)) != 1:
            raise ValueError('AFortPlayerState: duplicate or missing declaration: ' + name)
    if (re.search(r'\bnamespace\s+FortRecoveredPlayerName\s*\{', cpp)
            or re.search(r'^\s*#\s*include\s*[<"][^>"]*PlayerNameCodec\.inl[>"]', cpp, re.MULTILINE)
            or re.search(r'\bbUseCustomPlayerNames\s*=\s*true\s*;', header + cpp)):
        raise ValueError('AFortPlayerState: competing player name policy')
    # Place codec after all generated includes and before generated definitions.
    marker = '\n\n'
    position = cpp.find(marker, cpp.find('#include '))
    if position < 0:
        raise ValueError('AFortPlayerState: cannot locate generated include boundary')
    cpp = cpp[:position] + '\n\n' + (HERE / 'PlayerNameCodec.inl').read_text().rstrip() + cpp[position:]
    name = model['name']
    pattern = re.compile(r'\b(?:' + re.escape(name) + r'::)?' + re.escape(name) + r'\s*\([^;{}]*\)\s*(?::[^{};]*)?\{')
    found = [(which, m) for which, text in [('header', header), ('cpp', cpp)] for m in pattern.finditer(text)]
    if len(found) > 1:
        raise ValueError(name + ': multiple constructor bodies require explicit merge policy')
    body = '\n' + (HERE / 'constructor.body.inc').read_text().rstrip() + '\n'
    if found:
        which, match = found[0]
        text = header if which == 'header' else cpp
        depth, end = 1, match.end()
        while depth and end < len(text):
            if text[end] == '{': depth += 1
            if text[end] == '}': depth -= 1
            end += 1
        if depth:
            raise ValueError(name + ': unclosed constructor body')
        text = text[:end - 1] + body + text[end - 1:]
        return (text, cpp) if which == 'header' else (header, text)
    if re.search(r'\b' + re.escape(name) + r'\s*\(', header):
        raise ValueError(name + ': constructor declaration without known body')
    declaration = '\n    %s(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());\n' % name
    if 'public:' not in header:
        raise ValueError(name + ': missing public section')
    header = header.replace('public:', 'public:' + declaration, 1)
    cpp += '\n%s::%s(const FObjectInitializer& ObjectInitializer)\n    : Super(ObjectInitializer)\n{%s}\n' % (name, name, body)
    return header, cpp
