"""Guarded projection of base movement constructor air-control defaults."""
import re

CLASS = 'UFortMovementComp_Character'
FIELDS = (
    ('SkydivingControlParamsPassive', 0x718),
    ('SkydivingControlParamsActive', 0x818),
    ('ParachuteControlParamsSkydive', 0x918),
    ('ParachuteControlParamsJump', 0xa18),
    ('ParachuteControlParamsGliderRedeploy', 0xb18),
    ('BalloonControlParams', 0xc18),
    ('FloatingControlParams', 0xd18),
    ('VortexSkydivingControlParamsPassive', 0xe18),
    ('VortexSkydivingControlParamsActive', 0xf18),
    ('VortexSkydivingControlParamsActiveUpward', 0x1018),
    ('VortexParachuteControlParams', 0x1118),
    ('SkyTubeSkydivingControlParamsPassive', 0x1218),
    ('SkyTubeSkydivingControlParamsActive', 0x1318),
    ('SkyTubeSkydivingControlParamsActiveUpward', 0x1418),
)
MEMBERS = ('MaxAcceleration', 'LateralFriction', 'MaxLateralSpeed',
           'TerminalVelocity', 'UpwardTerminalVelocity', 'GravityScalar',
           'CustomGravityCeiling', 'CustomGravityCeilingWidth')
# Only explicit original helper calls; 36 other slots retain struct-ctor zero.
WRITES = (
    ('SkydivingControlParamsPassive', 'MaxAcceleration', '4096.0f', '0x1420d4ddb'),
    ('SkydivingControlParamsPassive', 'LateralFriction', '0.8f', '0x1420d4e03'),
    ('SkydivingControlParamsPassive', 'MaxLateralSpeed', '1200.0f', '0x1420d4e1a'),
    ('SkydivingControlParamsPassive', 'TerminalVelocity', '1200.0f', '0x1420d4e29'),
    ('SkydivingControlParamsPassive', 'GravityScalar', '1.0f', '0x1420d4dea'),
    ('SkydivingControlParamsActive', 'MaxAcceleration', '4096.0f', '0x1420d4e39'),
    ('SkydivingControlParamsActive', 'LateralFriction', '1.5f', '0x1420d4e5f'),
    ('SkydivingControlParamsActive', 'MaxLateralSpeed', '2400.0f', '0x1420d4e76'),
    ('SkydivingControlParamsActive', 'TerminalVelocity', '6000.0f', '0x1420d4e8a'),
    ('SkydivingControlParamsActive', 'GravityScalar', '1.0f', '0x1420d4e48'),
    ('ParachuteControlParamsSkydive', 'MaxAcceleration', '2048.0f', '0x1420d4cab'),
    ('ParachuteControlParamsSkydive', 'LateralFriction', '1.2f', '0x1420d4ce3'),
    ('ParachuteControlParamsSkydive', 'MaxLateralSpeed', '2000.0f', '0x1420d4cfc'),
    ('ParachuteControlParamsSkydive', 'TerminalVelocity', '500.0f', '0x1420d4d15'),
    ('ParachuteControlParamsSkydive', 'GravityScalar', '1.0f', '0x1420d4cca'),
    ('ParachuteControlParamsJump', 'MaxAcceleration', '2048.0f', '0x1420d4d25'),
    ('ParachuteControlParamsJump', 'LateralFriction', '1.3f', '0x1420d4d4b'),
    ('ParachuteControlParamsJump', 'MaxLateralSpeed', '800.0f', '0x1420d4d64'),
    ('ParachuteControlParamsJump', 'TerminalVelocity', '500.0f', '0x1420d4d74'),
    ('ParachuteControlParamsJump', 'GravityScalar', '1.0f', '0x1420d4d34'),
    ('ParachuteControlParamsGliderRedeploy', 'MaxAcceleration', '2048.0f', '0x1420d4d84'),
    ('ParachuteControlParamsGliderRedeploy', 'LateralFriction', '1.3f', '0x1420d4da2'),
    ('ParachuteControlParamsGliderRedeploy', 'MaxLateralSpeed', '800.0f', '0x1420d4db2'),
    ('ParachuteControlParamsGliderRedeploy', 'TerminalVelocity', '500.0f', '0x1420d4dc2'),
    ('ParachuteControlParamsGliderRedeploy', 'GravityScalar', '1.0f', '0x1420d4d93'),
    ('BalloonControlParams', 'UpwardTerminalVelocity', '2000.0f', '0x1420d4ec7'),
    ('BalloonControlParams', 'CustomGravityCeiling', '16000.0f', '0x1420d4e9e'),
    ('BalloonControlParams', 'CustomGravityCeilingWidth', '1000.0f', '0x1420d4eb7'),
    ('FloatingControlParams', 'MaxAcceleration', '4096.0f', '0x1420d4ed7'),
    ('FloatingControlParams', 'LateralFriction', '0.2f', '0x1420d4efa'),
    ('FloatingControlParams', 'MaxLateralSpeed', '400.0f', '0x1420d4f13'),
    ('FloatingControlParams', 'TerminalVelocity', '800.0f', '0x1420d4f23'),
    ('FloatingControlParams', 'UpwardTerminalVelocity', '800.0f', '0x1420d4f33'),
    ('FloatingControlParams', 'GravityScalar', '0.0f', '0x1420d4ee6'),
    ('VortexSkydivingControlParamsPassive', 'MaxAcceleration', '4096.0f', '0x1420d4f43'),
    ('VortexSkydivingControlParamsPassive', 'LateralFriction', '0.8f', '0x1420d4f67'),
    ('VortexSkydivingControlParamsPassive', 'MaxLateralSpeed', '1200.0f', '0x1420d4f7b'),
    ('VortexSkydivingControlParamsPassive', 'TerminalVelocity', '800.0f', '0x1420d4f8b'),
    ('VortexSkydivingControlParamsPassive', 'UpwardTerminalVelocity', '1500.0f', '0x1420d4fa4'),
    ('VortexSkydivingControlParamsPassive', 'GravityScalar', '-0.4f', '0x1420d4f57'),
    ('VortexSkydivingControlParamsActive', 'MaxAcceleration', '4096.0f', '0x1420d4fb4'),
    ('VortexSkydivingControlParamsActive', 'LateralFriction', '1.5f', '0x1420d4fd7'),
    ('VortexSkydivingControlParamsActive', 'MaxLateralSpeed', '2400.0f', '0x1420d4fe6'),
    ('VortexSkydivingControlParamsActive', 'TerminalVelocity', '1000.0f', '0x1420d4ff6'),
    ('VortexSkydivingControlParamsActive', 'UpwardTerminalVelocity', '1500.0f', '0x1420d5006'),
    ('VortexSkydivingControlParamsActive', 'GravityScalar', '-0.05f', '0x1420d4fc8'),
    ('VortexSkydivingControlParamsActiveUpward', 'MaxAcceleration', '4096.0f', '0x1420d5016'),
    ('VortexSkydivingControlParamsActiveUpward', 'LateralFriction', '1.5f', '0x1420d5039'),
    ('VortexSkydivingControlParamsActiveUpward', 'MaxLateralSpeed', '2400.0f', '0x1420d5048'),
    ('VortexSkydivingControlParamsActiveUpward', 'TerminalVelocity', '1000.0f', '0x1420d5058'),
    ('VortexSkydivingControlParamsActiveUpward', 'UpwardTerminalVelocity', '1500.0f', '0x1420d5068'),
    ('VortexSkydivingControlParamsActiveUpward', 'GravityScalar', '-0.25f', '0x1420d502a'),
    ('VortexParachuteControlParams', 'MaxAcceleration', '2048.0f', '0x1420d5078'),
    ('VortexParachuteControlParams', 'LateralFriction', '1.2f', '0x1420d509c'),
    ('VortexParachuteControlParams', 'MaxLateralSpeed', '2000.0f', '0x1420d50ac'),
    ('VortexParachuteControlParams', 'TerminalVelocity', '500.0f', '0x1420d50c5'),
    ('VortexParachuteControlParams', 'UpwardTerminalVelocity', '2000.0f', '0x1420d50d5'),
    ('VortexParachuteControlParams', 'GravityScalar', '-1.5f', '0x1420d508c'),
    ('SkyTubeSkydivingControlParamsPassive', 'MaxAcceleration', '4096.0f', '0x1420d50e5'),
    ('SkyTubeSkydivingControlParamsPassive', 'LateralFriction', '0.8f', '0x1420d5108'),
    ('SkyTubeSkydivingControlParamsPassive', 'MaxLateralSpeed', '1500.0f', '0x1420d5118'),
    ('SkyTubeSkydivingControlParamsPassive', 'TerminalVelocity', '250.0f', '0x1420d5131'),
    ('SkyTubeSkydivingControlParamsPassive', 'UpwardTerminalVelocity', '250.0f', '0x1420d5141'),
    ('SkyTubeSkydivingControlParamsPassive', 'GravityScalar', '0.0f', '0x1420d50f4'),
    ('SkyTubeSkydivingControlParamsActive', 'MaxAcceleration', '4096.0f', '0x1420d5151'),
    ('SkyTubeSkydivingControlParamsActive', 'LateralFriction', '0.6f', '0x1420d517c'),
    ('SkyTubeSkydivingControlParamsActive', 'MaxLateralSpeed', '1500.0f', '0x1420d518c'),
    ('SkyTubeSkydivingControlParamsActive', 'TerminalVelocity', '1200.0f', '0x1420d51a3'),
    ('SkyTubeSkydivingControlParamsActive', 'UpwardTerminalVelocity', '1200.0f', '0x1420d51b2'),
    ('SkyTubeSkydivingControlParamsActive', 'GravityScalar', '0.75f', '0x1420d5165'),
    ('SkyTubeSkydivingControlParamsActiveUpward', 'MaxAcceleration', '4096.0f', '0x1420d51c2'),
    ('SkyTubeSkydivingControlParamsActiveUpward', 'LateralFriction', '0.6f', '0x1420d51e5'),
    ('SkyTubeSkydivingControlParamsActiveUpward', 'MaxLateralSpeed', '1500.0f', '0x1420d51f5'),
    ('SkyTubeSkydivingControlParamsActiveUpward', 'TerminalVelocity', '1200.0f', '0x1420d5204'),
    ('SkyTubeSkydivingControlParamsActiveUpward', 'UpwardTerminalVelocity', '1200.0f', '0x1420d5213'),
    ('SkyTubeSkydivingControlParamsActiveUpward', 'GravityScalar', '-0.75f', '0x1420d51d6'),
)

def validate_struct(model, declarations):
    if model['cpp_name'] != 'FAirControlParams':
        return
    expected = [(i * 32, 'FScalableFloat %s;' % n) for i, n in enumerate(MEMBERS)]
    actual = [(m['offset'], m['decl']) for m in model['members']]
    initializers = ', '.join('%s(0.0f)' % n for n in MEMBERS)
    ctor = '\tFAirControlParams() : %s {}' % initializers
    if (model['package'] != '/Script/FortniteGame' or model['size'] != 256
            or actual != expected or declarations != [ctor, '']):
        raise ValueError('FAirControlParams layout or zero constructor changed')

def finalize(model, header, cpp):
    if model['name'] != CLASS:
        return header, cpp
    if model.get('parent') != 'UCharacterMovementComponent' or model.get('package') != '/Script/FortniteGame':
        raise ValueError(CLASS + ': unexpected identity or parent')
    for field, offset in FIELDS:
        declaration = 'FAirControlParams %s;' % field
        matches = [m for m in model.get('members_decl_order', []) if m.get('name') == field]
        found = re.findall(r'^\s*[^\n;{}]*\b' + field + r'\b[^\n;{}]*;', header, re.M)
        if (len(matches) != 1 or matches[0].get('offset') != offset
                or matches[0].get('decl', '').strip() != declaration
                or len(found) != 1 or found[0].strip() != declaration):
            raise ValueError(CLASS + ': unexpected field layout: ' + field)
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
    # Includes initializer list: any use could change constructor ordering or state.
    if any(re.search(r'\b' + field + r'\b', cpp[match.start():end - 1]) for field, _ in FIELDS):
        raise ValueError(CLASS + ': competing constructor field use')
    body = '\n    // Original base constructor 0x1420d4700 air-control projection.\n'
    for member in MEMBERS:
        body += '    static_assert(TIsSame<decltype(FAirControlParams::%s), FScalableFloat>::Value, "air-control member type changed");\n' % member
    for field, member, value, address in WRITES:
        body += '    %s.%s = FScalableFloat(%s); // %s\n' % (field, member, value, address)
    return header, cpp[:end - 1] + body + cpp[end - 1:]
