#include "FortAthenaLootRoll.h"
#include "FortAthenaLootData.h"
#include "FFortLootTierData.h"
#include "FFortLootPackageData.h"
#include "UFortItemDefinition.h"
#include <cmath>

namespace
{
    using namespace FortAthenaLootRoll;
    struct FEngine
    {
        const FRequest& Q;
        const FServices& S;
        FString& Error;
        TArray<FDrop> Drops;
        TSet<FName> Stack;
        int32 Decisions = 0;
        bool Fail(const FString& Why)
        {
            if (Error.IsEmpty()) Error = Why;
            return false;
        }
        bool Tick()
        {
            return ++Decisions <= Q.MaxDecisions || Fail(TEXT("decision budget exhausted"));
        }
        bool Bounds(int32 Min, int32 Max, bool& Match)
        {
            if (Min < -1 || Max < -1 || (Min >= 0 && Max >= 0 && Min > Max)) return Fail(TEXT("invalid world bounds"));
            Match = (Min == -1 || Q.WorldLevel >= Min) && (Max == -1 || Q.WorldLevel <= Max);
            return true;
        }
        bool Draw(double& Value)
        {
            if (!Tick()) return false;
            Value = S.Random01();
            return (std::isfinite(Value) && Value >= 0 && Value < 1) || Fail(TEXT("RNG must return finite [0,1)"));
        }
        bool Choose(const TArray<double>& Weights, int32& Index)
        {
            double Sum = 0;
            for (double W : Weights)
            {
                if (!std::isfinite(W) || W < 0) return Fail(TEXT("invalid selection weight"));
                Sum += W;
            }
            if (!std::isfinite(Sum) || Sum <= 0) return Fail(TEXT("no positive-weight candidates"));
            double R;
            if (!Draw(R)) return false;
            const double Target = R * Sum;
            double End = 0;
            Index = INDEX_NONE;
            for (int32 I = 0; I < Weights.Num(); ++I)
            {
                if (Weights[I] <= 0) continue;
                Index = I; End += Weights[I];
                if (Target < End) return true; // half-open intervals, exact boundary selects next.
            }
            // Floating multiplication may round a value just below 1 to Sum.
            return Index != INDEX_NONE;
        }
        bool TierSupported(const FFortLootTierData& T)
        {
            if (T.QuotaLevel != ELootQuotaLevel::Unlimited || !T.StreakBreakerCurrency.IsEmpty() ||
                T.StreakBreakerPointsMin || T.StreakBreakerPointsMax || T.StreakBreakerPointsSpend)
                return Fail(TEXT("unsupported tier quota/streak contract"));
            if (!T.GameplayTags.IsEmpty() || !T.RequiredGameplayTags.IsEmpty()) return Fail(TEXT("unsupported tier gameplay-tag contract"));
            return true;
        }
        bool PackageSupported(const FFortLootPackageData& P)
        {
            if (!P.NamedWeightMult.IsNone() || P.PotentialNamedWeights.Num()) return Fail(TEXT("unsupported named-weight contract"));
            if (!P.GameplayTags.IsEmpty() || !P.RequiredTag.IsNone()) return Fail(TEXT("unsupported package gameplay-tag contract"));
            if (!P.PersistentLevel.IsEmpty()) return Fail(TEXT("unsupported persistent-level contract"));
            if (P.Count < 0) return Fail(TEXT("negative package Count"));
            if (!P.LootPackageCall.IsEmpty() && !P.ItemDefinition.IsNull()) return Fail(TEXT("ambiguous package call and terminal item"));
            return true;
        }
        bool Candidates(FName Id, TArray<const FFortLootPackageData*>& Rows)
        {
            TArray<const FFortLootPackageData*> All;
            S.FindPackages(Id, All);
            for (const FFortLootPackageData* P : All)
            {
                if (!P) return Fail(TEXT("null package row"));
                bool Match;
                if (!Bounds(P->MinWorldLevel, P->MaxWorldLevel, Match)) return false;
                if (!Match) continue;
                if (!std::isfinite(P->Weight) || P->Weight < 0) return Fail(TEXT("invalid package weight"));
                if (P->Weight == 0) continue;
                if (!PackageSupported(*P)) return false;
                Rows.Add(P);
            }
            return true;
        }
        bool Package(FName Id, int32 Category, int32 Depth)
        {
            if (!Tick()) return false;
            if (Depth > Q.MaxDepth) return Fail(TEXT("package recursion depth exhausted"));
            if (Stack.Contains(Id)) return Fail(FString::Printf(TEXT("package cycle at %s"), *Id.ToString()));
            TArray<const FFortLootPackageData*> Rows;
            if (!Candidates(Id, Rows)) return false;
            TArray<double> Weights;
            for (const auto* P : Rows) Weights.Add(Category == -1 || P->LootPackageCategory == Category ? P->Weight : 0.0);
            int32 Index;
            if (!Choose(Weights, Index)) return false;
            const auto& P = *Rows[Index];
            UE_LOG(LogTemp, Display, TEXT("LootRoll: group=%s package=%s annotation=%s count=%d category=%d"),
                *Q.Group.ToString(), *Id.ToString(), *P.Annotation, P.Count, Category);
            if (P.Count == 0) return true; // Explicit authored no-drop; no fallback.
            if (!P.LootPackageCall.IsEmpty())
            {
                Stack.Add(Id);
                for (int32 N = 0; N < P.Count; ++N)
                    if (!Package(FName(*P.LootPackageCall), -1, Depth + 1)) { Stack.Remove(Id); return false; }
                Stack.Remove(Id);
                return true;
            }
            if (P.ItemDefinition.IsNull()) return Fail(TEXT("positive Count without item or package call"));
            UFortItemDefinition* Item = P.ItemDefinition.Get();
            if (!Item) return Fail(FString::Printf(TEXT("terminal item not preloaded: %s"), *P.ItemDefinition.ToSoftObjectPath().ToString()));
            FDrop Drop; Drop.Definition = Item; Drop.Count = P.Count;
            if (!S.ResolveItemLevel(Item, Q.LootLevel, Drop.Level) || Drop.Level < 0) return Fail(TEXT("item-level resolver rejected terminal item"));
            Drops.Add(Drop);
            return true;
        }
        bool Run()
        {
            if (Q.bRequestBonus) return Fail(TEXT("unsupported bonus trigger contract"));
            FName Group;
            if (!S.ResolveGroup(Q.Group, Group)) return Fail(TEXT("group redirect failed"));
            TArray<const FFortLootTierData*> All, Rows;
            S.FindTiers(Group, All);
            TArray<double> Weights;
            for (const auto* T : All)
            {
                if (!T) return Fail(TEXT("null tier row"));
                bool Match;
                if (!Bounds(T->MinWorldLevel, T->MaxWorldLevel, Match)) return false;
                if (!Match || (Q.LootTier != -1 && T->LootTier != Q.LootTier)) continue;
                if (!std::isfinite(T->Weight) || T->Weight < 0) return Fail(TEXT("invalid tier weight"));
                if (T->Weight == 0) continue;
                if (!TierSupported(*T)) return false;
                Rows.Add(T); Weights.Add(T->Weight);
            }
            int32 Index;
            if (!Choose(Weights, Index)) return false;
            const auto& T = *Rows[Index];
            if (!std::isfinite(T.NumLootPackageDrops) || T.NumLootPackageDrops < 0 || T.NumLootPackageDrops > Q.MaxDecisions)
                return Fail(TEXT("invalid package drop total"));
            int32 Total = static_cast<int32>(T.NumLootPackageDrops);
            const double Fraction = static_cast<double>(T.NumLootPackageDrops) - Total;
            if (Fraction > 0) { double R; if (!Draw(R)) return false; if (R < Fraction) ++Total; }
            UE_LOG(LogTemp, Display, TEXT("LootRoll: group=%s resolved=%s tier=%d annotation=%s total=%d"),
                *Q.Group.ToString(), *Group.ToString(), T.LootTier, *T.Annotation, Total);
            const int32 Cats = T.LootPackageCategoryWeightArray.Num();
            if (!Cats || T.LootPackageCategoryMinArray.Num() != Cats || T.LootPackageCategoryMaxArray.Num() != Cats)
                return Fail(TEXT("invalid category array shape"));
            TArray<int32> Used; Used.Init(0, Cats);
            int64 Minimum = 0;
            for (int32 C = 0; C < Cats; ++C)
            {
                int32 Min = T.LootPackageCategoryMinArray[C], Max = T.LootPackageCategoryMaxArray[C];
                if (Min < 0 || Max < -1 || (Max != -1 && Min > Max) || T.LootPackageCategoryWeightArray[C] < 0)
                    return Fail(TEXT("invalid category constraints"));
                Minimum += Min;
            }
            if (Minimum > Q.MaxDecisions) return Fail(TEXT("category minima exceed budget"));
            // Minima are guaranteed; NumLootPackageDrops supplies a target, not a cap.
            Total = FMath::Max(Total, static_cast<int32>(Minimum));
            for (int32 C = 0; C < Cats; ++C)
                for (int32 N = 0; N < T.LootPackageCategoryMinArray[C]; ++N)
                { if (!Package(T.LootPackage, C, 0)) return false; ++Used[C]; }
            TArray<const FFortLootPackageData*> RootRows;
            if (!Candidates(T.LootPackage, RootRows)) return false;
            for (int32 N = static_cast<int32>(Minimum); N < Total; ++N)
            {
                TArray<double> CW;
                for (int32 C = 0; C < Cats; ++C)
                {
                    bool Has = false;
                    for (const auto* P : RootRows) if (P->LootPackageCategory == C) { Has = true; break; }
                    int32 Max = T.LootPackageCategoryMaxArray[C];
                    CW.Add(Has && (Max == -1 || Used[C] < Max) ? T.LootPackageCategoryWeightArray[C] : 0.0);
                }
                int32 C;
                if (!Choose(CW, C) || !Package(T.LootPackage, C, 0)) return false;
                ++Used[C];
            }
            return true;
        }
    };
}

bool FortAthenaLootRoll::Roll(const FRequest& Request, const FServices& Services, TArray<FDrop>& Out, FString& Error)
{
    Out.Reset(); Error.Reset();
    if (!Services.ResolveGroup || !Services.FindTiers || !Services.FindPackages || !Services.Random01 || !Services.ResolveItemLevel)
    { Error = TEXT("missing roll service"); return false; }
    if (Request.Group.IsNone() || Request.WorldLevel < 0 || Request.LootTier < -1 || Request.LootLevel < 0 || Request.MaxDepth < 0 || Request.MaxDecisions < 1 || Request.MaxDecisions > 1000000)
    { Error = TEXT("invalid roll request"); return false; }
    FEngine Engine{Request, Services, Error};
    if (!Engine.Run())
    {
        UE_LOG(LogTemp, Warning, TEXT("LootRoll: group=%s failed=%s"), *Request.Group.ToString(), *Error);
        return false;
    }
    Out = MoveTemp(Engine.Drops);
    return true;
}

bool FortAthenaLootRoll::RollForMode(const AFortGameModeAthena* Mode, const FRequest& Request,
    TFunction<double()> Random01, TFunction<bool(UFortItemDefinition*, int32, int32&)> ResolveItemLevel,
    TArray<FDrop>& Out, FString& Error)
{
    if (!FortAthenaLootData::IsReady(Mode)) { Out.Reset(); Error = TEXT("loot data not ready"); return false; }
    FServices Services;
    Services.ResolveGroup = [Mode](FName In, FName& Result) { return FortAthenaLootData::ResolveGroup(Mode, In, Result); };
    Services.FindTiers = [Mode](FName Group, TArray<const FFortLootTierData*>& Rows) { FortAthenaLootData::FindTiers(Mode, Group, Rows); };
    Services.FindPackages = [Mode](FName Id, TArray<const FFortLootPackageData*>& Rows) { FortAthenaLootData::FindPackages(Mode, Id, Rows); };
    Services.Random01 = MoveTemp(Random01); Services.ResolveItemLevel = MoveTemp(ResolveItemLevel);
    return Roll(Request, Services, Out, Error);
}
