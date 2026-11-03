// Uncounted control-flow lifts. Adapter names encode original addresses/offsets;
// types, ownership and signatures are unresolved. Not a build input.
namespace playerstate_broad71 {
struct Object; struct State; struct Text; struct String; struct Map; struct Array;
struct TextValue; struct Local36 { unsigned char bytes[36]; }; struct FString;
const void* change_team_info(State*); int season_level(State*);
Object* world_v150(State*); Object* field(Object*, unsigned); bool is_a(Object*, const void*);
const void* player_state_class_142a8ae70(); const void* game_state_class();
const void* controller_class(); const void* class_1428ea7f0();
int int_field(const void*, unsigned); void* ptr_field(const void*, unsigned);
Text* text_default_142c0c240(); void text_destroy_140683490(Text*);
String* string_literal(const wchar_t*); String* name_1444ac7a0(Object*);
// Native printf call has an unobserved variadic argument; don't infer it from the branch.
String* format_level_142bafb80_unresolved(const String*); void string_destroy(String*);
Map* map_construct(); void map_destroy_140799530(Map*);
Text* text_from_string_142c26f20(const String*);
TextValue* value_copy_as_type4(Text*); void value_destroy_140bf86b0(TextValue*);
// Insert must retain key/value, perform the native sparse-map/hash insertion and
// duplicate-key behavior; temporary references are released by caller afterward.
void map_insert_sparse_1407babb0(Map*, const wchar_t*, TextValue*);
void map_insert_1416b7390(Map*, const wchar_t*, TextValue*, int);
Object* service_141e5b2a0(); Text* lookup_141e5f780(Object*, const void*);
Text* convert_142c0c460(Text*); Text* format_142c255f0(Text*, Map*);
void text_swap_native(Text*, Text*); bool text_predicate_142c2e440(Text*);
Object* game_state_141641350(Object*); void dispatch_a68(Object*, const Text*, int);

void OnRep_ChangeTeamInfo(State* self) {
  // P_FINISH is performed by the exec thunk before this body.
  if (int_field(change_team_info(self), 0x10) == 0) return;
  Object* other = static_cast<Object*>(ptr_field(change_team_info(self), 0));
  if (!other || !is_a(other, player_state_class_142a8ae70())) return;
  Text* output = text_default_142c0c240();
  String* victim_style = string_literal(L"Victim");
  String* killer_style = string_literal(L"Killer");
  String* player_name = name_1444ac7a0(reinterpret_cast<Object*>(self));
  String* killer_name = name_1444ac7a0(other);
  if (season_level(self) > 0) {
    String* replacement = format_level_142bafb80_unresolved(player_name);
    string_destroy(player_name); player_name = replacement;
  }
  if (int_field(other, 0xe78) > 0) {
    String* replacement = format_level_142bafb80_unresolved(killer_name);
    string_destroy(killer_name); killer_name = replacement;
  }
  Map* args = map_construct();
  Text* temporary = text_from_string_142c26f20(killer_style);
  TextValue* value = value_copy_as_type4(temporary);
  map_insert_sparse_1407babb0(args, L"KillerStyle", value);
  value_destroy_140bf86b0(value); text_destroy_140683490(temporary);
  temporary = text_from_string_142c26f20(killer_name);
  value = value_copy_as_type4(temporary);
  map_insert_sparse_1407babb0(args, L"Killer", value);
  value_destroy_140bf86b0(value); text_destroy_140683490(temporary);
  temporary = text_from_string_142c26f20(victim_style);
  value = value_copy_as_type4(temporary);
  map_insert_sparse_1407babb0(args, L"VictimStyle", value);
  value_destroy_140bf86b0(value); text_destroy_140683490(temporary);
  temporary = text_from_string_142c26f20(player_name);
  value = value_copy_as_type4(temporary);
  map_insert_1416b7390(args, L"PlayerName", value, 0);
  value_destroy_140bf86b0(value); text_destroy_140683490(temporary);
  Text* looked_up = lookup_141e5f780(service_141e5b2a0(),
                                    static_cast<const char*>(change_team_info(self)) + 8);
  Text* converted = convert_142c0c460(looked_up);
  Text* formatted = format_142c255f0(converted, args);
  text_swap_native(output, formatted);
  text_destroy_140683490(formatted); text_destroy_140683490(looked_up);
  Object* world = world_v150(self);
  if (world && !text_predicate_142c2e440(output)) {
    Object* state = game_state_141641350(world);
    if (state) dispatch_a68(state, output, 0);
  }
  map_destroy_140799530(args);
  string_destroy(killer_name); string_destroy(player_name);
  string_destroy(killer_style); string_destroy(victim_style);
  text_destroy_140683490(output);
}

Object* owner_d8(State*); Object* controller_child_v28(Object*); Object* child_6d8(Object*);
void mark_1419eded0(Object*, bool); Object* kill_target_1419e4e60(Object*);
Object* dbno_target_1419e4e30(Object*); void report_1406ba850(Object*, Object*);
bool predicate_14241bc10(Object*, Object*); bool runtime_option_122();
void thunk_142810956(Object*, int, int); void effect_14269e270(Object*, Local36*);
void effect_1419b6d20(Object*); void stub_empty();
void report_common_tail(Object* controller, Object* player) {
  Object* child = controller_child_v28(controller); // virtual at controller+598, slot +28
  if (child && child_6d8(child)) { Local36 output; effect_14269e270(child_6d8(child), &output); }
  if (player) effect_1419b6d20(player);
  stub_empty();
}
void ClientReportKill_Implementation(State* self, Object* player) {
  Object* controller = owner_d8(self);
  if (!controller || !is_a(controller, controller_class())) return;
  if (!predicate_14241bc10(world_v150(self), player)) {
    mark_1419eded0(controller, false);
    report_1406ba850(kill_target_1419e4e60(controller), player);
  }
  if (runtime_option_122()) thunk_142810956(controller, 0, 0);
  report_common_tail(controller, player);
}
void ClientReportDBNO_Implementation(State* self, Object* player) {
  Object* controller = owner_d8(self);
  if (!controller || !is_a(controller, controller_class())) return;
  mark_1419eded0(controller, true);
  report_1406ba850(dbno_target_1419e4e30(controller), player);
  report_common_tail(controller, player);
}
Object* get_141e639d0(Object*); void notify_1422aea50(Object*, const FString*, const FString*);
void ClientNotifyMatchEntered_Implementation(State* self, const FString* window, const FString* event) {
  Object* world = world_v150(self);
  if (!world) return;
  Object* object = field(world, 0x188);
  if (object && is_a(object, class_1428ea7f0()))
    notify_1422aea50(get_141e639d0(object), window, event);
}
struct ErrorText; int array_num(const Array*); const void* array_data_indirect(const Array*);
bool predicate_14199e3c0(Object*); ErrorText* error_text_141f9e080(const void*);
void dispatch_error_a68(Object*, ErrorText*, int); void error_text_release(ErrorText*);
void ClientAddKillFeedErrorMessage_Implementation(State* self, const Array* messages) {
  Object* world = world_v150(self);
  Object* gs = world ? field(world, 0x130) : nullptr;
  if (gs && !is_a(gs, game_state_class())) gs = nullptr;
  if (array_num(messages) <= 0 || !gs || !predicate_14199e3c0(gs)) return;
  const char* begin = static_cast<const char*>(array_data_indirect(messages));
  const char* end = begin + array_num(messages) * 0x30;
  for (const char* item = begin; item != end; item += 0x30) {
    ErrorText* temporary = error_text_141f9e080(item);
    dispatch_error_a68(gs, temporary, 0);
    error_text_release(temporary); // atomic strong release, then weak release on last strong
  }
}
}
