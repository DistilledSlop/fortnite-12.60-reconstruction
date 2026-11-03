// Legacy input tick lifecycle: declarations.
// Source image: /mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin (VA = 0x140000000 + offset).
// All "Unknown_<addr>" helpers are address-suffixed placeholders: semantics UNKNOWN, signature inferred from register use only.
// See NOTES.md for asm evidence and the UNKNOWN list.

#include <cstdint>
#include <cstring>

using uint8 = std::uint8_t;
using int32 = std::int32_t;
using uint32 = std::uint32_t;

// Raw field access at a byte offset from a receiver (no field names recovered).
template <typename T>
inline T& At(void* Base, int32 Off) { return *reinterpret_cast<T*>(static_cast<uint8*>(Base) + Off); }

// Vtable slot access: slot is the byte offset used in asm (0x28 = slot 5).
inline void** VTable(void* Obj) { return *reinterpret_cast<void***>(Obj); }
template <typename Fn>
inline Fn VSlot(void* Obj, int32 SlotOffset) { return reinterpret_cast<Fn>(VTable(Obj)[SlotOffset / 8]); }

// ---- Weak pointer helpers (TWeakObjectPtr-shaped 8-byte fields; semantics UNKNOWN) ----
bool  Unknown_142ee7880(const void* WeakField);            // IsValid-shaped: returns al
void* Unknown_142ee5dd0(const void* WeakField);            // Get-shaped: returns object ptr
void* Unknown_142ee3b30(void* OutTmp, void* Obj);          // builds 8-byte weak from Obj (Obj = null gives null weak); returns OutTmp

// ---- Cached widget / shared reference helper ----
void* Unknown_143d5a140(void* Owner, void* OutSharedRef);  // returns ptr whose first qword is the cached widget

// ---- Name / row / record helpers ----
bool  Unknown_142cd8ed0(int32 NameIndex, int32 Zero);      // row-null predicate component (UNKNOWN)
void* Unknown_14078c640(void* RowHandle, const void* Table); // row resolve; Table = image VA 0x1459fa130
void* Unknown_1407c20f0(void* ResolvedRow, void* RowOwner);  // legacy record lookup (global); no null check in asm
int32 Unknown_1407bd580(void* EntryArray, const void* Key);  // global record index find; -1 when absent
int32 Unknown_1407bdba0(void* EntryArray, int32* OutIdx, const void* Key); // panel index find via out param

// ---- Multicast / broadcast helpers ----
void Unknown_1407ac590(void* Multicast, float Progress);     // progress broadcast (xmm1 = clamped progress)
void Unknown_140684de0(void* Multicast);                     // complete broadcast (no args)
void Unknown_140690620(void* Multicast, void* Listener);     // multicast RemoveAll(Listener)-shaped
void Unknown_1406934e0(void* Multicast, bool bValue);        // Broadcast(bool)-shaped tail call
void Unknown_142c54b70(void);                                // verbose log; args not recovered

// ---- Popup helpers (panel) ----
void* Unknown_1426e3bd0(const void* KeyTmp);                 // popup lookup by key (UNKNOWN)
void* Unknown_140811c70(void* Popup);                        // returns registration record; caller adds 0x30
void  Unknown_1407f17d0(void* Popup);                        // RequestClose per HANDOFF; asm call verified only

// ---- Interface / registration helpers (global clear) ----
void* Unknown_14080e600(void* GlobalHandlerObj);             // registration record (+0x30 self ptr, +0x38 index)
void* Unknown_140806b60(void);                               // interface class getter (rcx not set before call)
void* Unknown_142ec9900(void* Obj, void* InterfaceClass);    // interface-address-shaped; returns 2nd pair word

// ---- Panel-side helpers ----
void* Unknown_14081b480(void* InputData);                    // result stored as r14 (current input data)
uint8 Unknown_14081b540(void* InputData);                    // byte query (called twice; first result discarded)
uint8 Unknown_14081b530(void* InputData);                    // index/type query (only used when type == 1)

// ---- Clear / panel clear (external to this pass for the panel one) ----
void Unknown_1407ae640(void* PanelObj);                      // panel clear (HANDOFF "NativeClearHeldAction"); body not translated here

// ---- Eligibility (other worker, treated as external) ----
bool Unknown_1407d0160(void* GlobalHandlerObj, const void* OwnerWeak);

// ---- Translated natives (this pass) ----
// Manager tick (FTickable secondary table 1459ed5f8 slot 0, receiver = manager+0x28).
void UCommonInputManager_Tick_1407ffc50(void* Self, float DeltaTime);
// Panel held tick (interface at panel+0x2d8, vtable slot 0x28).
void PanelHeldTick_140802200(void* Self, float DeltaTime);
// Global held tick (interface at global handler+0x28, vtable slot 0x28).
void GlobalHeldTick_140802440(void* Self, float DeltaTime);
// Global clear (receiver = global handler object).
void GlobalClearHeldOwner_1407ae750(void* GlobalHandlerObj);
// Subsystem Deinitialize (receiver = UCommonUISubsystem).
void CommonUISubsystem_Deinitialize_1407b8610(void* Subsystem);

// Data addresses (image VA).
inline constexpr std::uintptr_t kRowTableVA_1459fa130 = 0x1459fa130; // UTF-16 literal bytes; identity UNKNOWN
inline constexpr std::uintptr_t kGUObjectArrayItems_148263a18 = 0x148263a18;
inline constexpr std::uintptr_t kGUObjectArrayNum_148263a2c = 0x148263a2c;
inline constexpr std::uintptr_t kOneF_1459a86f8 = 0x1459a86f8; // float 1.0 (both clamps)
inline constexpr std::uintptr_t kRegSeed_1459c03e0 = 0x1459c03e0; // 16-byte init seed for scratch arrays (count 0)
