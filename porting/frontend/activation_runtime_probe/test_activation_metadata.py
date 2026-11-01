"""Guard the real UHT bool-setter/constructor symbol ambiguity."""
import unittest
import binder

binder.context()
import activation_metadata as metadata


class ConstructorSelectionTests(unittest.TestCase):
    def setUp(self):
        self.creator = (0x1000, 32, '_Z58Z_Construct_UFunction_UCommonActivatableWidget_IsActivatedv')
        self.setter = (0x2000, 16, '_ZN66Z_Construct_UFunction_UCommonActivatableWidget_IsActivated_Statics26NewProp_ReturnValue_SetBitEPv')

    def test_constructor_excludes_actual_bool_setter(self):
        symbols = [self.setter, self.creator]
        with self.assertRaises(ValueError):
            metadata.m.one_syms(symbols, 'Z_Construct_UFunction_UCommonActivatableWidget_IsActivated', 'v')
        self.assertEqual(metadata.function_constructor(symbols, 'IsActivated'), self.creator)

    def test_missing_or_duplicate_constructor_rejected(self):
        for symbols in ([self.setter], [self.creator, self.setter, self.creator]):
            with self.subTest(symbols=symbols), self.assertRaises(ValueError):
                metadata.function_constructor(symbols, 'IsActivated')


if __name__ == '__main__':
    unittest.main()
