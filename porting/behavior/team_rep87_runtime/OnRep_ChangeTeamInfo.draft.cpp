// Behavioral draft, not a generated/native build input. The external contracts
// below are names for call sites, not verified native signatures.
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AFortPlayerStateAthena.h"

namespace team_rep87 {
using State = AFortPlayerStateAthena;
struct Object;
struct Service;

// Accessors stand for observed storage and virtual calls, not replacement work.
extern Object* world_v150(State*);
extern bool is_player_state_142a8ae70(Object*); // native superclass-chain test
extern FString name_1444ac7a0(const void*);
extern Service* service_141e5b2a0();
extern FText lookup_141e5f780(Service*, const FGameplayTagContainer&);
// 142c0c460 returns a pointer used directly as 142c255f0's format argument;
// its actual return type, temporary ownership and destructor remain unresolved.
struct FormatView;
extern const FormatView& convert_142c0c460(const FText&);
extern FText format_142c255f0(const FormatView&, const FFormatNamedArguments&);
extern bool text_predicate_142c2e440(const FText&);
extern Object* game_state_141641350(Object* world);
extern void dispatch_virtual_a68(Object*, const FText&, int32);

void OnRep_ChangeTeamInfo(State* self)
{
    // The original exec executes P_FINISH before this body. +0x10 is the
    // ChangeTeamInfo tag container's first array count; it is tested for nonzero, not > 0.
    if (self->ChangeTeamInfo.ChangeTeamTags.GameplayTags.Num() == 0) return;
    Object* other = reinterpret_cast<Object*>(self->ChangeTeamInfo.Instigator);
    if (!other || !is_player_state_142a8ae70(other)) return;

    FText output; // 142c0c240
    FString victim_style(TEXT("Victim"));
    FString killer_style(TEXT("Killer"));
    FString victim_name = name_1444ac7a0(self);
    FString killer_name = name_1444ac7a0(other);
    const int32 victim_level = self->SeasonLevelUIDisplay;
    if (victim_level > 0) {
        FString replacement = FString::Printf(TEXT("%s (%d)"), *victim_name, victim_level);
        victim_name = MoveTemp(replacement);
    }
    const int32 killer_level = reinterpret_cast<State*>(other)->SeasonLevelUIDisplay;
    if (killer_level > 0) {
        FString replacement = FString::Printf(TEXT("%s (%d)"), *killer_name, killer_level);
        killer_name = MoveTemp(replacement);
    }

    // This standard UE map expresses the inlined sparse allocation, occupied
    // bit, hash insertion, key transfer and text-valued argument ownership.
    // Four distinct literal keys are inserted in this precise order. The
    // PlayerName insertion uses 1416b7390 instead of the first three call sites.
    FFormatNamedArguments args;
    {
        FText text = FText::FromString(killer_style);
        FFormatArgumentValue value(text);
        args.Add(FString(TEXT("KillerStyle")), value);
    }
    {
        FText text = FText::FromString(killer_name);
        FFormatArgumentValue value(text);
        args.Add(FString(TEXT("Killer")), value);
    }
    {
        FText text = FText::FromString(victim_style);
        FFormatArgumentValue value(text);
        args.Add(FString(TEXT("VictimStyle")), value);
    }
    {
        FText text = FText::FromString(victim_name);
        FFormatArgumentValue value(text);
        args.Add(FString(TEXT("PlayerName")), value);
    }
    {
        FText looked_up = lookup_141e5f780(service_141e5b2a0(), self->ChangeTeamInfo.ChangeTeamTags);
        const FormatView& view = convert_142c0c460(looked_up);
        FText formatted = format_142c255f0(view, args);
        // Native exchanges the text data words with the formatted temporary;
        // its flag handling needs verification before this becomes production.
        Swap(output, formatted);
    } // formatted then looked_up are destroyed before the virtual world call

    Object* world = world_v150(self);
    if (world && !text_predicate_142c2e440(output)) {
        Object* state = game_state_141641350(world);
        if (state) dispatch_virtual_a68(state, output, 0);
    }
    // Scope destruction: args, killer_name, victim_name, killer_style,
    // victim_style, output. UE text/map destruction releases their references.
}
} // namespace team_rep87
