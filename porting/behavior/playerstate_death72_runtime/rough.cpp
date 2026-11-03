// Behavioral lift of AFortPlayerStateAthena::OnRep_DeathInfo (0x142a8d9b0).
// Research artifact: types, ABI, offsets and ownership must be reconciled before use.
#include <cstdint>
#include <initializer_list>
#include "marker.rough.hpp"

namespace death72 {
struct Object; struct World; struct DeathInfo; struct Text; struct String;
struct Arguments; struct Tags; struct Marker; struct EventPayload;

// These adapters are declarations, not implementations. Every operation, including
// temporary cleanup, remains an integration contract; see HANDOFF.md.
struct Engine {
  void finish_frame();
  World* world_v150(Object*); Object* game_state_at_130(World*);
  Object* cast_game_state(Object*); Object* first_local_controller(World*);
  Object* cast_athena_controller(Object*); Object* cast_controller_142a69680(Object*);
  void init_default_death_info_once(); DeathInfo* actual_death_info(Object*);
  DeathInfo* default_death_info(); bool valid_24(DeathInfo*); bool down_10(DeathInfo*);
  uint8_t cause_11(DeathInfo*); float distance_14(DeathInfo*);
  Object* instigator_00(DeathInfo*); Tags* tags_28(DeathInfo*);
  bool exact_tag_either(Tags*, unsigned registry_offset); // checks both arrays, +28 and +38
  bool hierarchical_tag_either(Tags*, unsigned registry_offset); // FUN_14192b2d0; omitted decompiler arguments unresolved
  float option_distance_324(); bool option_83(); bool option_b1();
  Text* empty_text(); Text* distance_text(int meters); void destroy_text(Text*);
  Object* cast_player_state_142a8ae70(Object*); Object* controller_player_state_v2058(Object*);
  uint8_t team_v28(Object*); bool display_permitted(Object*); // flag +222 bit 3, or abandon bit and TeamIndex != 2
  void local_kill_delegate(Object* controller, bool down, Object* victim); // controller +2b60/+2b78, FUN_1406ba850
  bool flag_330_bit6(Object*); bool mode_probe_14228d390(World*); // includes shared-pointer cleanup
  Object* controller_player_state_141680740(Object*);
  String* literal(const wchar_t*); void destroy_string(String*);
  String* raw_player_name(Object*); String* controller_name_v1fc8(Object*, Object*, bool);
  int season_level(Object*); bool season_visible_141a4a2d0(Object*);
  void append_level(String*, int); bool same_team_14241bc10(World*, Object*);
  bool non_athena_instigator_141a2d130(Object*);
  Arguments* make_arguments(); void arg_string(Arguments*, const wchar_t*, String*);
  void arg_text(Arguments*, const wchar_t*, Text*); void arg_cube_markup(Arguments*);
  void destroy_arguments(Arguments*);
  // Selects FUN_141e5f780/720 (normal/down) or 141e63570/510 (self),
  // converts and formats, replaces text, and releases all intermediate FTexts.
  void format_tag_text(Text*, Tags*, Arguments*, bool down, bool self);
  // Constructs a temporary tag container from registry offset, formats as above,
  // and frees both arrays and intermediate texts after replacing the result.
  void format_registry_text(Text*, unsigned registry_offset, Arguments*, bool down, bool self);
  bool text_empty_142c2e440(Text*);
  bool world_144693aa0(World*); void notify_141a38140(Object*);
  bool controller_flag_28c_bit1(Object*); bool controller_v6a8(Object*);
  Object* player_controller_at_e0(Object*); bool is_controller_141a2d0b0(Object*);
  bool player_v840(Object*);
  // FUN_1423d2260; constructed with instigator/victim strings and category below.
  void compact_notification(Object* controller, String* instigator, String* victim, uint8_t category);
  bool valid_instigator_14192cc60(Object*); void game_state_notify_14198c020(Object*,Object*,Object*);
  bool game_state_filter_1419b0480(Object*,Object*,Object*);
  void game_state_broadcast_va68(Object*,Text*,int);
  bool controller_141754770(Object*); void controller_cause49_action(Object*); // 1419e4e40, 140684de0
  void controller_thunk_142810956(Object*,int,int); void player_141a619d0(Object*);
  bool world_144693a30(World*); bool world_f0_exists_and_ad1_zero(World*);
  Object* world_subsystem_1426402a0(World*); bool subsystem_14264e810(Object*);
  void subsystem_142642450(Object*,DeathInfo*,Object*);
  Object* instigator_component_278(Object*); bool component_141a2d130(Object*);
  void component_14233e4a0(Object*); // preceding v840 result discarded
  Object* victim_settings_1419d3ff0_141991bf0(Object*);
  bool settings_flag(Object*, unsigned offset); uint8_t squad_id(Object*);
  MarkerEngine& marker_engine(); // operation-level interface in marker.rough.hpp
};

void OnRep_DeathInfo(Engine& e, Object* victim) {
  e.finish_frame();
  World* world = e.world_v150(victim);
  Object* game_state = e.cast_game_state(e.game_state_at_130(world));
  Object* local_raw = e.first_local_controller(world);
  Object* local = e.cast_athena_controller(local_raw);
  e.init_default_death_info_once();
  DeathInfo* actual = e.actual_death_info(victim);
  DeathInfo* info = e.valid_24(actual) ? actual : e.default_death_info();
  Tags* tags = e.tags_28(info);
  bool show_distance = e.exact_tag_either(tags, 0xa80) || e.exact_tag_either(tags, 0x358);
  int meters = static_cast<int>(e.distance_14(info) * 0.01f);
  if (static_cast<float>(meters) > e.option_distance_324()) show_distance = true;
  Text* distance = e.empty_text();
  if (show_distance) { e.destroy_text(distance); distance = e.distance_text(meters); }
  Object* killer = e.cast_player_state_142a8ae70(e.instigator_00(info));
  Object* special_controller = e.cast_controller_142a69680(local_raw);
  if (special_controller && killer && e.controller_player_state_v2058(special_controller) == killer &&
      e.team_v28(killer) != e.team_v28(victim) && e.display_permitted(victim))
    e.local_kill_delegate(special_controller, e.down_10(info), victim);
  if (!e.flag_330_bit6(victim)) {
    bool victim_name_flag = true, killer_name_flag = true;
    if (e.option_83() && !(e.valid_24(actual) && e.down_10(actual)) && !special_controller &&
        e.mode_probe_14228d390(world)) {
      victim_name_flag = false;
      killer_name_flag = !(local && e.controller_player_state_141680740(local) == victim);
    }
    Text* message = e.empty_text();
    String* victim_style = e.literal(L"Victim");
    String* victim_name = local ? e.controller_name_v1fc8(local,victim,victim_name_flag)
                                : e.raw_player_name(victim);
    if (e.season_level(victim) > 0 && e.display_permitted(victim))
      e.append_level(victim_name,e.season_level(victim));

    if (!special_controller || !e.option_b1()) {
      if (!killer) {
        if (e.instigator_00(info) && e.non_athena_instigator_141a2d130(e.instigator_00(info))) {
          Arguments* args = e.make_arguments();
          e.arg_string(args,L"VictimStyle",victim_style); e.arg_string(args,L"PlayerName",victim_name);
          e.format_tag_text(message,tags,args,e.down_10(info),false);
          if (e.text_empty_142c2e440(message))
            e.format_registry_text(message,e.down_10(info) ? 0xc10 : 0xc08,args,e.down_10(info),false);
          e.destroy_arguments(args);
        }
      } else {
        if (e.same_team_14241bc10(world,victim)) { e.destroy_string(victim_style); victim_style=e.literal(L"TeamVictim"); }
        Arguments* args=e.make_arguments();
        e.arg_string(args,L"VictimStyle",victim_style); e.arg_string(args,L"PlayerName",victim_name);
        e.arg_text(args,L"DistanceText",distance); e.arg_cube_markup(args);
        unsigned selected=0;
        for (unsigned offset : {0xc20u,0x380u,0xc28u,0x3c0u,0x448u}) {
          if (e.hierarchical_tag_either(tags,offset)) { selected=offset; break; }
        }
        if (selected) e.format_registry_text(message,selected,args,false,false);
        else if (killer == victim) {
          e.format_tag_text(message,tags,args,e.down_10(info),true);
          if (e.text_empty_142c2e440(message))
            e.format_registry_text(message,e.down_10(info) ? 0xbf0 : 0xbe8,args,e.down_10(info),true);
        } else {
          String* killer_style=e.literal(L"Killer"); String* killer_name=e.raw_player_name(killer);
          if (local && e.controller_flag_28c_bit1(local) && e.controller_v6a8(local)) {
            String* name=e.controller_name_v1fc8(local,killer,killer_name_flag);
            e.destroy_string(killer_name); killer_name=name;
          }
          if (e.season_level(killer)>0 && e.season_visible_141a4a2d0(killer))
            e.append_level(killer_name,e.season_level(killer));
          Object* kc=e.player_controller_at_e0(killer);
          if (kc && e.is_controller_141a2d0b0(kc) && e.controller_v6a8(kc)) {
            e.destroy_string(killer_style); killer_style=e.literal(L"LocalKiller");
          } else if (e.same_team_14241bc10(world,killer)) {
            e.destroy_string(killer_style); killer_style=e.literal(L"TeamKiller");
          }
          e.arg_string(args,L"KillerStyle",killer_style); e.arg_string(args,L"Killer",killer_name);
          e.format_tag_text(message,tags,args,e.down_10(info),false);
          if (e.text_empty_142c2e440(message))
            e.format_registry_text(message,e.down_10(info) ? 0xc00 : 0xbf8,args,e.down_10(info),false);
          e.destroy_string(killer_name); e.destroy_string(killer_style);
        }
        if (e.world_144693aa0(world)) e.notify_141a38140(victim);
        e.destroy_arguments(args);
      }
    } else if (e.display_permitted(victim) &&
               (e.player_v840(victim) || (e.valid_24(actual) && e.down_10(actual)))) {
      String* instigator = killer ? e.raw_player_name(killer) : e.literal(L"");
      uint8_t category=0;
      if (e.down_10(info)) category=4;
      else if (e.cause_11(info)==0) category=1;
      else if (e.cause_11(info)==1) category=2;
      else if (e.cause_11(info)>=10 && e.cause_11(info)<14) category=3;
      e.compact_notification(special_controller,instigator,victim_name,category);
      e.destroy_string(instigator);
    }
    if (!e.text_empty_142c2e440(message) && game_state) {
      Object* instigator=e.instigator_00(info);
      if (!instigator || !e.valid_instigator_14192cc60(instigator)) instigator=nullptr;
      e.game_state_notify_14198c020(game_state,victim,instigator);
      if (!e.hierarchical_tag_either(tags,0xbd8)) {
        instigator=e.instigator_00(info);
        if (!instigator || !e.valid_instigator_14192cc60(instigator)) instigator=nullptr;
        if (e.game_state_filter_1419b0480(game_state,victim,instigator))
          e.game_state_broadcast_va68(game_state,message,0);
      }
    }
    Object* victim_controller=e.player_controller_at_e0(victim);
    if (victim_controller && e.controller_141754770(victim_controller)) {
      if (e.cause_11(info)==0x31) e.controller_cause49_action(victim_controller);
      e.controller_thunk_142810956(victim_controller,0,0);
    }
    e.player_141a619d0(victim);
    if (e.world_144693a30(world) && e.world_f0_exists_and_ad1_zero(world) &&
        e.season_visible_141a4a2d0(victim) &&
        (e.player_v840(victim) || (e.valid_24(actual) && e.down_10(actual)))) {
      Object* subsystem=e.world_subsystem_1426402a0(world);
      if (subsystem && e.subsystem_14264e810(subsystem)) e.subsystem_142642450(subsystem,info,victim);
      Object* component=killer ? e.instigator_component_278(killer) : nullptr;
      if (component && e.component_141a2d130(component)) { e.player_v840(victim); e.component_14233e4a0(component); }
    }
    if (local && e.valid_24(info) && !e.down_10(info)) {
      Object* settings=e.victim_settings_1419d3ff0_141991bf0(victim);
      if (settings && (e.settings_flag(settings,0x759) || e.settings_flag(settings,0x75a) || e.settings_flag(settings,0x75b)) &&
          !e.hierarchical_tag_either(tags,0xbe0)) {
        Object* observer=e.controller_player_state_141680740(local);
        if (observer && observer!=victim) {
          uint8_t os=e.squad_id(observer), ot=e.team_v28(observer);
          uint8_t vs=e.squad_id(victim), vt=e.team_v28(victim);
          if ((e.settings_flag(settings,0x759) && vs==os) ||
              (e.settings_flag(settings,0x75a) && vt==ot) ||
              (e.settings_flag(settings,0x75b) && vs!=os && vt!=ot)) {
            World* current=e.world_v150(victim);
            marker_append_and_emit(e.marker_engine(),game_state,current,info,local,victim_name,vs,vt);
          }
        }
      }
    }
    e.destroy_string(victim_name); e.destroy_string(victim_style); e.destroy_text(message);
  }
  e.destroy_text(distance);
}
} // namespace death72
