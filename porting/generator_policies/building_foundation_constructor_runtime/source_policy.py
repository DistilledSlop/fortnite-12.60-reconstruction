"""Project individually established ABuildingFoundation constructor fields.

Not a complete foundation constructor; see EVIDENCE.md.
"""
import re

CLASS = 'ABuildingFoundation'
OWN = ('TEnumAsByte<EBuildingFoundationType> FoundationType;',
       'float ProxyMeshMaxDrawDistanceMultiplier;', 'uint8 bConditionalFoundation : 1;',
       'uint8 bOverrideNavigationGraphCells : 1;', 'uint8 bHasExcludedZone : 1;',
       'uint8 bForceDitheringTransition : 1;', 'uint8 bStreamingDataBasedBounds : 1;',
       'uint8 bMergeHLODs : 1;', 'uint8 bExcludeFromHLODMerge : 1;',
       'uint8 bHLODMergeUseCustomTextureSize : 1;', 'FIntPoint HLODMergeCustomTextureSize;')
ASSIGNMENTS = (
    ('bRotateInPlaceEditor', 'false'), ('bAlwaysRelevant', 'true'),
    ('SnapOffset.X', '0.0f'), ('SnapOffset.Y', '0.0f'),
    ('BaseLocToPivotOffset.X', '0.0f'), ('BaseLocToPivotOffset.Y', '0.0f'),
    ('FoundationType', 'BFT_None'), ('bNoCollision', 'true'),
    ('bConditionalFoundation', 'false'), ('bOverrideNavigationGraphCells', 'true'),
    ('bHasExcludedZone', 'false'), ('bForceDitheringTransition', 'true'),
    ('bStreamingDataBasedBounds', 'false'), ('NetPriority', '10.0f'),
    ('DynamicBuildingPlacementType', 'EDynamicBuildingPlacementType::DestroyAnythingThatCollides'),
    ('AbilitySystemComponentCreationPolicy', 'EAbilitySystemComponentCreationPolicy::Lazy'),
    ('bMergeHLODs', 'true'), ('bExcludeFromHLODMerge', 'false'),
    ('bHLODMergeUseCustomTextureSize', 'false'),
    ('ProxyMeshMaxDrawDistanceMultiplier', '1.0f'),
    ('HLODMergeCustomTextureSize', 'FIntPoint(0, 0)'),
    ('bReplayRewindable', 'true'), ('bCanBeSavedInCreativeVolume', 'false'),
)


def finalize(model, header, cpp):
    if model['name'] == 'AFortWorldSettings':
        # Original 141fa4edd: MOV dword ptr [RBX+3cc],ffffffff.
        declaration = 'int32 SplineHLODIndex;'
        members = [m for m in model.get('members_decl_order', []) if m.get('name') == 'SplineHLODIndex']
        if (model.get('parent') != 'AWorldSettings' or len(members) != 1
                or members[0].get('decl', '').strip() != declaration
                or header.count(declaration) != 1):
            raise ValueError('AFortWorldSettings: SplineHLODIndex declaration changed')
        # A native CDO initializer; authored serialized values still override it.
        return header.replace(declaration, 'int32 SplineHLODIndex = -1;', 1), cpp
    if model['name'] != CLASS:
        return header, cpp
    if model.get('parent') != 'ABuildingSMActor':
        raise ValueError(CLASS + ': unexpected parent')
    for declaration in OWN:
        field = re.search(r'\b(\w+)\s*(?::\s*1)?\s*;$', declaration).group(1)
        members = [m for m in model.get('members_decl_order', []) if m.get('name') == field]
        candidates = re.findall(r'^\s*[^\n;{}]*\b' + field + r'\b[^\n;{}]*;', header, re.M)
        if len(members) != 1 or members[0].get('decl', '').strip() != declaration or len(candidates) != 1 or candidates[0].strip() != declaration:
            raise ValueError(CLASS + ': unexpected declaration for ' + field)
    decls = list(re.finditer(r'^\s*(?:explicit\s+)?' + CLASS + r'\s*\(', header, re.M))
    defs = list(re.finditer(r'\b' + CLASS + r'::' + CLASS + r'\s*\(', cpp))
    if len(decls) > 1 or len(defs) > 1 or bool(decls) != bool(defs):
        raise ValueError(CLASS + ': expected at most one matching constructor')
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
    if depth:
        raise ValueError(CLASS + ': unterminated constructor')
    constructor = cpp[match.start():end - 1]
    missing = []
    for field, value in ASSIGNMENTS:
        exact = re.compile(r'\b' + re.escape(field) + r'\s*=\s*' + re.escape(value) + r'\s*;')
        matches = list(exact.finditer(constructor))
        if len(matches) > 1:
            raise ValueError(CLASS + ': duplicate constructor field assignment: ' + field)
        if matches:
            constructor = exact.sub('', constructor)
        else:
            missing.append((field, value))
    for field, _ in ASSIGNMENTS:
        if re.search(r'\b' + re.escape(field.split('.')[0]) + r'\b', constructor):
            raise ValueError(CLASS + ': competing constructor field use: ' + field)
    body = '''    // Selected original constructor fields; see policy EVIDENCE.md for scope.
    static_assert(TIsSame<decltype(NetPriority), float>::Value, "NetPriority type changed");
    static_assert(TIsSame<decltype(SnapOffset), FVector>::Value, "SnapOffset type changed");
    static_assert(TIsSame<decltype(BaseLocToPivotOffset), FVector>::Value, "BaseLocToPivotOffset type changed");
    static_assert(TIsSame<decltype(AbilitySystemComponentCreationPolicy), EAbilitySystemComponentCreationPolicy>::Value, "creation policy type changed");
    static_assert(TIsSame<decltype(DynamicBuildingPlacementType), TEnumAsByte<EDynamicBuildingPlacementType::Type>>::Value, "placement type changed");
    static_assert(BFT_None == 3 && int32(EDynamicBuildingPlacementType::DestroyAnythingThatCollides) == 2 && int32(EAbilitySystemComponentCreationPolicy::Lazy) == 1, "original enum values changed");
'''
    body += ''.join('    %s = %s;\n' % item for item in missing)
    return header, cpp[:end - 1] + body + cpp[end - 1:]
