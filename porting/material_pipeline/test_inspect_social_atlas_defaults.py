#!/usr/bin/env python3
"""Test bounded original atlas package framing and fail-closed tag parsing."""
import importlib.util
from pathlib import Path
import struct
import unittest
spec=importlib.util.spec_from_file_location('atlas',Path(__file__).with_name('inspect_social_atlas_defaults.py'));m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
ASSET=Path('/mnt/vibrato/fortnite-extracted-full/FortniteGame/Content/UI/SocialPanel/Art/CA_SocialPanel')

@unittest.skipUnless(ASSET.with_suffix('.uasset').is_file() and ASSET.with_suffix('.uexp').is_file(),'original package unavailable')
class OriginalPackage(unittest.TestCase):
 def setUp(self):
  self.header=ASSET.with_suffix('.uasset').read_bytes();self.payload=ASSET.with_suffix('.uexp').read_bytes()
 def test_exact_tags_and_absence(self):
  result=m.decode_tags(self.header,self.payload)
  self.assertEqual([r['name'] for r in result['tags']],['TextureSize','GradientCurves','ImportedSize','LightingGuid'])
  self.assertEqual(result['absent_fields'],['SRGB','Filter','AddressX','AddressY','CompressionSettings','LODGroup'])
 def test_broken_name_type_terminator_and_boundary_rejected(self):
  for position,value in [(0,999),(8,999),(16,0x7fffffff),(232,0x7fffffff)]:
   bad=bytearray(self.payload);struct.pack_into('<i',bad,position,value)
   with self.subTest(position=position),self.assertRaises((ValueError,UnicodeError,struct.error,IndexError)):m.decode_tags(self.header,bad)
 def test_export_class_identity_rejected(self):
  bad=bytearray(self.header);struct.pack_into('<i',bad,2638,0)
  with self.assertRaises(ValueError):m.decode_tags(bad,self.payload)
if __name__=='__main__':unittest.main()
