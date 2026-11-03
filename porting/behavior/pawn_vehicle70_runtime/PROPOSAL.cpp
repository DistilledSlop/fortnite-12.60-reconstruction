// Uncounted control-flow drafts, not ABI-compatible native code.
#include <cstdint>
struct Pawn; struct Object; struct WrapContext; struct Material; struct Wrap; struct Controller;
struct Attachment { uint64_t first, second; }; // exact 16-byte representation, semantics unresolved
struct VariantArray; struct VariantInfo; struct ApplyArgs;
extern Object*& attachment_component(Pawn*); extern Attachment& attachment_mesh(Pawn*);
extern Object* pawn_mesh(Pawn*); extern unsigned char pawn_role(Pawn*); extern bool pawn_v6b0(Pawn*);
extern void update_attachment_142359070(Pawn*); extern void send_attachment_142a775e0(Pawn*, const Attachment&);
extern void destroy_component_1440d4a00(Object*); extern void mark_object_flag_20000000(Object*); // GUObjectArray lookup/mark
extern bool attachment_log_enabled_3(); extern void attachment_log_pawn(Pawn*, const wchar_t*); // formats pawn name and releases temporary
void ServerSetAttachment_Implementation(Pawn* p, const Attachment& input) {
 const uint64_t first=input.first;
 if (first) {
  const uint64_t second=input.second;
  const bool allowed=pawn_v6b0(p); const unsigned char role=pawn_role(p);
  if (!allowed && role!=3) {
   if (attachment_log_enabled_3()) attachment_log_pawn(p,L"AFortPlayerPawn::AddAttachment - Client is trying to add an attachment to someone else's pawn. Pawn: %s");
   return;
  }
  if (!pawn_mesh(p)) {
   if (attachment_log_enabled_3()) attachment_log_pawn(p,L"AFortPlayerPawn::AddAttachment - Cannot add attachment to a player pawn which does not have a mesh. Pawn: %s");
   return;
  }
  attachment_mesh(p)={first,second}; update_attachment_142359070(p);
  if (role!=3) { Attachment copy=attachment_mesh(p); send_attachment_142a775e0(p,copy); }
  return;
 }
 if (!attachment_mesh(p).first) return;
 const bool allowed=pawn_v6b0(p); const unsigned char role=pawn_role(p);
 if (!allowed && role!=3) {
  if (attachment_log_enabled_3()) attachment_log_pawn(p,L"AFortPlayerPawn::RemoveAttachment - Client is trying to remove an attachment from someone else's pawn. Pawn: %s");
  return;
 }
 attachment_mesh(p)={0,0};
 if (attachment_component(p)) {
  destroy_component_1440d4a00(attachment_component(p));
  mark_object_flag_20000000(attachment_component(p)); // re-read after call as original does
  attachment_component(p)=nullptr;
 }
 if (role!=3) { Attachment copy=attachment_mesh(p); send_attachment_142a775e0(p,copy); }
}

// Opaque storage supplied by integration; helpers below map to individually bounded native operations.
extern bool cosmetic_initialized(Pawn*); extern bool wrap_log_enabled_3();
extern void log_uninitialized_loadout(Pawn*,Object*); // name temporaries and cleanup
extern Material* mesh_get_material_v548(Object*,int); extern bool material_v450(Material*);
extern void wrap_context_zero(WrapContext&); extern void wrap_context_copy_140a003b0(WrapContext&,void*);
extern void* loadout_142323510(Pawn*,int); extern Wrap* find_wrap_141d4abb0(WrapContext&,int);
extern Object* pawn_controller(Pawn*); extern Controller* controller_if_142a3d9f0(Object*);
extern void variant_array_zero(VariantArray&); extern void gather_variants_141d50e00(WrapContext&,Wrap*,VariantArray&);
extern Material* select_material_141d85c80(Wrap*,int,VariantArray&,Controller*);
extern Object* mesh_set_material_v568(Object*,int,Material*); extern void call_144335300(Object*,Material*,bool);
extern void append_variant_deep_copies(WrapContext&,VariantArray&); // reserve and copy each 0x48 entry in native order
extern void apply_args_init(ApplyArgs&); extern void wrap_v4e0(Wrap*,int,WrapContext&,ApplyArgs&);
extern void apply_args_free_two_arrays(ApplyArgs&); extern void variant_array_destroy(VariantArray&);
extern void wrap_context_destroy(WrapContext&); // frees trailing arrays, each variant's three fields, array, leading arrays in native order
void ApplyWrapToSaveTheWorldHoverboard(Pawn* p, WrapContext& context, VariantArray& variants, ApplyArgs& args) {
 Object* mesh=attachment_component(p); if (!mesh) return;
 // The nested null-mesh / null-pawn diagnostics in the inline body are unreachable
 // after dereferencing p and returning for null mesh; preserve the reachable flow.
 Material* previous=mesh_get_material_v548(mesh,0);
 const bool mode=previous && material_v450(previous);
 wrap_context_zero(context);
 if (!cosmetic_initialized(p)) {
  if (wrap_log_enabled_3()) log_uninitialized_loadout(p,mesh);
  wrap_context_destroy(context); return;
 }
 wrap_context_copy_140a003b0(context,loadout_142323510(p,0));
 variant_array_zero(variants);
 Wrap* wrap=find_wrap_141d4abb0(context,0);
 if (wrap) {
  Object* controller=pawn_controller(p);
  Controller* typed=controller ? controller_if_142a3d9f0(controller) : nullptr;
  gather_variants_141d50e00(context,wrap,variants);
  Material* material=select_material_141d85c80(wrap,static_cast<int>(mode)+1,variants,typed);
  if (material && material!=previous) {
   Object* result=mesh_set_material_v568(mesh,0,material);
   if (result) call_144335300(result,previous,true);
  }
  append_variant_deep_copies(context,variants);
  apply_args_init(args); wrap_v4e0(wrap,0,context,args); apply_args_free_two_arrays(args);
 }
 variant_array_destroy(variants); wrap_context_destroy(context);
}
