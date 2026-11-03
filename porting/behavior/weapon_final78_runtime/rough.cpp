// UNCOUNTED rough native bodies, not canonical build inputs.
#include <cstdint>
namespace weapon_final78 {
using U = std::uint64_t;
using B = std::uint8_t;
using I = std::int32_t;
template<class T> T& at(U p, U offset = 0) { return *reinterpret_cast<T*>(p + offset); }
template<U Address, class... A> U call(A...);
// These dispatches have no visible native arguments in the generated source.
extern U virtual_no_visible_arguments(U object, unsigned slot);
extern U virtual_with_receiver(U object, unsigned slot);
// Original UTF-16 wcsstr and DAT_1459a98c4, including null-result behavior.
extern U find_wide_substring(U start, U needle);
extern U delimiter_1459a98c4();

// Preserve the native search from each successive UTF-16 code unit, including
// the final null match case: last + 4 is passed even if no match was found.
U last_match_plus_four(const char16_t* name) {
 U match = find_wide_substring(reinterpret_cast<U>(name), delimiter_1459a98c4());
 U last = 0;
 while (match) {
  last = match;
  match = find_wide_substring(match + 2, delimiter_1459a98c4());
 }
 return last + 4;
}
void bind_montage_callback(U delegate, U weapon, const char16_t* name) {
 U name_value; // Native helper constructs this output before delegate registration.
 call<0x142cd4ab0>(reinterpret_cast<U>(&name_value), last_match_plus_four(name), 1);
 call<0x1406d80a0>(delegate, weapon, name_value);
}
void Native142694ec0(U weapon) {
 U owner = call<0x141125940>(); // No argument appears in this decompilation.
 if (!owner) return;
 U type = call<0x142a174f0>();
 U owner_class = at<U>(owner, 0x10);
 if (at<I>(type, 0x38) > at<I>(owner_class, 0x38)) return;
 I index = at<I>(type, 0x38);
 if (at<U>(at<U>(owner_class, 0x30), static_cast<U>(static_cast<std::int64_t>(index) * 8)) != type + 0x30) return;
 if (!at<U>(owner, 0x278)) return;
 U pawn_anim = call<0x144114bd0>(); // Visible argument list is empty.
 if (!pawn_anim) return;
 U weapon_anim = virtual_with_receiver(weapon, 0xa30);
 if (!weapon_anim) return;
 bind_montage_callback(pawn_anim + 0x48, weapon, u"&AFortWeapon::OnPawnMontageStarted");
 bind_montage_callback(pawn_anim + 0x38, weapon, u"&AFortWeapon::OnPawnMontageBlendingOut");
 bind_montage_callback(weapon_anim + 0x48, weapon, u"&AFortWeapon::OnWeaponMontageStarted");
 bind_montage_callback(weapon_anim + 0x38, weapon, u"&AFortWeapon::OnWeaponMontageBlendingOut");
}

bool WeaponVirtual798(U weapon) { // 0x1426b4ce0
 U world = virtual_no_visible_arguments(weapon, 0x150);
 bool replay = world && static_cast<B>(call<0x144693a30>(world)) != 0;
 U pawn = call<0x141125940>(weapon);
 U component = 0;
 if (pawn) {
  U type = call<0x142a174f0>();
  U pawn_class = at<U>(pawn, 0x10);
  if (at<I>(pawn_class, 0x38) < at<I>(type, 0x38) ||
      at<U>(at<U>(pawn_class, 0x30), static_cast<U>(static_cast<std::int64_t>(at<I>(type, 0x38)) * 8)) != type + 0x30) {
   pawn = 0;
  } else {
   component = virtual_no_visible_arguments(pawn + 0x4c8, 0x10);
  }
 }
 if (static_cast<B>(call<0x141ed8a30>()) && at<U>(weapon, 0x350)) {
  if (static_cast<B>(call<0x141ef9880>())) return false;
 }
 if (replay && pawn && component && !(at<B>(pawn, 0x521) & 8)) {
  if (call<0x14070c910>(component)) return false;
 }
 return at<U>(weapon, 0xa48) != 0;
}
} // namespace weapon_final78
