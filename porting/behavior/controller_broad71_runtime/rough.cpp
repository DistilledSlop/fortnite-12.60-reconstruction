// Rough source lift only. Types and functions in A are unresolved adapters; see HANDOFF.md.
// Their names identify observed addresses/operations, not established engine APIs.
#include <cstdint>
#include <cstddef>
namespace A {
using Obj = void; using Word = uintptr_t; using Byte = uint8_t;
struct String { void* data; int32_t num, max; };
struct Pair { void* object; void* control; };
struct Timer { Word raw; };
struct RevivePayload; struct CreativeResult; struct DelegateCopy; struct ReportEvent;
extern Obj* world(Obj*); extern Obj* game_state(Obj*); extern bool is_a(Obj*, Word class_address);
extern Byte& byte_at(Obj*, size_t); extern int32_t& int_at(Obj*, size_t); extern float& float_at(Obj*, size_t);
extern Obj* ptr_at(Obj*, size_t); extern Timer& timer_at(Obj*, size_t);
extern Word timer_manager_controller(Obj*); extern Word timer_manager_world(Obj*);
extern bool timer_active_nonpaused(Word, Timer); // lookup 144621fb0, active iff entry && status != 2
extern float win_delay(Obj*, int); extern void* alloc(size_t); extern void free_mem(void*);
// These three adapters must perform observed allocation, weak-object/callback construction,
// delegate cloning, timer insertion, and both delegate/heap cleanups in source order, even on allocation failure.
extern void schedule_win(Word manager, Timer&, Obj* controller, Word raw_arg2, Word raw_arg3, Byte raw_arg4_low, float delay);
extern void schedule_simple(Word manager, Timer&, Obj* controller, Word callback, float delay);
extern void call_142919d10(Obj*); extern void call_14178abf0(Obj*); extern void broadcast_empty(Obj*, size_t);
extern Word intern_highlight(bool team); extern Obj* modular_features(); extern int feature_count(Obj*, Word);
extern Obj* feature_get_interface_minus_8(Obj*, Word, int); extern void highlight_capture(Obj*, void* group, const void* buffer, int32_t start, int32_t end);
extern void highlight_finish(Obj*, void* group); extern void highlight_buffer_init_copy(void*, size_t, size_t); extern void highlight_buffer_free(void*);
extern void* highlight_group(Obj*); extern bool win_suppressed(Obj*); extern bool highlight_enabled(Obj*);
extern Word athena_game_state_class(); extern int32_t& publish_delegate_lock(Obj*); extern Byte& revive_count(Obj*); extern int32_t revive_duration(Obj*); extern float& revive_end_time(Obj*); extern Word global_effect(Word);
extern Byte& win_flag(Obj*, bool); extern Timer& win_timer(Obj*); extern Timer& highlight_active_timer(Obj*); extern Timer& highlight_delay_timer(Obj*); extern Obj* match_report(Obj*);
extern void solo_report(Obj* controller, Obj* pawn, Obj* report); // expanded separately below
extern Word position_default_xy(); extern uint32_t position_default_z(); extern Word rotation_default_xy(); extern uint32_t rotation_default_z();
struct Vec3 { Word xy; uint32_t z; }; struct Vec4 { float x,y,z,w; };
extern Vec4& transform_current(Obj*); extern Vec4& transform_cached(Obj*); extern Vec3& cached_position(Obj*);
extern Vec3 transform_to_position(const Vec4&); extern Vec3 transform_rotation(Obj*);
extern Word world_report_scalar(Obj*); extern ReportEvent* report_event_construct(Obj* controller, Obj* world, Obj* state, Byte kind, Word scalar, Vec3 rotation, Vec3 position);
extern void report_submit(Obj*, ReportEvent*); extern void report_event_destroy(ReportEvent*);
extern String string_copy(const String&); extern void string_free(String&); extern void update_island_entries(Obj*, String&);
extern CreativeResult* creative_result_construct(); extern void creative_result_set_error(CreativeResult*, String&);
extern String creative_result_text_copy(CreativeResult*); extern void log_publish(const String&, const String&);
extern Obj* creative_error_service(Word key, Obj* world); extern Word creative_error_key(); extern Pair creative_error_interface(Obj*);
extern Pair creative_error_shared(CreativeResult*); extern Pair make_shared_alias(Pair, size_t); extern Pair controller_context(Obj*);
extern void error_dispatch(Obj*, Word context, Pair alias); extern void destroy_controller_context(Pair&);
extern void release_shared(Pair&, bool atomic); extern void creative_result_destroy(CreativeResult*);
extern int delegate_count(Obj*, size_t); extern Obj* delegate_entry(Obj*, size_t, int, int& bound); extern bool delegate_invoke(Obj*, const String&, const String&);
extern void compact_delegate(Obj*, size_t, int); extern void* callback_group(Obj*, size_t);
extern Obj* revive_ability_system(Obj*); extern Word revive_globals(); extern bool tag_query(Obj*, Word, int); extern void tag_remove(Obj*, Word, int);
extern Obj* player_state(Obj*); extern Obj* pawn_player_state(Obj*); extern void revive_state_started(Obj*, Byte); extern void revive_state_interrupted(Obj*, Byte);
extern RevivePayload* revive_payload_construct(int32_t); extern float revive_time(Obj*); extern void apply_revive_effect(Obj*, Word, RevivePayload*);
extern void revive_payload_destroy(RevivePayload*); extern void revive_changed(Obj*); extern void revive_counter_zero(Obj*);
extern bool revive_logging_enabled(); extern Obj* world_service(Obj*); extern String player_name(Obj*); extern int16_t player_number(Obj*);
extern String format_revive(bool started, const String& target, int target_num, const String& reviver, int reviver_num);
extern void submit_revive_log(Obj*, String&);
extern int log_verbosity(); extern void log_loading(Obj*, int message, Obj* world); extern bool ltm_globals_enabled();
extern void start_wait_server(Obj*);
}
using namespace A;

static void shared_win(Obj* self, Word arg2, Word arg3, Word arg4, bool team) {
  if (!world(self)) return;
  win_flag(self,team) = 1; // adapter offset resolved from named field
  Obj* gs = game_state(world(self));
  if (gs && !is_a(gs, athena_game_state_class())) gs = nullptr;
  if (!win_suppressed(self)) {
    Timer& timer = win_timer(self);
    if (!timer_active_nonpaused(timer_manager_controller(self), timer)) {
      float delay = gs ? win_delay(gs, 1) : 0.01f;
      if (delay <= 0.01f) delay = 0.01f;
      schedule_win(timer_manager_controller(self), timer, self, arg2, arg3, Byte(arg4), delay);
    }
  }
  // The decompiler treats raw arg2 as a pointer here, despite the recovered byte signature.
  if (arg2 && ptr_at(reinterpret_cast<Obj*>(arg2), 0x250) == self && gs && !byte_at(gs,0x12d0)) call_142919d10(gs);
  call_14178abf0(self);
  broadcast_empty(self, team ? 0x40 : 0x10 /* BattleLabInputComponent relative */);
  if (highlight_enabled(self)) {
    Word key = intern_highlight(team); Obj* features = modular_features();
    if (feature_count(features,key) > 0) {
      features = modular_features(); Obj* feature = feature_get_interface_minus_8(features,key,0);
      Timer& active = highlight_active_timer(self);
      if (!timer_active_nonpaused(timer_manager_controller(self),active)) {
        alignas(8) char buffer[16]; highlight_buffer_init_copy(buffer,4,8);
        highlight_capture(feature,highlight_group(self),buffer,-10000,5000); highlight_buffer_free(buffer);
      }
      alignas(8) char buffer[16]; highlight_buffer_init_copy(buffer,4,8);
      highlight_finish(feature,highlight_group(self)); highlight_buffer_free(buffer);
      Obj* w = world(self);
      schedule_simple(timer_manager_world(w), highlight_delay_timer(self), self,0x1419fcc80,5.5f);
    }
  }
  if (!team) {
    Obj* pawn = ptr_at(self,0x248); Obj* report = match_report(self);
    if (pawn && byte_at(self,0xf0)==2 && report) solo_report(self,pawn,report);
  }
}
void ClientNotifyWon(Obj* c, Word a2, Word a3, Word a4) { shared_win(c,a2,a3,a4,false); }
void ClientNotifyTeamWon(Obj* c, Word a2, Word a3, Word a4) { shared_win(c,a2,a3,a4,true); }
void A::solo_report(Obj* c, Obj* pawn, Obj* report) {
  Word scalar = world_report_scalar(world(c)); Obj* root = ptr_at(pawn,0x130); Vec3 position,rotation;
  if (!root) position={position_default_xy(),position_default_z()};
  else {
    Obj* original=root; Vec4 current=transform_current(root), old=transform_cached(root);
    if (current.x!=old.x || current.y!=old.y || current.z!=old.z || current.w!=old.w) {
      transform_cached(root)=current; cached_position(root)=transform_to_position(current); root=ptr_at(pawn,0x130);
    }
    // Read from original root even if the helper changed pawn's root.
    position=cached_position(original);
  }
  rotation = root ? transform_rotation(root) : Vec3{rotation_default_xy(),rotation_default_z()};
  Obj* w=world(c); ReportEvent* ev=report_event_construct(c,w,player_state(c),12,scalar,rotation,position);
  report_submit(report,ev); report_event_destroy(ev);
}

void ClientPublishCreativePlotComplete(Obj* self, const String& link, const String& error) {
  String copy=string_copy(link); update_island_entries(self,copy); string_free(copy);
  CreativeResult* result=creative_result_construct();
  if (link.num<2) { String e=string_copy(error); creative_result_set_error(result,e); string_free(e); }
  String text=creative_result_text_copy(result);
  if (log_verbosity()>3) log_publish(link,text);
  if (link.num<2 && text.num>1) {
    Word key=creative_error_key(); Obj* w=world(self); Obj* service=creative_error_service(key,w);
    if (service) {
      Pair iface=creative_error_interface(service);
      if (iface.object) {
        Pair shared=creative_error_shared(result); Pair alias=make_shared_alias(shared,16);
        Pair context=controller_context(self); error_dispatch(iface.object,reinterpret_cast<Word>(context.control),alias);
        destroy_controller_context(context); release_shared(shared,false);
      }
      release_shared(iface,true);
    }
  }
  constexpr size_t list=0x10; // relative to OnUpdateCreativeDescriptionTags
  ++publish_delegate_lock(self);
  bool compact=false;
  for (int i=delegate_count(self,list)-1; i>=0; --i) {
    int bound; Obj* entry=delegate_entry(self,list,i,bound);
    if (!bound || !entry || !delegate_invoke(entry,link,text)) compact=true;
  }
  --publish_delegate_lock(self);
  if (compact) compact_delegate(self,list,0);
  string_free(text); creative_result_destroy(result);
}

static void log_revive(Obj* self, Obj* pawn, bool started) {
  if (!revive_logging_enabled()) return;
  Obj* service=world_service(world(self));
  if (!service || !is_a(service,0x1428ea7f0)) return;
  Obj* reviver=player_state(self);
  if (!reviver || !is_a(reviver,0x142a8ae70)) return;
  Obj* target=pawn_player_state(pawn);
  if (!target || !is_a(target,0x142a8ae70)) return;
  String rn=player_name(reviver); String tn=player_name(target);
  String line=format_revive(started,tn,player_number(target),rn,player_number(reviver));
  string_free(tn); string_free(rn); submit_revive_log(service,line); string_free(line);
}
void ServerDBNOReviveStarted(Obj* self, Obj* pawn) {
  if (!pawn) return;
  Byte& count=revive_count(pawn); ++count;
  if (count==1) {
    Obj* asc=revive_ability_system(pawn);
    if (asc) {
      Word globals=revive_globals();
      if (tag_query(asc,globals+0x518,1)) tag_remove(asc,globals+0x518,1);
      Obj* state=player_state(self);
      if (state && is_a(state,0x142a8ae70)) revive_state_started(state,4);
      RevivePayload* payload=revive_payload_construct(revive_duration(pawn));
      revive_end_time(pawn)=revive_time(self)+float(revive_duration(pawn));
      globals=revive_globals(); apply_revive_effect(asc,global_effect(globals+0x1520),payload);
      log_revive(self,pawn,true); revive_payload_destroy(payload);
    }
  }
  revive_changed(pawn);
}
void ServerDBNOReviveInterrupted(Obj* self, Obj* pawn) {
  if (!pawn) return;
  Byte& count=revive_count(pawn);
  if (count && --count==0) revive_counter_zero(pawn);
  Obj* state=player_state(self);
  if (state && is_a(state,0x142a8ae70)) revive_state_interrupted(state,4);
  log_revive(self,pawn,false);
}
void ClientStartWaitLTMLoading(Obj* self) {
  Obj* w=world(self);
  if (!w) { if (log_verbosity()>2) log_loading(self,0,nullptr); return; }
  if (!ltm_globals_enabled()) return;
  if (log_verbosity()>4) log_loading(self,1,w);
  byte_at(self,0x2d69)=1;
  if (byte_at(self,0xf0)==3) { start_wait_server(self); return; }
  Timer& timer=timer_at(self,0x2e90);
  if (!timer.raw) schedule_simple(timer_manager_world(w),timer,self,0x1419c41f0,0.1f);
  else if (log_verbosity()>=3) log_loading(self,2,w);
}
