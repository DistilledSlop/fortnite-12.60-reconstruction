"""Focused admission guards; behavior_harness.cpp exercises the emitted bodies."""
import copy
import importlib.util
from pathlib import Path
import re
import unittest

HERE = Path(__file__).resolve().parent
SPEC = importlib.util.spec_from_file_location('playlist_policy', HERE / 'source_policy.py')
P = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(P)


def fixture(owner):
    model = dict(name=owner, parent=P.PARENTS[owner], package='/Script/FortniteGame',
                 functions=[], helpers=[], static_helpers=[],
                 members_decl_order=[dict(name=re.search(r'(\w+);$', d).group(1), decl=d)
                                     for d in P.FIELDS.get(owner, ())], splice=dict(accepted=[], rejected=[]))
    header = '\n'.join(P.DECLARATIONS.get(owner, ()))
    cpp = '#include "' + owner + '.h"\n'
    cpp += '\n'.join('#include "' + i + '"' for i in P.INCLUDES.get(owner, ()))
    for entry in P.ENTRIES:
        if entry['class'] != owner:
            continue
        function = entry['function']
        match = re.fullmatch(r'(.*?) ' + function + r'\((.*?)\)', entry['signature'])
        model['helpers'].append(dict(name=function, ret=match.group(1), args=match.group(2).split(', ') if match.group(2) else [],
                                     const=False, static=False, access='public', kind='helper',
                                     virtual=entry.get('virtual', False), override=entry.get('override', False), body=P.body_for(entry)))
        model['splice']['accepted'].append(owner + '::' + function)
        cpp += '\n' + entry['signature'].replace(' ' + function + '(', ' ' + owner + '::' + function + '(')
        cpp += '\n{\n' + P.body_for(entry) + '}\n'
    return model, header, cpp


class GuardTests(unittest.TestCase):
    def test_real_generator_snippet_admission(self):
        # Exercise the actual reader rather than reproducing its metadata rules.
        import sys
        root = HERE.parents[1]
        sys.path.insert(0, str(root))
        spec = importlib.util.spec_from_file_location('playlist_admission_generator', root / 'gen_native_classes.py')
        generator = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(generator)
        for entry in P.ENTRIES:
            body, rejection = generator.read_snippet(entry)
            self.assertIsNone(rejection, entry['class'] + '::' + entry['function'])
            self.assertEqual(body.strip(), P.body_for(entry).strip())

    def test_full_family(self):
        P.validate_required_classes(P.PARENTS)
        for owner in P.PARENTS:
            model, header, cpp = fixture(owner)
            P.finalize(model, header, cpp)

    def test_missing_class(self):
        with self.assertRaises(ValueError):
            P.validate_required_classes(set(P.PARENTS) - {'UFortGameInstance'})

    def test_changed_ancestry_and_fields(self):
        for owner in P.PARENTS:
            model, header, cpp = fixture(owner)
            model['parent'] = 'UObject'
            if P.PARENTS[owner] != 'UObject':
                with self.assertRaises(ValueError): P.finalize(model, header, cpp)
        for owner in P.FIELDS:
            model, header, cpp = fixture(owner)
            model['members_decl_order'][0]['decl'] = 'int32 Bad;'
            with self.assertRaises(ValueError): P.finalize(model, header, cpp)

    def test_partial_or_changed_admission(self):
        for owner in P.DECLARATIONS:
            for mutation in ('missing', 'rejected', 'body', 'virtual', 'duplicate'):
                model, header, cpp = fixture(owner)
                helper = model['helpers'][0]
                if mutation == 'missing': model['helpers'].pop(0)
                if mutation == 'rejected': model['splice']['rejected'].append({'entry': owner + '::' + helper['name']})
                if mutation == 'body': helper['body'] += '\nreturn;'
                if mutation == 'virtual': helper['virtual'] = not helper['virtual']
                if mutation == 'duplicate': model['helpers'].append(copy.deepcopy(helper))
                with self.assertRaises(ValueError): P.finalize(model, header, cpp)

    def test_rendered_contracts(self):
        for owner in P.DECLARATIONS:
            for mutation in ('declaration', 'definition', 'body', 'include', 'category'):
                model, header, cpp = fixture(owner)
                if mutation == 'declaration': header = ''
                if mutation == 'definition': cpp += cpp
                if mutation == 'body': cpp = cpp.replace('{', '{ Changed;', 1)
                if mutation == 'include': cpp = cpp.replace('#include "UFortPlaylistAthena.h"', '')
                if mutation == 'category': cpp += '\nDEFINE_LOG_CATEGORY_STATIC(LogFortPlaylistSelection, Log, All);'
                with self.assertRaises(ValueError): P.finalize(model, header, cpp)


if __name__ == '__main__':
    unittest.main()
