    // Original +310; +360 remains the existing NativeActionRecords map.
    struct FNativeActionBindingKeyFuncs : BaseKeyFuncs<TPair<FDataTableRowHandle, int32>, FDataTableRowHandle, false>
    {
        using ElementInitType = TTypeTraits<TPair<FDataTableRowHandle, int32>>::ConstInitType;
        using KeyInitType = TTypeTraits<FDataTableRowHandle>::ConstInitType;
        static KeyInitType GetSetKey(ElementInitType Element) { return Element.Key; }
        static bool Matches(KeyInitType A, KeyInitType B) { return A == B; }
        static uint32 GetKeyHash(KeyInitType Key)
        { return HashCombine(HashCombine(0, GetTypeHash(Key.RowName)), PointerHash(Key.DataTable)); }
    };
    TMap<FDataTableRowHandle, int32, FDefaultSetAllocator, FNativeActionBindingKeyFuncs> NativeActionBindingRows;
