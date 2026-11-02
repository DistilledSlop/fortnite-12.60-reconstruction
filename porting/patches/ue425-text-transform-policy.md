# Text transform policy backport

Apply `ue425-text-transform-policy.patch` from the Engine checkout root with
`git apply --check` followed by `git apply`. The patch is already applied in Engine
commit `87acab901`; its parent is `d7e04d4b65fb8ee464082849414cf5f7ae4d7bca`.
Do not apply it twice or overwrite existing local changes.

This focused ten-file feature comes from local UE 4.26 reference revision
`2bf1a5b83a7076a0fd275887b373f8ec9e99d431`. It adds the reflected Slate enum,
UTextBlock property/API, STextBlock attribute, cache remarshal and layout model
transformation. Existing Core conversion and shaping remain in use. Rich text and
editable hint callers explicitly retain None. Unrelated reference changes are excluded.

CommonTextBlock.SetTextCase stores inherited None/ToUpper and always dispatches
virtual SynchronizeProperties. Raw FText and the separate bDisplayAllCaps flag are
preserved. The original source contract pins setter, sync propagation, lower/upper
policy dispatch and unequal-length refusal before run-model assembly.

Full layout refuses length-changing conversions, matching the original branch;
the reference ensure diagnostic is retained. SimpleTextMode retains reference
measurement/paint bypass. Neither original simple-mode equivalence nor all
locales, shaping, wrapping, rendering pixels or rebuild lifecycle is established.

Original contracts and complete chained code-fragment replays are retained in
`/mnt/vibrato/fortnite-work/common-text-block-text-case-contract-1/` and
`/mnt/vibrato/fortnite-work/original-text-transform-model-1/`. The feature candidate
and base/reference hashes are in `text-transform-engine-backport-candidate-1/`.
The maintained CommonTextTransformProbe tests actual UMG/Slate geometry and a
separate genuine text-layout model; it does not expose a private widget cache.

Patch SHA256: `df3edb649ec80e07e13e03a2d27dc72a5aafca84fed39216ad4e0a2499d6ddad`.
Build/runtime results are recorded separately in the current project handoff.
