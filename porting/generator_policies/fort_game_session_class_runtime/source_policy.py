"""Emit the recovered AFortGameMode session-class virtual override."""
import re

CLASS = 'AFortGameMode'
GETTER = 'GetGameSessionClass'


def finalize(model, header, cpp):
    if model['name'] != CLASS:
        return header, cpp
    if model.get('parent') != 'AFortGameModeBase':
        raise ValueError(CLASS + ': unexpected parent')
    signature = r'\bclass\s+\w+_API\s+AFortGameMode\s*:\s*public\s+AFortGameModeBase\s*\{'
    if len(re.findall(signature, header)) != 1 or header.count('public:') != 1:
        raise ValueError(CLASS + ': unexpected class declaration or public section')
    # Reject any pre-existing overload, declaration, definition, or use; silently
    # merging it would risk changing a different signature or hiding behavior.
    if re.search(r'\b' + GETTER + r'\b', header + '\n' + cpp):
        raise ValueError(CLASS + ': competing session-class getter')
    own_include = '#include "AFortGameMode.h"'
    session_include = '#include "AFortGameSession.h"'
    if cpp.count(own_include) != 1 or cpp.count(session_include) > 1:
        raise ValueError(CLASS + ': unexpected cpp includes')
    if session_include not in cpp:
        cpp = cpp.replace(own_include, own_include + '\n' + session_include, 1)
    declaration = '\tvirtual TSubclassOf<AGameSession> GetGameSessionClass() const override;\n'
    header = header.replace('public:', 'public:\n' + declaration, 1)
    cpp += '''\nTSubclassOf<AGameSession> AFortGameMode::GetGameSessionClass() const
{
    // Original vtable getter 0x141e60820; see policy evidence.
    return AFortGameSession::StaticClass();
}
'''
    return header, cpp
