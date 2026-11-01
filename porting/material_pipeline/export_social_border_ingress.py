#!/usr/bin/env python3
"""Build a bounded dynamic TextureSize2D Social fixture from pinned original proofs.

Only the directly serialized vector-root size property is supported. Numeric
expectations are an independent source oracle; runtime IR retains the texture
lookup as an operation and does not replace it with a constant.
"""
import argparse
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import struct

HERE=Path(__file__).resolve().parent

def sibling(name):
 spec=importlib.util.spec_from_file_location(name,HERE/(name+'.py'));module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);return module
social=sibling('social_border_source');h=social.h;evaluator=h.evaluator;atlas_proof=sibling('inspect_social_atlas_defaults')
ROOT=h.ROOT; GENERATED=h.GENERATED
SOURCE=ROOT/'social_border-source-1/source.json'; SOURCE_SHA='765b8321e271cfe3e11b195b6ad4ba94ba9c61d7f652f0ba728a5fea8b3500ec'
FROZEN=ROOT/'social_border-frozen-inputs-1/receipt.json'; FROZEN_SHA='037123d240c58df507d01142339eafd5369056b2e648731ddeec052ad4ced9e3'
CALLER=ROOT/'texture-property-caller-proof-2/receipt.json'; CALLER_SHA='03a19ce546da78b2a0229d909d107c5e8f76af51fc05d78a440c839101535c13'
GETTER=GENERATED/'frontend-native-evidence/original-curve-atlas-reference-proof-1/receipt.json'; GETTER_SHA='f007855d75984f806382e05b2b79a1cc2899b28edc28ee032fc1372569e2a202'
DEFAULTS_PROOF=ROOT/'social-atlas-defaults-proof-1/receipt.json'; DEFAULTS_SHA='787cd43f8289a5ddad33905c4ef9e8c22bad628f6d1a0ee690a45d26fd0b6815'
TAG_PROOF=ROOT/'social-atlas-independent-tags-1/receipt.json'; TAG_SHA='c33b34e17d6fea309b682e4c15ad6cf57999fdb561ac9e1cb1f3d878b48edd94'
DUMP=GENERATED/'dump/FortniteClient-Win64-Shipping.runtime.bin'
ATLAS_PATH='/Game/UI/SocialPanel/Art/CA_SocialPanel.CA_SocialPanel'
PREFIX='FMaterialUniformExpression'; SOURCE_TEXTURE=dict(texture_index=0,layer_index=-1,page_table_layer_index=-1,sampler_source=2,virtual_texture=False)


def digest(raw):return hashlib.sha256(raw).hexdigest()

def checked_json(path, expected):
 raw=path.read_bytes()
 if digest(raw)!=expected:raise ValueError('pinned source or proof hash differs: '+str(path))
 return json.loads(raw)

def verify_original_proof(receipt,path):
 if receipt.get('runtime_claim') is not False or receipt.get('original',{}).get('sha256')!='604900985a724144b6e0fc6e033b0cef2adb74a295a65a4fe01aceebb52df93d':
  raise ValueError('original proof identity differs')
 for row in receipt.get('inputs',[]):
  item=Path(row['snapshot']);item=item if item.is_absolute() else path.parent/item
  if not item.is_file() or digest(item.read_bytes())!=row['sha256']:raise ValueError('proof input snapshot changed')
 for row in receipt['outputs']:
  item=Path(row['path']);item=item if item.is_absolute() else path.parent/item
  if not item.is_file() or digest(item.read_bytes())!=row['sha256'] or ('bytes' in row and item.stat().st_size!=row['bytes']):raise ValueError('proof output snapshot changed')

def verify_atlas_defaults(meta):
 tags_proof=checked_json(TAG_PROOF,TAG_SHA)
 if tags_proof['schema']!='social-atlas-independent-tags-v1' or tags_proof['constructor_proof_sha256']!=DEFAULTS_SHA or tags_proof['image_sha256']!=atlas_proof.IMAGE or tags_proof['no_runtime_claim'] is not True:
  raise ValueError('atlas independent tag proof identity differs')
 for key in ('inputs','outputs'):
  for row in tags_proof[key]:
   item=Path(row.get('snapshot',row.get('path'))); item=item if item.is_absolute() else TAG_PROOF.parent/item
   raw=item.read_bytes()
   if digest(raw)!=row['sha256'] or ('bytes' in row and len(raw)!=row['bytes']):raise ValueError('atlas tag proof snapshot differs')
 atlas_proof.verify_proof(DEFAULTS_PROOF,DUMP.read_bytes())
 asset=social.EXTRACTED/social.ATLAS
 header=atlas_proof.read(asset.with_suffix('.uasset'),atlas_proof.ASSET)
 payload=atlas_proof.read(asset.with_suffix('.uexp'),atlas_proof.EXPORT)
 actual=atlas_proof.decode_tags(header,payload)
 if actual!=json.loads((TAG_PROOF.parent/'serialized-tags.json').read_bytes()):raise ValueError('atlas serialized tag replay differs')
 if [row['name'] for row in actual['tags']]!=[row['name'] for row in meta['properties']]:raise ValueError('parsed tags differ from source report')
 return actual


def atlas_defaults(meta):
 # CUE's generic texture defaults are not this atlas subclass's defaults.
 # Tagged overrides require decoding and cannot use these constructor values.
 if meta['export_type']!='CurveLinearColorAtlas' or meta['package']!=social.ATLAS:
  raise ValueError('unexpected atlas class or package')
 if {p['name'] for p in meta['properties']} & {'SRGB','Filter','AddressX','AddressY','CompressionSettings','LODGroup'}:
  raise ValueError('atlas texture property override requires explicit decoding')
 return dict(meta,width=32,height=32,srgb=False,filter='TF_Bilinear',addressX='TA_Clamp',addressY='TA_Clamp',compressionSettings='TC_HDR')


def derive_references(source):
 trace=source['source_context']['trace']; expected=social.ATLAS_OBJECT
 if (source['source_context']['ordered_texture_references'] is not None or source['source_context']['candidate_atlas']!=expected or
     [r['position'] for r in trace]!=[3,5,9] or [r['kind'] for r in trace]!=['MaterialExpressionCurveAtlasRowParameter','MaterialExpressionMaterialFunctionCall','MaterialExpressionCurveAtlasRowParameter'] or
     trace[0].get('atlas_object')!=expected or trace[2].get('atlas_object')!=expected or trace[1].get('function')!=social.FUNCTION or trace[1].get('texture_appends')!=[]):
  raise ValueError('original ordered expression/function context differs')
 # Both virtual getters return the same atlas; AddUnique preserves the first.
 return [ATLAS_PATH]

def check_size_root(expr):
 if (expr.get('type')!=PREFIX+'TextureProperty' or expr.get('property')!=0 or len(expr.get('inputs',[]))!=1 or
     expr['inputs'][0].get('type')!=PREFIX+'Texture' or 'parameter' in expr['inputs'][0] or expr['inputs'][0].get('texture')!=SOURCE_TEXTURE):
  raise ValueError('direct original texture-size root or texture identity differs')

def flatten(arrays):
 nodes=[];roots={};size_count=0
 def visit(expr,depth,kind,index):
  nonlocal size_count
  if depth>=64 or len(nodes)>=4096:raise ValueError('expression bound exceeded')
  typ=expr.get('type','')
  if typ==PREFIX+'TextureProperty':
   if depth!=0 or kind!='vector' or index!=2:raise ValueError('nested or misplaced texture-size expression unsupported')
   check_size_root(expr);size_count+=1;nodes.append(dict(type='TextureSize2D',texture_parameter_index=0));return len(nodes)-1
  name=typ.removeprefix(PREFIX)
  if not typ.startswith(PREFIX) or name not in h.bundle.fixture_exporter.KINDS:raise ValueError('unsupported numeric expression: '+str(typ))
  item={k:copy.deepcopy(v) for k,v in expr.items() if k not in ('offset','end','inputs','type')};item['type']=name
  if 'inputs' in expr:item['inputs']=[visit(child,depth+1,kind,index) for child in expr['inputs']]
  nodes.append(item);return len(nodes)-1
 for kind in ('vector','scalar'):
  array=arrays[kind]
  if len(array['expressions'])!=array['count']:raise ValueError('numeric root count differs')
  roots[kind+'_roots']=[visit(e,0,kind,i) for i,e in enumerate(array['expressions'])]
 if size_count!=1:raise ValueError('expected one direct original size root')
 return nodes,roots

def evaluate(parsed, overrides, dimensions):
 if dimensions is not None and (not isinstance(dimensions,tuple) or len(dimensions)!=2 or any(type(v) is not int or v<=0 for v in dimensions)):
  raise ValueError('invalid original texture dimensions')
 arrays=parsed['uniform_expression_arrays']; used=set();budget=[evaluator.MAX_NODES]; result={}
 for kind in ('vector','scalar'):
  result[kind]=[]
  for index,expr in enumerate(arrays[kind]['expressions']):
   if expr['type']==PREFIX+'TextureProperty':
    if kind!='vector' or index!=2:raise ValueError('nested or misplaced texture-size expression unsupported')
    check_size_root(expr); result[kind].append([float(dimensions[0]),float(dimensions[1]),0.0,0.0] if dimensions else [0.0]*4)
   else:result[kind].append(evaluator.evaluate(expr,overrides,used,budget))
 if set(overrides)!=used:raise ValueError('unused numeric parameter overrides')
 values=[v for row in result['vector'] for v in row]+[row[0] for row in result['scalar']]+[0.0]*((-len(result['scalar']))%4)
 return struct.pack('<'+'f'*len(values),*values)

def cases(parsed,nodes,dimensions):
 params={}
 for node in nodes:
  if node['type'] in ('ScalarParameter','VectorParameter'):
   name=node['parameter']['name']['name'];item=dict(type='scalar' if node['type']=='ScalarParameter' else 'vector',parameter=node['parameter'])
   if name in params and params[name]!=item:raise ValueError('ambiguous numeric parameter identity')
   params[name]=item
 changes=[('RGB_SolidFill',[0.9,0.1,0.2,1.0]),('SelectionColor',[0.1,0.8,0.2,0.7]),('GradientCurve',0.25),('GradientFill',0.7)]
 rows=[]
 def append(name,values,unmatched=False):
  data=evaluate(parsed,evaluator.overrides_from_json([] if unmatched else values),dimensions)
  rows.append(dict(name=name,overrides=values,numeric_values=list(struct.unpack('<'+'f'*(len(data)//4),data)),numeric_sha256=digest(data),abs_tolerance=0.00001,rel_tolerance=0.00001))
 append('defaults',[]); combined=[]
 for name,value in changes:
  if name not in params:raise ValueError('missing original numeric parameter')
  entry=dict(copy.deepcopy(params[name]),value=value);combined.append(entry);append(name,[entry])
 append('combined',combined)
 for field,value in [('association',0),('index',7),('number',1)]:
  item=copy.deepcopy(combined[0]);(item['parameter']['name'] if field=='number' else item['parameter'])[field]=value;append('unmatched_'+field,[item],True)
 return rows


def make_bundle():
 source=checked_json(SOURCE,SOURCE_SHA); frozen=checked_json(FROZEN,FROZEN_SHA); caller=checked_json(CALLER,CALLER_SHA); getter=checked_json(GETTER,GETTER_SHA)
 if source['schema']!='fortnite.social-border-source-evidence.v1' or source['owner_path']!=social.OWNER or source['runtime_fixture'] is not None or source['numeric_values'] is not None:
  raise ValueError('unexpected unmodified source-only evidence')
 if frozen['schema']!='fortnite.social-border-source-frozen-inputs.v1' or len(frozen['files'])!=46:raise ValueError('source snapshot inventory differs')
 for entry in frozen['files']:
  path=Path(entry['snapshot'])
  with path.open('rb') as stream: actual=hashlib.file_digest(stream,'sha256').hexdigest()
  if path.stat().st_size!=entry['bytes'] or actual!=entry['sha256']:raise ValueError('source snapshot differs')
 verify_original_proof(caller,CALLER);verify_original_proof(getter,GETTER)
 c=caller['caller']
 if (caller['schema']!='fortnite.original-texture-property-direct-vector-caller.v1' or
     c['texture_property_evaluator']!='0x14433e4b0' or c['loop']['output_argument']!='R8 = RBP+8' or
     c['loop']['initialization']!='four positive float zeros before every call' or c['loop']['call_slot']!=24 or
     c['loop']['result_copy_bytes']!=16 or c['direct_size_property_success_lanes']!=['float(resource width)','float(resource height)','+0.0','+0.0'] or
     c['missing_texture_or_resource_lanes']!=['+0.0','+0.0','+0.0','+0.0']):
  raise ValueError('original direct-vector caller proof differs')
 if getter['schema']!='original-curve-atlas-reference-v1':raise ValueError('original Atlas getter proof differs')
 references=derive_references(source);parsed=source['decoded'];property_contract=social.check_arrays(parsed)
 if 'parameter' in parsed['uniform_expression_arrays']['texture_2d']['expressions'][0]:
  raise ValueError('original plain texture must not have a parameter identity')
 if (source['resource_sha1']!='c505c4a9977115634e87809ccd0b5e981bfc417d' or
     digest(bytes.fromhex(source['resource_raw_hex']))!=social.RESOURCE_SHA256 or len(source['original_shader_code'])!=19 or len(source['shader_records'])!=7):
  raise ValueError('original resource or shader association differs')
 atlas=source['atlas_frame'];meta=source['atlas_metadata'];verify_atlas_defaults(meta)
 if (atlas['logical_width'],atlas['logical_height'],atlas['format'],atlas['raw_sha256'])!=(32,32,'PF_FloatRGBA','36e082f7fafe908ba57a4e2964752bab086991cd7bace6a902af701d92d79836'):
  raise ValueError('original logical atlas dimensions or source pixels differ')
 original_frame=social.atlas_frame(meta,(social.EXTRACTED/(social.ATLAS+'.uexp')).read_bytes(),(social.EXTRACTED/(social.ATLAS+'.uasset')).stat().st_size)
 if original_frame!=atlas:raise ValueError('original atlas frame differs')
 nodes,roots=flatten(parsed['uniform_expression_arrays']);dimensions=(atlas['logical_width'],atlas['logical_height'])
 uniform_cases=cases(parsed,nodes,dimensions)
 default_bytes=evaluate(parsed,{},dimensions);missing_bytes=evaluate(parsed,{},None)
 if default_bytes[32:48]!=struct.pack('<4f',32,32,0,0) or missing_bytes[32:48]!=bytes(16):raise ValueError('texture-size oracle differs')
 shaders=[]
 for item in source['shader_records']:
  code=Path(item['reflection']['output']).read_bytes()
  if digest(code)!=item['reflection']['spirv_sha256']:raise ValueError('original mapped shader bytes changed')
  shaders.append(dict(record=item['record'],mapped=item['mapped'],reflection=item['reflection'],spirv_sha1=hashlib.sha1(code).hexdigest()))
 texture=dict(expression_kind='Texture',parameter=None,texture_index=0,texture_layer_index=-1,page_table_layer_index=-1,sampler_source=2,virtual_texture=False,default_texture_path=ATLAS_PATH)
 default_meta=atlas_defaults(meta)
 fixture=dict(schema='fortnite.legacy-slate.v5',material={k:parsed[k] for k in ('friendly_name','material_path','quality_level','feature_level','cooked_shader_map_id_hash','debug_description')},
  expected_layout=copy.deepcopy(social.LAYOUT),nodes=nodes,**roots,shaders=shaders,numeric_comparison_bytes=116,
  uniform_cases=uniform_cases,owner_uniform_cases=copy.deepcopy(uniform_cases),owner_parameter_expectations=[],
  draw_parameter='RGB_SolidFill',texture_2d_parameters=[texture],ordered_texture_references=references,
  texture_context_proof=dict(source_trace=source['source_context']['trace'],original_getter_proof_sha256=GETTER_SHA,
      algorithm='Original PostLoad ordered getter results and AddUnique; two identical source Atlas pointers yield one index'),
  texture_contract=dict(expression_kind='Texture',parameter=None,default_texture_path=ATLAS_PATH,default=default_meta,
      source_platform_data=atlas,named_overrides_allowed=False,scope='Original CurveLinearColorAtlas source, dynamic direct-root size and ordinary indexed texture'),
  texture_size_contract=dict(expression_kind='TextureSize2D',texture_parameter_index=0,original_property=0,direct_vector_root=2,
      original_getter_evidence_sha256=CALLER_SHA,source_dimensions=[32,32],source_result_bits=['42000000','42000000','00000000','00000000'],
      missing_resource_result_bits=['00000000']*4,scope='only the original direct vector root; runtime size must be queried dynamically'),
  shader_binding_contracts=[dict(shader_type=r['record']['type'],library_index=r['record']['library_association']['index'],**r['contract']) for r in source['shader_records']],
  expected_uniform_layout_hash=h.verifier.uniform_layout_hash(size=160,offsets=social.LAYOUT['resource_offsets'],types=social.LAYOUT['resource_types']),
  provenance=dict(resource_sha256=social.RESOURCE_SHA256,shader_library_sha256=source['provenance']['shader_library']['sha256'],atlas_defaults_receipt_sha256=DEFAULTS_SHA,atlas_tags_receipt_sha256=TAG_SHA,original_image_sha256=atlas_proof.IMAGE))
 return dict(schema='fortnite.legacy-material-ingress.v5',owner_path=social.OWNER,owner_type='UMaterial',parent_path='',shader_identity=social.OWNER,resource_owner_path=social.OWNER,
  framing=source['framing'],decoded=parsed,runtime_fixture=fixture,resource_raw_hex=source['resource_raw_hex'],original_shader_code=source['original_shader_code'],
  provenance=dict(source_sha256=SOURCE_SHA,source_snapshot_receipt_sha256=FROZEN_SHA,caller_receipt_sha256=CALLER_SHA,atlas_getter_receipt_sha256=GETTER_SHA,atlas_defaults_receipt_sha256=DEFAULTS_SHA,atlas_tags_receipt_sha256=TAG_SHA,original_image_sha256=atlas_proof.IMAGE,
      lod_group_scope='World retained from parser metadata; original constructor/initializer default unproven',resource_sha1=source['resource_sha1'],exporter_sha256=digest(Path(__file__).read_bytes())),
  limitations=['World LODGroup is retained from parser metadata; original inherited initialization remains unproven',
    'Only original direct vector TextureProperty size root is supported; no nested TextureProperty operation',
    'TextureSize2D is retained dynamically; [32,32,+0,+0] is source-derived test expectation, not a replacement node',
    'Source evidence and mapped shaders only; actual GT/RT attachment, live dynamic uniform data and rendering remain unverified'])


def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--output',type=Path,default=ROOT/'social_border-ingress-3/bundle.json');args=ap.parse_args()
 try:
  fixture=args.output.with_name('runtime-fixture.json')
  if GENERATED.resolve() not in args.output.resolve().parents or args.output.exists() or fixture.exists() or args.output==fixture:raise ValueError('choose fresh generated outputs')
  result=make_bundle();raw=(json.dumps(result,indent=2,allow_nan=False)+'\n').encode();fr=(json.dumps(result['runtime_fixture'],indent=2,allow_nan=False)+'\n').encode()
  args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_bytes(raw);fixture.write_bytes(fr)
  print(json.dumps(dict(path=str(args.output.resolve()),bundle_sha1=hashlib.sha1(raw).hexdigest(),bundle_sha256=digest(raw),fixture_sha256=digest(fr),resource_sha1=result['provenance']['resource_sha1'])))
 except (OSError,ValueError,KeyError,TypeError,IndexError,struct.error) as error:ap.exit(1,'error: '+str(error)+'\n')
if __name__=='__main__':main()
