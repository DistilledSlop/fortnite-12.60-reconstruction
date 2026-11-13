"""Restore the original per-instance playlist storm-stop sentinel."""
import re

CLASS = 'UFortPlaylistAthena'
FIELD = 'LastSafeZoneIndex'


def finalize(model, header, cpp):
    if model['name'] != CLASS:
        return header, cpp
    if model.get('parent') != 'UFortPlaylist':
        raise ValueError(CLASS + ': storm defaults policy parent changed')
    if (re.search(r'^\s*(?:explicit\s+)?' + CLASS + r'\s*\(', header, re.MULTILINE)
            or re.search(r'\b' + CLASS + '::' + CLASS + r'\s*\(', cpp)):
        raise ValueError(CLASS + ': merge storm default with existing constructor explicitly')
    matches = [member for member in model.get('members_decl_order', []) if member.get('name') == FIELD]
    if len(matches) != 1 or matches[0].get('decl', '').strip() != 'int32 ' + FIELD + ';':
        raise ValueError(CLASS + ': unexpected model declaration for ' + FIELD)
    declaration = re.compile(r'^(\s*)int32\s+' + FIELD + r'\s*;', re.MULTILINE)
    candidates = re.findall(r'^\s*[^\n;{}]*\b' + FIELD + r'\b[^\n;{}]*;', header, re.MULTILINE)
    if len(candidates) != 1 or len(declaration.findall(header)) != 1:
        raise ValueError(CLASS + ': missing or competing declaration for ' + FIELD)
    header = declaration.sub(lambda match: match.group(1) + 'int32 ' + FIELD + ' = -1;', header, count=1)
    return header, cpp
