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


## Plain triangle mesh (type 11)

All scalars little-endian; float means IEEE float32. Every array starts with int32 count; every bool occupies uint32 on wire. Ordered field sequence:

| Order | Field and wire data | Original evidence / effective branch |
|---|---|---|
|1|bIsConvex uint32, stored bDoCollide uint32|1437feff0 reads offsets+9,+10 for Destruction>=5. Destruction10 then **forces bDoCollide=true** because version<11. No CollisionType byte. bHasBoundingBox is constructor state, not serialized here.|
|2|Particles: uint32 view flag, int32 vertex count, float3 vertices|141537e60, same as convex. Fixture view=1.|
|3|uint32 bRequiresLargeIndices; selected int32 triangle count and triples|14387f850: External>=28 reads selector. True:14151b5c0 /1415144e0 three int32; false:143864b00 /143839c00 three uint16. Older<28 always large, no selector.|
|4|LocalBoundingBox min float3,max float3|141538690. External>=25 raw bounds.|
|5|Acceleration structure described below|`0x14387f930`: External>=13 ->1437fbb90. Older<10 rebuild only,10..12 old bounding volume then rebuild.|
|6|MaterialIndices: int32 count,uint16 values|1435be980, External>=19.|
|7|ExternalFaceIndexMap: int32 count,int32 values|14153ebe0, External>=33. Pointer allocation is in-memory only; no pointer bool before this array.|

External>=1 omits obsolete ConvexHulls pointer entirely. No volume/COM fields or extra trailing mesh flags. Destruction10 does not add any further triangle field.

### Triangle acceleration tree at External37

1437fbb90 serializes this exact sequence:

1. FullBounds=min float3,max float3 (141538690).
2. Nodes array (1435bee50): each1437fc500 node is two bounds (48 bytes), two int32 child indices (8), uint32 bLeaf (4), total60 bytes. External>=25 makes node bounds raw float3 pairs too.
3. Leaves outer int32 count. Each leaf is an array of payload/bounds elements (1435bf380): int32 element count, then each **int32 payload first**, min float3,max float3. Leaf cached bounds in memory are not serialized.
4. DirtyElements array, same payload/bounds layout (1435bf380).
5. GlobalPayloads array, same payload/bounds layout (1435bf380).
6. uint32 bSerializePayloadToInfo (External>=27); if false, skip next item.
7. If true, PayloadToInfo via1437fc6d0: int32 key count and all int32 keys, **then** for each key three int32 values GlobalPayloadIdx,DirtyPayloadIdx,LeafIdx. This is not interleaved key/value pairs. Loaded map is discarded after consumption in this immutable original tree. Bound key/count before allocation.
8. int32 MaxChildrenInLeaf, int32 MaxTreeDepth, float MaxPayloadBounds.

Correlates to `/opt/fortnite/UnrealEngine/Engine/Source/Runtime/Experimental/Chaos/Public/Chaos/TriangleMeshImplicitObject.h` SerializeImp and `Chaos/AABBTree.h` Serialize/node/leaf/payloadinfo, but the original function addresses establish the selected sequence. Current ImplicitObject's CollisionType serialization must not be used. FullBounds and root child bounds in this real tiny fixture are zeros; validation uses independently checked leaf triangle bounds and does **not** assert original broadphase correctness from root zeros.

### Validated triangle fixture and limits

`python3 porting/tools/inspect_original_oilrig_collision.py` independently pins
and consumes all 609 bytes of the staged OilRig triangle-floor collision payload.
It contains six vertices, eight nondegenerate uint16-indexed triangles, a BVH
node and leaf, eight material entries and an identity face map. Every leaf payload
bound matches its triangle's independently computed bound. The final four UV/remap
arrays are empty. Stored implicit flags are false/false; the original Destruction10
reader nevertheless sets effective collision enabled. Root and full BVH bounds
are zero in this fixture, so successful payload parsing does not prove those
original broadphase bounds are usable in a new engine.

Original triangle dispatch is shared-pointer `0x1437aaf50`, raw-pointer
`0x1437ab140`, object `0x1437abeb0`, factory `0x1437fae10`, constructor
`0x1437b5a50`, serializer `0x14387f810` to `0x14387f930`. The constructor's vtable
is `0x1467431c0`; the serializer occupies slot +0x80. The nearby literal
`0x145cf0018` is a type-name string, not a vtable. Pointer framing has a uint32
present flag, then int32 identity; existing identities reference previous objects
without another type/body. The registry is shared across both object arrays.

This second inspector supports its pinned fixture only. It adds no collision
installation or runtime claim. Wrapped type 0x4b/0x8b serializers and nonempty
UV/remap tails need separate validation. Production parsing must handle ownership,
references and supported object families explicitly, then preserve face/material
correspondence when cooking and installing physics meshes. Evidence and all raw
captures remain in ignored `porting/generated/agents/original-chaos-trimesh-schema36/`.
