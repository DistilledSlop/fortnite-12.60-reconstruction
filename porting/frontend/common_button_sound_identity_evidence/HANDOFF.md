# CommonButton selection feedback: Slate application and sound identity

The prior singleton inference is now joined to a concrete original producer and
consumer chain. This establishes the narrow mapping used by
`FSlateApplication::Get().PlaySound(NormalStyle.PressedSlateSound, 0)`; it does
not establish audible parity or finish other button dependencies.

* Original `0x143005fc9..0x143006165` allocates `0x600` bytes, calls
  `0x142ffbbd0`, wraps the returned pointer in a shared reference, writes it to
  `0x148271318`/control block `0x148271320`, copies both to a second global
  root `0x1482675c8`, then passes the same object adjusted by `+0x118` as a
  shared platform message handler through the platform application's vtable.
  This independently matches target `FSlateApplication::Create`: construct
  `CurrentApplication`, assign `CurrentBaseApplication`, then call
  `PlatformApplication->SetMessageHandler(CurrentApplication.ToSharedRef())`.
  The adjustment is consistent with a message-handler subobject. Shutdown
  `0x14303476c..0x1430347f4` releases the platform, application and base shared
  roots in that order, also matching source. Global identity is established by
  this producer/consumer sequence, not a symbol-name guess.
* The constructor sets the object at `+0x330` to a newly constructed object
  with vtable `0x146654d20` and a shared control block. Its vtable's first
  entries point to `0x1406801c0` (returns without action) and `0x1406964e0`
  (returns float zero), matching target `FNullSlateSoundDevice::PlaySound` and
  `GetSoundDuration`, and the interface's first two virtuals. Source constructor
  installs that same null device; `InitializeSound` can replace it.
* Selection `0x1407f98ef..0x1407f990c` checks feedback, loads that global,
  passes button `+0x5a8` and user index zero to `0x143022250`; the latter loads
  `receiver+0x330` and tail-dispatches vtable slot zero, leaving the sound and
  user arguments intact. Target Get returns CurrentApplication, and target
  PlaySound forwards those arguments through SlateSoundDevice's PlaySound.
  The selection policy separately guards the style/offset identity.

`policy.validate_original_image(image_path)` verifies eight bounded original
instruction/data windows with `os.pread`; no full image read/hash. Call
`policy.validate_source(engine_root)` with the UnrealEngine checkout root before
setting `common_button_feedback_sound_identity_proven`. It fail-closes on the
exact target Get, creation/root/message-handler join, null-device constructor,
InitializeSound, PlaySound, interface order and null implementations. The caller
must also run the existing focus/selection policy original/metadata guards and
validate the generated candidate, including its exact feedback expression.

Validation: `python3 porting/frontend/common_button_sound_identity_evidence/replay.py`
PASS on `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`
and `/opt/fortnite/UnrealEngine`; it also rejects a mutated original span and
mutated target sound dispatch in sparse/temp fixtures. The replay is static and
is not a runtime/audio test. Inspection used the seek/read decoder
`python3 /mnt/vibrato/fortnite-work/common-action-widget-refresh-identity-1/decode.py`
with windows `143005f70:160 143034700:1b0 143022250:40`,
`142ffbbd0:290 1430060a5:130`, and `1407f9880:b0`.
A targeted 1 MiB chunked `os.pread` scan for RIP-relative *stores* to the global
in `0x140000000..0x146000000` found `0x143006031` and `0x1430347bf`; it did not
repeat the prior load scan. No generation, native/engine edit, build or runtime
flow was performed. Root owns integration and the combined verification batch.
