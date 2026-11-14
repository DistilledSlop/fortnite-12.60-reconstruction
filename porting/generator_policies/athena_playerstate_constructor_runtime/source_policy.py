"""Project proven reflected writes from the original Athena player-state constructor."""
import re

CLASS = 'AFortPlayerStateAthena'
OWN = (
    'int32 PersonalLobbyAction;', 'int32 TeamScore;', 'int32 SeasonLevelUIDisplay;',
    'int32 SecondsAlive;', 'int32 TimeOfPawnCreation;', 'bool bResurrectionChipDisabled;',
    'bool bResurrectingNow;', 'int32 RebootCounter;', 'bool bHoldsRebootVanLock;',
    'EMatchAbandonState MatchAbandonState;', 'uint8 bIsAnAthenaGameParticipant : 1;', 'bool bPreserveSquad;',
)
FIELDS = tuple(re.search(r'\b(\w+)\s*(?::\s*1)?\s*;$', d).group(1) for d in OWN) + (
    'bSkipReplicatedStats', 'NetUpdateFrequency', 'SetShouldUpdateReplicatedPing', 'bShouldUpdateReplicatedPing',
)


def finalize(model, header, cpp):
    if model['name'] != CLASS:
        return header, cpp
    if model.get('parent') != 'AFortPlayerStateZone':
        raise ValueError(CLASS + ': unexpected parent')
    for declaration in OWN:
        field = re.search(r'\b(\w+)\s*(?::\s*1)?\s*;$', declaration).group(1)
        members = [m for m in model.get('members_decl_order', []) if m.get('name') == field]
        found = re.findall(r'^\s*[^\n;{}]*\b' + field + r'\b[^\n;{}]*;', header, re.M)
        if len(members) != 1 or members[0].get('decl', '').strip() != declaration or len(found) != 1 or found[0].strip() != declaration:
            raise ValueError(CLASS + ': unexpected field declaration: ' + field)
    decls = list(re.finditer(r'^\s*(?:explicit\s+)?' + CLASS + r'\s*\(', header, re.M))
    defs = list(re.finditer(r'\b' + CLASS + r'::' + CLASS + r'\s*\(', cpp))
    if len(decls) > 1 or len(defs) > 1 or bool(decls) != bool(defs):
        raise ValueError(CLASS + ': expected at most one matching constructor')
    if not defs:
        if 'public:' not in header:
            raise ValueError(CLASS + ': expected a public section')
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
    if any(re.search(r'\b' + field + r'\b', cpp[match.start():end - 1]) for field in FIELDS):
        raise ValueError(CLASS + ': competing field use in constructor')
    body = '''    // Reflected-field projection of original constructor 0x141a2e060; see policy evidence.
    static_assert(TIsSame<decltype(bSkipReplicatedStats), bool>::Value, "player-state field type changed");
    static_assert(TIsSame<decltype(NetUpdateFrequency), float>::Value, "actor field type changed");
    SeasonLevelUIDisplay = 0;
    bResurrectionChipDisabled = false;
    bResurrectingNow = false;
    RebootCounter = 0;
    bHoldsRebootVanLock = false;
    bPreserveSquad = false;
    SecondsAlive = 0;
    TimeOfPawnCreation = 0;
    SetShouldUpdateReplicatedPing(false);
    bSkipReplicatedStats = true;
    NetUpdateFrequency = 1.0f;
    bIsAnAthenaGameParticipant = true;
    PersonalLobbyAction = 0;
    MatchAbandonState = static_cast<EMatchAbandonState>(0);
    TeamScore = -1;
'''
    return header, cpp[:end - 1] + body + cpp[end - 1:]
