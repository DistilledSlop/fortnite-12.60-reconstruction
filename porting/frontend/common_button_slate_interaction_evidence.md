# Original CommonButton internal Slate interaction

The internal widget creates an original `SCommonButton`, with object vtable
`1459f15d0` and predicate slot `+1c8` targeting `1407cf560`. Construction copies
the reflected internal `bInteractionEnabled` byte into Slate `+5d9`. The exact
getter returns that byte being nonzero and byte `+28` bit zero being set; it does
not read the adjacent Slate `+5d8` button-enabled byte.

The root review also followed the actual base-constructor call `1430824d0`.
It installs vtable `14665bcf8`, whose same predicate slot targets `1407cf580`.
That complete seven-byte getter returns byte `+28` bit zero. Thus the custom
predicate adds its interaction gate to the base predicate. This supports a typed
adapter using the interaction flag and qualified `SButton::IsInteractable()`;
local SButton.cpp:416 returns IsEnabled(). Original private field spelling and
all enabled-attribute machinery are not recovered by this proof.

The existing setter `1407f7830` changes the internal and Slate interaction state,
compares the predicate before/after, and invokes one of the Slate delegate fields
on transitions. Its caller weak-resolves the internal object and then calls it
unconditionally: no successful-resolution guard has been proven. The recorded
`1407bc690` body checks a single delegate's bound instance and safe-execution
predicate before tail-dispatch. Exact callback member identity, remaining Slate
overrides, complete constructor/RebuildWidget behavior and widget runtime remain
separate work. A generic SButton or flag-only setter is insufficient.

Replay the original image and 17 bounded windows with:

```sh
python3 -I porting/frontend/inspect_common_button_slate_interaction.py
```

The maintained manifest derives from the bounded worker checkpoint
`/tmp/button-slate-interactable-target-1/receipt.json`, plus four root-reviewed
base/delegate windows. Root independently replayed the original 13-window proof
before extending it. Windows bind the original late image SHA-256
`338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680`.
This is original source evidence, with no native implementation or runtime credit.
