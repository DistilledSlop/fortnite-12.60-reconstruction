# Shared native load diagnostic support

The source bundle emits the actual partial player loading-screen controller type,
the borrowed diagnostic map alias and the original Fort CSV category (enabled by
default, initializer 1402cb1d0). 142cc40c0 matches FCsvProfiler::RecordEventfInternal:
category capture test, 256-character formatting buffer, CSVEvent trace, per-thread
queue and start/stop-on-event behavior. Native callback should use CSV_EVENT(Fort,
TEXT("Sync load : %s"), *Path), not a bookmark or a log substitute. Original category
name and default are pinned; local CsvProfiler.cpp:3013 is the exact API match.

Borrowed static Athena phase strings are recovered for all six phases and Unknown;
full switch, jump table and every pointed-to string are pinned. Engine14468f490 is
UWorld::GetMapName, including persistent streaming-level selection, outermost package
fallback and FPackageName::GetLongPackageAssetName (World.cpp:4771).

Original windows are checked by source_policy before generation. The loading-screen
object is partial and its existing GI initialization/show/hide flow remains open;
future recovery must extend this same object. Diagnostic map writers/key policy and
full FFortAssets owner installation remain open. No builds/tests until2,040 bodies.
