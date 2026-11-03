// Rough source-recovery draft, not a buildable replacement. Client 12.60 CL 13477524.
// Provenance: generated/decomp/AFortPlayerControllerAthena/{named function}.c.
// Types and names below are semantic placeholders; U_* and vtable slots remain unresolved.
// Timer offsets are controller byte offsets. FFrame plumbing, logs and delegate ownership omitted.
using Object = void; using World = void; using TimerManager = void;
struct Vec3 { float x,y,z; }; struct Vec4 { float x,y,z,w; };
struct Fade { Vec4 color; float duration; float delay; };
struct Controller; struct Pawn; struct GameState; struct StreamingLevel;

// exec 142a5d5f0, body at 1419fc...; no arguments.
void StartRespawnLoading(Controller* self) {
    Pawn* pawn = U_controller_pawn(self); // +0x220
    if (!pawn || !U_is_class_142a8ae70(pawn)) return;
    U_v1518(self, U_int_2d98(self), true);
    Vec3 location = U_default_location(); Vec3 rotation = U_default_rotation(); float value = 0;
    U_141a481a0(pawn, &location, &rotation, &value);
    U_141a049f0(self, location, rotation, value);
    U_vd10_respawn_effect(self, U_ptr_2e70(self), /*packed params: bit0=0, byte4=1, float=2*/);
    U_1419f81e0(self);
    World* world = U_world_v150(self);
    if (!world) return;
    // Delegate uses weak self, callback 1419c45c0. Loop true; rate .1; first delay 2.
    U_set_timer(U_world_timer_manager(world), self, 0x2e98, 0.1f, true, 2.0f, U_1419c45c0);
}

// exec 142a564e0, body at 1419c4...; no arguments.
void CheckIfSafeToRespawn(Controller* self) {
    World* world = U_world_v150(self);
    if (!world || !U_1419dbc20(self)) return;
    if (U_14238ec60(self)) {
        Object* p = U_v698(self);
        if (!p || !U_is_class_1447ae380(p)) return;
    }
    GameState* state = U_world_game_state(world); // world +130
    if (state && U_is_class_14290f690(state) && !U_14199e2e0(state,self)) return;
    U_1419c5210(self);
    TimerManager* timers = U_world_timer_manager(world);
    if (U_timer_exists(timers,self,0x2e98)) U_clear_timer(timers,self,0x2e98);
    U_zero_timer_handle(self,0x2e98);
    U_142a53ef0(self);
    Fade fade = U_default_respawn_fade(); // default duration 1.5, delay 5
    if (auto* row = U_141f60890(U_141f5d820(), U_key_2ce4(self))) {
        fade.color = U_row_vec4(row,8); fade.duration = U_row_float(row,0x1c);
        if (U_row_float(row,0x24)>0) fade.delay = U_row_float(row,0x24);
    }
    bool allowed = true;
    state = U_world_game_state(world);
    if (state && U_is_class_14290f690(state)) allowed = U_141989b30(state);
    if (!U_141c794e0(U_controller_pawn(self)) && allowed) {
        // Weak-self delegate 1447baae0: fade alpha to 0; delay is one-shot.
        U_set_fade_timer(timers,self,0x2ea8,fade.delay,fade.color,fade.duration);
    }
    U_1419c4c30(self);
}

// exec 142a56500, body at 1419c4...; no arguments.
void CheckIfSafeToTurnOnCamera(Controller* self) {
    World* world = U_world_v150(self); if (!world) return;
    Object* target = U_v698(self);
    bool safe = target && U_is_class_142a82010(target);
    GameState* state = U_world_game_state(world);
    if (state && U_is_class_14290f690(state)) U_14199ece0(state,&safe,U_v698(self));
    if (safe) {
        target = U_v698(self);
        if (target && U_is_class_142a82010(target)) {
            float value = 0;
            Pawn* pawn = U_controller_pawn(self);
            if (pawn && U_is_class_142a8ae70(pawn)) value = U_141a41950(pawn);
            // target+130 holds camera transform cache: update +180 and derived +190
            // when its +1c0 quaternion differs; use defaults if target+130 is null.
            auto transform = U_refresh_camera_transform_1419c4(target);
            U_141a049f0(self,transform.location,transform.rotation,value);
        }
        U_1419eeb90(self);
    } else {
        // One-shot weak-self delegate 1419c4c30, 0.01 seconds.
        U_set_timer(U_world_timer_manager(world),self,0x2ea0,0.01f,false,-1.0f,U_1419c4c30);
    }
}

// exec 142a5a1d0, body at 1419ee...; no arguments.
void RespawnTurnCameraBackOn(Controller* self) {
    TimerManager* timers = U_140f95ae0(self);
    for (unsigned off : {0x2ea0u,0x2ea8u}) {
        if (U_timer_exists(timers,self,off)) U_clear_timer(timers,self,off);
        U_zero_timer_handle(self,off);
    }
    Fade fade = U_default_respawn_fade(); // duration defaults to 1.5
    if (auto* row = U_141f60890(U_141f5d820(),U_key_2ce4(self))) {
        fade.color = U_row_vec4(row,8); fade.duration = U_row_float(row,0x1c);
    }
    Object* camera = U_camera_2b0(self); // original dereferences without a null check
    U_camera_v7c0_start_fade(camera,U_camera_alpha_254(camera),0,fade.duration,fade.color,true,false);
    U_v1518(self,U_int_2d98(self),false);
    U_1419c2530(self,U_1407c2c80(),false);
    Pawn* pawn = U_controller_pawn(self);
    if (pawn && U_is_class_142a8ae70(pawn)) {
        Object* component = U_141c794e0(pawn);
        if (component && U_byte_870(component)>=1 && U_byte_870(component)<=5 && U_14209aa00(component,pawn))
            U_v1830(self,0,0);
    }
    World* world = U_world_v150(self); if (!world) return;
    timers = U_world_timer_manager(world);
    if (camera && U_is_class_142a35cb0(camera))
        U_set_next_tick_weak(timers,camera,U_141cc1480);
    U_set_timer(timers,self,0x2eb8,U_float_2ca8(self),false,-1.0f,U_1419eeab0);
    U_set_timer(timers,self,0x2ec0,0.01f,true,U_float_2cac(self),U_1419ef1f0);
}

// exec 142a564a0, inlined body 1419c4...; no arguments.
void CheckIfClientLoadedLTM(Controller* self) {
    if (U_flag_1478c2814()) {
        Object* component = U_141c794e0(U_controller_pawn(self));
        if (component && !U_142098cf0(component)) {
            U_log_waiting_relevant_minigame(self); return;
        }
    }
    if (U_flag_1478c2818()) {
        World* world = U_world_v150(self); if (!world) return;
        for (StreamingLevel* level : U_streaming_levels_98(world)) {
            if (!level || U_level_byte_b8(level)==2) continue;
            if (!U_1442dd540(level) && U_level_v270(level)) {
                U_log_waiting_visible(self,level); return;
            }
            if (!U_level_ptr_128(level) && U_level_v268(level)) {
                U_log_waiting_loaded(self,level); return;
            }
        }
    }
    if (U_flag_1478c281c()) {
        Object* owner = U_141c74820(self);
        Object* volume = owner ? U_141c7d420(owner,U_controller_pawn(self)) : nullptr;
        if (volume && U_141c7f410(volume)) {
            U_log_waiting_volume(self,volume); return;
        }
    }
    U_1419fd000(self,true);
}

// exec 142a596c0, inline body; no arguments.
void OnMinigameStarted(Controller* self) {
    U_v1828(self);
    U_140684de0(U_address_3530(self));
    U_1423c06c0(self);
    U_v1a60(self); U_v1a68(self);
    U_1423c8b70(self);
    World* world = U_world_v150(self);
    GameState* state = world ? U_world_game_state(world) : nullptr;
    if (state && U_is_class_14290f690(state)) {
        // Independent predicates, not an else-if chain. Each passes a copied setting
        // from 141ed1800()+offset, true, and the indicated diagnostic name to v1f68.
        if (U_14199e3c0(state)) U_v1f68_setting(self,0x24d0,true,"Creative mini game started");
        if (U_14199e830(state)) U_v1f68_setting(self,0x24d8,true,"Playground mini game started");
        if (U_14199e380(state)) U_v1f68_setting(self,0x24e0,true,"Creative LTM mini game started");
        if (U_14199e2c0(state)) U_v1f68_setting(self,0x24e8,true,"Battle Lab mini game started");
    }
    Object* subsystem = U_141ed4b50(U_world_v150(self));
    if (subsystem) U_141edb1a0(subsystem,true);
    Object* obj = U_14237f3b0(self);
    if (obj) U_14226f940(U_140d91b30(U_142290c00(obj)),false);
}
