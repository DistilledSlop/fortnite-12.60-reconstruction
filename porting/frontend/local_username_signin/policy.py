"""Local username adaptation on the original sign-in widget controls."""
from pathlib import Path

HERE = Path(__file__).resolve().parent
FLAG = 'local_username_signin'


def apply(model, block):
    if model.get('name') != 'UFortSignInWidget':
        return model
    if model.get('package') != '/Script/FortniteUI' or model.get('parent') != 'UCommonActivatablePanel':
        raise ValueError('sign-in class identity changed')
    if (block.get('block_va') != '0x146fd9a30' or block.get('class_noregister_fn') != '0x1452dac10'
            or block.get('base_fn') != '0x145299720'):
        raise ValueError('sign-in reflection owner changed')
    functions = block.get('functions', [])
    def function(name):
        found = [f for f in functions if f.get('name') == name]
        if len(found) != 1:
            raise ValueError('sign-in reflected function missing or duplicated: ' + name)
        return found[0]
    text = {'index': 0, 'name': 'Text', 'kind': 'Text', 'kind_raw': '0x1d',
            'property_flags': '0x10000008000182', 'replicated': False, 'rep_notify': None,
            'array_dim': 1, 'object_flags': '0x45', 'offset': 0}
    for name, creator, flags, params in (
            ('StartSignIn', '0x1452dad30', '0x4080401', []),
            ('UpdateSignInButton', '0x1452dad60', '0x4480401', [text])):
        item = function(name)
        if (item.get('create_func') != creator or item.get('outer_fn') != '0x1452dacd0'
                or item.get('flags') != [flags] or item.get('params') != params):
            raise ValueError('sign-in reflected function signature changed: ' + name)
    committed = function('HandleTextCommitted')
    commit = {'index': 0, 'name': 'CommitMethod', 'kind': 'Byte', 'kind_raw': '0x0',
              'property_flags': '0x10000000000080', 'replicated': False, 'rep_notify': None,
              'array_dim': 1, 'object_flags': '0x45', 'offset': 24,
              'type_refs': {'enum': '0x142fdaea0'}}
    commit_text = dict(text, index=1)
    candidates = [c for c in committed.get('candidates', []) if c.get('outer_fn') == '0x1452dacd0']
    if (committed.get('create_func') != '0x1452dad00' or len(candidates) != 1
            or candidates[0].get('flags') != '0x4480401' or candidates[0].get('nprop') != 2
            or candidates[0].get('params') != [commit, commit_text]):
        raise ValueError('sign-in HandleTextCommitted owner/signature changed')
    expected = {'EditText_Email' : 'UEditableText*', 'EditText_Password': 'UEditableText*',
                'Border_Password': 'UCommonBorder*', 'Button_SignIn': 'UCommonButton*',
                'Button_ForgotPassword': 'UCommonButton*'}
    members = model.get('members_decl_order', [])
    for name, kind in expected.items():
        matches = [m for m in members if m.get('name') == name]
        if len(matches) != 1 or kind not in matches[0].get('decl', ''):
            raise ValueError('sign-in control schema changed: ' + name)
    forbidden = {'StartSignIn', 'UpdateSignInButton', 'HandleTextCommitted', 'NativeConstruct', 'NativeDestruct', 'HandleLocalUsernameChanged',
                 'HandleLocalSignInClicked', 'HandleLocalSelectionComplete',
                 'LocalSelectionStatus', 'bLocalSelectionPending'}
    if model.get(FLAG) or any(m.get('name') in forbidden for group in ('functions', 'fields', 'helpers', 'members_decl_order') for m in model.get(group, [])):
        raise ValueError('sign-in local adapter collision')
    model[FLAG] = True
    model.setdefault('runtime_module_dependencies', []).append('Account')
    return model


def configure_model(model):
    if model.get(FLAG):
        model['includes'] = sorted(set(model.get('includes', [])) | {'Components/EditableText.h'})
        model['cpp_includes'] = sorted(set(model.get('cpp_includes', [])) | {'LocalAccountSubsystem.h', 'Engine/GameInstance.h', 'Misc/CommandLine.h', 'Misc/Parse.h'})


def declarations(model):
    return (HERE / 'declarations.inc').read_text() if model.get(FLAG) else ''


def definitions(model):
    return (HERE / 'definitions.inc').read_text() if model.get(FLAG) else ''


def validate_output(model, header, cpp):
    if model.get(FLAG) and (header.count(declarations(model).strip()) != 1 or cpp.count(definitions(model).strip()) != 1):
        raise ValueError('sign-in local adapter missing or duplicated')
