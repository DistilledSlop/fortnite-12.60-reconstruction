// Uncompiled control-flow lift of 0x142a7dc50. Adapter names and types below
// describe unresolved operations, not an implemented API. See accompanying handoff.
// P_FINISH occurs in the exec wrapper before this body.
namespace cosmetic_loadout_rough {

// RequestHandle is the two-word object/control-block pair at the listed offsets.
// LoadoutHandle offsets are relative to CosmeticLoadout, never to the pawn base.
// Transfer must follow the original alias and control-block comparisons exactly.
static void TransferRequest(RequestHandle& destination, RequestHandle& returned,
                            RequestHandle& outputTemporary)
{
    if (&destination != &returned) {
        destination.object = returned.object;
        returned.object = nullptr;
        auto* oldControl = destination.control;
        if (oldControl != returned.control) {
            destination.control = returned.control;
            returned.control = nullptr;
            ReleaseStrongThenWeakIfZero(oldControl); // null-safe; original non-atomic counts
        }
    }
    ReleaseStrongThenWeakIfZero(outputTemporary.control);
}

// The three single-ID paths share the same allocation and manager call; do not
// add the pending-handle guard to the +0x20 path.
static void RequestSingle(AFortPlayerPawn* pawn, void* item, size_t handleOffset,
                          CallbackAddress callback, StackHighBits stackHigh)
{
    auto id = ItemVtable190_GetId(item); // 16-byte value, vtable byte offset 400
    IdArray ids;
    ids.Add(id);
    BundleArray bundles;
    bundles.Add(Global_14812b6e0);
    auto* manager = FUN_141df62a0();
    auto delegate = Make48ByteWeakPawnDelegate(pawn, callback); // FUN_142ee3b30, FUN_142bad740
    auto result = ManagerVtable340(manager, ids, bundles, stackHigh.WithLowByte(0),
                                   Move(delegate), 100);
    TransferRequest(LoadoutHandle(pawn->CosmeticLoadout, handleOffset), *result.returned, result.outputTemporary);
    bundles.FreeStorage();
    ids.FreeStorage();
}

void OnRep_CosmeticLoadout(AFortPlayerPawn* pawn)
{
    auto& loadout = pawn->CosmeticLoadout;
    LoadoutByte(loadout, 0xf0) = 1;
    const bool mode = FUN_141ed8de0(pawn, 0) != 0;
    FUN_141d60a30(loadout);

    VariantAssetArray variantAssets; // elements 24 bytes; owned field at +8
    VariantContext variantContext;   // initial -1, null arrays, zero and low flag byte 0
    InitVariantContext(variantContext);
    FUN_142ee3b30(variantContext, pawn);
    FUN_141d45af0(loadout, pawn, variantContext, variantAssets);
    if (variantAssets.Num() > 0) {
        auto& handle = LoadoutHandle(loadout, 0xf8);
        if (!handle.object || HandleByte(handle.object, 0x12) != 0 ||
                              HandleByte(handle.object, 0x11) != 0) {
            OwnedString requestName = MakeString(L"VariantAssets");
            auto* manager = FUN_141df62a0();
            auto delegate = Make48ByteWeakPawnDelegate(pawn, FUN_14230efb0);
            auto result = ManagerVtable348(manager, variantAssets, Move(delegate), 100,
                                            requestName, 0);
            TransferRequest(handle, *result.returned, result.outputTemporary);
            requestName.FreeStorage();
        }
    }
    variantContext.FreeArrayAt90();
    variantContext.FreeArrayAtA8();
    for (auto& entry : variantAssets) entry.FreeOwnedFieldAt8();
    variantAssets.FreeStorage();

    void* primary = LoadoutObject(loadout, 0x40);
    auto& primaryHandle = LoadoutHandle(loadout, 0x108);
    if (primary && (!primaryHandle.object || HandleByte(primaryHandle.object, 0x12) != 0 ||
                                             HandleByte(primaryHandle.object, 0x11) != 0)) {
        IdArray ids;
        ids.Add(ItemVtable190_GetId(primary));
        if (!mode || RuntimeOptionsByte(UFortRuntimeOptions_GetDefault(), 0x7fe) != 0) {
            for (void* additional : LoadoutObjectArray(loadout, 0xd8, 0xe0)) {
                if (additional) ids.Add(ItemVtable190_GetId(additional));
            }
        }
        BundleArray bundles;
        bundles.Add(Global_14812b6e0);
        NameArray primaryNamesA, primaryNamesB;
        ItemSubobjectVtable10_GetNames(primary, 0x30, primaryNamesA, primaryNamesB);
        NameArray pawnNamesA, pawnNamesB;
        FUN_142318c50(pawn, pawnNamesA, pawnNamesB, primary);
        FUN_141ed2350(); // return ignored, call is observable
        auto* config = FUN_141ed1800();
        auto firstName = ConfigName(config, 0x1978);
        bool expand = !IsNoneName(firstName) &&
                      (primaryNamesA.Contains(firstName) || primaryNamesB.Contains(firstName));
        if (!expand) {
            config = FUN_141ed1800();
            auto secondName = ConfigName(config, 0x1a40);
            expand = !IsNoneName(secondName) &&
                     (pawnNamesA.Contains(secondName) || pawnNamesB.Contains(secondName));
        }
        if (expand) {
            ExtraAssetArray extras;
            ItemVtable4D8_GetExtraAssets(LoadoutObject(loadout, 0x40), extras);
            for (auto& extra : extras) {
                // Copies fixed fields and owned data, then derives a manager lookup key.
                ExtraLookupTemporary temporary(extra); // FUN_140682ce0, FUN_142ebd9e0
                auto* manager = FUN_141df62a0();
                auto extraId = ManagerVtable2D0_LookupId(manager, temporary.key);
                if (!ids.ContainsExactFourWords(extraId)) ids.Add(extraId);
                temporary.FreeArrayAt98();
            }
            for (auto& extra : extras) extra.FreeOwnedFieldAt18();
            extras.FreeStorage();
        }
        if (ids.Num() > 0) {
            auto* manager = FUN_141df62a0();
            auto delegate = Make48ByteWeakPawnDelegate(pawn, FUN_14230eea0);
            auto result = ManagerVtable340(manager, ids, bundles, StackHighBits().WithLowByte(0),
                                           Move(delegate), 100);
            TransferRequest(primaryHandle, *result.returned, result.outputTemporary);
        }
        if (GlobalByte(FUN_141ed2350(), 0x210) != 0)
            FUN_141d5e260(LoadoutObject(loadout, 0x40), pawn);
        pawnNamesB.FreeStorage();
        pawnNamesA.FreeStorage();
        primaryNamesB.FreeStorage();
        primaryNamesA.FreeStorage();
        bundles.FreeStorage();
        ids.FreeStorage();
    }

    void* item68 = LoadoutObject(loadout, 0x68);
    auto& handle118 = LoadoutHandle(loadout, 0x118);
    if (item68 && (!handle118.object || HandleByte(handle118.object, 0x12) != 0 ||
                                        HandleByte(handle118.object, 0x11) != 0))
        RequestSingle(pawn, item68, 0x118, FUN_14230eea0, StackHighBits());

    void* item30 = LoadoutObject(loadout, 0x30);
    auto& handle128 = LoadoutHandle(loadout, 0x128);
    if (item30 && (!handle128.object || HandleByte(handle128.object, 0x12) != 0 ||
                                        HandleByte(handle128.object, 0x11) != 0))
        RequestSingle(pawn, item30, 0x128, FUN_14230eea0, StackHighBits());

    void* item20 = LoadoutObject(loadout, 0x20);
    if (item20 && !mode)
        RequestSingle(pawn, item20, 0x138, FUN_14230ef30, StackHighBits());

    FUN_142344af0(pawn);
    FUN_142305850(pawn);
    auto* owner = PawnPointerAt(pawn, 0x250); // _gap_0x8 + 0x248; verify absolute layout
    if (owner) {
        auto* klass = FUN_142a6ee20();
        if (OwnerClassIndex(owner) >= ClassIndex(klass) &&
            OwnerClassAncestorAt(owner, ClassIndex(klass)) == ClassAncestorToken(klass))
            OwnerSubobjectVtable20(owner, 0x5a8, loadout);
    }
}
} // namespace cosmetic_loadout_rough
