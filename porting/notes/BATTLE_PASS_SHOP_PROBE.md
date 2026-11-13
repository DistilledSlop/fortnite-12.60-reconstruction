# Original-client Battle Pass and shop probe

This is the earlier empty-catalog baseline. The later [purchasing probes](PURCHASE_PROBE.md)
supersede its shop, Battle Pass purchase, and local V-Bucks limitations. Keep this
record as historical evidence, rather than the current G1 status.

With the original client, local backend, TLS and AppES diagnostic runtime
patches, and DXVK using lavapipe, the Battle Pass tab opened its headquarters
and displayed level 1 and 10,000 XP to level 2. The Item Shop and V-Bucks tabs
each displayed "No Offers Available" and the balance displayed zero.

This matches the local defaults: an unpurchased level-one Battle Pass, zero
`Currency:MtxGiveaway`, and an empty storefront catalog. Catalog purchases are
not implemented. This probe does not establish working reward previews,
progression, claiming, Battle Pass purchase, or real-money transactions.
Screenshots are in the ignored `porting/generated/runs/20261109-pass-shop/`
directory.
