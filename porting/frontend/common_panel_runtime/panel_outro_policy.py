"""Bounded original Panel outro and held cleanup; interface routing is not ported."""
import json
from pathlib import Path
HERE=Path(__file__).resolve().parent
CONTRACT=json.loads((HERE/'panel-outro-original-policy.json').read_text())
EXPECTED={n:{k:v for k,v in f.items() if k!='original_native_exec'} for n,f in CONTRACT.items()}
TYPE_CONTRACT=json.loads((HERE/'panel-outro-type-policy.json').read_text())
HELD_PROPERTY={'index':5,'name':'CurrentlyHeldActionInputHandler','kind':'Interface','kind_raw':'0x13','property_flags':'0x44000000000000','replicated':False,'rep_notify':None,'array_dim':1,'object_flags':'0x45','offset':168,'type_refs':{'interface_class':'0x140806da0'}}
NATIVE_NAMES={'OnBeginOutro_Implementation','NativeDeactivateRequest','NativeHandleDeactivated','NativeOutroComplete','NativeClearHeldAction','NativeHeldActionDuration','NativeHeldAction','NativeActionRecords','NativePreDeactivation','NativePostDeactivation'}

def apply(model,block,owner_fns,native_execs=None):
    if model['name']=='UCommonInputManager':
        if [p for p in block.get('properties',[]) if p.get('name')=='CurrentlyHeldActionInputHandler']!=[HELD_PROPERTY]:
            raise ValueError('original held manager interface property mismatch')
        emitted=[p for p in model.get('members_decl_order',[]) if p.get('name')=='CurrentlyHeldActionInputHandler']
        if len(emitted)!=1 or emitted[0].get('decl')!='TScriptInterface<ICommonActionHandlerInterface> CurrentlyHeldActionInputHandler;':
            raise ValueError('held manager interface field unavailable (requires structural interface alias resolver)')
        return model
    if model['name']!='UCommonActivatablePanel':return model
    if model.get('package')!='/Script/CommonUI' or model.get('parent')!='UCommonActivatableWidget' or block.get('block_va')!='0x145a00360' or '0x140807980' not in owner_fns:
        raise ValueError('original Panel outro owner mismatch')
    if not model.get('panel_intro_activation_runtime'):raise ValueError('Panel outro requires original intro activation runtime')
    if model.get('panel_outro_runtime') or any(item.get('name') in NATIVE_NAMES|set(EXPECTED) for group in ('functions','members_table_order','members_decl_order','fields','helpers','static_helpers') for item in model.get(group,[])):
        raise ValueError('Panel outro declaration collision')
    for n,expected in EXPECTED.items():
        if [f for f in block.get('functions',[]) if f.get('name')==n]!=[expected]:
            raise ValueError('original Panel outro signature mismatch: '+n)
        if not isinstance(native_execs,dict) or native_execs.get(n)!=CONTRACT[n]['original_native_exec']:
            raise ValueError('original Panel outro native exec mismatch: '+n)
    for n in EXPECTED:
        event=n=='OnBeginOutro'
        model['functions'].append({'name':n,'rpc':False,'service':None,'ret':'void','args':[],
            'const':False,'notify':False,'splice':False,'blueprint_event':event,'access':'protected',
            'runtime_implementation':not event,'specifiers':['BlueprintNativeEvent'] if event else ['BlueprintCallable'],'used':{}})
    model['panel_outro_runtime']=True
    # Full canonical record owns a reflected handler data value and both delegate types.
    # Use the standard generator type closure, including the handler's enum/row dependencies.
    used=model.setdefault('used',{})
    for fn,record in TYPE_CONTRACT.items():used.setdefault(fn,set()).add('Struct' if record['name']=='CommonInputActionHandlerData' else 'Delegate')
    return model

def validate_types(gate):
    for fn,expected in TYPE_CONTRACT.items():
        info=gate.info(fn)
        if info is None or info.rec!=expected or info.package!='/Script/CommonUI':
            raise ValueError('original Panel action record type mismatch: '+fn)
        reason=gate.reason(info)
        if reason is not None:raise ValueError('Panel action record type unavailable: '+fn+': '+reason)
        if info.kind=='struct':
            model=gate.models[fn]
            if model.get('panel_action_native_popup'):raise ValueError('Panel action native popup already emitted')
            model['panel_action_native_popup']=True
            model['forward_decls']=sorted(set(model['forward_decls'])|{'UCommonPopupMenu'})

def struct_declarations(model):
    if model.get('panel_action_native_popup'):
        return ['\t// Original unreflected weak popup from action registration and held completion.', '\tTWeakObjectPtr<UCommonPopupMenu> NativePopupMenu;', '']
    return []

def header_prelude(model):
    result=dict(model)
    if model.get('panel_outro_runtime'):result['includes']=sorted(set(result.get('includes',[]))|{'CommonPanelActionNative.h'})
    return result

def declarations(model):return (HERE/'panel-outro-declarations.inc').read_text() if model.get('panel_outro_runtime') else ''
def definitions(model):return (HERE/'panel-outro-definitions.inc').read_text() if model.get('panel_outro_runtime') else ''
def emit_headers(source_root,generated_names):
    if 'UCommonActivatablePanel' in generated_names:
        target=Path(source_root)/'CommonUI/Generated';target.mkdir(parents=True,exist_ok=True)
        (target/'CommonPanelActionNative.h').write_text((HERE/'CommonPanelActionNative.h').read_text())
