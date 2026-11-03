// Uncounted source lift; object/container layouts and individual call ABIs are unresolved.
#include <cstdint>
namespace round_winner80 {
using B = std::uint8_t;
using U32 = std::uint32_t;
using U64 = std::uint64_t;
using P = B*;
template<class T> T& at(P p, std::int64_t o) { return *reinterpret_cast<T*>(p + o); }

// Individual missing native effects / ABI contracts, not implementations.
extern int log_verbosity();
extern void log_non_team_14208c5c0();
extern void log_winning_bucket_missing_varargs_14208c5c0();
extern bool stub_return_false();
extern void initialize_142c0c240(P record);
// The two stack inputs retain indeterminate high bits; only their low bytes are cleared.
extern U64 indeterminate_stack_58();
extern U32 indeterminate_stack_60();
extern char evaluate_bucket_142094720(P self, int index, P record, P field800,
                                     U64 stack58, U32 stack60, int zero, int four);
struct Results { P data; U32 count; U32 capacity; };
extern void grow_results_14072b8d0(Results* array, U64 old_count);
extern void free_memory(P);
extern void strong_add_ref(P control);
extern int strong_release_atomic(P control);
extern int weak_release_atomic(P control);
extern void destroy_with_control(P control); // vtable[0](control)
extern void destroy_missing_args(P control); // vtable[0]() in decompilation
extern void delete_with_control_and_one(P control); // vtable[1](control, 1)
extern void delete_missing_args(P control); // vtable[1]() in decompilation
extern B indeterminate_sort_stack_byte();
extern void sort_14207b3e0(P data, U32 count, B stack_byte, P stack_address);
extern char predicate_142080390_missing_args();
extern void cleanup_nested_14207f650(P field);
extern void grow_winners_140690b30(P array, int old_count);

void evaluate_round_winner_14208c5c0(P self) {
    if (at<B>(self, 0xf0) != 3) return;
    U32 winner;
    bool chosen = false;
    if (at<B>(self, 0xc08) != 0xff) {
        if (!at<B>(self, 0x91a) || !at<B>(self, 0x91b)) {
            if (log_verbosity() >= 5) log_non_team_14208c5c0();
            return;
        }
        if (stub_return_false()) {
            winner = at<B>(self, 0xc08);
            chosen = true;
        } else {
            winner = 0;
            while (static_cast<int>(winner) < at<int>(self, 0xc88)) {
                if (at<B>(self, 0xc08) == at<B>(at<P>(self, 0xc80), 1 + winner * 0x18ULL)) {
                    chosen = true;
                    break;
                }
                ++winner;
            }
        }
    }
    if (!chosen) {
        const B saved = at<B>(self, 0xad0);
        at<B>(self, 0xad0) = 0;
        Results array{nullptr, 0, 0};
        P& results = array.data;
        U32& count = array.count;
        U32& capacity = array.capacity;
        // The original reuses these stack slots across iterations. Acquire their
        // indeterminate contents only when the loop is entered.
        alignas(8) B record[0x50];
        U64 stack58;
        U32 stack60;
        if (0 < at<int>(self, 0xc88)) {
            stack58 = indeterminate_stack_58();
            stack60 = indeterminate_stack_60();
        }
        for (int index = 0; index < at<int>(self, 0xc88); ++index) {
            // Unwritten record bytes carry over in the reused stack storage.
            initialize_142c0c240(record);
            stack58 &= ~U64(0xff);
            stack60 &= ~U32(0xff);
            at<U32>(record, 0x18) = 0xffffffff;
            at<P>(record, 0x20) = nullptr;
            at<U64>(record, 0x28) = 0;
            at<U32>(record, 0x30) = 0;
            at<B>(record, 0x34) = 0;
            at<U64>(record, 0x38) = ~U64(0);
            at<P>(record, 0x40) = nullptr;
            at<U64>(record, 0x48) = 0;
            char ok = evaluate_bucket_142094720(self, index, record, self + 0x800, stack58, stack60, 0, 4);
            U32 remaining = 0;
            if (!ok) {
                remaining = at<U32>(record, 0x48);
            } else {
                U32 old_count = count++;
                if (static_cast<int>(capacity) < static_cast<int>(count))
                    grow_results_14072b8d0(&array, old_count);
                P dst = results + static_cast<std::int64_t>(static_cast<int>(old_count)) * 0x50;
                at<U64>(dst, 0) = at<U64>(record, 0);
                at<P>(dst, 8) = at<P>(record, 8);
                if (at<P>(record, 8)) strong_add_ref(at<P>(record, 8));
                at<U32>(dst, 0x10) = at<U32>(record, 0x10);
                at<U64>(dst, 0x20) = 0;
                at<U32>(dst, 0x18) = at<U32>(record, 0x18);
                at<P>(dst, 0x20) = at<P>(record, 0x20);
                at<P>(record, 0x20) = nullptr;
                at<U64>(dst, 0x28) = at<U64>(record, 0x28);
                at<U64>(record, 0x28) = 0;
                at<U32>(dst, 0x30) = at<U32>(record, 0x30);
                at<B>(dst, 0x34) = at<B>(record, 0x34);
                at<U32>(dst, 0x38) = at<U32>(record, 0x38);
                at<U64>(dst, 0x40) = 0;
                at<U32>(dst, 0x3c) = at<U32>(record, 0x3c);
                at<P>(dst, 0x40) = at<P>(record, 0x40);
                at<P>(record, 0x40) = nullptr;
                at<U64>(dst, 0x48) = at<U64>(record, 0x48);
                at<U64>(record, 0x48) = 0;
            }
            P entry = at<P>(record, 0x40);
            if (remaining) {
                entry += 8;
                do {
                    if (at<P>(entry, 0x18)) free_memory(at<P>(entry, 0x18));
                    P control = at<P>(entry, 0);
                    if (control && strong_release_atomic(control) == 1) {
                        destroy_with_control(control);
                        if (weak_release_atomic(control) == 1) delete_with_control_and_one(control);
                    }
                    entry += 0x40;
                } while (--remaining != 0);
            }
            if (at<P>(record, 0x40)) free_memory(at<P>(record, 0x40));
            if (at<P>(record, 0x20)) free_memory(at<P>(record, 0x20));
            P control = at<P>(record, 8);
            if (control && strong_release_atomic(control) == 1) {
                destroy_missing_args(control);
                if (weak_release_atomic(control) == 1) delete_with_control_and_one(control);
            }
        }
        P saved_results = results;
        at<B>(self, 0xad0) = saved;
        if (static_cast<int>(count) < 2) {
            winner = 0xffffffff;
            if (count == 1) winner = at<U32>(results, 0x3c);
        } else {
            B sort_stack[8];
            sort_stack[0] = indeterminate_sort_stack_byte();
            sort_14207b3e0(results, count, sort_stack[0], sort_stack);
            winner = 0xffffffff;
            if (predicate_142080390_missing_args()) winner = at<U32>(saved_results, 0x3c);
        }
        if (count != 0) {
            P field = results + 0x20;
            int left = static_cast<int>(count);
            do {
                cleanup_nested_14207f650(field + 0x20);
                if (at<P>(field, 0)) free_memory(at<P>(field, 0));
                P control = at<P>(field, -0x18);
                if (control && strong_release_atomic(control) == 1) {
                    destroy_missing_args(control);
                    if (weak_release_atomic(control) == 1) delete_missing_args(control);
                }
                field += 0x50;
            } while (--left != 0);
        }
        if (results) free_memory(results);
    }
    if (log_verbosity() >= 5) log_winning_bucket_missing_varargs_14208c5c0();
    int old_count = at<int>(self, 0xc78);
    at<int>(self, 0xc78) = old_count + 1;
    if (at<int>(self, 0xc7c) < old_count + 1) grow_winners_140690b30(self + 0xc70, old_count);
    at<U32>(at<P>(self, 0xc70), static_cast<std::int64_t>(old_count) * 4) = winner;
}
}
