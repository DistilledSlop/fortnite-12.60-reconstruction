"""Compile and exercise the production selection core, including reentrancy."""
from pathlib import Path
import json
import subprocess
import tempfile
import unittest

HERE = Path(__file__).resolve().parent


class SelectionCore(unittest.TestCase):
    def test_original_signatures(self):
        record = json.loads((HERE / 'common-button-selection-evidence.json').read_text())
        functions = json.loads((HERE.parents[1] / 'generated/reflection/native_funcs.json').read_text())['UCommonButton']
        for name, signature in record['reflected_functions'].items():
            with self.subTest(name=name):
                self.assertEqual({key: functions[name][key] for key in signature}, signature)

    def test_execution_order_and_reentrancy(self):
        with tempfile.TemporaryDirectory() as directory:
            executable = Path(directory) / 'selection'
            subprocess.run([
                'c++', '-std=c++14', '-Wall', '-Wextra', '-Werror',
                '-I', str(HERE), str(HERE / 'test_button_selection_core.cpp'),
                '-o', str(executable),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(executable)], check=True, capture_output=True, text=True)


if __name__ == '__main__':
    unittest.main()
