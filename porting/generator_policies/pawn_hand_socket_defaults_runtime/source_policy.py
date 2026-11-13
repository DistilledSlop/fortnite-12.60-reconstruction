"""Restore AFortPawn's original native hand socket constructor defaults."""
import re

CLASS = 'AFortPawn'
FIELDS = ('WeaponHandSocketName', 'LeftHandWeaponHandSocketName')


def finalize(model, header, cpp):
    if model['name'] != CLASS:
        return header, cpp
    if model.get('parent') != 'ACharacter':
        raise ValueError(CLASS + ': hand socket default policy parent changed')
    for name in FIELDS:
        if len(re.findall(r'\bFName\s+' + name + r'\s*;', header)) != 1:
            raise ValueError(CLASS + ': missing or competing FName field: ' + name)
    pattern = re.compile(r'\bAFortPawn::AFortPawn\s*\(\s*const\s+FObjectInitializer\s*&\s*ObjectInitializer\s*\)\s*:[^{};]*\{')
    matches = list(pattern.finditer(cpp))
    if len(matches) != 1:
        raise ValueError(CLASS + ': expected one existing native constructor')
    match = matches[0]
    depth, end = 1, match.end()
    while depth and end < len(cpp):
        if cpp[end] == '{':
            depth += 1
        elif cpp[end] == '}':
            depth -= 1
        end += 1
    if depth:
        raise ValueError(CLASS + ': unterminated native constructor')
    body = cpp[match.end():end - 1]
    if any(re.search(r'\b' + name + r'\b', body) for name in FIELDS):
        raise ValueError(CLASS + ': hand socket defaults already assigned; merge explicitly')
    defaults = ('    // Original constructor 0x1422fce30: stores at 0x1422fd75a and 0x1422fcfd4.\n'
                '    WeaponHandSocketName = FName(TEXT("RightHand"));\n'
                '    LeftHandWeaponHandSocketName = NAME_None;\n')
    return header, cpp[:end - 1] + defaults + cpp[end - 1:]
