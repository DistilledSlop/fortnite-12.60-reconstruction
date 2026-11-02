"""Controlled real text-transform UI/layout diagnostic transcript."""
import re
EVENTS = ('begin', 'reflection_enum_property', 'enum_storage', 'metadata', 'cdo_default', 'true_no_slate', 'repeat_true', 'processevent_false_after_lower', 'repeat_false', 'live_widget', 'bound_policy_volatile', 'original_font', 'layout_none_multiline', 'layout_lower', 'layout_upper', 'layout_restore_cache', 'widget_geometry_transition', 'bound_source_change', 'unicode_same_length', 'unicode_length_refusal', 'simple_reference_measure_bypass', 'complete')
OPTIONAL = 'unicode_length_refusal'
MARKER = 'TEXTTRANSFORM'

def verify(text):
    events=[]; unavailable=None
    for line in text.splitlines():
        if MARKER not in line: continue
        row=line[line.index(MARKER):]
        m=re.fullmatch(r'TEXTTRANSFORM v=1 event=unicode_length_refusal_unavailable source_len=(\d+) transformed_len=(\d+)',row)
        if m:
            if unavailable is not None or m[1]!=m[2] or len(events)!=EVENTS.index(OPTIONAL): raise ValueError('invalid unavailable case')
            unavailable={'source_length':int(m[1]),'transformed_length':int(m[2])};continue
        m=re.fullmatch(r'TEXTTRANSFORM v=1 seq=(\d+) event=(\w+) ok=1',row)
        if not m or int(m[1])!=len(events)+1: raise ValueError('failed, malformed or out-of-order row')
        events.append(m[2])
    expected=[e for e in EVENTS if e!=OPTIONAL or unavailable is None]
    if events!=expected: raise ValueError('incomplete or mismatched event sequence')
    assets=[line.split('TEXT_TRANSFORM_ASSET style=',1)[1] for line in text.splitlines() if 'TEXT_TRANSFORM_ASSET style=' in line]
    if assets!=['/Game/UI/Foundation/Buttons/TextStyles/TextStyle-Button-Primary-M.TextStyle-Button-Primary-M_C']: raise ValueError('original font asset not verified')
    return {'passed':True,'rows':len(events),'unicode_length_refusal_exercised':unavailable is None,'unicode_length_unavailable':unavailable,'serialized_style':assets[0],
            'scope':'Actual UMG/Slate policy, separate genuine text layout model, raw/bound preservation and geometry; not original pixels/all shaping or localization parity'}
