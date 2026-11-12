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

After adding the Season 12 reward schedule and restoring the first-level
rewards to the existing test account, the client purchased level 2 for 150.
The request returned 200, the client reported success and advanced its reward
screen, the balance fell from 251 to 101, and the profile gained the level-2
`AthenaDance:EID_CallMe` emote. The first-level Maya and Agent Peely items were
present after the repair. At that point, a fresh pass purchase and purchase presentation still needed
verification; both were checked in the subsequent run below.

A fresh account subsequently purchased Arctic Camo from the shop. Adding
`primary: true` to the CatalogPurchase notification made the client display
the acquired item with Claim and Equip controls. The same account purchased
the pass for 950: the client reported success, the balance became 750 after
the earlier 300-V-Bucks wrap purchase, and the profile gained both Maya and
Agent Peely as part of the purchase.

The local V-Bucks storefront was tested in the original client with a free,
repeatable 1,000-V-Bucks pack. A purchase increased the displayed balance from
750 to 1,750; after restarting the client, another purchase succeeded and showed
an acquired 1,000 V-Bucks reward with a Claim button. The pack adds local test
currency only and does not use a payment provider.

On 2026-11-12, an isolated `LobbyG1Probe` account opened the reconstructed
May 20 release-day catalog. Featured offers rendered as separate cards with
their names, artwork and archived prices. Buying Lace for 1,500 returned 200,
displayed an Items Acquired screen for Lace and Stitches, marked both owned,
and reduced the displayed and persisted balance from 3,000 to 1,500. The
client's Equip buttons sent successful locker updates; the persisted active
Character and Backpack slots contained Lace and Stitches. Lace's Edit Style
screen showed both initial styles. Selecting the second style and saving
sent a successful locker update and persisted Material Mat2 with both styles
owned. Restart style-selector verification remains pending. That purchase run's
3D preview was black. A later diagnostic using DXVK 3.1.1 on software Vulkan
rendered the equipped Lace character and lobby background after restart; it
also restored the displayed 1,500 balance. DXVK is used for visual checks, while
backend-only tests do not need 3D rendering. Screenshots and redacted request
records are under ignored `porting/generated/g1-isolated-probe/`.

A later fresh client again rendered the saved Lace style in the lobby and
locker, then exited with status 139 while opening its style selector. The
selector's complete selected-style readback is still unverified; the last log
records rendering hitches but does not establish the crash cause.

The gift button displayed “2FA required to send gifts” because the local
account response has `tfaEnabled: false`. Backend gift transactions passing
tests do not establish that this client gift flow is accessible.

A further DXVK run verified friend-nickname persistence and deletion, then exited
139 after entering the locker, before the outfit picker could be reopened. Wine
SEH tracing did not report a Windows access violation; the runner printed a Linux
segmentation fault. This does not identify the crashing component. The last
locker frame rendered the saved Lace style, but full style-selector readback
remains pending. Logs and screenshots are retained under ignored
`generated/g1-isolated-probe/attempt-alias-restart-delete/`.
