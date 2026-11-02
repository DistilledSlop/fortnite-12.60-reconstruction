# CommonButton state and text-style readers

This extension restores the original reflected GetSelected and
GetCurrentTextStyleClass readers and the native IsHovered override. Selection is
one mutable, non-reflected per-object bool with the source-proved false default.
Hover reads the valid weak internal button and invokes that object's virtual
hover query; the original CommonButton overrides its parent's hover method.
The constructor initializes that weak reference empty. Future actual Initialize
and selection producers must write these same state members.

The text-style selector reads the actual Style CDO and lazily resolves typed
candidate CDOs, preserving disabled precedence, selected and hovered fallback,
repeated hover samples and the returned object's actual class. Source guards pin
class, property, function and native-exec identity. Existing text-style evidence
and the five spans in button-text-style-state-original-runtime.json pin the
constructor, selected-read/selector contract and distinct native hover target.

This is an explicit accessor/state foundation. Production selection transitions,
callbacks, Initialize/NativeConstruct, input-action refresh and router/lifecycle
integration remain unfinished. No selection setter with incomplete effects is
added. Controlled diagnostic mutation is not hardware input or UI parity.

The new 26-row CommonButtonTextStyleProbe checks real CDOs/UObjects, direct and
ProcessEvent metadata/results, independent selected values, five style branches,
fallback order, actual weak internal hover and GC expiry, invalid class values,
and restoration. It additionally requires a serialized Style and normal text CDO
from the original IconTextButton class; unavailable data fails the diagnostic.
The runner supports the existing source-pinned material manifest arguments for
this original asset lookup. Actual UE validation is pending; earlier green
binding-creation receipts remain historical baselines.
