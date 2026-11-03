# Direct literal scalar return scan

`python3 porting/behavior/bulk_direct_literal_runtime/prepare.py` scans the
SHA-256-pinned original runtime image and emits proposal and evidence JSON under
`/tmp/gameplay-bulk-direct-literal/`. The 2026-11-03 run examined 23,758 native
records and found **zero** unported proposals. It grants no body credit. The
report contains the full rejection counts and any matched instruction sequences;
outputs are disposable local artifacts and are not runtime validation.

The finite patterns cover the complete zero-argument FFrame advance plus either
an inline immediate store or a direct call to an exact immediate-register or
zero-register return leaf. The Float leaf additionally allows exactly `movss
xmm0, [rip+disp]; ret` if its four bytes lie wholly in one readable,
initialized, nonwritable original PE section. Evidence records the address,
section, raw bits and full wrapper and leaf. Finite Float values use nine
significant decimal digits and must round-trip to the original binary32; NaN,
infinity and negative zero are rejected. Integer results are interpreted at the
actual wrapper store width. Virtual dispatch, branchy wrappers, events and RPCs
are excluded.

The scanner requires an unambiguous selected class, correct game/UI package and
unique matching generated module header; it excludes existing ported bodies,
maintained definitions, generated definitions and generated declarations. It
preserves reflection static, const and access flags in any proposal. A zero
result applies only to these patterns and current inputs. No Native generation,
builds or runtime tests were performed; tested baseline remains 1,040 bodies.
