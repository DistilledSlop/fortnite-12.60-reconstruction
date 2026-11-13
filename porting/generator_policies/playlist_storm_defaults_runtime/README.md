# Playlist storm-stop default

The original `UFortPlaylistAthena` constructor at `0x141a2f140` stores `-1`
into `LastSafeZoneIndex` at `0x141a2fb97` (both cached constructor-default
extractions). Restore that per-instance default with an in-class initializer;
serialized playlist values can still override it during object loading. This is
not a blanket override of playlist settings.

The original safe-zone update stops when this index is nonnegative and equals
the current phase. In run `20261113-legacy-wallet-building-retry-1`, the native
server loaded `Playlist_DefaultSolo` with zero and stopped immediately at phase
0 despite a ten-phase map. The default sentinel removes that unintended stop;
validate actual loaded values and phase progression in a fresh run.

The policy fails closed on parent, model or generated declaration drift, an
existing initializer, or a constructor requiring an explicit merge.
