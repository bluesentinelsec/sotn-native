#include "player_internal.h"
#include <sotn/bat_runtime.h>
_Static_assert(SOTN_PLAYER_WOLF == 24 && SOTN_PLAYER_UNMORPH_WOLF == 25 && SOTN_BUTTON_WOLF == 2,
               "Wolf retains original US states and R2 word");
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s: %s", __LINE__, #v, SDL_GetError());                               \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static SotnPlayer Player(void)
{
    return (SotnPlayer){.x = {.value = 100 * 65536},
                        .y = {.value = 100 * 65536},
                        .animation = {.id = 22, .sprite = 7},
                        .animation_bank = 1,
                        .hp = 70,
                        .hp_max = 70,
                        .mp = 20,
                        .wolf_context = true,
                        .wolf = {.priority = 146},
                        .wolf_body_commit_frame = 0};
}
static bool GatesAndPending(void)
{
    SotnPlayer p = Player();
    p.buttons = p.tapped = SOTN_BUTTON_WOLF;
    SotnPlayer_WolfInput(&p);
    CHECK(!p.wolf.active);
    p.relics[SOTN_RELIC_WOLF] = 1;
    SotnPlayer_WolfInput(&p);
    CHECK(!p.wolf.active);
    p.relics[SOTN_RELIC_WOLF] = 3;
    p.mp = 1;
    SotnPlayer_WolfInput(&p);
    CHECK(!p.wolf.active);
    p.mp = 20;
    p.mist_underwater = true;
    SotnPlayer_WolfInput(&p);
    CHECK(!p.wolf.active);
    p.mist_underwater = false;
    SotnPlayer_WolfInput(&p);
    CHECK(p.wolf.active && p.step == SOTN_PLAYER_WOLF && p.sound == 0x668 && !p.substep);
    SotnPlayer_WolfTick(&p);
    CHECK(!p.failed && !p.wolf.started && !p.wolf.body_started && !p.wolf.generation &&
          !p.substep && p.animation_bank == 1);
    p.wolf_factory_available = true;
    SotnPlayer_WolfTick(&p);
    CHECK(!p.failed && p.wolf.started && p.wolf.body_started && p.wolf.generation &&
          p.wolf.body_generation == p.wolf.generation && p.wolf.signal == 1 &&
          p.animation_bank == 15 && p.substep == 8 && p.wolf.capture_bank == 1 &&
          p.wolf.capture_sprite == 7);
    SotnWolfRuntime body = {0};
    CHECK(SotnWolfRuntime_Begin(&body, p.wolf.body_generation, 0));
    CHECK(!SotnPlayer_SetWolfContext(&p, &body, NULL, NULL, 0, true, 0, 0));
    SotnPlayerFactory factory = {0};
    CHECK(SotnPlayerFactory_Begin(&factory, (SotnPlayerFactorySource){.priority = 146}, 0x23002cU,
                                  0));
    CHECK(SotnPlayer_SetWolfContext(&p, &body, NULL, &factory, p.wolf.generation, true, 0, 0));
    for (Uint16 t = 0; t < 100; ++t)
    {
        SotnPlayer_WolfTick(&p);
        CHECK(!p.failed && p.substep == 8 && p.wolf.signal == 1 && !p.wolf.body_started);
    }
    const SotnPlayer before = p;
    factory.parameter = 0x2100;
    CHECK(!SotnPlayer_SetWolfContext(&p, &body, NULL, &factory, p.wolf.generation, true, 0, 0) &&
          !SDL_memcmp(&p, &before, sizeof(p)));
    return true;
}
static bool CommandAndReturn(void)
{
    SotnPlayer p = Player();
    p.buttons = SOTN_BUTTON_DOWN;
    SotnPlayer_WolfInput(&p);
    CHECK(p.wolf_combo == 1 && p.wolf_combo_timer == 20);
    p.facing_left = true;
    p.buttons = SOTN_BUTTON_DOWN | SOTN_BUTTON_RIGHT;
    SotnPlayer_WolfInput(&p);
    CHECK(p.wolf_combo == 2 && p.wolf_combo_timer == 20);
    p.buttons = SOTN_BUTTON_RIGHT;
    SotnPlayer_WolfInput(&p);
    CHECK(p.wolf_combo == 255 && p.wolf_combo_timer == 20 && !p.wolf_combo_facing);
    p.buttons = 0;
    for (Uint16 t = 0; t < 19; ++t)
        SotnPlayer_WolfInput(&p);
    CHECK(p.wolf_combo == 255 && p.wolf_combo_timer == 1);
    SotnPlayer_WolfInput(&p);
    CHECK(!p.wolf_combo && !p.wolf_combo_timer && p.wolf_combo_facing);
    p.wolf_combo = 255;
    for (Uint16 t = 0; t < 5; ++t)
        SotnPlayer_WolfInput(&p);
    CHECK(p.wolf_combo == 255 && !p.wolf_combo_timer);
    p.wolf_combo = 1;
    p.wolf_combo_timer = 1;
    p.wolf_combo_facing = false;
    SotnPlayer_WolfInput(&p);
    CHECK(!p.wolf_combo && !p.wolf_combo_timer && !p.wolf_combo_facing);
    p.wolf_combo = 255;
    p.wolf_combo_timer = 1;
    p.buttons = SOTN_BUTTON_DOWN;
    SotnPlayer_WolfInput(&p);
    CHECK(!p.wolf_combo && !p.wolf_combo_timer && p.wolf_combo_facing);
    p.buttons = 0;
    p.wolf_combo = 0;
    p.wolf_combo_timer = SDL_MIN_SINT16;
    SotnPlayer_WolfInput(&p);
    CHECK(p.wolf_combo_timer == SDL_MAX_SINT16);
    p = Player();
    p.previous_step = SOTN_PLAYER_UNMORPH_WOLF;
    p.contact = 1;
    const Sint32 y = p.y.value;
    SotnPlayer_WolfReturn(&p, false);
    CHECK(p.step == SOTN_PLAYER_JUMP && p.y.value == y && p.vy == -65536 && p.mist_lockout == 24 &&
          (p.movement_flags & 0x101U) == 0x101U);
    p = Player();
    p.contact = 0x8001;
    SotnPlayer_WolfReturn(&p, false);
    CHECK(p.vy == -319488 && p.y.value == y);
    return true;
}
static bool MissingReturnFactory(void)
{
    SotnPlayer p = Player();
    p.relics[SOTN_RELIC_WOLF] = 3;
    p.step = SOTN_PLAYER_WOLF;
    p.substep = 1;
    p.animation_bank = 15;
    p.wolf.active = true;
    p.wolf.generation = 1;
    p.wolf.body_generation = 1;
    p.tapped = SOTN_BUTTON_WOLF;
    p.wolf_factory_available = false;
    SotnPlayer_WolfTick(&p);
    CHECK(!p.failed && p.step == SOTN_PLAYER_UNMORPH_WOLF && p.wolf.return_factory_missing &&
          !p.wolf.return_started && !p.wolf.generation && !p.wolf.signal && p.mp == 20);
    for (Uint16 i = 0; i < 100; ++i)
        SotnPlayer_UnmorphWolfTick(&p);
    CHECK(!p.failed && p.step == SOTN_PLAYER_UNMORPH_WOLF && p.substep == 1 && p.wolf.signal == 1 &&
          p.wolf.active && p.wolf.return_factory_missing);
    return true;
}
static bool CollisionPasses(void)
{
    Uint8 tiles[32 * 16 * 2] = {0}, types[1] = {0};
    SotnPlayer p = Player();
    p.map = (SotnCollisionMap){{tiles, sizeof(tiles)}, {types, sizeof(types)}, 32, 16};
    p.vx = 1;
    p.step = SOTN_PLAYER_WOLF;
    p.substep = 1;
    p.status = 4;
    const Sint32 x = p.x.value;
    SotnPlayer_Collide(&p);
    CHECK(!p.failed && p.x.value == x && !p.vx && p.ceiling_sensors[0].y == 0);
    p = Player();
    p.map = (SotnCollisionMap){{tiles, sizeof(tiles)}, {types, sizeof(types)}, 32, 16};
    p.vx = 1;
    SotnPlayer_Collide(&p);
    CHECK(!p.failed && p.x.value == x + 1 && p.vx == 1);
    return true;
}
static bool ForcedSlots(void)
{
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    CHECK(r);
    r->children[0] = (SotnBatRuntimeChild){.kind = 11, .generation = 1};
    r->children[1] = (SotnBatRuntimeChild){.kind = 36, .generation = 2, .group = 3};
    r->children[14] = (SotnBatRuntimeChild){.kind = 35, .generation = 3};
    r->children[15] = (SotnBatRuntimeChild){.kind = 36, .generation = 4, .group = 3};
    r->children[16] = (SotnBatRuntimeChild){.kind = 45, .generation = 5, .group = 7};
    r->wing_mask = 0x7fff;
    r->shared.wolf_lock = true;
    CHECK(SotnBatRuntime_ReplaceWolfSlots(r));
    CHECK(r->children[0].generation == 1 && !r->children[1].generation &&
          !r->children[14].generation && r->children[15].generation == 4 &&
          r->children[16].generation == 5 && r->wing_mask == 0x4000 && r->groups[3] == 1 &&
          r->groups[7] == 1 && r->shared.wolf_lock);
    r->children[1] = (SotnBatRuntimeChild){.kind = 36, .generation = 6, .group = 3};
    r->children[15].group = 255;
    const SotnBatRuntime before = *r;
    CHECK(!SotnBatRuntime_ReplaceWolfSlots(r) && !SDL_memcmp(r, &before, sizeof(*r)));
    SDL_free(r);
    return true;
}
static bool RequestOrigins(void)
{
    Uint8 tiles[32 * 16 * 2] = {0}, types[1] = {0};
    const SotnCollisionMap map = {{tiles, sizeof(tiles)}, {types, sizeof(types)}, 32, 16};
    SotnPlayer p = Player();
    p.map = map;
    p.step = SOTN_PLAYER_WOLF;
    p.substep = 2;
    p.wolf.active = true;
    p.wolf.controller.motion.mode = 2;
    p.wolf.controller.motion.body_angle = 2048;
    p.contact = 1;
    p.vx = 4 * 65536;
    p.relics[SOTN_RELIC_WOLF] = 3;
    p.wolf_camera_x = 40;
    p.wolf_camera_y = 30;
    p.mist_game_timer = 6;
    SotnPlayer_WolfTick(&p);
    CHECK(!p.failed && p.wolf.commands.count &&
          p.wolf.commands.requests[0].kind == SOTN_WOLF_FACTORY &&
          p.wolf.commands.requests[0].parameter == 0x10045 &&
          p.wolf.commands.requests[0].source.x == 100 * 65536 &&
          p.wolf.commands.requests[0].source.y == 100 * 65536);
    const Uint32 frame = p.frame_number;
    CHECK(SotnPlayer_ChangeRoom(&p, map, -16, 8) &&
          p.wolf.commands.requests[0].source.x == 84 * 65536 &&
          p.wolf.commands.requests[0].source.y == 108 * 65536 && p.frame_number == frame);
    p.wolf.commands.requests[0].source.x = SDL_MIN_SINT32;
    const SotnPlayer before = p;
    CHECK(!SotnPlayer_ChangeRoom(&p, map, -1, 0) && !SDL_memcmp(&before, &p, sizeof(p)));
    return true;
}
static bool CrossFormGates(void)
{
    for (Uint16 target = 0; target < 2; ++target)
        for (Uint16 phase = 0; phase < 10; ++phase)
            for (Uint16 gate = 0; gate < 6; ++gate)
            {
                SotnPlayer p = Player();
                p.step = SOTN_PLAYER_WOLF;
                p.substep = phase;
                p.animation_bank = 15;
                p.animation.sprite = 6;
                p.wolf = (SotnPlayerWolf){.active = true,
                                          .generation = 9,
                                          .body_generation = 9,
                                          .capture_bank = 13,
                                          .capture_sprite = 7,
                                          .draw_flags = 4,
                                          .rotation = 900,
                                          .blend = 0x60};
                p.bat_context = p.mist_context = true;
                p.relics[target ? SOTN_RELIC_MIST : SOTN_RELIC_BAT] = gate == 1 ? 1 : 3;
                p.tapped = target ? SOTN_BUTTON_MIST : SOTN_BUTTON_BAT;
                p.mp = gate == 2 ? 1 : 20;
                p.mist_lockout = gate == 3 ? 2 : 0;
                p.script_hold = gate == 4;
                p.mist_underwater = gate == 5;
                const Uint16 mp = p.mp;
                if (target)
                    SotnPlayer_MistInput(&p);
                else
                    SotnPlayer_BatInput(&p);
                const bool allowed = !gate && phase && phase != 8;
                CHECK(!p.failed && p.mp == mp && p.wolf.active == !allowed &&
                      p.wolf.capture_bank == 13 && p.wolf.capture_sprite == 7 &&
                      p.wolf.body_generation == 9 && p.animation_bank == 15);
                CHECK(p.step ==
                      (allowed ? target ? SOTN_PLAYER_MIST : SOTN_PLAYER_BAT : SOTN_PLAYER_WOLF));
                if (allowed)
                {
                    CHECK(!p.substep && p.sound == 0x6a0);
                    if (target)
                        CHECK(p.mist.active && p.mist.started && p.mist.capture_bank == 15 &&
                              p.mist.capture_sprite == 6 && !p.wolf.draw_flags && !p.wolf.blend);
                    else
                    {
                        CHECK(p.bat.active && !p.bat.generation && !p.bat.signal);
                        p.bat_factory_available = true;
                        SotnPlayer_BatTick(&p);
                        CHECK(!p.failed && p.bat.started && p.bat.signal == 1 &&
                              p.wolf.palette == 0x810d && !p.wolf.rotation &&
                              p.wolf.draw_flags == 4 && !p.wolf.blend && p.animation_bank == 15);
                    }
                }
            }
    return true;
}
static bool RetainedWolfMistEntry(void)
{
    SotnPlayer p = Player();
    p.step = SOTN_PLAYER_MIST;
    p.substep = 1;
    p.animation_bank = 15;
    p.animation.sprite = 6;
    p.mist.active = true;
    p.mist.generation = 3;
    p.mist_visual_phase = 1;
    p.relics[SOTN_RELIC_WOLF] = 3;
    p.tapped = SOTN_BUTTON_WOLF;
    SotnPlayer_WolfInput(&p);
    CHECK(p.step == SOTN_PLAYER_WOLF && p.wolf.active && p.mist.active);
    SotnPlayer_WolfTick(&p);
    CHECK(!p.failed && p.substep == 8 && p.wolf.body_started && !p.wolf.started &&
          p.wolf.capture_bank == 15 && p.wolf.capture_sprite == 6 && p.mist.request_restore &&
          !p.mist.retire && !p.wolf.finish_flash);
    p.mist_visual_phase = 4;
    SotnPlayer_WolfTick(&p);
    CHECK(!p.failed && p.substep == 8 && p.mist.active && !p.wolf.finish_flash);
    p.mist_visual_phase = 5;
    SotnPlayer_WolfTick(&p);
    CHECK(!p.failed && p.substep == 5 && !p.mist.active && p.mist.retire && p.wolf.finish_flash &&
          !p.wolf.started && p.wolf.capture_bank == 15);
    return true;
}
int main(void)
{
    return GatesAndPending() && CommandAndReturn() && CollisionPasses() && ForcedSlots() &&
                   MissingReturnFactory() && RequestOrigins() && CrossFormGates() &&
                   RetainedWolfMistEntry()
               ? 0
               : 1;
}
