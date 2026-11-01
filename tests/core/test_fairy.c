#include <SDL3/SDL.h>
#include <sotn/fairy.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d %s: %s", __LINE__, #v, SDL_GetError());                                \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static void Put16(Uint8 *p, Uint16 value)
{
    p[0] = (Uint8)value;
    p[1] = (Uint8)(value >> 8);
}
static void Put32(Uint8 *p, Uint32 value)
{
    Put16(p, (Uint16)value);
    Put16(p + 2, (Uint16)(value >> 16));
}
static void Data(Uint8 *p)
{
    for (Uint16 i = 0; i < 10; ++i)
    {
        Put16(p + 0x2408 + i * 14U, 7);
        for (Uint16 j = 1; j < 7; ++j)
            Put16(p + 0x2408 + i * 14U + j * 2U, 255);
    }
    for (Uint16 i = 0; i < 46; ++i)
        Put32(p + 0x2b14 + i * 4U, 0x80172d00 + i * 8U);
    for (Uint16 i = 0; i < 46; ++i)
    {
        Put16(p + 0x2d00 + i * 8U, 2);
        Put16(p + 0x2d02 + i * 8U, (Uint16)(i + 1));
        Put16(p + 0x2d04 + i * 8U, 0xffff);
    }
    for (Uint16 i = 0; i < 6; ++i)
    {
        Put16(p + 0x2494 + i * 8U, (Uint16)(5 + i));
        Put16(p + 0x2496 + i * 8U, (Uint16)(153 + i));
        Put16(p + 0x2498 + i * 8U, (Uint16)(9 + i));
        Put16(p + 0x249a + i * 8U, (Uint16)(5 + i));
        Put32(p + 0x2bcc + i * 4U, (Uint32)(0x800 + i));
    }
    Put16(p + 0x24c4, 159);
    Put16(p + 0x24c6, 16);
    Put16(p + 0x24c8, 160);
    Put16(p + 0x24ca, 17);
    Put32(p + 0x2be4, 255);
    Put32(p + 0x2be8, 0x810);
}
static bool Selectors(SotnFairyInput in)
{
    SotnFairy f = {.mode = SOTN_FAIRY_FOLLOW, .phase = 1, .timer = -1};
    SotnFairyHistory h = {.cached_hp = 70};
    Uint32 rng = 3;
    in.player.status = 0x40000;
    CHECK(SotnFairy_SelectAbility(&f, &h, in, &rng) && f.mode == SOTN_FAIRY_APPLE && !f.phase &&
          rng != 3 && f.timer == -1); // Death precedes negative use timer and bag availability.
    f = (SotnFairy){.mode = SOTN_FAIRY_FOLLOW};
    in.player.status = 0xc000;
    in.player.step = 11;
    in.hand_count[147] = 1;
    CHECK(SotnFairy_SelectAbility(&f, &h, in, &rng) && f.mode == SOTN_FAIRY_HAMMER);
    f.mode = SOTN_FAIRY_FOLLOW;
    f.timer = 0;
    in.hand_count[147] = 0;
    CHECK(SotnFairy_SelectAbility(&f, &h, in, &rng) && f.mode == SOTN_FAIRY_UNCURSE &&
          f.uncurse_luck == 1);
    f.mode = SOTN_FAIRY_FOLLOW;
    f.timer = 0;
    CHECK(SotnFairy_SelectAbility(&f, &h, in, &rng) && f.mode == SOTN_FAIRY_ANTIVENOM &&
          f.antivenom_luck == 1);
    f.mode = SOTN_FAIRY_FOLLOW;
    f.timer = 0;
    const Uint32 held_rng = rng;
    CHECK(SotnFairy_SelectAbility(&f, &h, in, &rng) && f.mode == SOTN_FAIRY_FOLLOW &&
          rng == held_rng);
    in.player.status = 0;
    in.player.damage_effects = 0x8000;
    in.hand_count[153] = 1;
    h.elements = 0xc000;
    h.resist_delay = 9;
    f.timer = 0;
    CHECK(SotnFairy_SelectAbility(&f, &h, in, &rng) && f.mode == SOTN_FAIRY_RESIST &&
          !f.parameter && h.elements == 0x8000 && !h.resist_delay);
    in.player.damage_effects = 0;
    in.hp = 9;
    in.hp_max = 160;
    h = (SotnFairyHistory){.cached_hp = 9, .hp_loss = {15}};
    f = (SotnFairy){.mode = SOTN_FAIRY_FOLLOW};
    CHECK(SotnFairy_SelectAbility(&f, &h, in, &rng) && f.mode == SOTN_FAIRY_POTION &&
          !f.parameter && f.potion_luck && !h.hp_loss[0]);
    // The later quarter-HP/history branch overwrites the earlier high-potion urgency.
    f.mode = SOTN_FAIRY_FOLLOW;
    f.timer = 0;
    h.hp_loss[0] = 100;
    CHECK(SotnFairy_SelectAbility(&f, &h, in, &rng) && f.mode == SOTN_FAIRY_FOLLOW);
    in.hand_count[159] = 1;
    f.timer = 0;
    CHECK(SotnFairy_SelectAbility(&f, &h, in, &rng) && f.mode == SOTN_FAIRY_POTION &&
          f.parameter == 1 && f.potion_luck);
    return true;
}
static bool Items(SotnFairyInput in)
{
    SotnFairy f = {.mode = SOTN_FAIRY_POTION,
                   .phase = 2,
                   .parameter = 1,
                   .timer = -1,
                   .animation = {.address = 0x80172d00}};
    SotnFairyHistory h = {.cached_hp = 70};
    SotnFairyEvents e = {0};
    Uint32 rng = 3;
    in.hand_count[159] = 2;
    CHECK(SotnFairy_Tick(&f, &h, in, &rng, &e) && f.phase == 3 && !f.parameter && !e.consume &&
          !e.factory);
    f.animation.pose = 11;
    f.animation.remaining = -1;
    // Authored timeline has only one usable pose; malformed late animation must roll back costs.
    const SotnFairy before = f;
    const SotnFairyHistory history = h;
    const SotnFairyEvents output = {.consume = 999};
    e = output;
    const Uint32 before_rng = rng;
    SotnFairyInput truncated = in;
    truncated.overlay.size = 0x2d95;
    CHECK(!SotnFairy_Tick(&f, &h, truncated, &rng, &e) && rng == before_rng &&
          !SDL_memcmp(&f, &before, sizeof(f)) && !SDL_memcmp(&h, &history, sizeof(h)) &&
          !SDL_memcmp(&e, &output, sizeof(e)));
    f = (SotnFairy){
        .mode = SOTN_FAIRY_POTION, .phase = 2, .timer = -1, .animation = {.address = 0x80172d00}};
    in.medicine_active = true;
    CHECK(SotnFairy_Tick(&f, &h, in, &rng, &e) && f.mode == SOTN_FAIRY_FOLLOW && !f.phase &&
          !e.consume && !e.factory);
    f = (SotnFairy){
        .mode = SOTN_FAIRY_HAMMER, .phase = 2, .timer = -1, .animation = {.address = 0x80172d00}};
    in.hand_count[147] = 1;
    CHECK(SotnFairy_Tick(&f, &h, in, &rng, &e) && f.phase == 3 && e.sound_count == 1 &&
          !e.consume); // Hammer deliberately ignores the active39 gate.
    return true;
}
static bool AppleAndWings(SotnFairyInput in)
{
    SotnFairy f = {
        .mode = SOTN_FAIRY_APPLE, .phase = 1, .frames = 23, .animation = {.address = 0x80172d00}};
    SotnFairyHistory h = {.cached_hp = 70, .priority = 100, .room_special = 5};
    SotnFairyEvents e = {0};
    Uint32 rng = 4;
    in.hand_count[146] = 1;
    in.scene = 3;
    CHECK(SotnFairy_Tick(&f, &h, in, &rng, &e) && f.frames == 24 && f.phase == 1 &&
          !e.scene_write && rng == 4);
    CHECK(SotnFairy_Tick(&f, &h, in, &rng, &e) && f.phase == 3 && !e.consume);
    in.scene = 0;
    CHECK(SotnFairy_Tick(&f, &h, in, &rng, &e) && f.phase == 4 && e.consume == 146 &&
          e.scene_write && e.scene == 0xfff && !e.restore_resources);
    f.phase = 7;
    f.frames = 25; // The initial25 frames intentionally carry through approach and casting.
    for (Uint16 i = 0; i < 35; ++i)
        CHECK(SotnFairy_Tick(&f, &h, in, &rng, &e) && f.phase == 7 && !e.event_mode);
    CHECK(SotnFairy_Tick(&f, &h, in, &rng, &e) && f.phase == 8 && !f.frames &&
          e.event_mode == 0xdf && !e.event_parameter);
    f.phase = 9;
    f.frames = 90;
    CHECK(SotnFairy_Tick(&f, &h, in, &rng, &e) && f.phase == 10 && !f.frames && e.clear_shared &&
          e.restore_resources && e.clear_flash_if29 && !e.scene_write);
    f.frames = 90;
    CHECK(SotnFairy_Tick(&f, &h, in, &rng, &e) && f.phase == 11 && f.frames == 91 &&
          e.scene_write && !e.scene && !e.restore_resources);
    CHECK(SotnFairy_Tick(&f, &h, in, &rng, &e) && f.mode == SOTN_FAIRY_FOLLOW && !f.phase);
    SotnFairy wings = {.mode = SOTN_FAIRY_WINGS};
    f.animation.address = 0x80172d30;
    f.x = 0x1234567;
    f.y = -987;
    f.facing_left = true;
    CHECK(SotnFairy_Wings(&wings, &f, &h, in.overlay) && wings.phase == 2 && wings.x == f.x &&
          wings.y == f.y && wings.facing_left && wings.priority == 101 && !h.room_special &&
          wings.animation.sprite == 1);
    const SotnFairy held = wings;
    f.animation.address = 0;
    h.room_special = 7;
    CHECK(!SotnFairy_Wings(&wings, &f, &h, in.overlay) && h.room_special == 7 &&
          !SDL_memcmp(&held, &wings, sizeof(wings)));
    return true;
}
static bool Narrative(void)
{
    Uint8 overlay[0x4000] = {0};
    Data(overlay);
    Uint8 *boot = SDL_calloc(1, 0x24000);
    CHECK(boot);
    for (Uint16 i = 0; i < 192; ++i)
        Put16(boot + 0x1e3e0 + i * 2U, 4096);
    // Authored valid table ranges isolate controller branches; these are not SDK parity data.
    for (Uint16 i = 0; i < 46; ++i)
    {
        Put32(overlay + 0x2b14 + i * 4U, 0x80173000 + i * 8U);
        Put16(overlay + 0x3000 + i * 8U, 2);
        Put16(overlay + 0x3002 + i * 8U, (Uint16)(i + 1));
        Put16(overlay + 0x3004 + i * 8U, 0xffff);
    }
    Put32(overlay + 0x2c3c, 255);
    Put32(overlay + 0x2c40, 0x80172ce0);
    Put32(overlay + 0x2c64, 255);
    Put32(overlay + 0x2c68, 0x80172ce0);
    Put16(overlay + 0x2ce2, 14);
    Put16(overlay + 0x2ce4, 0x480);
    Put16(overlay + 0x2ce6, 0xffff);
    Put16(overlay + 0x2ce8, 14);
    Put32(overlay + 0x2c04, 255);
    Put32(overlay + 0x2c08, 0x811);
    Put32(overlay + 0x2cb0, SDL_MAX_UINT32);
    Put32(overlay + 0x2cb4, SDL_MAX_UINT32);
    Put32(overlay + 0x2cb8, 0x80172ce0);
    SotnFairyInput in = {.overlay = {overlay, sizeof(overlay)},
                         .executable = {boot, 0x24000},
                         .hp = 70,
                         .hp_max = 70,
                         .level = 1,
                         .load_count = 1,
                         .sound_step = 99,
                         .player = {.x = 128 * 65536, .y = 132 * 65536},
                         .tilemap_x = 268,
                         .tilemap_y = 546};
    SotnFairy f = {.mode = SOTN_FAIRY_INTRO,
                   .phase = 2,
                   .timer = -1,
                   .x = 128 * 65536,
                   .y = 100 * 65536,
                   .animation = {.address = 0x80173070}};
    SotnFairyHistory h = {.cached_hp = 70};
    SotnFairyEvents e = {0};
    Uint32 rng = 7;
    CHECK(SotnFairy_Tick(&f, &h, in, &rng, &e) && f.phase == 3 && f.sound_event == 0x80172ce0 &&
          !f.sound_flag && e.pause_write && !e.pause_allowed && rng != 7);
    CHECK(SotnFairy_Tick(&f, &h, in, &rng, &e) && f.phase == 3 && f.sound_event == 0x80172ce6 &&
          f.sound_flag == -1 && e.event_mode == 0xde && e.event_parameter == 0x480);
    in.sound_step = 4;
    in.sound_present = true;
    CHECK(SotnFairy_Tick(&f, &h, in, &rng, &e) && f.phase == 3 && !e.event_mode);
    in.sound_step = 5;
    CHECK(SotnFairy_Tick(&f, &h, in, &rng, &e) && f.phase == 4 && !e.pause_write);
    in.sound_step = 99;
    CHECK(SotnFairy_Tick(&f, &h, in, &rng, &e) && f.phase == 5 && !e.pause_write &&
          SotnFairy_Tick(&f, &h, in, &rng, &e) && f.mode == SOTN_FAIRY_FOLLOW && !f.phase &&
          e.pause_write && e.pause_allowed);
    f.mode = SOTN_FAIRY_HINT;
    f.phase = 0;
    f.ability = SOTN_FAIRY_FOLLOW;
    CHECK(SotnFairy_Tick(&f, &h, in, &rng, &e) && f.phase == 1 && h.hint_x == 267 &&
          h.hint_y == 545 && f.tilemap_x == 268 && f.tilemap_y == 546);
    f.phase = 2;
    in.hp = 0;
    in.player.status = 0x40000;
    CHECK(SotnFairy_Tick(&f, &h, in, &rng, &e) && f.mode == SOTN_FAIRY_HINT && f.phase == 3 &&
          e.pause_write && !e.pause_allowed && !e.process_room_events);
    f.sound_event = 0x80173fff;
    const SotnFairy before = f;
    const SotnFairyHistory history = h;
    const SotnFairyEvents output = {.consume = 123};
    e = output;
    const Uint32 saved_rng = rng;
    CHECK(!SotnFairy_Tick(&f, &h, in, &rng, &e) && !SDL_memcmp(&before, &f, sizeof(f)) &&
          !SDL_memcmp(&history, &h, sizeof(h)) && !SDL_memcmp(&output, &e, sizeof(e)) &&
          rng == saved_rng);
    in.hp = 70;
    in.player.status = 0;
    f.mode = SOTN_FAIRY_SHOULDER;
    f.phase = 6;
    f.timer = -1;
    f.sound_event = 0;
    f.flags = 0x04060000;
    CHECK(SotnFairy_Tick(&f, &h, in, &rng, &e) && f.phase == 7 && f.frames >= 1023 &&
          f.frames <= 3070 && rng != saved_rng);
    in.player.velocity_x = 65536;
    f.facing_left = true;
    CHECK(SotnFairy_Tick(&f, &h, in, &rng, &e) && f.phase == 10 && f.velocity_x == -16384 &&
          f.velocity_y == 65536 && e.sound_count == 1 && e.sounds[0] == 0x811 &&
          f.flags == 0x0c020000);
    const Sint32 x = f.x, y = f.y;
    h.shoulder_status = -1;
    CHECK(SotnFairy_Tick(&f, &h, in, &rng, &e) && f.mode == SOTN_FAIRY_FOLLOW && !f.phase &&
          f.x == x - 16384 && f.y == y + 65536 && f.velocity_y == 63488);
    // Wrapped target deltas must narrow after original word subtraction, without host overflow.
    in.tilemap_x = 8;
    in.tilemap_y = 0;
    in.player.velocity_x = 0;
    h.hint_x = SDL_MAX_SINT32;
    h.hint_y = 0;
    f = (SotnFairy){.mode = SOTN_FAIRY_HINT,
                    .phase = 3,
                    .parameter = 1,
                    .x = SDL_MIN_SINT32,
                    .sound_event = 0x80172ce0,
                    .animation = {.address = 0x80173070}};
    in.sound_step = 0;
    CHECK(SotnFairy_Tick(&f, &h, in, &rng, &e) && f.x == SDL_MIN_SINT32 && f.phase == 3);
    // Wave addition also wraps before steering; authored quarter-turn cosine gives an8px offset.
    Put16(boot + 0x1dbcc + 1024 * 2, 4096);
    in.tilemap_x = 0;
    f = (SotnFairy){.mode = SOTN_FAIRY_HINT,
                    .phase = 3,
                    .parameter = 1,
                    .distance = 8,
                    .sound_event = 0x80172ce0,
                    .animation = {.address = 0x80173070}};
    CHECK(SotnFairy_Tick(&f, &h, in, &rng, &e) && f.x == 32768 && f.phase == 3);
    SDL_free(boot);
    return true;
}
static bool Run(void)
{
    Uint8 data[0x3000] = {0};
    Data(data);
    SotnFairyInput in = {.overlay = {data, sizeof(data)}, .hp = 70, .hp_max = 70, .level = 1};
    SotnFairy root = {0}, wings = {0};
    SotnFairyHistory history = {
        .cached_hp = 7, .hp_loss = {8}, .room_special = 4, .restore_priority = true};
    bool reset = true;
    in.player = (SotnFairyPlayer){
        .x = SDL_MAX_SINT32 - 1, .y = SDL_MIN_SINT32 + 1, .priority = 2, .facing_left = true};
    CHECK(SotnFairy_Init(&root, &wings, &history, in, 0, &reset) && !reset &&
          root.mode == SOTN_FAIRY_FOLLOW && root.priority == 0 && wings.priority == 65535 &&
          root.x == SDL_MIN_SINT32 + 24 * 65536 - 2 && root.y == SDL_MAX_SINT32 - 32 * 65536 + 2 &&
          !history.room_special && !history.restore_priority && history.cached_hp == 7 &&
          history.hp_loss[0] == 8);
    const SotnFairy held = root;
    CHECK(SotnFairy_Init(&root, &wings, &history, in, 3, &reset) && reset &&
          !SDL_memcmp(&held, &root, sizeof(root)));
    CHECK(SotnFairy_Init(&root, &wings, &history, in, 1, &reset) && reset &&
          root.mode == SOTN_FAIRY_INTRO && root.x == 128 * 65536 && root.y == -32 * 65536);
    in.room_flags = 32;
    CHECK(SotnFairy_Init(&root, &wings, &history, in, 2, &reset) && !reset &&
          root.x == 192 * 65536 && root.y == 160 * 65536);
    in.inverted = true;
    CHECK(SotnFairy_Init(&root, &wings, &history, in, 2, &reset) && root.x == 64 * 65536);
    in.player = (SotnFairyPlayer){0};
    CHECK(!SotnFairy_MovementAllowed((SotnFairyPlayer){0}, true) &&
          SotnFairy_MovementAllowed((SotnFairyPlayer){.velocity_y = -1}, false) &&
          !SotnFairy_MovementAllowed((SotnFairyPlayer){.velocity_y = -1, .collision = 1}, false) &&
          SotnFairy_MovementAllowed((SotnFairyPlayer){.substep = 4}, true) &&
          !SotnFairy_MovementAllowed((SotnFairyPlayer){.substep = 4, .previous_substep = 4}, true));
    CHECK(SotnFairy_StepAngle(16, 4090, 32) == 26 && SotnFairy_StepAngle(0, 2048, 32) == 2080 &&
          SotnFairy_StepAngle(2048, 0, 32) == 4064);
    CHECK(Selectors(in) && Items(in) && AppleAndWings(in) && Narrative());
    SotnFamiliarSfx sound = {.parameter = 0x472};
    SotnFamiliarSfxInput si = {0};
    Uint16 command = 123;
    Sint32 step = 123;
    CHECK(SotnFamiliarSfx_Tick(&sound, si, &command, &step) && !command && !step &&
          sound.flags == 0x04020000);
    si.cd_queue_empty = true;
    CHECK(SotnFamiliarSfx_Tick(&sound, si, &command, &step) && command == 0x10 && step == 1);
    CHECK(SotnFamiliarSfx_Tick(&sound, si, &command, &step) && !command && step == 1);
    si.command_acknowledged = true;
    CHECK(SotnFamiliarSfx_Tick(&sound, si, &command, &step) && !command && step == 2 &&
          SotnFamiliarSfx_Tick(&sound, si, &command, &step) && command == 0x472 && step == 3);
    CHECK(SotnFamiliarSfx_Tick(&sound, si, &command, &step) && step == 3);
    si.xa_playing = true;
    CHECK(SotnFamiliarSfx_Tick(&sound, si, &command, &step) && step == 4 &&
          SotnFamiliarSfx_Tick(&sound, si, &command, &step) && step == 4);
    si.xa_playing = false;
    CHECK(SotnFamiliarSfx_Tick(&sound, si, &command, &step) && step == 5);
    si.cd_queue_empty = false;
    CHECK(SotnFamiliarSfx_Tick(&sound, si, &command, &step) && step == 5 && !command);
    si.cd_queue_empty = true;
    CHECK(SotnFamiliarSfx_Tick(&sound, si, &command, &step) && step == 6 && command == 0x11 &&
          SotnFamiliarSfx_Tick(&sound, si, &command, &step) && step == 99 && sound.removed);
    sound = (SotnFamiliarSfx){.phase = 5};
    si.room_flags = 32;
    CHECK(SotnFamiliarSfx_Tick(&sound, si, &command, &step) && !command && sound.removed &&
          step == 99);
    sound = (SotnFamiliarSfx){0};
    si.cutscene = true;
    CHECK(SotnFamiliarSfx_Tick(&sound, si, &command, &step) && !command && sound.removed &&
          step == 99);
    sound = (SotnFamiliarSfx){.phase = 7};
    CHECK(SotnFamiliarSfx_Tick(&sound, si, &command, &step) && command == 0xa && step == 4);
    sound = (SotnFamiliarSfx){.phase = 2};
    command = 123;
    step = 123;
    CHECK(!SotnFamiliarSfx_Tick(&sound, si, &command, &step) && sound.phase == 2 &&
          command == 123 && step == 123);
    Uint16 sound_phase = 4;
    CHECK(SotnFairy_RoomEvent(&history, 0, true, &sound_phase) && history.room_special == 1 &&
          sound_phase == 8 && SotnFairy_RoomEvent(&history, 10, false, NULL) &&
          history.room_special == 11 && SotnFairy_RoomEvent(&history, 15, false, NULL) &&
          history.restore_priority);
    return true;
}
int main(void)
{
    return Run() ? 0 : 1;
}
