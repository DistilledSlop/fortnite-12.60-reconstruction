#!/usr/bin/env python3
"""Reject changed conversion evidence even if its report link is rehashed."""
import json
from pathlib import Path
import shutil
import tempfile
import unittest

import export_paper2d_shader_manifest as exporter


class ManifestTest(unittest.TestCase):
    def test_changed_conversion_input_is_rejected(self):
        if not exporter.ARCHIVE.exists() or not exporter.EVIDENCE.exists():
            self.skipTest('original assets and generated conversion evidence are required')
        with tempfile.TemporaryDirectory() as temporary:
            evidence = Path(temporary)
            for name in ('parent-resource.json', 'mi-resource.json',
                         'parent-pipelines-prefix.json', 'mi-complete-prefix.json',
                         'conversion/source.json', 'conversion/d3d.json',
                         'conversion/mapped/mapped-translation-report.json', 'conversion/reflection.json'):
                dest = evidence / name
                dest.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(exporter.EVIDENCE / name, dest)
            mapped_path = evidence / 'conversion/mapped/mapped-translation-report.json'
            mapped = json.loads(mapped_path.read_text())
            mapped['records'][0]['input_sha256'] = '0' * 64
            mapped_path.write_text(json.dumps(mapped))
            reflection_path = evidence / 'conversion/reflection.json'
            reflection = json.loads(reflection_path.read_text())
            reflection['mapped_report_sha256'] = exporter.digest(mapped_path.read_bytes())
            reflection_path.write_text(json.dumps(reflection))
            with self.assertRaisesRegex(ValueError, 'conversion input identity mismatch'):
                exporter.build(evidence=evidence)


if __name__ == '__main__':
    unittest.main()
