// Behavioral sketches, not compilable: U_ names identify unresolved native calls/ABI.
// Arguments are in the generated declaration order; the improved exec decompiler
// assigns several parameter names in reverse. Struct copies have normal RAII here.

bool UpdateCreativePlotData_Validate(Volume* volume, CreativeIslandInfo info) {
    if (!volume) return false;
    // FString Num includes the terminating null. An empty FString has Num == 0.
    int title = info.IslandTitle.Num() ? info.IslandTitle.Num() - 1 : 0;
    int introduction = info.IslandIntroduction.Num() ? info.IslandIntroduction.Num() - 1 : 0;
    int locale = info.UserLocale.Num() ? info.UserLocale.Num() - 1 : 0;
    bool valid = title >= 3 && title <= 40 && introduction <= 1024 && locale <= 10;
    if (!valid) U_log_invalid_island_info_if_category_level_gt_3(info);
    return valid;
}
void UpdateCreativePlotData_Implementation(Volume*, CreativeIslandInfo) {
    // Native body only destroys the by-value copy of its three FString fields.
}

bool PublishCreativePlot_Validate(Volume* volume, CreativePublishOptions options) {
    if (!volume) return false;
    auto tags = U_1419d9d90_copy_or_transform_tags(options.DescriptionTags);
    U_140a1aad0_assign_tags(options.DescriptionTags, tags);
    return U_1419ddfd0_validate_publish_options(options);
}
void PublishCreativePlot_Implementation(Volume*, CreativePublishOptions) {
    // Native body only destroys the by-value copy, including every tag string.
}

bool SendPhysicsBallHitToServer_Validate(/* original six RPC arguments */) { return true; }
void SendPhysicsBallHitToServer_Implementation(/* original six RPC arguments */) {}
// The reflected exec validates first, invokes RPC_ValidateFailed on false, and
// otherwise calls the virtual implementation. Exact native signature is absent.

void OnRep_CreativeIslands(Controller* self) {
    if (self->CreativeIslands.Num() == 0) {
        U_1406a9dc0_broadcast(self->OnIslandEntriesChanged);
        return;
    }
    auto* runtime = UFortRuntimeOptions_GetDefault();
    for (auto& island : self->CreativeIslands) {
        // 20-byte shared/weak-like value at island + 0x10; helper takes runtime
        // and fields at runtime + 0x9b0 / + 0x9c0. Previous value is released.
        island.unknown_10 = U_142705ec0(runtime, island.unknown_10,
                                       runtime->unknown_9b0, runtime->unknown_9c0);
    }
    auto contextId = U_14447ee40(self) ? U_14447ee40(self)->field_b8 : 0;
    auto online = U_OnlineSubsystem_interface_and_local_user(contextId);
    if (!online) {
        U_1406a9dc0_broadcast(self->OnIslandEntriesChanged);
        return;
    }
    Array<String> ids;
    for (auto& island : self->CreativeIslands) {
        ids.Add(U_142c40540_string_from_value(island.unknown_10));
        island.unknown_10 = U_142c0c240_default_value();
    }
    auto callback = U_construct_delegate_with_weak_controller(self, U_142bad740());
    U_online_vtable_10(online, ids, callback); // exact ABI and callback effects unresolved
    // Temporary strings, delegate, and shared interface references released here.
    // No immediate OnIslandEntriesChanged broadcast on this path.
}

void OnServerMarkerAdded(Controller* self, WorldMarkerData marker) {
    WorldMarkerData ownedCopy = marker; // string deep copy, ref-counted value retained
    U_1419e8930(self, ownedCopy); // consumes or destroys this owned copy; effect unresolved
    // Original reflected argument's string, shared value, and other owned members released.
}

void OnMinigameEnded(Controller* self) {
    U_140684de0(self->field_3548); // delegate at controller + 0x3548
    U_1423c06c0(self);
    self->vfn_1a60();
    self->vfn_1a68();
    auto* world = self->vfn_150();
    auto* gs = world ? world->field_130 : nullptr;
    if (gs && U_is_AFortGameStateAthena(gs)) {
        if (U_14199e3c0(gs)) U_vfn_1f68(self, U_config_field_24d0(), false, "Creative mini game ended");
        if (U_14199e830(gs)) U_vfn_1f68(self, U_config_field_24d8(), false, "Playground mini game ended");
        if (U_14199e380(gs)) U_vfn_1f68(self, U_config_field_24e0(), false, "Creative LTM mini game ended");
        if (U_14199e2c0(gs)) U_vfn_1f68(self, U_config_field_24e8(), false, "Battle Lab mini game ended");
    }
    U_1423e5ec0(self);
}
