#pragma once

// Original CommonInput Slate preprocessor dispatch. The adapter owns exact UE
// event/key types, blocking state, local-player lookup, and setter invocation.
namespace CommonInputPreprocessorCore {
enum class Type : unsigned char { MouseAndKeyboard = 0, Gamepad = 1, Touch = 2 };
template<class A, class Event>
Type KeyType(A& a, const Event& event) {
    if (!a.IsGamepadKey(event) && !a.IsSimulatedClick()) return Type::MouseAndKeyboard;
    return a.IsAndroidSystemKey(event) ? Type::Touch : Type::Gamepad;
}
template<class A, class Event>
Type PointerType(A& a, const Event& event) {
    if (a.IsTouchEvent(event)) return Type::Touch;
    return a.IsSimulatedClick() ? Type::Gamepad : Type::MouseAndKeyboard;
}
template<class A, class Event>
bool KeyDown(A& a, const Event& event) {
    if (!a.IsActive() || !a.MatchesUser(event)) return false;
    const auto type = KeyType(a, event);
    if (a.IsBlocked(type)) return true;
    a.SetInputType(type);
    return false;
}
template<class A, class Event>
bool Analog(A& a, const Event& event) {
    return a.IsActive() && a.MatchesUser(event) && a.IsBlocked(KeyType(a, event));
}
template<class A, class Event>
bool PointerButton(A& a, const Event& event) {
    if (!a.IsActive() || !a.MatchesUser(event)) return false;
    const auto type = PointerType(a, event);
    if (a.IsBlocked(type)) return true;
    a.SetInputType(type);
    return false;
}
template<class A, class Event>
bool PointerMove(A& a, const Event& event) {
    if (!a.IsActive() || !a.MatchesUser(event)) return false;
    if (a.IgnoreNextMouseMove()) { a.ClearIgnoreNextMouseMove(); return false; }
    // The original compares each absolute component separately and strictly.
    if (!(a.AbsDeltaX(event) > 0.0001f || a.AbsDeltaY(event) > 0.0001f)) return false;
    const auto type = PointerType(a, event);
    if (a.IsBlocked(type)) return true;
    a.SetInputType(type);
    return false;
}
}
