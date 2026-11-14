"""Unactivated reflected projection of UFortMovementSet native constructor."""
import hashlib
import re

CLASS = 'UFortMovementSet'
FIELDS = ('WalkSpeed', 'RunSpeed', 'SprintSpeed', 'FlySpeed', 'CrouchedRunSpeed',
          'CrouchedSprintSpeed', 'BackwardSpeedMultiplier', 'JumpHeight',
          'GravityZScale', 'VehicleGravityZScale', 'SpeedMultiplier')
ONES = ('BackwardSpeedMultiplier', 'GravityZScale', 'VehicleGravityZScale', 'SpeedMultiplier')
STRUCT_DECL_HASH = '28e2ea849d389a55c441e862bf65d812cf492518c6ac63f64179b15ee70d9a7c'


def validate_struct(model, declarations):
    if model.get('cpp_name') != 'FFortGameplayAttributeData':
        return
    digest = hashlib.sha256('\n'.join(declarations).encode()).hexdigest()
    if digest != STRUCT_DECL_HASH:
        raise ValueError(CLASS + ': requires pinned recovered attribute constructor and setters')


def finalize(model, header, cpp):
    if model.get('name') != CLASS:
        return header, cpp
    if model.get('parent') != 'UFortAttributeSet':
        raise ValueError(CLASS + ': unexpected parent')
    for i, field in enumerate(FIELDS):
        members = [m for m in model.get('members_decl_order', []) if m.get('name') == field]
        declaration = 'FFortGameplayAttributeData ' + field + ';'
        found = re.findall(r'^\s*[^\n;{}]*\b' + field + r'\b[^\n;{}]*;', header, re.M)
        if (len(members) != 1 or members[0].get('decl', '').strip() != declaration or
                members[0].get('offset') != 0x30 + 0x28*i or len(found) != 1 or found[0].strip() != declaration):
            raise ValueError(CLASS + ': unexpected field ' + field)
    decls = list(re.finditer(r'^\s*(?:explicit\s+)?' + CLASS + r'\s*\(', header, re.M))
    defs = list(re.finditer(r'\b' + CLASS + r'::' + CLASS + r'\s*\(', cpp))
    if len(decls) > 1 or len(defs) > 1 or bool(decls) != bool(defs):
        raise ValueError(CLASS + ': mismatched constructors')
    if not defs:
        if header.count('public:') != 1:
            raise ValueError(CLASS + ': expected one public section')
        header = header.replace('public:', 'public:\n\t' + CLASS + '(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());\n', 1)
        cpp += '\n%s::%s(const FObjectInitializer& ObjectInitializer)\n    : Super(ObjectInitializer)\n{\n}\n' % (CLASS, CLASS)
    pattern = re.compile(r'\b' + CLASS + r'::' + CLASS + r'\s*\(\s*const\s+FObjectInitializer\s*&\s*ObjectInitializer\s*\)\s*:[^{};]*\{')
    matches = list(pattern.finditer(cpp))
    if len(matches) != 1:
        raise ValueError(CLASS + ': unsupported constructor')
    match = matches[0]
    depth, end = 1, match.end()
    while depth and end < len(cpp):
        if cpp[end] == '{': depth += 1
        elif cpp[end] == '}': depth -= 1
        end += 1
    if depth or any(re.search(r'\b' + field + r'\b', cpp[match.start():end-1]) for field in FIELDS):
        raise ValueError(CLASS + ': conflicting constructor or unterminated body')
    body = '    // Reflected projection of original constructor 0x141b8bd50.\n'
    for field in ONES:
        body += '    %s.SetBaseValue(1.0f);\n    %s.SetCurrentValue(1.0f);\n' % (field, field)
    return header, cpp[:end-1] + body + cpp[end-1:]
