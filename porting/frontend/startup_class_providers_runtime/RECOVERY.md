# Startup typed soft-class providers

The four bodies are the same specialization pattern: replay `145141550`,
frontend `145141760`, login `145141970`, and subgame `145141b80`. Bounded
windows of `0x210` bytes from each entry cover the returns. Each first checks
the path for null, obtains the cached/resolved object, checks that it is a
UClass and a child of its specialization, then calls `141e12f10` if that typed
result failed. The result of the load is again checked as a UClass and a child
of the specialization. The input soft pointer's Get handles its own weak
cache and resolution; the explicitly loaded object is not assigned back.

The `141e12f10:1a5` window shows a nonempty path always loads, regardless of
the boolean. `141e12f6e..80` uses the boolean only to gate a timer after a
cached command-line test initialized at `141e13071..b0`. The bounded UTF-16
strings at `145fe0c28` and `145fe0c50` are `ShowDelayLoadTimes` and
`Loaded delay-load asset %s`. The timer constructor's final argument is 2
(seconds). `141e1300d..3c` selects path TryLoad when the asset manager is not
valid and otherwise calls its streamable manager's synchronous load with both
optional arguments zero. `141e1303c..70` destroys the timer after the load.

Decoder: `PYTHONPATH=/tmp/common-input-capstone python3
/mnt/vibrato/fortnite-work/common-action-widget-refresh-identity-1/decode.py
145141550:210 145141760:210 145141970:210 145141b80:210` and targeted windows
`141e12f10:180`, `141e13071:c0`, `142cd3330:70`, `142cd3420:60`. Only bounded
image reads were used. The provider remains unregistered and unbuilt.

## Configuration virtuals

The class registration's constructor wrapper `14530d4c0` calls `144da9030`,
whose `+21..28` installs vtable `146d74538`. Bounded vtable reads at offsets
`250..280` identify `+260 = 144dbcb80` and `+268 = 144dba460`.
`144dba460:24` passes `this+388` and true to typed HUD loader `144c7d620`;
reflection identifies that field as `AthenaStateWidget_HUD`. The same loader
pattern is used at `144da33b0` for the NUI state class.

`144dbcb80:ee` selects `StateWidgetClasses[Selector].Class` (base +60,
stride 30), except selector 4 calls `141ed4e30` with null context and uses
`CampaignFrontEndWidget.Class` (+2d0) if the returned byte is zero. There is
no bounds check. An earlier branch for selector bits in 0x12a0 calls
`140680300`, which is `xor eax,eax; ret`; this build therefore always proceeds
to the ordinary selection. The configuration method remains dependent on the
unclosed `NativeGetGameMode141ed4e30`; its null-context path checks the global
engine and asset manager, calls `141df62a0` (the checked FortAssetManager
getter), then returns FortAssetManager's native byte at +6cc, or 2 if the
engine or asset manager is unavailable. The byte is absent from the generated
reflected class; its actual storage and writes are not yet supplied. Do not
replace that provider with a constant or read a Windows offset from the Linux
object.

`144db86e0` remains unimplemented: it checks a weak cached object, obtains the
configuration CDO, chooses one of its three configured class paths using
platform/cook logic, loads the chosen class and retrieves its CDO; it has a
separate transient-object fallback and extra conditional loads. The bounded
strings at its command-line tests are `run=`, `cook`, `targetplatform=`, and
`Switch`. The remaining platform helper and object-lifetime details require
recovery before integration; no substitute singleton is supplied.
