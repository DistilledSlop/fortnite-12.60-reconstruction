// CONFIDENCE: LOW. Broadcasts the process-wide building resave delegate with this building as the payload. Listeners that are invalid or report failure are removed from the invocation list.
// Original address: 0x142737ee0 (exec). Native body is an inline multicast-invocation loop over the static delegate at 0x1478c3468.
BuildingResaveDelegate.Broadcast(this);
