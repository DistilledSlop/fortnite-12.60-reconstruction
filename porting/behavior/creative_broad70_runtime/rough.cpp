// Uncounted behavioral lifts; adapters below have no implementation and this is not a build input.
namespace creative_broad70 {
struct Controller; struct Profile; struct String; struct Tags; struct Island; struct PlotType;
struct FormatArgs; struct Text; struct Callback; struct IslandList; struct FourStrings;
Profile* profile(Controller*); int mode_143f7a920(Controller*); int profile_max_1421d7c50(Profile*);
int cached_max(Controller*); void set_cached_max(Controller*, int); int count_1419d60f0(Controller*, int);
int array_num(const String*); Island* find_1419b6c30(Controller*, const String*);
unsigned long long island_stamp_198(Island*); constexpr unsigned long long active_stamp = 0x2bca2875f4373fffULL;
// Formatting constructs and destroys the map and reference-counted text temporaries exactly as source.
FormatArgs* named_arg_1407babb0(const wchar_t*, const String*); void destroy_args_140799530(FormatArgs*);
Text* named_text_142c255f0(const wchar_t*, const wchar_t*, FormatArgs*);
Text* slots_text_142c253c0(int max, int current); Text* simple_text_142bedc40(const wchar_t*, const wchar_t*);
void error_log_if_gt1(Controller*, const wchar_t* operation, Text*, const String* missing_log_arg);
void response_142a52910(Controller*, int, Text*); void response_142a52a50(Controller*, int, Text*);
void response_142a52870(Controller*, int, Text*); void response_142a529b0(Controller*, int, Text*);
void response_142a52af0(Controller*, int, Text*); void release_text(Text*);
// 1419b6100 constructs a bound callback using the indicated target and copied context.
// 1410eb5f0 wraps it; release_builder reproduces conditional v38/realloc/free.
Callback* bound_callback(Controller*, unsigned long long target, const String* context);
Callback* create_callback_1419e2340(Controller*); void release_builder(Callback*);
void release_callback_envelope(Callback*); // invokes contained v10 if present; frees string and 140692d60
void duplicate_1429a8d80(Profile*, const String*, const String*, const String*, Callback*);
void restore_1429a9dd0(Profile*, const String*, Callback*); void destroy_1429a8b60(Profile*, const String*, Callback*);
void create_from_link_1429a83e0(Profile*, const String*, const String*, const String*, Callback*);
// Title vtable +260 result and its temporary allocation are preserved here, despite recovered FString signature.
void create_1429a8130_title_v260(Profile*, const String*, const String*, PlotType*, Callback*);
int max_plots(Controller* c) {
  if (mode_143f7a920(c) == 1) {
    Profile* p = profile(c); if (!p) return 0;
    int n = profile_max_1421d7c50(p); set_cached_max(c, n); return n;
  }
  return cached_max(c);
}
void fail_named(Controller* c, int kind, const wchar_t* operation, const wchar_t* key,
                const wchar_t* msg, FormatArgs* args, const String* log_arg) {
  Text* t = named_text_142c255f0(msg, key, args);
  error_log_if_gt1(c, operation, t, log_arg);
  if (kind == 0) response_142a52910(c, 0, t);
  else if (kind == 1) response_142a52a50(c, 0, t);
  else response_142a52870(c, 0, t);
  release_text(t);
}
void fail_slots(Controller* c, int kind, const wchar_t* operation, int max, int current) {
  Text* t = slots_text_142c253c0(max, current);
  error_log_if_gt1(c, operation, t, nullptr);
  if (kind == 0) response_142a52910(c, 0, t);
  else if (kind == 1) response_142a52a50(c, 0, t);
  else response_142a529b0(c, 0, t);
  release_text(t);
}
void DuplicateCreativePlot_Implementation(Controller* c, const String* title, const String* locale, const String* id) {
  if (!profile(c)) return;
  Island* island = find_1419b6c30(c, id); // decompiler omitted arguments; verify ABI
  FormatArgs* args = named_arg_1407babb0(L"MCPDuplicateMissing", title);
  int max = max_plots(c), current = count_1419d60f0(c, 0);
  if (current >= max) fail_slots(c, 0, L"DuplicateCreativePlot", max, current);
  else if (!island) fail_named(c, 0, L"DuplicateCreativePlot", L"ErrorCreativeDuplicatePlotErrorMissing", L"Unable to find island {MCPDuplicateMissing}", args, id);
  else if (island_stamp_198(island) != active_stamp) fail_named(c, 0, L"DuplicateCreativePlot", L"ErrorCreativeDuplicatePlotErrorDeleted", L"Island {MCPDuplicateMissing} is deleted!", args, nullptr);
  else if (array_num(locale) < 2) fail_named(c, 0, L"DuplicateCreativePlot", L"ErrorCreativeDuplicatePlotErrorNoLocale", L"We could not figure out what locale {MCPDuplicateMissing} was in!", args, nullptr);
  else { Callback* cb = bound_callback(c, 0x1419e0c20, title); release_builder(cb); duplicate_1429a8d80(profile(c), title, locale, id, cb); release_callback_envelope(cb); }
  destroy_args_140799530(args);
}
void RestoreCreativePlot_Implementation(Controller* c, const String* id) {
  if (!profile(c)) return;
  Island* island = find_1419b6c30(c, id);
  FormatArgs* args = named_arg_1407babb0(L"MCPDestroyMissing", id);
  int max = max_plots(c), current = count_1419d60f0(c, 0);
  if (current >= max) fail_slots(c, 1, L"RestoreCreativePlot", max, current);
  else if (!island) fail_named(c, 1, L"RestoreCreativePlot", L"ErrorCreativePlotErrorMissing", L"Unable to find island {MCPDestroyMissing}", args, id);
  else if (island_stamp_198(island) == active_stamp) fail_named(c, 1, L"RestoreCreativePlot", L"ErrorCreativePlotErrorNotDeleted", L"Island {MCPDestroyMissing} is not deleted", args, nullptr);
  else { Callback* cb = bound_callback(c, 0x1419e8000, id); release_builder(cb); restore_1429a9dd0(profile(c), id, cb); release_callback_envelope(cb); }
  destroy_args_140799530(args);
}
void DestroyCreativePlot_Implementation(Controller* c, const String* id) {
  if (!profile(c)) return;
  Island* island = find_1419b6c30(c, id);
  FormatArgs* args = named_arg_1407babb0(L"MCPDestroyMissing", id);
  if (!island) fail_named(c, 2, L"DestroyCreativePlot", L"DestroyCreativePlotErrorMissing", L"Unable to find island {MCPDestroyMissing}", args, id);
  else if (island_stamp_198(island) != active_stamp) fail_named(c, 2, L"DestroyCreativePlot", L"DestroyCreativePlotErrorAlreadyDeleted", L"Island {MCPDestroyMissing} is already deleted", args, nullptr);
  else { Callback* cb = bound_callback(c, 0x1419e0520, id); release_builder(cb); destroy_1429a8b60(profile(c), id, cb); release_callback_envelope(cb); }
  destroy_args_140799530(args);
}
void MakeNewCreativePlotFromLinkCode_Implementation(Controller* c, const String* title, const String* locale, const String* code) {
  if (!profile(c)) return;
  if (array_num(locale) < 2) { Text* t = simple_text_142bedc40(L"Could not determine Locale for Island.", L"IslandCreateNoLocale"); error_log_if_gt1(c,L"MakeNewCreativePlotFromLinkCode",t,nullptr); response_142a529b0(c,0,t); release_text(t); return; }
  int max = max_plots(c), current = count_1419d60f0(c, 0);
  if (current >= max) { fail_slots(c, 2, L"MakeNewCreativePlotFromLinkCode", max, current); return; }
  Callback* cb = create_callback_1419e2340(c); release_builder(cb);
  create_from_link_1429a83e0(profile(c), title, locale, code, cb); release_callback_envelope(cb);
}
void MakeNewCreativePlot_Implementation(Controller* c, const String* title, const String* locale, PlotType* type) {
  if (!profile(c)) return;
  int max = max_plots(c), current = count_1419d60f0(c, 0);
  if (current >= max) { fail_slots(c, 2, L"MakeNewCreativePlot", max, current); return; }
  Callback* cb = create_callback_1419e2340(c); release_builder(cb);
  create_1429a8130_title_v260(profile(c), title, locale, type, cb); release_callback_envelope(cb);
}
// 1421d91b0 returns the profile's pointer array; copy pointer slots before iteration.
IslandList* copy_island_pointer_array_1421d91b0(Profile*); int island_list_num(IslandList*);
Island* island_list_at(IslandList*, int); void free_island_pointer_array(IslandList*);
// Compares the first description tag (or empty literal) with a copied island+90 string.
int compare_first_tag_to_island_90_142bcf780(const Tags*, Island*);
FourStrings* copy_strings_100_110_130_158(Island*); void free_four_strings(FourStrings*);
void update_tags_1429aae00(Profile*, const Tags*, const String*, FourStrings*, void* converted_id, Callback*);
void* convert_id_1419d9d90(const String*); void free_converted_id_strings(void*);
void UpdateCreativeIslandDescriptionTags_Implementation(Controller* c, const Tags* tags, const String* locale, const String* id) {
  if (!profile(c)) return;
  IslandList* list = copy_island_pointer_array_1421d91b0(profile(c));
  FourStrings* fields = nullptr;
  for (int i = 0; i < island_list_num(list); ++i) {
    Island* island = island_list_at(list, i);
    if (compare_first_tag_to_island_90_142bcf780(tags, island) != 0) continue;
    fields = copy_strings_100_110_130_158(island);
    if (island_stamp_198(island) != active_stamp) {
      Text* t = simple_text_142bedc40(L"You cannot modify tags on deleted Islands.", L"IslandModifyTagsDeletedIsland");
      error_log_if_gt1(c, L"DeletedCreativePlot", t, nullptr); response_142a52af0(c, 0, t); release_text(t);
    } else {
      // Decompiler passes tags to a String-typed copy helper; preserve the raw context pending ABI recovery.
      Callback* cb = bound_callback(c, 0x1419e9220, reinterpret_cast<const String*>(tags)); release_builder(cb);
      void* converted = convert_id_1419d9d90(id);
      update_tags_1429aae00(profile(c), tags, locale, fields, converted, cb);
      free_converted_id_strings(converted); release_callback_envelope(cb);
    }
    free_four_strings(fields); free_island_pointer_array(list); return;
  }
  Text* t = simple_text_142bedc40(L"The Island you tried to modify is not one of your valid islands.", L"IslandModifyTagsNotAValidIsland");
  error_log_if_gt1(c, L"InvalidCreativePlot", t, nullptr); response_142a52af0(c, 0, t); release_text(t);
  free_four_strings(fields); free_island_pointer_array(list);
}
}
