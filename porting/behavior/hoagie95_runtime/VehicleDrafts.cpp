// UNCOUNTED ROUGH LIFTS. Not compiled or included in manifest.
// Unresolved names below are placeholders for native bindings, never fallback implementations.
// These sketches preserve call order and branches pending identification of virtual slots,
// global vector constants, transform helpers, and exact original ABI.

// 0x1418ee2a0: GetShootCenterPoint
Vec3 ShootCenterPoint(Vehicle& v) {
    auto* mesh = UNRESOLVED_VIRTUAL_7A8(v);
    if (mesh) return UNRESOLVED_VIRTUAL_410(mesh, v.ShootCenterPointSocket);
    auto* root = UNRESOLVED_ROOT_AT_130(v);
    return root ? UNRESOLVED_ROOT_LOCATION_1D0(root) : UNRESOLVED_VECTOR_148218630;
}
// 0x1418ee350: GetShootCenterPointLocal; 1 is passed to the socket transform call.
Vec3 ShootCenterPointLocal(Vehicle& v) {
    auto* mesh = UNRESOLVED_VIRTUAL_7A8(v);
    if (!mesh) return UNRESOLVED_VECTOR_148218630;
    return UNRESOLVED_VIRTUAL_408(mesh, v.ShootCenterPointSocket, 1).Translation;
}
// 0x1418ea440: GetAimPoint. Original resolves forward before invoking shoot point.
Vec3 AimPoint(Vehicle& v) {
    auto* root = UNRESOLVED_ROOT_AT_130(v);
    Vec3 forward = root ? UNRESOLVED_1440E9D30(root) : UNRESOLVED_VECTOR_147C31CD0;
    Vec3 point = ShootCenterPoint(v);
    return point + forward * v.ShootAimAheadDistance;
}
// 0x1418ea500: GetAimPointLocal; slot 0x408 returns a transform; use translation.
Vec3 AimPointLocal(Vehicle& v) {
    auto* mesh = UNRESOLVED_VIRTUAL_7A8(v);
    Vec3 point = mesh ? UNRESOLVED_VIRTUAL_408(mesh, v.ShootCenterPointSocket, 1).Translation
                      : UNRESOLVED_VECTOR_148218630;
    return point + UNRESOLVED_VECTOR_147C31CD0 * v.ShootAimAheadDistance;
}
// 0x14084c250: GetRotorCenterPosition
Vec3 RotorCenterPosition(Vehicle& v) {
    auto* root = UNRESOLVED_ROOT_AT_130(v);
    Vec3 location = root ? UNRESOLVED_ROOT_LOCATION_1D0(root) : UNRESOLVED_VECTOR_148218630;
    auto* config = v.FortHoagieVehicleConfigs;
    if (!config) return location;
    // The original compares all four root quaternion lanes against the cached quaternion.
    // If any differ it updates that cache and calls 0x142c5d000, storing its three-float
    // result into root+0x190. These writes and helper effects must be preserved.
    auto rotation = root ? UNRESOLVED_ROOT_ROTATION_WITH_CACHE_UPDATE(root)
                         : UNRESOLVED_ROTATION_148218640;
    auto offset = UNRESOLVED_142C5CCF0(rotation, UNRESOLVED_CONFIG_VECTOR_630(config));
    return location + offset;
}
// 0x14084c5c0: GetThrustDirection. Constants and the exact root virtual are unresolved.
Vec3 ThrustDirection(Vehicle& v, bool world) {
    if (!world) {
        float forward = UNRESOLVED_VIRTUAL_B08(v).FloatAt00;
        float side = UNRESOLVED_VIRTUAL_B08(v).FloatAt04;
        return UNRESOLVED_VECTOR_147C31CD0 * forward + UNRESOLVED_VECTOR_147C31CF0 * side;
    }
    auto* root = UNRESOLVED_VIRTUAL_7A0(v);
    if (!root) return UNRESOLVED_VECTOR_148218630;
    // 0x14084c5c0 expands quaternion rotation of each axis after scaling by root's
    // three scale floats at +0x1e0; preserve nonuniform scale, call order and FP order.
    auto scaledSide = UNRESOLVED_VECTOR_147C31CF0 * UNRESOLVED_ROOT_SCALE_1E0(root);
    auto scaledForward = UNRESOLVED_VECTOR_147C31CD0 * UNRESOLVED_ROOT_SCALE_1E0(root);
    auto rotation = UNRESOLVED_ROOT_QUATERNION_1C0(root);
    auto rotatedSide = UNRESOLVED_EXPANDED_QUAT_ROTATE(rotation, scaledSide);
    auto rotatedForward = UNRESOLVED_EXPANDED_QUAT_ROTATE(rotation, scaledForward);
    float forward = UNRESOLVED_VIRTUAL_B08(v).FloatAt00;
    float side = UNRESOLVED_VIRTUAL_B08(v).FloatAt04;
    return rotatedForward * forward + rotatedSide * side;
}
