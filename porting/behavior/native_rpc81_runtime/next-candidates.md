# Bounded read-only inventory for the next broad batch

Terra6.1 medium, fork-none scout found two relatively straightforward substantial
skipped bodies with no named draft overlap: controller ServerEndLoadingVolume
0x1419f19c0 (58 decompile lines) and ServerEndUnloadingVolume 0x1419f1b80 (59).
Both scan mapped VolumesLoading/VolumesUnloading arrays, remove matching volume
via140726fc0, and log missing entries. Verify signatures and removal semantics.

Further candidates need field/delegate/signature work: ClientReportTournamentPlacementPointsScored
1419c73e0, ClientNotifyLost1419c57a0, ServerUpdateLevelStreamingHandshakeData
1419f3770, ServerAddToCachedPurchased1419f0c30. Decompiler names/argument types
are unreliable. ServerSetTeam, SetCanEditCreativeIsland and creative validation
have dependencies or existing draft overlap and are not quick integrations.

These are candidates, not recovered or accepted bodies. No tests/probes/mutations
were performed by the scout. Root owns integration and review.
