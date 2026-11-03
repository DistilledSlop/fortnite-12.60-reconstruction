// Uncounted source lifts. Adapter declarations are unresolved contracts, not implementations.
// Original addresses and limitations are recorded in README.md. This file is not a build input.
namespace controller_broad69 {
struct Controller; struct World; struct Object; struct Delegate; struct Entry;
struct Report; struct StatsWriter; struct String; struct Name; struct Rotation;
struct TimerManager; struct Array; struct Highlight; struct State;

// All adapter calls retain the indicated source offset, vtable slot, or helper identity.
World* world_v150(Controller*); Object* at(Object*, unsigned); Object* as_object(Controller*);
int int_at(Object*, unsigned); unsigned char byte_at(Object*, unsigned);
void set_int_at(Object*, unsigned, int); int delegate_count(Delegate*); int delegate_lock(Delegate*);
void delegate_set_lock(Delegate*, int); Entry* delegate_entry(Delegate*, int);
bool entry_bound(Entry*); bool entry_invoke_50(Entry*, Object*); void delegate_compact_140685370(Delegate*, int);
Delegate* skydive_delegate(Controller*); Delegate* party_delegate(Controller*);
Object* skydive_leader(Controller*); Object* player_state(Controller*); Object* pawn(Controller*);
Object* party_data(Controller*); void dynamic_skydive_1406a9dc0(Delegate*, bool, bool, Object*);
void pawn_component_v590(Object*, int, int); bool controller_v2190(Controller*);
Rotation camera_v700(Object*); void set_interpolated_rotation(Controller*, const Rotation&);
void broadcast_reverse(Delegate* d, Object* argument) {
  delegate_set_lock(d, delegate_lock(d) + 1);
  bool compact = false;
  for (int i = delegate_count(d) - 1; i >= 0; --i) {
    Entry* e = delegate_entry(d, i);
    if (!entry_bound(e) || !entry_invoke_50(e, argument)) compact = true;
  }
  delegate_set_lock(d, delegate_lock(d) - 1);
  if (compact) delegate_compact_140685370(d, 0);
}
void OnRep_PartyAssistedMemberData(Controller* c) { broadcast_reverse(party_delegate(c), party_data(c)); }
void OnRep_SkydiveLeader(Controller* c) {
  Object* leader = skydive_leader(c);
  if (leader) broadcast_reverse(skydive_delegate(c), leader);
  leader = skydive_leader(c);
  dynamic_skydive_1406a9dc0(skydive_delegate(c), leader != nullptr, leader == player_state(c), leader);
  Object* p = pawn(c);
  if (p && (byte_at(p, 0x10e7) & 1) && byte_at(p, 0x10e6) == 0) {
    int mode = (skydive_leader(c) == player_state(c) || !skydive_leader(c)) ? 4 : 5;
    pawn_component_v590(at(p, 0x280), 6, mode);
  }
  if (controller_v2190(c)) {
    Object* camera = at(as_object(c), 0x2b0); // _gap_0x8 + 0x2a8
    if (camera) set_interpolated_rotation(c, camera_v700(camera));
  }
}

bool highlights_enabled(Controller*); Name highlight_name(); int highlight_count(Name);
Highlight* highlight_at_zero_adjusted(Name); void highlight_v28(Highlight*, Controller*);
int highlight_multiple(Controller*); float world_time(World*); float first_kill(Controller*);
float cooldown(Controller*); float rewind_time(Controller*); void highlight_v38(Highlight*, Controller*, const wchar_t*, int, int);
void reset_kill_and_down_multiple(Controller*);
void MultiKillExpired(Controller* c) {
  if (!highlights_enabled(c)) return;
  Name name = highlight_name();
  if (world_v150(c) && highlight_count(name) > 0) {
    Highlight* h = highlight_at_zero_adjusted(name);
    World* w = world_v150(c);
    int multiple = highlight_multiple(c);
    float now = world_time(w), first = first_kill(c), cool = cooldown(c), rewind = rewind_time(c);
    const wchar_t* label = multiple < 1 ? L"Down" : multiple == 1 ? L"Kill" : multiple == 2 ? L"Double Kill" : multiple == 3 ? L"Triple Kill" : L"Multi Kill";
    // Truncate the summed float first; multiply the resulting integer by -1000.
    highlight_v38(h, c, label, static_cast<int>(((now - first) + cool) + rewind) * -1000, 0);
  }
  reset_kill_and_down_multiple(c);
}

// These preserve the two independently initialized static names and serialization order.
Report* match_report(Controller*); unsigned char controller_gap8_byte_e8(Controller*);
Object* world_analytics(World*); String* match_stats_name(); String* team_stats_name();
unsigned long long fun_1419d92d0(); StatsWriter* writer_construct_142ccbd80();
void writer_init_142cd24e0(StatsWriter*, int); void writer_vd8(StatsWriter*, int);
void serialize_match_1419f0550(StatsWriter*, Report*, unsigned); void serialize_team_1419f0aa0(StatsWriter*, Report*, unsigned);
Object* writer_output_buffer_address(StatsWriter*);
void submit_14418adb0(Object*, String*, unsigned long long, String*, Object*);
void writer_destroy_142ccc820_and_free_buffer(StatsWriter*);
void RecordMatchStats(Controller* c) {
  if (controller_gap8_byte_e8(c) != 2) return;
  Report* r = match_report(c); if (!r || !byte_at(reinterpret_cast<Object*>(r), 0x7c)) return;
  World* w = world_v150(c); if (!w) return;
  Object* analytics = world_analytics(w); if (!analytics) return;
  String* n = match_stats_name(); auto id = fun_1419d92d0();
  StatsWriter* writer = writer_construct_142ccbd80(); writer_init_142cd24e0(writer, 1); writer_vd8(writer, 0);
  serialize_match_1419f0550(writer, r, 0x80); submit_14418adb0(analytics, n, id, n, writer_output_buffer_address(writer));
  writer_destroy_142ccc820_and_free_buffer(writer);
}
void RecordTeamStats(Controller* c) {
  if (controller_gap8_byte_e8(c) != 2) return;
  Report* r = match_report(c); if (!r || !byte_at(reinterpret_cast<Object*>(r), 0x140)) return;
  World* w = world_v150(c); if (!w) return;
  Object* analytics = world_analytics(w); if (!analytics) return;
  String* n = team_stats_name(); auto id = fun_1419d92d0();
  StatsWriter* writer = writer_construct_142ccbd80(); writer_init_142cd24e0(writer, 1); writer_vd8(writer, 0);
  serialize_team_1419f0aa0(writer, r, 0x144); submit_14418adb0(analytics, n, id, n, writer_output_buffer_address(writer));
  writer_destroy_142ccc820_and_free_buffer(writer);
}

Object* world_188(World*); bool class_check_1428ea7f0(Object*);
Object* fun_141e613c0(); bool log_level_at_least_5(); void log_ready();
void remove_binding_140690620(Object*, unsigned, Controller*); Object* fun_14237f3b0(Controller*);
Object* fun_142290c00(Object*); Object* fun_140d91b30(Object*); void fun_14226f840(Object*, int);
void broadcast_ready_1406a9dc0(Controller*);
void OnMatchmakingPartyReady(Controller* c) {
  World* w = world_v150(c); if (!w) return;
  Object* owner = world_188(w); if (!owner || !class_check_1428ea7f0(owner)) return;
  Object* service = fun_141e613c0(); if (!service) return;
  if (log_level_at_least_5()) log_ready();
  remove_binding_140690620(service, 0x378, c);
  Object* object = fun_14237f3b0(c);
  if (object) fun_14226f840(fun_140d91b30(fun_142290c00(object)), 2);
  broadcast_ready_1406a9dc0(c);
}

Array* building_analytics(Controller*); float tracking_start(Controller*);
void write_building_141787550(Controller*, Array*, float); void clear_num_and_shrink_1406911b0(Array*);
TimerManager* world_timer_manager(World*); Object* tracking_timer_handle(Controller*);
Object* find_timer_144621fb0(TimerManager*, Object*); void clear_timer(TimerManager*, Object*);
void zero_tracking_timer_handle(Controller*);
void WriteCreateBuildingAnalytics(Controller* c) {
  World* w = world_v150(c); if (!w) return;
  write_building_141787550(c, building_analytics(c), world_time(w) - tracking_start(c));
  clear_num_and_shrink_1406911b0(building_analytics(c));
  TimerManager* timer = world_timer_manager(w);
  if (find_timer_144621fb0(timer, tracking_timer_handle(c))) clear_timer(timer, tracking_timer_handle(c));
  zero_tracking_timer_handle(c);
}
} // namespace controller_broad69
