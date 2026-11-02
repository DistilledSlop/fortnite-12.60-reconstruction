"""Verify CommonInput observations plus module/cache ownership and invalidation."""
import common_input_action_row_verifier_v2 as previous
import common_input_action_row_verifier as original
ROWS = ('cache_module_instance', 'cache_resolve_and_copy', 'cache_return_by_value', 'cache_membership_invalidation')

def verify(text):
    lines = text.splitlines()
    marked = [i for i, line in enumerate(lines) if original.MARKER in line]
    positions = marked[-len(ROWS)-1:-1]
    expected = [f'{original.MARKER} row={row} passed=1' for row in ROWS]
    if len(positions) != len(ROWS) or [lines[i][lines[i].index(original.MARKER):].rstrip(' \t') for i in positions] != expected:
        raise ValueError('missing, failed, malformed, or reordered cache lifecycle row')
    result = previous.verify('\n'.join(line for i, line in enumerate(lines) if i not in positions))
    result['rows'] += len(ROWS)
    result['scope'] += '; module instance/root and controlled settings cache copy/invalidation'
    return result
