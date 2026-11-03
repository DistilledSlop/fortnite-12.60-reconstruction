"""Source includes for four independently closed Replay graph dependencies."""

def apply(model, block, owner_fns):
    if model['name'] == 'AFortPlayerControllerSpectating':
        model['cpp_includes'] = sorted(set(model.get('cpp_includes', [])) | {
            'AFortPlayerState.h', 'Engine/World.h', 'GameFramework/GameStateBase.h',
            'GameFramework/PlayerState.h',
        })
        model['splice'].setdefault('header_includes', set()).add('GameFramework/OnlineReplStructs.h')
    return model
