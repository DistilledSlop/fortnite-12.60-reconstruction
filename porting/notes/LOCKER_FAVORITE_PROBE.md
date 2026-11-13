# Locker favorite probe (2026-11-09)

With the original Wine client, the local backend, the TLS and AppES runtime
patches, and DXVK on software Vulkan, the Squire Shield was selected in the
back bling picker and marked as a favorite. Saving and exiting sent
`SetItemFavoriteStatusBatch`, returned HTTP 200, and changed the durable
Athena item attribute from `false` to `true`. Reopening the picker in the same
session displayed its favorite star. Repeating the action cleared the star and
restored the original `false` value with another successful profile update.
The prior preset and equipment checks are documented in the repository README.

Two subsequent fresh sessions with the favorite set signed in and reached the
BR lobby, then exited with status 139 before the favorite marker could be
checked. Their logs show severe software-rendering stalls, but no recorded
exception or cause. A control session with the favorite cleared reached the
lobby and remained usable long enough to repeat the favorite operation. This
small sample does not establish whether the saved favorite caused the crashes;
the marker has not yet been verified after restarting the client.

Request logs, comparison screenshots and the two crash logs are in
`generated/runs/20261109-favorite-clean/` (ignored by Git).

A further run with the favorite set reached the lobby and remained running until
the 330-second observation timeout; no crash dump was produced. The item was
not reopened before timeout, so fresh-session marker readback remains unverified.
This run shows the earlier crashes are not deterministic for a favorited item.

A later isolated-profile run used the narrower timer diagnostic, marked the
Squire Shield non-favorite and then favorite through the client, and received
HTTP 200 for both updates. After the client and backend restarted, the client
reopened the back bling picker and displayed the favorite star on the shield.
The final profile held `favorite: true`. Screenshots and request metadata are in
`porting/generated/runs/20261109-favorite-restart/` (ignored).
