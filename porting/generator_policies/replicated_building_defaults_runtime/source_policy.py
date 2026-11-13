"""Restore recovered per-instance native building defaults in GameStateZone."""
import re

CLASS = 'AFortGameStateZone'
FIELDS = {
    'MaxPlayerStructures': ('int32', '-1'),
    'bAllowBuildingAtLayoutRequirements': ('bool', 'true'),
    'bAllowBuildingWithoutLayoutRequirements': ('bool', 'true'),
    'bAllowLayoutRequirementsFeature': ('bool', 'true'),
}


def finalize(model, header, cpp):
    if model['name'] != CLASS:
        return header, cpp
    if model.get('parent') != 'AFortGameState_InGame':
        raise ValueError(CLASS + ': building defaults policy parent changed')
    if (re.search(r'^\s*(?:explicit\s+)?' + CLASS + r'\s*\(', header, re.MULTILINE)
            or re.search(r'\b' + CLASS + '::' + CLASS + r'\s*\(', cpp)):
        raise ValueError(CLASS + ': merge building defaults with existing constructor explicitly')
    members = model.get('members_decl_order', [])
    for name, (typ, value) in FIELDS.items():
        matches = [member for member in members if member.get('name') == name]
        if len(matches) != 1 or matches[0].get('decl', '').strip() != typ + ' ' + name + ';':
            raise ValueError(CLASS + ': unexpected model declaration for ' + name)
        declaration = re.compile(r'^(\s*)' + typ + r'\s+' + name + r'\s*;', re.MULTILINE)
        # Count any same-name member declaration, including an existing initializer.
        candidates = re.findall(r'^\s*[^\n;{}]*\b' + name + r'\b[^\n;{}]*;', header, re.MULTILINE)
        if len(candidates) != 1 or len(declaration.findall(header)) != 1:
            raise ValueError(CLASS + ': missing or competing declaration for ' + name)
        header = declaration.sub(lambda match: match.group(1) + typ + ' ' + name + ' = ' + value + ';', header, count=1)
    return header, cpp
