// Behavioral reconstruction, not an integration-ready ABI declaration.
// Reflected FEvaluationResult (EvaluationResult constructor1428df090), size48:
// allowed=bSucceeded +0; reason=ErrorCode FString +8; metadata=ErrorMessage FText +24.
// Signature arguments: InstigatorActor, ActorClassToSpawn, Location, Rotation.
// Opaque adapters below retain native representation pending typed integration.
// Source: 14198bb80 and 14198ec00 in native91-fresh/native91-body.
#include <cstdint>
#include <cstddef>
#include <cstring>

namespace spawn_policy91 {
using Word = std::uintptr_t;
struct String16 { char16_t* data; int32_t count, capacity; };
struct SharedMetadata { void* value; void* controller; uint32_t extra; };
struct Result {
    uint8_t allowed;
    uint8_t padding[7];
    String16 reason;
    SharedMetadata metadata;
};
struct RuleEntry { void* object; int32_t present; int32_t unknown; };
struct Rules { RuleEntry* data; int32_t count, capacity; };
static_assert(sizeof(String16) == 0x10 && sizeof(RuleEntry) == 0x10);
static_assert(offsetof(Result, reason) == 8 && offsetof(Result, metadata) == 0x18);

// Unresolved ABI adapters. These name the observed operations, not recovered APIs.
// create_class_default invokes the class vtable slot +390; class_default reads +130.
extern void* class_default(void* actor_class);
extern void create_class_default(void* actor_class);
extern Rules* rules_at_game_state_20f8(void* game_state);
// Calls rule vtable +48 with (rule, out, context, class_default, arg5, arg6).
extern void evaluate_rule(void* rule, Result* out, Word context, void* class_default,
                          Word arg5, Word arg6);
extern const SharedMetadata* default_metadata_1407c2c80();
// Strong add/release are atomic. Release decrements strong count at +8; if it
// reaches zero call controller vtable +0, decrement weak count at +c, and if
// that reaches zero call controller vtable +8 with argument 1.
extern void add_strong(void* controller);
extern void release_strong(void* controller);
extern void free_engine_allocation(void* ptr); // 142bee710
// Literal construction mirrors the two array calls and the independent deep copy.
extern void reserve_string_140680160(String16*, int32_t additional);
extern void grow_string_1406800e0(String16*);
extern void allocate_result_string_140690ac0(String16*, int32_t count, int32_t zero);

static void copy_metadata(SharedMetadata& out, const SharedMetadata& source) {
    out.value = source.value;
    out.controller = source.controller;
    if (out.controller) add_strong(out.controller);
    out.extra = source.extra;
}

// 14198ec00: ordered short-circuit evaluation of a 16-byte-entry array.
Result* EvaluateSpawnRules(Rules* rules, Result* out, Word context,
                           void* class_default_object, Word arg5, Word arg6) {
    RuleEntry* end = rules->data + rules->count;
    for (RuleEntry* entry = rules->data; entry != end; ++entry) {
        // A zero presence field leads to a null virtual dispatch in the original;
        // no skip, defensive check, or alternative policy is implied.
        void* rule = entry->present == 0 ? nullptr : entry->object;
        Result temporary; // callee constructs all fields used below
        evaluate_rule(rule, &temporary, context, class_default_object, arg5, arg6);
        if (!temporary.allowed) {
            out->allowed = 0;
            out->reason = temporary.reason; // move allocation, count, capacity
            temporary.reason = {nullptr, 0, 0};
            copy_metadata(out->metadata, temporary.metadata);
            if (temporary.metadata.controller) release_strong(temporary.metadata.controller);
            if (temporary.reason.data) free_engine_allocation(temporary.reason.data);
            return out;
        }
        if (temporary.metadata.controller) release_strong(temporary.metadata.controller);
        if (temporary.reason.data) free_engine_allocation(temporary.reason.data);
    }
    const SharedMetadata* metadata = default_metadata_1407c2c80();
    out->allowed = 1;
    out->reason = {nullptr, 0, 0};
    copy_metadata(out->metadata, *metadata);
    return out;
}

// 14198bb80: valid class -> policy, otherwise false with Invalid_Class.
Result* CanSpawnActorOfClass(void* game_state, Result* out, Word context,
                             void* actor_class, Word arg5, Word arg6) {
    void* cdo = nullptr;
    if (actor_class) {
        cdo = class_default(actor_class);
        if (!cdo) {
            create_class_default(actor_class);
            cdo = class_default(actor_class);
        }
    }
    if (cdo)
        return EvaluateSpawnRules(rules_at_game_state_20f8(game_state), out,
                                  context, cdo, arg5, arg6);

    String16 tmp{nullptr, 0, 0};
    reserve_string_140680160(&tmp, 14);
    tmp.count += 14;
    if (tmp.count > tmp.capacity) grow_string_1406800e0(&tmp);
    static constexpr char16_t invalid_class[] = u"Invalid_Class";
    std::memcpy(tmp.data, invalid_class, 28); // includes terminating UTF-16 zero
    const SharedMetadata* metadata = default_metadata_1407c2c80();
    out->allowed = 0;
    out->reason.data = nullptr;
    out->reason.count = tmp.count;
    if (tmp.count == 0) out->reason.capacity = 0;
    else {
        allocate_result_string_140690ac0(&out->reason, tmp.count, 0);
        std::memcpy(out->reason.data, tmp.data, static_cast<int64_t>(tmp.count) * 2);
    }
    copy_metadata(out->metadata, *metadata);
    if (tmp.data) free_engine_allocation(tmp.data);
    return out;
}
} // namespace spawn_policy91
