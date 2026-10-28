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
int main(void)
{
    return GatesAndPending() && CommandAndReturn() && CollisionPasses() && ForcedSlots() &&
                   MissingReturnFactory()
               ? 0
               : 1;
}
