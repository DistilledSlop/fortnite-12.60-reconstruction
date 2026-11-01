#!/usr/bin/env python3
"""Record bounded Paper2D report evidence; never infer stripped graph parity."""
import argparse
import hashlib
import json
from pathlib import Path


def load(path):
    raw = Path(path).read_bytes()
    # Cue4Inspect prints the provider index count before JSON.
    report = json.loads(raw[raw.index(b'{'):])
    return report, hashlib.sha256(raw).hexdigest()


def audit(original_parent, current_parent, original_instance, current_instance):
    def props(report):
        return {p['name']: p['structured'] for p in report['materialProperties'][0]['properties']}
    op, cp, oi, ci = map(props, (original_parent, current_parent, original_instance, current_instance))
    original_expr = {e['name'] for e in original_parent['expressionExports']}
    current_expr = {e['name'] for e in current_parent['expressionExports']}
    expected = original_parent['materials'][0]['expressionCount']
    original_instance_params = original_instance['instanceParameters'][0]
    current_instance_params = current_instance['instanceParameters'][0]
    return {
        'parent_material_settings_match': all(original_parent['materials'][0][k] == current_parent['materials'][0][k] for k in ('blendMode', 'shadingModel', 'twoSided', 'opacityMaskClipValue')),
        'original_expression_reference_count': expected,
        'original_exported_expressions': sorted(original_expr),
        'current_exported_expressions': sorted(current_expr),
        'original_graph_complete': len(original_expr) == expected,
        'original_emissive_root': op.get('EmissiveColor'),
        'current_emissive_root': cp.get('EmissiveColor'),
        'original_sampler_coordinates': next((p['structured'] for e in original_parent['expressionExports'] if e['exportType'] == 'MaterialExpressionSpriteTextureSampler' for p in e['properties'] if p['name'] == 'Coordinates'), None),
        'original_instance_overrides': oi.get('BasePropertyOverrides'),
        'current_instance_overrides': ci.get('BasePropertyOverrides'),
        'current_legacy_override_flag': ci.get('bOverrideBaseProperties'),
        'instance_parameters': {'original': original_instance_params, 'current': current_instance_params},
        'parity': 'unproven',
        'limitations': ['Cooked original graph exports and root inputs may be stripped; matching names do not prove graph edges.', 'Legacy source instance overrides may be serialized in an engine-native tail not decoded by this report.', 'Imported textures are not proof of active compiled textures.'],
    }


def main():
    p = argparse.ArgumentParser()
    p.add_argument('reports', nargs=4, help='original parent, current parent, original instance, current instance')
    p.add_argument('--output', required=True)
    a = p.parse_args()
    loaded = [load(x) for x in a.reports]
    result = audit(*(x[0] for x in loaded))
    result['report_sha256'] = dict(zip(a.reports, (x[1] for x in loaded)))
    Path(a.output).write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
