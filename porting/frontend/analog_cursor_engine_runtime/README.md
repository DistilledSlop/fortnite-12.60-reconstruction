# Typed analog cursor relevance

The recovered cursor overrides key and analog relevance separately. UE4.25
FAnalogCursor exposes only the generic FInputEvent virtual; UE4.26 adds the key,
analog and pointer overloads, each forwarding a sliced FInputEvent to the generic
virtual. Original 143014980 copies those base event fields and dispatches the
generic virtual; 1430149d0 compares its user index with GetOwnerUserIndex.

The patch backports the three overloads from the local UE4.26 source. Existing
base key/analog handlers now dispatch through the proper typed override. A
subclass-only helper cannot reproduce that virtual dispatch inside base handlers.
No unrelated Engine files are included. Build/runtime results belong to the
connected routing integration batch; source correspondence alone is not a result.

The tracked zero-context patch replays with `git apply --unidiff-zero Engine.patch`.
