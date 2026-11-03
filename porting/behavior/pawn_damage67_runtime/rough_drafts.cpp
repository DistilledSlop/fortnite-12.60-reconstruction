// Behavioral reconstruction, NOT a drop-in translation unit. Adapter operations below
// are unresolved external effects; names containing offsets are evidence, not SDK names.
// Cue parameters are passed by value by the native ABI; their destructor must run on
// every exit. Virtual calls here deliberately remain virtual adapter calls.
#include <cstdint>
struct AFortPawn; struct UObject; struct FGameplayCueParameters;
struct FGameplayEffectContextHandle; struct FHitResult; struct FVector;
struct FGameplayTagContainer; struct FSentence; struct FSentenceView;
struct FWorld; struct FTimerManager; struct FAbilitySystem; struct FTagSet;
struct DamageSource; struct DamageInstigator; struct CallbackList;
namespace unresolved {
extern UObject* pawn_field_588(AFortPawn&); // exact type not established
extern void relay_cue(UObject&, uint8_t, const FGameplayCueParameters&); // 1406c6af0; includes 14076f120/142ec9900 lookup
extern void increment_damage_iteration(AFortPawn&); extern void decrement_damage_iteration(AFortPawn&);
extern int damage_listener_count(AFortPawn&); extern bool call_listener_50(AFortPawn&, int, uint8_t, const FGameplayCueParameters&);
extern void compact_listeners_140685370(AFortPawn&);
extern DamageSource* source_1415f4460(const FGameplayCueParameters&);
extern bool source_flag_e(DamageSource&); extern bool parameters_match_global_tag_248(const FGameplayCueParameters&);
extern bool pawn_flag_520_08(AFortPawn&); extern bool pawn_flag_520_10(AFortPawn&); extern bool pawn_flag_53a_02(AFortPawn&);
extern float cue_magnitude(const FGameplayCueParameters&); // +4
extern bool role_is_3(AFortPawn&); extern void virtual_5e8(AFortPawn&);
extern FTagSet tags_from_28(const FGameplayCueParameters&); extern void append_tags_30(FTagSet&,const FGameplayCueParameters&);
extern UObject* source_actor(DamageSource*); extern DamageSource* source_controller_if_class(UObject*);
extern DamageInstigator* effect_instigator_if_class(const FGameplayCueParameters&);
extern UObject* effect_context_virtual_30(const FGameplayCueParameters&);
extern UObject* instant_death_instigator(const FGameplayCueParameters&); // context virtual 20, class check, fallback conversion 1419b6d20
extern void virtual_c40(AFortPawn&,float,const FTagSet&,const FGameplayCueParameters&,UObject*,UObject*);
extern void virtual_c30(AFortPawn&,float,const FTagSet&,const FGameplayCueParameters&,UObject*,UObject*);
extern void virtual_a88(AFortPawn&,float,const FTagSet&,const FGameplayCueParameters&,UObject*,UObject*);
extern bool helper_142cd8ed0(AFortPawn&); extern bool field_69c_nonzero(AFortPawn&);
extern void helper_141624d00(AFortPawn&,const FGameplayCueParameters&,bool);
extern bool helper_14207c440(AFortPawn&);
// Executes controller/class/interface/tag checks in 142318265..142318415; true on
// absent controller, missing tag restriction, or non-matching controller tags.
extern bool controller_tags_allow_damage(AFortPawn&);
extern bool source_controller_virtual_6a8(DamageSource&); extern bool instigator_virtual_888(DamageInstigator&);
extern FSentenceView sentence_view_141d50140(AFortPawn&); extern bool sentence_test_142c2e440(const FSentenceView&);
extern void sentence_apply_142a18ed0(AFortPawn&,const FSentenceView&); extern void sentence_clear_142a18540(AFortPawn&);
extern bool virtual_c70(AFortPawn&); extern void callback_14230f6f0(AFortPawn&);
extern FWorld* virtual_150_world(AFortPawn&); extern FTimerManager& timer_manager(FWorld&);
extern void schedule_sentence_1446278d0(FTimerManager&,AFortPawn&); // CurrentSentence+200, delegate to 14230f6f0, global 1478de6b0
extern FAbilitySystem* ability_system(AFortPawn&); extern bool weapon_activated(AFortPawn&);
extern uint64_t& batch_timer_handle(AFortPawn&); // AccumulatedBatchData_NonShared+0x10
extern void* timer_lookup_144621fb0(FTimerManager&,uint64_t); extern uint8_t timer_state_byte_1(void*);
extern void clear_timer(FTimerManager&,uint64_t); extern void invoke_weapon_cue_1406ce540(FAbilitySystem&,uint8_t); // global 141ed1800()+1518, empty cue parameters
}
using namespace unresolved;

void GameplayCue_base(AFortPawn& pawn, uint8_t event, FGameplayCueParameters parameters) {
    if (UObject* target = pawn_field_588(pawn)) relay_cue(*target,event,parameters);
}
void GameplayCue_Damage_base(AFortPawn& pawn, uint8_t event, FGameplayCueParameters parameters) {
    increment_damage_iteration(pawn);
    bool compact = false;
    for (int i = damage_listener_count(pawn)-1; i >= 0; --i)
        if (!call_listener_50(pawn,i,event,parameters)) compact = true; // also false for invalid listener
    decrement_damage_iteration(pawn);
    if (compact) compact_listeners_140685370(pawn);
    DamageSource* source = source_1415f4460(parameters);
    bool special = source && (source_flag_e(*source) || (parameters_match_global_tag_248(parameters) && pawn_flag_520_08(pawn)));
    if (event != 2 || ((cue_magnitude(parameters) <= 0 || pawn_flag_520_10(pawn)) && !special)) return;
    if (role_is_3(pawn)) virtual_5e8(pawn);
    FTagSet tags = tags_from_28(parameters); append_tags_30(tags,parameters);
    UObject* actor = source_actor(source);
    DamageSource* controller = source_controller_if_class(actor);
    DamageInstigator* instigator = effect_instigator_if_class(parameters);
    if ((special && !pawn_flag_520_10(pawn)) || pawn_flag_53a_02(pawn)) {
        virtual_c40(pawn,cue_magnitude(parameters),tags,parameters,actor,effect_context_virtual_30(parameters));
        virtual_a88(pawn,cue_magnitude(parameters),tags,parameters,actor,effect_context_virtual_30(parameters));
        if (!role_is_3(pawn)) {
            bool helper_result = helper_142cd8ed0(pawn);
            if (field_69c_nonzero(pawn) || !helper_result) helper_141624d00(pawn,parameters,true);
        }
    } else if (!pawn_flag_520_08(pawn)) {
        bool allowed = helper_14207c440(pawn) || controller_tags_allow_damage(pawn);
        if ((!controller || !source_controller_virtual_6a8(*controller) || !instigator || instigator_virtual_888(*instigator)) && allowed)
            virtual_c40(pawn,cue_magnitude(parameters),tags,parameters,actor,effect_context_virtual_30(parameters));
        virtual_c30(pawn,cue_magnitude(parameters),tags,parameters,actor,effect_context_virtual_30(parameters));
    }
}
void GameplayCue_InstantDeath_base(AFortPawn& pawn, uint8_t event, FGameplayCueParameters parameters) {
    if (event != 2) return;
    if (role_is_3(pawn)) virtual_5e8(pawn);
    FTagSet tags = tags_from_28(parameters);
    UObject* actor = instant_death_instigator(parameters);
    virtual_a88(pawn,cue_magnitude(parameters),tags,parameters,actor,effect_context_virtual_30(parameters));
    if (!role_is_3(pawn)) {
        bool helper_result = helper_142cd8ed0(pawn);
        if (field_69c_nonzero(pawn) || !helper_result) helper_141624d00(pawn,parameters,true);
    }
}
// Base native virtual 142337330 only destroys its by-value final argument; it has
// no additional effect. RAII handles destruction in a source-level implementation.
void OnDeathPlayEffects_base(AFortPawn&, const FGameplayEffectContextHandle&, UObject*, UObject*, const FHitResult&, FVector, const FGameplayTagContainer&, float) {}
void OnRep_Sentence_draft(AFortPawn& pawn) {
    FSentenceView view = sentence_view_141d50140(pawn);
    if (!sentence_test_142c2e440(view)) sentence_apply_142a18ed0(pawn,view);
    else sentence_clear_142a18540(pawn);
    if (virtual_c70(pawn)) callback_14230f6f0(pawn);
    else if (FWorld* world = virtual_150_world(pawn)) schedule_sentence_1446278d0(timer_manager(*world),pawn);
}
void OnRep_WeaponActivated_draft(AFortPawn& pawn) {
    FAbilitySystem* asc = ability_system(pawn);
    if (!asc) return;
    if (!weapon_activated(pawn)) { invoke_weapon_cue_1406ce540(*asc,3); return; }
    if (FWorld* world = virtual_150_world(pawn)) {
        FTimerManager& tm = timer_manager(*world);
        uint64_t& handle = batch_timer_handle(pawn);
        void* timer = timer_lookup_144621fb0(tm,handle);
        if (!timer || timer_state_byte_1(timer) == 2) invoke_weapon_cue_1406ce540(*ability_system(pawn),1);
        else { if (timer_lookup_144621fb0(tm,handle)) clear_timer(tm,handle); handle = 0; }
    }
}
