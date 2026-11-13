# Replicated building defaults

`AFortGameStateZone` original constructor at `0x141e8aa60` writes `-1` to
`MaxPlayerStructures` at `0x141e8b1fe` and writes `0x00010101` at
`0x141e8b208` to the three consecutive full-byte layout flags. The policy
adds equivalent member initializers, so native construction initializes every
instance. The existing `OriginalDefaults.json` CDO repair alone does not do
that: native-owned non-config properties are generally omitted from Unreal's
post-construction CDO copy, except for Blueprint-specific overrides.

Call `finalize(model, header, cpp)` from the generator's policy stage. It fails
closed on changed parent, model declarations, header declarations, or an
existing constructor that needs an explicit merge. Runtime verification is
pending integration and a server/client test.
