"""Guarded source policy. Only the shared fields needed by recovered inventory queries.
Call finalize after existing constructor policies; retain their bodies and reject conflicting stores.
These are field-default projections, not claims to recover entire original constructors.
"""
import re

DEFAULTS = {
    'UFortItemDefinition': ('bAllowMultipleStacks', 'uint8 bAllowMultipleStacks : 1;'),
    'UFortWorldItemDefinition': ('bAllowMultipleStacks', None),
    'UFortAmmoItemDefinition': ('bIsConsumed', 'bool bIsConsumed;'),
}

def finalize(model, header, cpp):
    cls = model.get('name')
    if cls not in DEFAULTS:
        return header, cpp
    parents = {'UFortItemDefinition': 'UMcpItemDefinitionBase',
               'UFortWorldItemDefinition': 'UFortItemDefinition',
               'UFortAmmoItemDefinition': 'UFortWorldItemDefinition'}
    if model.get('package') != '/Script/FortniteGame' or model.get('parent') != parents[cls]:
        raise ValueError(cls + ': unexpected class identity or ancestry')
    field, declaration = DEFAULTS[cls]
    if declaration and len(re.findall(re.escape(declaration), header)) != 1:
        raise ValueError(cls + ': shared field declaration changed')
    decls = list(re.finditer(r'^\s*(?:explicit\s+)?' + cls + r'\s*\(', header, re.M))
    defs = list(re.finditer(r'\b' + cls + r'::' + cls + r'\s*\(', cpp))
    if len(decls) > 1 or len(defs) > 1 or bool(decls) != bool(defs):
        raise ValueError(cls + ': constructor ownership changed')
    if not defs:
        if header.count('public:') != 1:
            raise ValueError(cls + ': expected one public section')
        header = header.replace('public:', 'public:\n\t' + cls + '(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());\n', 1)
        cpp += '\n%s::%s(const FObjectInitializer& ObjectInitializer)\n    : Super(ObjectInitializer)\n{\n}\n' % (cls, cls)
    pattern = r'\b' + cls + r'::' + cls + r'\s*\(\s*const\s+FObjectInitializer\s*&\s*ObjectInitializer\s*\)\s*:\s*Super\(ObjectInitializer\)\s*\{'
    matches = list(re.finditer(pattern, cpp))
    if len(matches) != 1:
        raise ValueError(cls + ': unsupported constructor')
    begin = matches[0].end()
    depth, end = 1, begin
    while depth and end < len(cpp):
        depth += (cpp[end] == '{') - (cpp[end] == '}')
        end += 1
    if depth or re.search(r'\b' + field + r'\b', cpp[begin:end - 1]):
        raise ValueError(cls + ': conflicting initialization or unbalanced constructor')
    cpp = cpp[:end - 1] + '    ' + field + ' = true;\n' + cpp[end - 1:]
    return header, cpp
