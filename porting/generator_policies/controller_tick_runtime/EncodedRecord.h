#pragma once
#include <cstdint>

// 0x1420f8560 / 0x1420f91f0 / 0x1420fe100. The original static
// storage is zero-filled before its constructor; bytes not assigned by the
// constructor must not be assumed initialized for arbitrary storage.
struct FEncodedRecord1420f8560 {
    std::uint8_t ValueSlots[4];
    std::uint8_t ValueIndex;
    std::uint8_t Value;
    std::uint8_t LimitSlots[4];
    std::uint8_t LimitIndex;
    std::uint8_t Limit;
    std::uint8_t Flags;

    void Construct() {
        Flags &= 0xe1;
        ValueIndex = 0; Value = 0x4e;
        LimitIndex = 1; Limit = 0x59; LimitSlots[1] = 0x59;
    }
    void Initialize(std::uint8_t target) {
        for (unsigned i = 0; i < 256 && Value != target; ++i) {
            Value = std::uint8_t(Value + 11);
            ValueIndex = std::uint8_t((ValueIndex + 1) & 3);
            ValueSlots[ValueIndex] = std::uint8_t((Value | 9) ^ i);
        }
        Limit = std::uint8_t(Limit + 11);
        LimitIndex = std::uint8_t((LimitIndex + 1) & 3);
        LimitSlots[LimitIndex] = std::uint8_t(Limit | 9);
        Flags = std::uint8_t((Flags & ~8u) | (Value == target ? 8u : 0u));
    }
    bool Advance(std::uint8_t target) {
        if (Value == target) return true;
        std::uint8_t count = std::uint8_t(LimitSlots[LimitIndex] ^ (Limit | 9));
        Limit = std::uint8_t(Limit + 11);
        LimitIndex = std::uint8_t((LimitIndex + 1) & 3);
        LimitSlots[LimitIndex] = std::uint8_t(Limit | 9);
        if (count > 0xf9) count = 0xff;
        for (unsigned i = 0; i < count && Value != target; ++i) {
            Value = std::uint8_t(Value + 11);
            ValueIndex = std::uint8_t((ValueIndex + 1) & 3);
            ValueSlots[ValueIndex] = std::uint8_t((Value | 9) ^ i);
        }
        return Value == target;
    }
};
static_assert(sizeof(FEncodedRecord1420f8560) == 13, "record layout");

// Apply only after local-controller, PlayerCameraManager camera-zone type and initialized-bit gates.
// A nonzero mask is passed to 0x142111870(controller, mask).
inline std::uint16_t ComputeMask1420fe100(FEncodedRecord1420f8560 (&records)[4], const std::uint8_t (&targets)[4]) {
    std::uint16_t mask = 0;
    for (unsigned i = 0; i != 4; ++i) {
        if (!records[i].Advance(targets[i])) mask |= std::uint16_t(1u << i);
        if (!(records[i].Flags & 8)) mask |= std::uint16_t(1u << (8 + i));
    }
    return mask;
}
