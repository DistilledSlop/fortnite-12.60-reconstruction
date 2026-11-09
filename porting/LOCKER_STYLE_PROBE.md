# Original-client locker style probe

Using the original client with the local backend, the TLS verification and
AppES diagnostic runtime patches, and DXVK with lavapipe, a disposable profile
was granted Drift with Stage 0 and Stage 1 owned. The client unlocked and
previewed Stage 1. Saving it sent `SetCosmeticLockerSlot` with a Progressive
variant update; the backend returned 200 and persisted Stage 1 in the item and
locker. After closing and relaunching the client and backend, the locker rendered
Stage 1 and the style editor showed Stage 1 selected. The other four stages
remained locked.

The owned stages were seeded only in the disposable profile for this probe;
this does not establish how the original service granted or unlocked styles.
The backend's item and locker update policy is inferred and requires broader
coverage across cosmetics and locker operations. Screenshots from the run are
in the ignored `porting/generated/wine-investigation/20261109-style/` directory.
