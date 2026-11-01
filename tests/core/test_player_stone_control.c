#include "player_internal.h"
#include <sotn/bat_runtime.h>
#include <sotn/combat.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s: %s", __LINE__, #v, SDL_GetError());                               \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static SotnPlayer Player(Uint8 *dra)
{
    return (SotnPlayer){.dra = {dra, 0x11000},
                        .hp = 70,
                        .hp_max = 70,
                        .step = SOTN_PLAYER_STONE,
                        .substep = 2,
                        .stone = {.active = true, .shakes = 2},
                        .x = {.value = 100 * 65536 + 123},
                        .y = {.value = 140 * 65536 + 456},
                        .contact = 1,
                        .previous_contact = 1,
                        .palette = 0x819e,
                        .animation = {.id = 0x38},
                        .contact_dispatch = true};
}
static bool Landing(Uint8 *dra)
{
    bool rare = false, ordinary = false;
    for (Uint32 seed = 0; seed < 256 && (!rare || !ordinary); ++seed)
    {
        SotnPlayer p = Player(dra);
        p.substep = 1;
        p.damage_random = seed;
        SotnPlayer_StoneTick(&p);
        CHECK(!p.failed && p.step == SOTN_PLAYER_STONE && p.substep == 2 && !p.vx && !p.vy &&
              !p.movement_effects.smoke && p.hurt.count == 3 && !p.hurt.factories[0].blueprint &&
              p.hurt.factories[1].blueprint == 39 && p.hurt.factories[0].source.x == p.x.value &&
              p.hurt.factories[0].source.y == p.y.value && p.hurt.shake_requested &&
              p.hurt.shake == 1 && p.sound == 0x647 && p.extra_sound == 0x644 && !p.third_sound &&
              p.invincibility == 4 && p.palette == (0x8000U | p.stone.palette));
        if (p.animation.id == 0x3a)
        {
            rare = true;
            CHECK(p.hurt.factories[2].blueprint == 0x7001dU && p.palette == 0x819e);
        }
        else
        {
            ordinary = true;
            CHECK(p.hurt.factories[2].blueprint == 0x30010U &&
                  (p.animation.id == 0x38 || p.animation.id == 0x39));
        }
    }
    CHECK(rare && ordinary);
    return true;
}
static bool Wiggles(Uint8 *dra)
{
    for (Uint16 rare = 0; rare < 2; ++rare)
    {
        SotnPlayer p = Player(dra);
        p.animation.id = rare ? 0x3a : 0x38;
        p.stone.impact = true;
        SotnPlayer_StoneTick(&p);
        CHECK(!p.failed && !p.stone.impact && p.stone.shakes == 1 && p.tapped == 0xf000 &&
              p.substep == 3 && p.invincibility == 28 && p.hurt.count == 2 &&
              p.hurt.factories[0].blueprint == (rare ? 0x8000dU : 0x3000dU) &&
              p.hurt.factories[1].blueprint == (rare ? 0x7001eU : 0x3001fU));
        p = Player(dra);
        p.animation.id = rare ? 0x3a : 0x38;
        p.previous_contact = 0;
        p.stone.shakes = 1;
        p.stone.impact = true;
        SotnPlayer_StoneTick(&p);
        CHECK(!p.failed && p.step == SOTN_PLAYER_HIT && p.substep == 6 && !p.stone.active &&
              p.animation.id == 0x3b && p.palette == 0x8100 && p.tapped == 0xf000 &&
              p.hurt.count == 1 && p.hurt.factories[0].blueprint == 0x30010U && p.sound == 0x644 &&
              p.extra_sound == 0x608 && p.third_sound == 0x6e7);
    }
    SotnPlayer p = Player(dra);
    p.contact = 2;
    p.vy = -2 * 65536;
    SotnPlayer_StoneTick(&p);
    CHECK(!p.failed && p.vy == -2 * 65536 + 11264 && p.invincibility == 4);
    p = Player(dra);
    p.substep = 3;
    p.animation.id = 0x3a;
    p.animation.remaining = 1;
    dra[0xcf96] = 0xf0;
    dra[0xcf97] = 0xff; // Authored signed angle -16, not an original table copy.
    SotnPlayer_StoneTick(&p);
    CHECK(!p.failed && p.form_rotated && p.stone.rotation == -1 && p.draw_pivot_y == 20 &&
          p.invincibility == 4);
    p.animation.remaining = -1;
    SotnPlayer_StoneTick(&p);
    CHECK(p.substep == 2 && !p.form_rotated && p.stone.rotation == -1);
    return true;
}
static bool Presence(void *context, bool *present)
{
    SotnBatRuntime *r = context;
    *present = false;
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
    {
        if (!SotnBatRuntime_DissolveValid(&r->children[i]))
            return SDL_SetError("Invalid authored dissolve generation");
        *present |= r->children[i].kind == 32;
    }
    return true;
}
static bool Death(Uint8 *dra)
{
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    CHECK(r);
    SotnPlayer p = Player(dra);
    CHECK(!SotnPlayer_SetStoneContext(NULL, Presence, r) &&
          !SotnPlayer_SetStoneContext(&p, NULL, r) &&
          !SotnPlayer_SetStoneContext(&p, Presence, NULL));
    for (Uint16 mode = 0; mode < 3; ++mode)
    {
        p = Player(dra);
        CHECK(SotnPlayer_SetStoneContext(&p, Presence, r));
        SDL_zero(*r);
        if (mode == 1)
            r->factories[0].cursor.entity_id = 1, r->factories[0].cursor.blueprint = 39;
        if (mode == 2)
            r->children[47] = (SotnBatRuntimeChild){.kind = 32, .generation = 1};
        CHECK(SotnPlayer_Petrify(&p, 70, 80, 0) && !p.hp && p.step == SOTN_PLAYER_KILL);
        CHECK(mode == 2 ? p.death.phase == 16 && p.stone.active && !p.vx && !p.vy
                        : p.death.phase == 1 && !p.stone.active && p.animation.id == 0xc0);
    }
    p = Player(dra);
    CHECK(SotnPlayer_SetStoneContext(&p, Presence, r));
    r->children[47].generation = 0;
    const SotnPlayer before = p;
    CHECK(!SotnPlayer_Petrify(&p, 70, 80, 0) && !SDL_memcmp(&before, &p, sizeof(p)));
    p = Player(dra);
    p.step = SOTN_PLAYER_KILL;
    p.death.phase = 16;
    SotnPlayer_StoneDeathTick(&p);
    CHECK(p.death.phase == p.substep && p.death.phase == 17 && p.death.timer == 80 &&
          p.form_rotated);
    Uint16 automatic = 0;
    for (Uint16 t = 0; t < 80; ++t)
    {
        p.tapped = p.sound = 0;
        SotnPlayer_StoneDeathTick(&p);
        if (p.sound == 0x608)
        {
            CHECK(p.tapped == SOTN_BUTTON_UP);
            ++automatic;
        }
    }
    CHECK(automatic == 5 && p.stone.shakes == 5 && p.stone.hidden && p.palette == 0x810d &&
          p.death.phase == 18 && p.substep == 18 && p.hurt.count == 1 &&
          p.hurt.factories[0].blueprint == 0x30010U && p.animation.id == 0x3e);
    SDL_free(r);
    return true;
}
int main(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000);
    if (!dra)
        return 1;
    const bool ok = Landing(dra) && Wiggles(dra) && Death(dra);
    SDL_free(dra);
    return ok ? 0 : 1;
}
