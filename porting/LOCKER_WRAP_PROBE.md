# Original-client locker wrap probe

With the original client, local backend, TLS and AppES diagnostic runtime
patches, and DXVK using lavapipe, a disposable profile was granted Arctic Camo
and Floss. The client equipped Floss in the second emote slot and Arctic Camo
in an individual weapon wrap slot; both requests succeeded and were persisted.

The client's Misc Wrap tile uses wrap slot index 6. Its Apply to All action
sends slot index -1. Both operations initially failed validation. After the
backend was updated, the client saved Misc Wrap successfully, then applied
Arctic Camo to all seven wrap slots. Both requests returned 200, the locker
showed seven wrap icons, and the persisted profile contained the wrap in all
seven slots. Screenshots and sanitized request observations are in the ignored
`porting/generated/wine-investigation/20261109-wrap-all/` directory.

The cosmetic grants were seeded in a disposable profile for this probe. This
checks these client interactions, not every cosmetic or locker operation. The
AppES runtime patch remains a diagnostic workaround, and software rendering is
slow and can be unstable.
