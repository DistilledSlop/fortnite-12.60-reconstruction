"""Class-owned visibility defaults recovered from constructor 0x142623b40."""


def declarations(model):
    if model['name'] != 'UFortVisibilityComponent':
        return []
    if model.get('block_va') != '0x146564e30':
        raise ValueError('visibility component reflected class identity changed')
    return [
        'public:',
        '    UFortVisibilityComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())',
        '        : Super(ObjectInitializer)',
        '    {',
        '        // Original class flags at +0xb0 are 0x25 on zero-initialized storage.',
        '        bRegisterWithVisibilityManager = true;',
        '        bObserver = false;',
        '        bDistanceCheck2D = true;',
        '        bCheckFacing = false;',
        '        bCheckLOS = false;',
        '        bRevealWithMap = true;',
        '        VisibilityBehavior = EFortVisibilityBehavior::Proximity;',
        '        Radius = 1000.0f;',
        '    }',
    ]
