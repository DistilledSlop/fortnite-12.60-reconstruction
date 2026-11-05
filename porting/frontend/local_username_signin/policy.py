"""Local username adaptation on the original sign-in widget controls."""
from pathlib import Path

HERE = Path(__file__).resolve().parent
FLAG = 'local_username_signin'


def apply(model):
    if model.get('name') != 'UFortSignInWidget':
        return model
    if model.get('package') != '/Script/FortniteUI' or model.get('parent') != 'UCommonActivatablePanel':
        raise ValueError('sign-in class identity changed')
    expected = {'EditText_Email': 'UEditableText*', 'EditText_Password': 'UEditableText*',
                'Border_Password': 'UCommonBorder*', 'Button_SignIn': 'UCommonButton*',
                'Button_ForgotPassword': 'UCommonButton*'}
    members = model.get('members_decl_order', [])
    for name, kind in expected.items():
        matches = [m for m in members if m.get('name') == name]
        if len(matches) != 1 or kind not in matches[0].get('decl', ''):
            raise ValueError('sign-in control schema changed: ' + name)
    forbidden = {'NativeConstruct', 'NativeDestruct', 'HandleLocalUsernameChanged',
                 'HandleLocalUsernameCommitted', 'HandleLocalSignInClicked', 'HandleLocalSelectionComplete',
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
