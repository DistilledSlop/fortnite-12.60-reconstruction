// UNCOUNTED DRAFT - 0x1419dbc20 (355-line decompile; wrapper 0x142a564c0 steps no params and calls this).
// void AFortPlayerControllerAthena::CheckIfLevelStreamingCompleteForHandshake()
// Flow: world = GetWorld() (UObject virtual +0x150). Block 1 (no RespawnCameraActor or global 0x14811365c == 0): any streaming level that is
// not FailedToLoad and still pending => return. Block 2: iterate a filtered object list; for each actor's streaming object that is pending
// and within 10000uu (2D distance from the camera to the actor's bounds) => return. If nothing blocks: walk the listener array at +0x2da0
// from the back, stopping at the first listener that returns false.
#include "Unresolved97.h"

namespace
{
	template <typename T> FORCEINLINE T& Hs_Raw(void* Base, uint32 Off) { return *reinterpret_cast<T*>(reinterpret_cast<uint8*>(Base) + Off); }

	enum EHsIterFlags : uint32 { Hs_SkipPendingKill = 1, Hs_Bit1_StubGated = 2, Hs_Bit2_ActorStateFilter = 4 };

	// Raw stand-in for the stack iterator at lStack_a8..; layout from the decompile. Constructor/destructor bodies are unresolved.
	struct FRawObjectIterState
	{
		UWorld*  Root = nullptr;       // lStack_a8
		UObject** ChunkA = nullptr;    // puStack_a0, CountA = iStack_98
		int32    CountA = 0;
		UObject** ChunkB = nullptr;    // puStack_78, CountB = iStack_70
		int32    CountB = 0;
		int32    Index = -1;           // iStack_90 (ctor-initialised; native scans from Index+1)
		bool     bDone = false;        // cStack_8c
		UObject* Current = nullptr;    // lStack_80
		int32    Visited = 0;          // iStack_88
		uint32   Flags = Hs_SkipPendingKill | Hs_Bit2_ActorStateFilter; // uStack_b0 = 5
		uint64   Token = 0;            // uStack_60 (second arg of the dtor helper)
		bool     bConstructed = false; // cStack_58
		~FRawObjectIterState()
		{
			if (bConstructed) { bConstructed = false; Unresolved_ObjectIterDtor_14469a960(this); FMemory::Free(ChunkB); FMemory::Free(ChunkA); }
		}
	};

	// The three duplicated scan loops of the native are this one function (compiler inlining). Accept rules in native order.
	bool Hs_Advance(FRawObjectIterState& It)
	{
		for (int32 i = It.Index + 1; i < It.CountA + It.CountB; ++i)
		{
			UObject* Obj = (i < It.CountA) ? It.ChunkA[i] : It.ChunkB[i - It.CountA];
			++It.Visited;
			if (!Obj) continue;
			UObject* Actor = Unresolved_ObjectItemResolveActor_143f76730(Obj);
			if (!Actor) continue;
			if ((It.Flags & Hs_SkipPendingKill) && Obj->HasAnyInternalFlags(EInternalObjectFlags::PendingKill)) continue; // FUObjectItem flags bit 29
			if ((It.Flags & Hs_Bit1_StubGated) && !Unresolved_IterStub_ReturnFalse()) continue; // gate: bit set and stub false => skip
			if (It.Flags & Hs_Bit2_ActorStateFilter)
			{
				// byte@+0x1f4 & 0x20 or byte@+0x1f5 & 0x40 must be set; state object @+0x248 (first byte==2 or null) / @+0xb8 test must pass
				const bool bFlagA = (Hs_Raw<uint8>(Actor, 0x1f4) & 0x20) || (Hs_Raw<uint8>(Actor, 0x1f5) & 0x40);
				UObject* StateObj = Hs_Raw<UObject*>(Actor, 0x248);
				bool bB = (Hs_Raw<UObject*>(Actor, 0xb8) == nullptr);
				if (!bB) { UObject* Ref = Unresolved_ActorStateRef_14468e190(); bB = (Ref == nullptr) || (StateObj == Ref); }
				const bool bC = (StateObj == nullptr) || (Hs_Raw<uint8>(StateObj, 0) == 2);
				if (!bFlagA || !(bB || bC)) continue;
			}
			if (Unresolved_ActorResolveIterWorld_1413064f0(Actor) != It.Root) continue;
			if (Actor != Hs_Raw<UObject*>(It.Root, 0x30)) // World->PersistentLevel
			{
				if (Obj->GetClass()->IsChildOf(Unresolved_StaticClass_14480eb00())) continue;
			}
			It.Index = i; It.Current = Obj;
			return true;
		}
		It.bDone = true; It.Current = nullptr; // LAB_1419dbed3
		return false;
	}

	FORCEINLINE bool Hs_LevelStatePending_Block1(UObject* L)
	{
		return (!Unresolved_LevelStreamingPredicate_1442dd540(L) && Unresolved_LevelStreaming_vt270(L))
			|| (Hs_Raw<UObject*>(L, 0x128) != nullptr && !Unresolved_LevelStreaming_vt268(L));
	}
}

void AFortPlayerControllerAthena::CheckIfLevelStreamingCompleteForHandshake()
{
	UWorld* World = GetWorld();
	if (!World) return;

	AActor* Cam = RespawnCameraActor; // +0x2e70 reflected
	if (!Cam || Unresolved_Global_14811365c == 0)
	{
		// World+0x98/+0xa0 = StreamingLevels
		for (ULevelStreaming* L : World->GetStreamingLevels())
		{
			if (L && Hs_Raw<uint8>(L, 0xb8) != 2 /* ECurrentState::FailedToLoad */ && Hs_LevelStatePending_Block1(L)) return;
		}
	}
	else
	{
		// camera X/Y: root component (+0x130) location at +0x1d0 (12 bytes) else the global zero vector at 0x148218630/38
		FVector CamLoc(0.f, 0.f, 0.f);
		if (USceneComponent* Root = Cam->GetRootComponent()) CamLoc = Hs_Raw<FVector>(Root, 0x1d0);
		const float X = CamLoc.X, Y = CamLoc.Y;

		// iterate class: A = class 0x14273ec00 (called twice); kept only when A IsChildOf the second call's result
		UClass* A = Unresolved_StaticClass_14273ec00(); UClass* B = Unresolved_StaticClass_14273ec00();
		UClass* IterClass = (A && B && A->IsChildOf(B)) ? A : nullptr;

		{
			FRawObjectIterState It;
			Unresolved_ObjectIterCtor_140894360(&It, World, IterClass);
			It.bConstructed = true;
			while (Hs_Advance(It))
			{
				if (It.bDone) break;
				UObject* Streaming = Hs_Raw<UObject*>(It.Current, 0xda0); // per-actor streaming object
				if (Streaming && Hs_Raw<uint8>(Streaming, 0xb8) != 2)
				{
					if (Hs_Raw<UObject*>(Streaming, 0x128) != nullptr && !Unresolved_LevelStreaming_vt268(Streaming)) return;
					if (Unresolved_LevelStreaming_vt270(Streaming) && !Unresolved_LevelStreamingPredicate_1442dd540(Streaming))
					{
						const FBox& Box = *Unresolved_ActorStreamingBounds_141bdf240(It.Current);
						float D2 = 0.f;
						if (X < Box.Min.X || Box.Max.X < X) { const float E = X - (X < Box.Min.X ? Box.Min.X : Box.Max.X); D2 = E * E; }
						if (Y < Box.Min.Y || Box.Max.Y < Y) { const float E = Y - (Y < Box.Min.Y ? Box.Min.Y : Box.Max.Y); D2 += E * E; }
						if (D2 <= 10000.0f * 10000.0f) return; // threshold constant at 0x1478c283c = 10000.0f
					}
				}
			}
		} // iterator destroyed before the listener walk, as in native
	}

	// Listener walk (array of 16-byte {Object, Interface} entries at +0x2da0, Num at +0x2da8), newest first.
	TArray<FRawInterfaceRef>& Listeners = Hs_Raw<TArray<FRawInterfaceRef>>(this, 0x2da0);
	int32 i = Listeners.Num();
	while (--i >= 0)
	{
		const FRawInterfaceRef& E = Listeners[i];
		const bool bValid = (reinterpret_cast<uintptr_t>(E.Interface) & 0xffffffffu) != 0 && E.Object && Unresolved_Listener_vt28(E.Object);
		if (bValid)
		{
			UObject* Obj = (reinterpret_cast<uintptr_t>(Listeners[i].Interface) & 0xffffffffu) ? Listeners[i].Object : nullptr; // native re-reads the entry
			if (!Unresolved_Listener_vt48(Obj, this)) break;
		}
		if (i < 0 || i >= Listeners.Num()) i = Listeners.Num(); // callbacks may have mutated the array: restart from the (new) end
	}
}
