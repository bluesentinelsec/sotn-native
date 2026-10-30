#include "player_internal.h"
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s: %s", __LINE__, #v, SDL_GetError());                               \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
typedef struct Fixture
{
    Uint8 dra[0x11000], tiles[32 * 16 * 2], types[2];
} Fixture;
static void P32(Uint8 *p, Uint32 v)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (i * 8U));
}
static bool Movement(Fixture *f)
{
    for (Uint16 i = 0; i < 247; ++i)
        P32(f->dra + 0x101b8 + i * 4U, 0x800a0100);
    for (Uint16 i = 0; i < 6; ++i)
        P32(f->dra + 0x100 + i * 4U, 2U | (Uint32)(i + 1U) << 16);
    f->types[1] = 3;
    for (Uint16 y = 10; y < 16; ++y)
        for (Uint16 x = 0; x < 32; ++x)
            f->tiles[(y * 32 + x) * 2] = 1;
    const SotnCollisionMap map = {
        {f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 16};
    const SotnDataView dra = {f->dra, sizeof(f->dra)};
    SotnPlayer *p = NULL;
    CHECK(SotnPlayer_Create(dra, map, 64, 135, &p));
    CHECK(SotnPlayer_SetWaterContext(p, (SotnPlayerWater){40, 24, 0}));
    CHECK(SotnPlayer_Tick(p, SOTN_BUTTON_RIGHT) && p->x.value == 64 * 65536 + 98304 &&
          p->vx == 98304);
    SotnPlayer_Close(p);
    p = NULL;
    CHECK(SotnPlayer_Create(dra, map, 64, 135, &p));
    p->invincibility = 99;
    CHECK(SotnPlayer_SetWaterContext(p, (SotnPlayerWater){41, 25, 0}));
    CHECK(SotnPlayer_Tick(p, SOTN_BUTTON_RIGHT) && p->x.value == 64 * 65536 + 73728 &&
          p->vx == 98304 && p->invincibility == 98);
    SotnPlayerState snapshot = {0};
    CHECK(SotnPlayer_GetState(p, &snapshot) && snapshot.water.feet == 41 &&
          snapshot.water.waist == 25 && (snapshot.status & 0x08020000U) == 0x08020000U);
    const Sint32 y = p->y.value;
    CHECK(SotnPlayer_Tick(p, SOTN_BUTTON_JUMP) && p->vy == -319488 && p->y.value == y - 239616);
    CHECK(SotnPlayer_Tick(p, 0) && p->vy == -62720 && p->y.value == y - 239616 - 47040);
    CHECK(SotnPlayer_ChangeRoom(p, map, 0, 0) && !p->water.feet && !p->water.waist &&
          !p->water.head);
    SotnPlayer_Close(p);
    p = NULL;
    CHECK(SotnPlayer_Create(dra, map, 64, 135, &p));
    CHECK(SotnPlayer_SetWaterContext(p, (SotnPlayerWater){41, 25, 0}));
    p->collision_invincibility = 1;
    CHECK(SotnPlayer_Tick(p, SOTN_BUTTON_RIGHT) && p->x.value == 64 * 65536 + 98304 &&
          !p->collision_invincibility);
    CHECK(SotnPlayer_Tick(p, SOTN_BUTTON_RIGHT) && p->x.value == 64 * 65536 + 98304 + 73728);
    SotnPlayer_Close(p);
    return true;
}
static bool Gates(void)
{
    SotnPlayer p = {0};
    CHECK(SotnPlayer_SetWaterContext(&p, (SotnPlayerWater){12, 4, 0}) && !SotnPlayer_WaterDrag(&p));
    p.step = SOTN_PLAYER_WOLF;
    CHECK(!SotnPlayer_WaterDrag(&p));
    CHECK(SotnPlayer_SetWaterContext(&p, (SotnPlayerWater){13, 5, 0}) && SotnPlayer_WaterDrag(&p));
    p.step = SOTN_PLAYER_STAND;
    CHECK(!SotnPlayer_WaterDrag(&p));
    CHECK(SotnPlayer_SetWaterContext(&p, (SotnPlayerWater){41, 25, 0}) && SotnPlayer_WaterDrag(&p));
    p.invincibility = 100;
    CHECK(SotnPlayer_WaterDrag(&p));
    p.collision_invincibility = 1;
    CHECK(!SotnPlayer_WaterDrag(&p));
    p.collision_invincibility = 0;
    p.step = SOTN_PLAYER_STUCK;
    CHECK(!SotnPlayer_WaterDrag(&p));
    p.step = SOTN_PLAYER_BOSS_GRAB;
    CHECK(!SotnPlayer_WaterDrag(&p));
    const SotnPlayer before = p;
    CHECK(!SotnPlayer_SetWaterContext(&p, (SotnPlayerWater){10, 11, 0}) &&
          !SDL_memcmp(&p, &before, sizeof(p)));
    CHECK(!SotnPlayer_SetWaterContext(&p, (SotnPlayerWater){32768, 0, 0}) &&
          !SDL_memcmp(&p, &before, sizeof(p)));
    CHECK(SotnPlayer_WaterScale(-3, false) == -2 && SotnPlayer_WaterScale(-2, true) == -2 &&
          SotnPlayer_WaterScale(7, false) == 5 && SotnPlayer_WaterScale(5, true) == 6);
    CHECK(SotnPlayer_WaterScale(SDL_MAX_SINT32, false) == 536870911 &&
          SotnPlayer_WaterScale(SDL_MIN_SINT32, true) == 0);
    return true;
}
static bool Wolf(void)
{
    SotnPlayer p = {.x = {.value = 100 * 65536},
                    .y = {.value = 100 * 65536},
                    .mp = 20,
                    .step = SOTN_PLAYER_WOLF,
                    .substep = 1,
                    .animation = {.id = 22, .sprite = 7},
                    .animation_bank = 15,
                    .wolf_context = true,
                    .wolf = {.active = true, .priority = 146},
                    .buttons = SOTN_BUTTON_CANCEL};
    p.relics[SOTN_RELIC_WOLF] = p.relics[SOTN_RELIC_SKILL_WOLF] = p.relics[14] = 3;
    CHECK(SotnPlayer_SetWaterContext(&p, (SotnPlayerWater){30, 22, 6}));
    p.mist_underwater = true;
    SotnPlayer_WolfTick(&p);
    CHECK(!p.failed && p.substep == 9 && p.wolf.controller.water_level == 22 &&
          p.wolf.controller.fall_water_level == 30);
    return true;
}
static bool Stone(void)
{
    SotnPlayer p = {.step = SOTN_PLAYER_STONE, .substep = 1, .hp = 70, .water = {41, 25, 0}};
    SotnPlayer_StoneTick(&p);
    CHECK(!p.failed && p.vy == 1408 && p.substep == 1);
    p.vy = 0;
    p.substep = 2;
    SotnPlayer_StoneTick(&p);
    CHECK(!p.failed && p.vy == 2816);
    p.vy = 0;
    p.water = (SotnPlayerWater){40, 24, 0};
    SotnPlayer_StoneTick(&p);
    CHECK(!p.failed && p.vy == 11264);
    return true;
}
static bool Breathing(void)
{
    SotnPlayer p = {.x = {.value = 100 * 65536 + 123},
                    .y = {.value = 140 * 65536 + 456},
                    .water = {49, 33, 1},
                    .damage_random = 17};
    const Sint32 x = p.x.value, y = p.y.value;
    SotnPlayer_WaterBreath(&p, false, false);
    CHECK(!p.failed && p.water_breath_timer == 96 && p.movement_effects.water_bubble &&
          p.movement_effects.bubble_source.x == x + 4 * 65536 &&
          p.movement_effects.bubble_source.y == y - 16 * 65536 &&
          p.movement_effects.bubble_source.priority == 148 && p.x.value == x && p.y.value == y &&
          p.damage_random == 17);
    p.movement_effects = (SotnPlayerMovementEffects){0};
    SotnPlayer_WaterBreath(&p, false, true);
    CHECK(p.water_breath_timer == 96 && !p.movement_effects.water_bubble);
    SotnPlayer_WaterBreath(&p, true, true);
    CHECK(p.water_breath_timer == 95 && !p.movement_effects.water_bubble);
    CHECK(SotnPlayer_SetWaterBreathTimer(&p, 1));
    SotnPlayer_WaterBreath(&p, false, false);
    CHECK(!p.water_breath_timer && !p.movement_effects.water_bubble);
    SotnPlayer_WaterBreath(&p, false, false);
    CHECK(p.water_breath_timer == 96 && p.movement_effects.water_bubble);
    p.movement_effects = (SotnPlayerMovementEffects){0};
    CHECK(SotnPlayer_SetWaterBreathTimer(&p, 0));
    p.water.feet = 48;
    SotnPlayer_WaterBreath(&p, false, false);
    CHECK(!p.water_breath_timer && !p.movement_effects.water_bubble);
    p.water.feet = 24;
    SotnPlayer_WaterBreath(&p, true, false);
    CHECK(!p.water_breath_timer && !p.movement_effects.water_bubble);
    p.water.feet = 25;
    SotnPlayer_WaterBreath(&p, true, true);
    CHECK(!p.water_breath_timer && !p.movement_effects.water_bubble);
    p.facing_left = true;
    SotnPlayer_WaterBreath(&p, true, false);
    CHECK(p.water_breath_timer == 96 && p.movement_effects.water_bubble &&
          p.movement_effects.bubble_source.x == x - 12 * 65536 &&
          p.movement_effects.bubble_source.y == y + 2 * 65536 &&
          p.movement_effects.bubble_source.facing_left);
    const SotnPlayer before = p;
    CHECK(!SotnPlayer_SetWaterBreathTimer(&p, 97) && !SDL_memcmp(&p, &before, sizeof(p)));
    return true;
}
static bool BreathingUpdates(Fixture *f)
{
    const SotnCollisionMap map = {
        {f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 16};
    SotnPlayer *p = NULL;
    CHECK(SotnPlayer_Create((SotnDataView){f->dra, sizeof(f->dra)}, map, 64, 135, &p));
    CHECK(SotnPlayer_SetWaterContext(p, (SotnPlayerWater){49, 33, 1}));
    CHECK(SotnPlayer_Tick(p, 0) && p->movement_effects.water_bubble && p->water_breath_timer == 96);
    const Sint32 origin = p->movement_effects.bubble_source.x;
    CHECK(SotnPlayer_ChangeRoom(p, map, 16, 0) && p->water_breath_timer == 96 &&
          p->movement_effects.bubble_source.x == origin + 16 * 65536);
    CHECK(SotnPlayer_TickSpellFreeze(p, 0, false) && !p->movement_effects.water_bubble &&
          p->water_breath_timer == 96);
    CHECK(SotnPlayer_Tick(p, SOTN_BUTTON_RIGHT) && p->water_breath_timer == 95 &&
          p->step == SOTN_PLAYER_WALK && !p->movement_effects.water_bubble);
    CHECK(SotnPlayer_Tick(p, SOTN_BUTTON_RIGHT) && p->water_breath_timer == 95);
    CHECK(SotnPlayer_Tick(p, 0) && p->step == SOTN_PLAYER_STAND && p->water_breath_timer == 95);
    CHECK(SotnPlayer_Tick(p, 0) && p->water_breath_timer == 94);
    SotnPlayerState snapshot = {0};
    CHECK(SotnPlayer_GetState(p, &snapshot) && snapshot.water_breath_timer == 94);
    SotnPlayer_Close(p);
    p = NULL;
    CHECK(SotnPlayer_Create((SotnDataView){f->dra, sizeof(f->dra)}, map, 64, 135, &p));
    CHECK(SotnPlayer_SetWaterContext(p, (SotnPlayerWater){49, 33, 1}));
    CHECK(SotnPlayer_Tick(p, SOTN_BUTTON_RIGHT) && p->step == SOTN_PLAYER_WALK &&
          p->water_breath_timer == 96 && p->movement_effects.water_bubble &&
          p->movement_effects.smoke && p->movement_effects.bubble_source.x == 68 * 65536 &&
          p->movement_effects.smoke_source.x == 64 * 65536);
    SotnPlayer_Close(p);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok =
        Gates() && Movement(f) && Wolf() && Stone() && Breathing() && BreathingUpdates(f);
    SDL_free(f);
    if (ok)
        SDL_Log("PLAYER_WATER: PASS authored depth/status/drag/gravity/collision-byte/Wolf/room "
                "contexts");
    return ok ? 0 : 1;
}
