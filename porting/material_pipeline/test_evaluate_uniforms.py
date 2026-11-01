import math
import struct
import unittest

import evaluate_uniforms as u


def const(*v):
    return {'type': u.PREFIX + 'Constant', 'value_type': 15, 'value': list(v)}


def param(name, default, typ='ScalarParameter'):
    return {'type': u.PREFIX + typ, 'parameter': {'name': {'name': name, 'number': 0}, 'association': 2, 'index': -1}, 'default': default}


def op(typ, inputs, **kwargs):
    return {'type': u.PREFIX + typ, 'inputs': inputs, **kwargs}


def report(vectors, scalars):
    return {'virtual_texture_stacks': {'count': 0}, 'uniform_expression_arrays': {
        'vector': {'count': len(vectors), 'expressions': vectors},
        'scalar': {'count': len(scalars), 'expressions': scalars},
        'texture_virtual': {'count': 0}}}


class UniformEvaluationTests(unittest.TestCase):
    def test_layout_and_full_parameter_identity(self):
        vector = param('V', [1, 2, 3, 4], 'VectorParameter')
        scalar = param('S', 5)
        override = u.overrides_from_json([{'type': 'scalar', 'parameter': scalar['parameter'], 'value': 7}])
        data, results, padding = u.evaluate_report(report([vector], [scalar, const(8, 9, 10, 11)]), override)
        self.assertEqual(struct.unpack('<8f', data), (1, 2, 3, 4, 7, 8, 0, 0))
        self.assertEqual(results['scalar'][0], [7] * 4)
        self.assertEqual(padding, 2)
        other = dict(scalar['parameter'], index=3)
        with self.assertRaisesRegex(ValueError, 'unused'):
            u.evaluate_report(report([], [scalar]), u.overrides_from_json([{'type': 'scalar', 'parameter': other, 'value': 7}]))

    def test_swizzle_append_trig_and_float_rounding(self):
        v = const(1, 2, 3, 4)
        s = op('ComponentSwizzle', [v], channels=[2, -1, -1, -1], elements=1)
        self.assertEqual(u.evaluate(s, {}, set()), [3, 3, 3, 3])
        s = op('ComponentSwizzle', [v], channels=[2, 0, 3, -1], elements=3)
        self.assertEqual(u.evaluate(s, {}, set()), [3, 1, 4, 0])
        a = op('AppendVector', [s, v], components_a=2)
        self.assertEqual(u.evaluate(a, {}, set()), [3, 1, 1, 2])
        t = op('TrigMath', [const(0, 0, 0, 0), v], operation=1)
        self.assertEqual(u.evaluate(t, {}, set()), [1] * 4)
        multiply = op('FoldedMath', [const(.1, .1, .1, .1), const(.2, .2, .2, .2)], operation=2, value_type=15)
        self.assertEqual(u.evaluate(multiply, {}, set())[0], u.f32(u.f32(.1) * u.f32(.2)))

    def test_safe_divisor(self):
        node = op('FoldedMath', [const(1, 1, 1, 1), const(0, -0.0, -0.000001, 2)], operation=3, value_type=15)
        result = u.evaluate(node, {}, set())
        self.assertEqual(result, [u.f32(1/u.f32(.00001)), u.f32(1/u.f32(.00001)), u.f32(-1/u.f32(.00001)), .5])

    def test_unknowns_and_invalid_inputs_fail(self):
        with self.assertRaisesRegex(ValueError, 'unsupported'):
            u.evaluate(op('FoldedMath', [const(1, 1, 1, 1)] * 2, operation=4, value_type=15), {}, set())
        with self.assertRaisesRegex(ValueError, 'unsupported'):
            u.evaluate({'type': u.PREFIX + 'TextureParameter'}, {}, set())
        with self.assertRaisesRegex(ValueError, 'finite'):
            u.f32(math.inf)
        with self.assertRaisesRegex(ValueError, 'virtual texture'):
            r = report([], []); r['virtual_texture_stacks']['count'] = 1
            u.evaluate_report(r)


if __name__ == '__main__':
    unittest.main()
