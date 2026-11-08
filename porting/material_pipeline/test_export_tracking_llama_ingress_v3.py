import hashlib, importlib.util, json, unittest
from pathlib import Path
HERE=Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location('llama3',HERE/'export_tracking_llama_ingress_v3.py'); llama3=importlib.util.module_from_spec(spec); spec.loader.exec_module(llama3)
BUNDLE3=llama3.GENERATED/'material_pipeline/tracking-llama-bundle-3/M_UI_TrackingLlama.json'
NATIVE=HERE.parent/'native/Source/FortniteGame/Private/LegacyMaterialShaderMapProbe.cpp'
class TrackingLlamaV3Test(unittest.TestCase):
    def test_properties_are_class_defaults(self):
        proof=llama3.prove_properties()
        self.assertEqual(proof['serialized_properties'],['ImportedSize','LightingGuid'])
    def test_bundle2_pin_and_effective_values(self):
        item=llama3.build(); t=item['runtime_fixture']['texture_contract']['textures'][0]
        self.assertEqual((t['effective_srgb'],t['effective_sampler']['sampler_source_mode'],t['effective_sampler']['filter']),(True,'SSM_FromTextureAsset','TF_Default'))
        self.assertEqual((t['source_platform_data']['pixel_format'],t['source_platform_data']['mip_count']),('PF_DXT5',9))
    def test_native_pin_matches_bundle3(self):
        if not BUNDLE3.exists(): self.skipTest('bundle-3 not generated')
        sha1=hashlib.sha1(BUNDLE3.read_bytes()).hexdigest(); resource=json.loads(BUNDLE3.read_text())['provenance']['resource_sha1']
        text=NATIVE.read_text(); self.assertIn(sha1,text); self.assertIn(resource,text)
        self.assertIn('tracking-llama-bundle-3/M_UI_TrackingLlama.json',(HERE.parent/'frontend/run_frontend_probe.sh').read_text())
if __name__=='__main__': unittest.main()
