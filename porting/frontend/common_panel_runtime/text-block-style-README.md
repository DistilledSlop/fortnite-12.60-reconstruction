# CommonTextBlock style application

SetStyle always stores the incoming class and dispatches the existing virtual
SynchronizeProperties, including repeated and null values. The CommonTextBlock
override validates the real style CDO, then directly assigns inherited Font,
StrikeBrush, Margin, LineHeightPercentage, SlateColor and shadow state in source
order. It conditionally sets actual visibility from raw Text.IsEmpty, then calls
UTextBlock::SynchronizeProperties. Per-property setters would update Slate early
and are deliberately not used. Parent/derived display-text virtual slots match;
no new style-changed or display-text hook is invented.

Relevant reflected constructor defaults are restored from the original: null
Style/ScrollStyle, false display-all-caps/auto-collapse and mobile multiplier1.
Serialized data remains authoritative. Exact class/property/function/native-exec
records guard generation. Original runtime windows and parent/emptiness evidence
are in text-block-style-{original-runtime,parent-source-evidence}.json; source
contracts/replays are under /mnt/vibrato/fortnite-work/common-text-block-{set-style-contract-1,style-binding-1}/.

The opt-in CommonTextBlockStyleProbe checks22actual metadata/UObject/direct and
reflected cases, source virtual dispatch, shadow/invalid/null/repeated behavior,
raw versus bound text and whitespace, visibility-before-parent order, actual
STextBlock synchronization and font-size desired-geometry changes. It requires
the original TextStyle-Button-Primary-M CDO. It restores its changed style CDO.
Generation/model/source/portable checks pass; UE build/runtime validation is
pending. This does not complete text producers, scrolling, full rendering or UI
parity. Actual viewport retry and retained checks must remain independent.
