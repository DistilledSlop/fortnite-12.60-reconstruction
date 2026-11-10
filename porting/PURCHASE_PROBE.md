# Original-client local purchase probe

With the original client, local backend, TLS and AppES diagnostic runtime
patches, and DXVK using lavapipe, an isolated ShopProbe profile was credited
with 2,001 local V-Bucks. The client displayed that balance after its wallet's
`platform` attribute was set to `EpicPC`. It displayed and previewed the local
Floss offer, and the client purchase returned HTTP 200. The balance fell to
1,501, the store showed Floss as owned, and Floss appeared in the locker.
The persisted profile contained both the deducted balance and the emote.

The original purchase response lacked a loot notification; the client opened an
empty reward presentation before returning to the offer. A notification based
on reflected structures is under investigation. Screenshots are in the ignored
`porting/generated/wine-investigation/20261109-purchase/` directory.
The catalog and wallet use local virtual currency and do not process payments.

A subsequent run reopened the same account with the 1,501 balance and Floss
marked owned. Arctic Camo was purchased for 300 and added to the profile; its
reward presentation was still blank even with a `CatalogPurchase` notification
constructed from reflected fields. In the same run the client opened the Season
12 Battle Pass purchase screen and purchased the 950 V-Bucks offer: the client
reported success, the balance became 251, and `book_purchased` persisted as
true. The UI showed the level-one rewards as unlocked, but the first version of
the backend did not grant their inventory items; level purchases also stalled
because their catalog offers were absent. These gaps require follow-up.
