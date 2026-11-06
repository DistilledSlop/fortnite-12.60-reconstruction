"""Restore native-only FFortWindResponder storage proven by registration/copy captures.

Generator integration must call fields_after for each reflected member and
postamble after the closing struct declaration. No reflected property is changed.
"""

NAME = 'FFortWindResponder'
_EXPECTED_MEMBERS = [
    (0x00, 'ABuildingSMActor* WindUpdatingBuildingSMActor;'),
    (0x08, 'UCurveLinearColor* WindSpeedCurve;'),
    (0x10, 'UCurveLinearColor* WindPannerSpeedCurve;'),
    (0x18, 'USoundBase* WindAudio;'),
    (0x20, 'TArray<UMaterialInterface*> MildWindMaterialInstances;'),
    (0x30, 'TArray<UMaterialInterface*> IntenseWindMaterialInstances;'),
    (0x40, 'TArray<FFortWindMaterialData> MaterialsData;'),
    (0x50, 'int32 MaterialParameterPairIndices;'),
    (0x58, 'TArray<FFortWindResponderMaterialVariablePairData> PairedVariablesData;'),
    (0x84, 'float WindSpeed;'),
    (None, 'bool bHasSetupAnimatingMaterials;'),
]


def _validate(model):
    if model['cpp_name'] != NAME:
        return False
    actual = [(m['offset'], m['decl']) for m in model['members']]
    if (model['package'] != '/Script/FortniteGame' or model['size'] != 0x90
            or model['align'] != 8 or model.get('parent') or actual != _EXPECTED_MEMBERS):
        raise ValueError('FFortWindResponder reflected identity or layout changed')
    return True


def fields_after(model, member):
    if not _validate(model):
        return []
    if member['offset'] == 0x58:
        return [
            '\t// Actor bounds copied by native registration and responder copy.',
            '\tuint8 NativeBounds_0x68[0x1c];',
            '',
        ]
    if member['offset'] == 0x84:
        return [
            '\t// Effective distance selected during native registration.',
            '\tfloat NativeEffectiveDistance_0x88;',
            '',
        ]
    return []


def postamble(model):
    if not _validate(model):
        return []
    return [
        'static_assert(sizeof(FFortWindResponder) == 0x90, "FFortWindResponder native stride changed");',
        'static_assert(alignof(FFortWindResponder) == 8, "FFortWindResponder native alignment changed");',
        'static_assert(STRUCT_OFFSET(FFortWindResponder, MaterialParameterPairIndices) == 0x50, "FFortWindResponder pair index offset changed");',
        'static_assert(STRUCT_OFFSET(FFortWindResponder, PairedVariablesData) == 0x58, "FFortWindResponder paired data offset changed");',
        'static_assert(STRUCT_OFFSET(FFortWindResponder, NativeBounds_0x68) == 0x68, "FFortWindResponder bounds offset changed");',
        'static_assert(STRUCT_OFFSET(FFortWindResponder, WindSpeed) == 0x84, "FFortWindResponder speed offset changed");',
        'static_assert(STRUCT_OFFSET(FFortWindResponder, NativeEffectiveDistance_0x88) == 0x88, "FFortWindResponder distance offset changed");',
        'static_assert(STRUCT_OFFSET(FFortWindResponder, bHasSetupAnimatingMaterials) == 0x8c, "FFortWindResponder setup offset changed");',
        '',
    ]
