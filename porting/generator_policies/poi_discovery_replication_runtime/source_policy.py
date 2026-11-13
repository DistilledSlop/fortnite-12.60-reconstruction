"""Restore the original POI discoverability component replication default."""
import re

CLASS = 'UFortControllerComponent_MapDiscoverability'


def finalize(model, header, cpp):
    if model['name'] != CLASS:
        return header, cpp
    if model.get('parent') != 'UFortControllerComponent_HasCoupledWidgets':
        raise ValueError(CLASS + ': unexpected parent')
    if re.search(r'\bSetIsReplicated(?:ByDefault)?\s*\(', header + cpp):
        raise ValueError(CLASS + ': competing replication default')
    pattern = re.compile(r'\b(?:' + CLASS + r'::)?' + CLASS + r'\s*\([^;{}]*\)\s*(?::[^{};]*)?\{')
    found = [(which, match) for which, text in (('header', header), ('cpp', cpp))
             for match in pattern.finditer(text)]
    if len(found) > 1:
        raise ValueError(CLASS + ': multiple constructors require explicit merge')
    body = ('\n    // Original constructor 0x1418b5f70 calls SetIsReplicated(true).\n'
            '    SetIsReplicatedByDefault(true);\n')
    if found:
        which, match = found[0]
        text = header if which == 'header' else cpp
        depth, end = 1, match.end()
        while depth and end < len(text):
            if text[end] == '{':
                depth += 1
            elif text[end] == '}':
                depth -= 1
            end += 1
        if depth:
            raise ValueError(CLASS + ': unclosed constructor')
        text = text[:end - 1] + body + text[end - 1:]
        return (text, cpp) if which == 'header' else (header, text)
    if re.search(r'\b' + CLASS + r'\s*\(', header):
        raise ValueError(CLASS + ': constructor declaration without known body')
    if 'public:' not in header:
        raise ValueError(CLASS + ': missing public section')
    header = header.replace('public:', 'public:\n    ' + CLASS + '(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());', 1)
    cpp += ('\n' + CLASS + '::' + CLASS + '(const FObjectInitializer& ObjectInitializer)\n'
            '    : Super(ObjectInitializer)\n{' + body + '}\n')
    return header, cpp
