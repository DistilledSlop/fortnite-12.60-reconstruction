# Original 12.60 cooked collision schema

The original client reads Chaos data under the `PhysXPC` bulk-data key. Changing
that key to a PhysX platform fallback would send the wrong format to the reader.
The current engine's Chaos serializer also has a different layout.

`python3 porting/tools/inspect_original_sidewalk_collision.py` is a read-only
fixture inspector. It pins the staged sidewalk package hashes and bulk header,
consumes all 934 bytes, and checks both convex hulls against their planes, bounds,
volume and center of mass. Unsupported objects, references, nonempty trailing
arrays, truncated data and trailing bytes fail validation. Optimized Python is
rejected because validation uses assertions. The script contains no game assets.
It proves this fixture's schema and geometry; it does not enable runtime collision.

## Original binary evidence

Addresses refer to the original runtime image, SHA-256
`5f056080237b5a0fdab284700d5bd975acf8c30ffeeb37472f5ba5542fa2dc38`.
Captured decompiles and assembly remain in ignored
`porting/generated/agents/original-chaos-convex-schema30/`.

- BodySetup `0x14444d2f0` retrieves `PhysXPC` and calls reader `0x1444434d0`.
  That reader constructs a fresh buffer reader (`0x142ccbd80`) and Chaos archive
  proxy (`0x142ccbad0`), requires four-byte precision and reads convex objects,
  triangle objects, UV indices, UV positions, UV channels and face remapping.
- Fresh archive custom versions come from the registered loading-archive registry
  (`0x142cceb30`, reset flag set by `0x140814390`); proxy `0x14119b7b0` forwards
  the query. No package custom versions are transferred into this reader.
- Registration `0x140476b30` establishes ExternalPhysics version 37, GUID
  `35F94A83-E258406C-A31809F5-9610247C`; `0x140476a70` establishes Destruction
  version 10, GUID `174F1F0B-B4C645A5-B13F2EE8-D0FB917D`.
- Implicit serializer `0x1437feff0` reads two uint32 booleans for this version.
  It serializes no CollisionType byte. This is the first divergence from the
  current engine reader.
- Convex serializers `0x141538060` / `0x141538340` select concrete planes at
  external version 31+, raw bounds at 25+, and volume/center of mass at 20+.
  Plane helper `0x14151b420` reads an int32 count then origin/normal float3 pairs.
  Particles helper `0x141537e60` reads a uint32 view boolean and float3 array.
  Bounds helper `0x141538690` reads two float3 values. Version 37 independently
  selects these branches; version values were not chosen to make the fixture parse.
- Installation `0x14444e0d0` maps convex object index i directly to
  `AggGeom.ConvexElems[i]` (original element stride 0xb0, Chaos pointer +0xa0),
  then calls `0x14444b0f0`. This assignment has no mirrored-index pairing or
  coordinate transform. It does not independently prove this package's aggregate
  element count or a PhysX conversion's transform convention.

## Validated fixture and remaining integration

The fixture has two identical convex objects, each with 12 planes and eight
vertices. Their pointer framing occupies bytes [8,461) and [461,914); the final
20 bytes are five empty array counts, as independently established by the original
reader. All geometry is finite. Planes have unit normals, support at least three
vertices each and enclose every vertex; vertex-derived bounds, cuboid corners,
volume and center of mass agree. The complete payload SHA-256 is
`7caf4a28a96eeee4761dcf43702e7b71b7a48237f20e2ce36a47e68ebcbfdc14`.

A production adapter still needs bounded parsing of the supported original
object families and triangle/UV/material data, aggregate index/count and coordinate
space validation, and active physics mesh creation. The existing PhysX cooking
interface can create convex meshes, followed by
`UBodySetup::FinishCreatingPhysicsMeshes_PhysX`; cooking availability must be
verified in the actual server. Every mesh pointer and array count needs validation
before installation: the existing helper can report success after a convex cook
fails, and the installation method can clear or mis-index aggregate elements on
mismatched arrays. Preserve collision flags, ownership, mirrored geometry and
material mapping. Do not silently inflate a failed hull or claim this decoder
repairs POI collision.
