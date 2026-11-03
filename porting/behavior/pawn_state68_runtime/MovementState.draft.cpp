// Uncounted typed behavioral lift, not compiled integration source.
// Original runtime image, native SetInVortex 0x14234d650 and HandleInStasis
// inlined by exec 0x142a7cc50. U_* are unresolved calls/fields, not stubs.
// Pointer validity, component lifetime and native multicast binding APIs remain open.

void SetInVortex_draft(Pawn* self, bool entering, float upwardVelocity,
                       float floorAltitude, float floorWidth, float gravityScalar,
                       float terminalVelocity)
{
    if (((U_byte(self, 0x10e7) >> 5) & 1) == entering) return;
    if (entering && U_role(self) == 3 && U_is_class_14285d610(self) &&
        U_time_144217c50(self) - U_float(self, 0x2654) < 2.0f)
        return;
    U_write_byte(self, 0x10e7, (U_byte(self, 0x10e7) & 0xdf) | (uint8(entering) << 5));
    U_virtual_5e8(self);
    if (U_role(self) != 1 && (U_byte(self, 0x10e7) & 0x20))
    {
        auto* movement = U_ptr(self, 0x280);
        if (terminalVelocity >= 0.0f) terminalVelocity = 0.0f;
        U_write_float_bits(self, 0x22c4, U_float_bits(floorWidth) & 0x7fffffff);
        U_write_float(self, 0x22c0, floorAltitude);
        U_write_float(self, 0x22c8, gravityScalar);
        U_write_float(self, 0x22cc, terminalVelocity);
        bool excludedMode = U_byte(movement, 0x168) == 6 &&
            (U_byte(movement, 0x169) == 4 || U_byte(movement, 0x169) == 3);
        if (!excludedMode && !U_ptr(self, 0x1578))
        {
            U_write_byte(self, 0x10e7, U_byte(self, 0x10e7) | 0x10);
            U_14230e1e0(self);
            if (upwardVelocity <= 100.0f) upwardVelocity = 100.0f;
            auto* controller = U_ptr(self, 0x250);
            if (controller && U_is_FortPlayerController(controller))
                U_1423bd2b0(controller, U_shared_name_148157ed8, self, false, -1.0f);
            U_write_byte(self, 0x520, U_byte(self, 0x520) | 4);
            U_virtual_7c8(self, Vector(0.0f, 0.0f, upwardVelocity), false, false);
        }
        const uint8 flagsBeforeUpdate = U_byte(self, 0x10e7);
        U_14230e1e0(self);
        if (flagsBeforeUpdate & 4)
        {
            auto* gameMode = U_141b85db0(U_world(self));
            if (gameMode)
            {
                auto* target = U_virtual_bf0(gameMode);
                if (target) U_141a57170(target, U_1422fbf40(self));
            }
        }
        U_write_byte(self, 0x10e7, U_byte(self, 0x10e7) & 0xf7);
        U_write_float(self, 0x2654, U_time_144217c50(self));
    }
}

void HandleInStasis_draft(Pawn* self)
{
    const uint8 previousGuard = U_byte(self, 0x1066);
    U_write_byte(self, 0x1066, 1);
    auto* controller = U_ptr(self, 0x250);
    if (controller && U_is_FortPlayerController(controller))
    {
        switch (U_byte(controller, 0x1fa0))
        {
        case 1: U_controller_virtual_710(controller, false); break;
        case 2: case 4:
            U_controller_virtual_710(controller, false);
            U_controller_virtual_728(controller, false); break;
        case 3: case 5:
            U_controller_virtual_710(controller, false);
            U_movement_virtual_590(U_ptr(self, 0x280), true); break;
        }
        U_write_byte(controller, 0x1fa0, U_byte(self, 0x1065));
        switch (U_byte(self, 0x1065))
        {
        case 0:
            if (U_byte(self, 0x10e7) & 1)
                U_thunk_140c31764(self, (U_byte(self, 0x10e7) >> 2) & 1);
            break;
        case 1: U_controller_virtual_710(controller, true); break;
        case 2: case 4:
            U_controller_virtual_710(controller, true);
            U_controller_virtual_728(controller, true); break;
        case 3: case 5:
            U_controller_virtual_710(controller, true);
            U_virtual_420(U_virtual_628(self));
            U_movement_virtual_5b8(U_ptr(self, 0x280)); break;
        }
    }
    const uint8 modeForRoleCheck = U_byte(self, 0x1065);
    if (U_role(self) == 1 && ((uint32(modeForRoleCheck) - 1) & 0xfc) == 0 && modeForRoleCheck != 3)
        U_virtual_420(U_virtual_628(self));
    const uint8 broadcastMode = U_byte(self, 0x1065);
    // Native delegate +0x10c8 uses lock count +0x10dc, reverse iteration,
    // ExecuteIfSafe(self, broadcastMode), then compact after decrement if needed.
    U_native_multicast_reverse_execute_and_compact(self, 0x10c8, 0x10dc, self, broadcastMode);
    U_write_byte(self, 0x1066, previousGuard);
}
