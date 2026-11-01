"""ONE original path: Panel OnBeginIntro -> EndIntro -> full activation handler."""
import json
from pathlib import Path
HERE=Path(__file__).resolve().parent
EXPECTED=json.loads((HERE/'panel-intro-original-policy.json').read_text())
INPUT_OWNERS=json.loads((HERE/'panel-input-owner-original-policy.json').read_text())

def apply(model,block,owner_fns):
    name=model['name']
    if name in ('UCommonInputSubsystem','UCommonGlobalInputHandler'):
        va='0x145a0e0d0' if name=='UCommonInputSubsystem' else '0x145a058e0'
        package='/Script/CommonInput' if name=='UCommonInputSubsystem' else '/Script/CommonUI'
        required_owner='0x14081e010' if name=='UCommonInputSubsystem' else '0x14080fa30'
        if required_owner not in owner_fns:raise ValueError('original input native owner mismatch: '+name)
        if model.get('package')!=package or block.get('block_va')!=va or block.get('properties')!=INPUT_OWNERS[va]['properties']:
            raise ValueError('original activation input owner/property mismatch: '+name)
        native_name='NativeInputMethodChanged' if name=='UCommonInputSubsystem' else 'NativePriorityFilter'
        if model.get('panel_activation_input_native') or any(item.get('name')==native_name for group in ('functions','members_decl_order','fields','helpers') for item in model.get(group,[])):
            raise ValueError('activation input native declaration collision: '+name)
        model['panel_activation_input_native']=name
        return model
    if name!='UCommonActivatablePanel':return model
    if model.get('package')!='/Script/CommonUI' or block.get('block_va')!='0x145a00360' or '0x140807980' not in owner_fns:
        raise ValueError('original Panel owner mismatch')
    native_names={'bNativeIsIntroed','NativeActivateRequest','NativeInputManager','NativeInputMethodHandle','NativePreActivation','NativePostActivation','NativeHandleActivated','NativeIntroComplete','NativeInputMethodChangedHook','NativeGetGlobalInputPriorityFilter','NativeInputMethodChanged'}
    if model.get('panel_intro_activation_runtime') or any(item.get('name') in native_names for group in ('functions','members_decl_order','fields','helpers') for item in model.get(group,[])):
        raise ValueError('Panel activation native declaration collision')
    for n,expected in EXPECTED.items():
        if [f for f in block.get('functions',[]) if f.get('name')==n]!=[expected]:
            raise ValueError('original Panel activation signature mismatch: '+n)
        if any(f['name']==n for f in model['functions']):raise ValueError('Panel activation method already emitted: '+n)
        event=n!='EndIntro'
        model['functions'].append({'name':n,'rpc':False,'service':None,'ret':'void',
            'args':['bool bUsingGamepad'] if n=='OnInputModeChanged' else [],
            'const':False,'notify':False,'splice':False,'blueprint_event':event,
            'access':'protected','runtime_implementation':not event,
            'specifiers':['BlueprintNativeEvent'] if n=='OnBeginIntro' else ['BlueprintImplementableEvent'] if event else ['BlueprintCallable'],
            'used':{}})
    model['panel_intro_activation_runtime']=True
    model['runtime_module_dependencies']=['CommonInput']
    model['forward_decls']=sorted(set(model.get('forward_decls',[]))|{'UCommonInputManager'})
    return model

def header_prelude(model):
    result=dict(model)
    headers=[]
    if model.get('panel_intro_activation_runtime'):headers=['CommonPanelActivationNative.h']
    if model.get('panel_activation_input_native')=='UCommonInputSubsystem':headers=['CommonInputMethodNative.h']
    if headers:result['includes']=sorted(set(result.get('includes',[]))|set(headers))
    return result

def declarations(model):
    if model.get('panel_intro_activation_runtime'):return (HERE/'panel-intro-declarations.inc').read_text()
    n=model.get('panel_activation_input_native')
    if n:return (HERE/('panel-input-subsystem-declarations.inc' if n=='UCommonInputSubsystem' else 'panel-global-handler-declarations.inc')).read_text()
    return ''

def definitions(model):
    return (HERE/'panel-intro-definitions.inc').read_text() if model.get('panel_intro_activation_runtime') else ''

def emit_headers(source_root,generated_names):
    if 'UCommonActivatablePanel' in generated_names:
        target=Path(source_root)/'CommonUI/Generated';target.mkdir(parents=True,exist_ok=True)
        (target/'CommonPanelActivationNative.h').write_text((HERE/'CommonPanelActivationNative.h').read_text())
    if 'UCommonInputSubsystem' in generated_names:
        target=Path(source_root)/'CommonInput/Generated';target.mkdir(parents=True,exist_ok=True)
        (target/'CommonInputMethodNative.h').write_text((HERE/'CommonInputMethodNative.h').read_text())
