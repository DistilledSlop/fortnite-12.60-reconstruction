	// Native NetDeltaSerialize recovered from 0x142a9ccc0: sends 1 bit + optional BasePlaylist object ref,
	// then packed bit length via SerializeInt(NumBits, MAX_int32) followed by the FastArrayDeltaSerialize payload bits.
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		struct FPlaylistPropertyBaseState : public INetDeltaBaseState
		{
			TSharedPtr<INetDeltaBaseState> InnerState;
			int32 PlaylistReplicationKey = INDEX_NONE;

			virtual bool IsStateEqual(INetDeltaBaseState* OtherState) override
			{
				FPlaylistPropertyBaseState* Other = static_cast<FPlaylistPropertyBaseState*>(OtherState);
				if (!Other || PlaylistReplicationKey != Other->PlaylistReplicationKey)
				{
					return false;
				}
				if (InnerState.IsValid() != Other->InnerState.IsValid())
				{
					return false;
				}
				return !InnerState.IsValid() || InnerState->IsStateEqual(Other->InnerState.Get());
			}
		};

		FPlaylistPropertyBaseState* OldCustom = static_cast<FPlaylistPropertyBaseState*>(DeltaParms.OldState);
		TSharedPtr<INetDeltaBaseState> OldInnerState = OldCustom ? OldCustom->InnerState : nullptr;
		TSharedPtr<INetDeltaBaseState> NewInnerState;

		FNetDeltaSerializeInfo LocalParms = DeltaParms;
		LocalParms.Writer = nullptr;
		LocalParms.Reader = nullptr;
		LocalParms.NewState = &NewInnerState;
		LocalParms.OldState = OldInnerState.Get();

		if (DeltaParms.Writer != nullptr)
		{
			const bool bWrotePlaylist = (OldCustom == nullptr) || (PlaylistReplicationKey != OldCustom->PlaylistReplicationKey);
			FNetBitWriter TempWriter(DeltaParms.Map, 0);
			LocalParms.Writer = &TempWriter;
			const bool bWroteArray = FFastArraySerializer::FastArrayDeltaSerialize<FPropertyOverride, FPlaylistPropertyArray>(PropertyOverrides, LocalParms, *this);
			if (!bWrotePlaylist && !bWroteArray)
			{
				if (OldCustom && DeltaParms.NewState)
				{
					*DeltaParms.NewState = OldCustom->AsShared();
				}
				return false;
			}
			if (DeltaParms.NewState)
			{
				TSharedPtr<FPlaylistPropertyBaseState> NewCustom = MakeShared<FPlaylistPropertyBaseState>();
				NewCustom->PlaylistReplicationKey = PlaylistReplicationKey;
				NewCustom->InnerState = NewInnerState;
				*DeltaParms.NewState = NewCustom;
			}
			DeltaParms.Writer->WriteBit(bWrotePlaylist ? 1 : 0);
			if (bWrotePlaylist)
			{
				UObject* BasePlaylistObj = reinterpret_cast<UObject*>(BasePlaylist);
				*DeltaParms.Writer << BasePlaylistObj;
			}
			uint32 NumBits = static_cast<uint32>(TempWriter.GetNumBits());
			DeltaParms.Writer->SerializeInt(NumBits, MAX_int32);
			if (NumBits > 0)
			{
				DeltaParms.Writer->SerializeBits(TempWriter.GetData(), NumBits);
			}
			return true;
		}

		if (DeltaParms.Reader != nullptr)
		{
			const bool bHasPlaylist = (DeltaParms.Reader->ReadBit() != 0);
			if (bHasPlaylist)
			{
				UObject* BasePlaylistObj = reinterpret_cast<UObject*>(BasePlaylist);
				*DeltaParms.Reader << BasePlaylistObj;
				BasePlaylist = reinterpret_cast<UFortPlaylistAthena*>(BasePlaylistObj);
				++PlaylistReplicationKey;
			}
			uint32 NumBits = 0;
			DeltaParms.Reader->SerializeInt(NumBits, MAX_int32);
			if (NumBits != 0)
			{
				LocalParms.Reader = DeltaParms.Reader;
				FFastArraySerializer::FastArrayDeltaSerialize<FPropertyOverride, FPlaylistPropertyArray>(PropertyOverrides, LocalParms, *this);
				DeltaParms.bGuidListsChanged = LocalParms.bGuidListsChanged;
				DeltaParms.bOutSomeObjectsWereMapped = LocalParms.bOutSomeObjectsWereMapped;
				DeltaParms.bOutHasMoreUnmapped = LocalParms.bOutHasMoreUnmapped;
			}
			return true;
		}

		const bool bResult = FFastArraySerializer::FastArrayDeltaSerialize<FPropertyOverride, FPlaylistPropertyArray>(PropertyOverrides, LocalParms, *this);
		DeltaParms.bGuidListsChanged = LocalParms.bGuidListsChanged;
		DeltaParms.bOutSomeObjectsWereMapped = LocalParms.bOutSomeObjectsWereMapped;
		DeltaParms.bOutHasMoreUnmapped = LocalParms.bOutHasMoreUnmapped;
		return bResult;
	}
