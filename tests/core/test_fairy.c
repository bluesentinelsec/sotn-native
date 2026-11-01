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
    CHECK(Selectors(in) && Items(in) && AppleAndWings(in));
    return true;
}
int main(void)
{
    return Run() ? 0 : 1;
}
