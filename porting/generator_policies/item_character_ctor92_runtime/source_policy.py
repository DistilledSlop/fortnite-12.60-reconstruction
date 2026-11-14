"""Narrow projections of original item and cosmetic-part constructors."""
import re

FIELDS = {
    'UFortPersistableItem': ('UFortItem', [('Quantity', 'int32 Quantity;', 176), ('bIsLocalOnlyItem', 'bool bIsLocalOnlyItem;', None)]),
    'UFortAccountItem': ('UFortPersistableItem', [('level', 'int32 level;', 184)]),
    'UCustomCharacterPart': ('UPrimaryDataAsset', [('GenderPermitted', 'TEnumAsByte<EFortCustomGender::Type> GenderPermitted;', 48), ('BodyTypesPermitted', 'TEnumAsByte<EFortCustomBodyType::Type> BodyTypesPermitted;', 49)]),
}
BODIES = {
    'UFortAccountItem': ('0x141ffb050', ('Quantity = 1;', 'level = 1;', 'bIsLocalOnlyItem = true;')),
    'UCustomCharacterPart': ('0x141d6b6c0', ('GenderPermitted = static_cast<EFortCustomGender::Type>(1);', 'BodyTypesPermitted = static_cast<EFortCustomBodyType::Type>(7);')),
}

def finalize(model, header, cpp):
    cls = model.get('name')
    if cls not in FIELDS:
        return header, cpp
    parent, fields = FIELDS[cls]
    if model.get('parent') != parent:
        raise ValueError(cls + ': unexpected parent')
    for field, declaration, offset in fields:
        members = [m for m in model.get('members_decl_order', []) if m.get('name') == field]
        found = re.findall(r'^\s*[^\n;{}]*\b' + field + r'\b[^\n;{}]*;', header, re.M)
        if (len(members) != 1 or members[0].get('decl', '').strip() != declaration or
            members[0].get('offset') != offset or len(found) != 1 or found[0].strip() != declaration):
            raise ValueError(cls + ': unexpected field ' + field)
    if cls not in BODIES:
        return header, cpp
    decls = list(re.finditer(r'^\s*(?:explicit\s+)?' + cls + r'\s*\(', header, re.M))
    defs = list(re.finditer(r'\b' + cls + r'::' + cls + r'\s*\(', cpp))
    if len(decls) > 1 or len(defs) > 1 or bool(decls) != bool(defs):
        raise ValueError(cls + ': mismatched constructors')
    if not defs:
        if header.count('public:') != 1:
            raise ValueError(cls + ': expected one public section')
        header = header.replace('public:', 'public:\n\t' + cls + '(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());\n', 1)
        cpp += '\n%s::%s(const FObjectInitializer& ObjectInitializer)\n    : Super(ObjectInitializer)\n{\n}\n' % (cls, cls)
    pattern = re.compile(r'\b' + cls + r'::' + cls + r'\s*\(\s*const\s+FObjectInitializer\s*&\s*ObjectInitializer\s*\)\s*:[^{};]*\{')
    matches = list(pattern.finditer(cpp))
    if len(matches) != 1:
        raise ValueError(cls + ': unsupported constructor')
    match = matches[0]
    depth, end = 1, match.end()
    while depth and end < len(cpp):
        if cpp[end] == '{': depth += 1
        elif cpp[end] == '}': depth -= 1
        end += 1
    names = ('Quantity', 'level', 'bIsLocalOnlyItem') if cls == 'UFortAccountItem' else ('GenderPermitted', 'BodyTypesPermitted')
    if depth or any(re.search(r'\b' + field + r'\b', cpp[match.start():end-1]) for field in names):
        raise ValueError(cls + ': conflicting constructor or unterminated body')
    address, statements = BODIES[cls]
    body = '    // Narrow reflected projection of original constructor ' + address + '.\n'
    body += ''.join('    ' + statement + '\n' for statement in statements)
    return header, cpp[:end-1] + body + cpp[end-1:]
