"""Candidate: restore AFortPawn native capsule constructor writes."""
import re


def finalize(model, header, cpp):
    if model['name'] != 'AFortPawn':
        return header, cpp
    if model.get('parent') != 'ACharacter':
        raise ValueError('AFortPawn: unexpected parent')
    pattern = re.compile(r'\bAFortPawn::AFortPawn\s*\(\s*const\s+FObjectInitializer\s*&\s*ObjectInitializer\s*\)\s*:[^{};]*\{')
    matches = list(pattern.finditer(cpp))
    if len(matches) != 1:
        raise ValueError('AFortPawn: expected one native constructor')
    match = matches[0]
    depth, end = 1, match.end()
    while depth and end < len(cpp):
        if cpp[end] == '{': depth += 1
        elif cpp[end] == '}': depth -= 1
        end += 1
    if depth:
        raise ValueError('AFortPawn: unterminated constructor')
    if re.search(r'\b(?:GetCapsuleComponent|CapsuleComponent|InitCapsuleSize|SetCollisionProfileName)\b', cpp[match.end():end - 1]):
        raise ValueError('AFortPawn: competing capsule initialization')
    inc = '#include "Components/CapsuleComponent.h"'
    if cpp.count(inc) > 1:
        raise ValueError('AFortPawn: duplicate capsule include')
    body = '''    // Original AFortPawn constructor 0x1422fd7d0..0x1422fd84a.
    GetCapsuleComponent()->InitCapsuleSize(42.0f, 88.0f);
    GetCapsuleComponent()->SetCollisionProfileName(FName(TEXT("FortPawnCapsule")), true);
'''
    cpp = cpp[:end - 1] + body + cpp[end - 1:]
    if inc not in cpp:
        anchor = '#include "AFortPawn.h"'
        if cpp.count(anchor) != 1:
            raise ValueError('AFortPawn: missing unique include anchor')
        cpp = cpp.replace(anchor, anchor + '\n' + inc, 1)
    return header, cpp
