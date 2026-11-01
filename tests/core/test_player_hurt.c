#include "player_internal.h"
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
                        .hp = 200,
                        .hp_max = 200,
                        .palette = 0x8100,
                        .step = SOTN_PLAYER_STAND,
                        .x = {.value = 100 * 65536 + 123},
                        .y = {.value = 135 * 65536 + 456},
                        .contact_dispatch = true};
}
static bool Births(Uint8 *dra)
{
    SotnPlayer p = Player(dra);
    CHECK(SotnPlayer_ContactDamage(&p, 10, 80, 0, 2, 0) && p.hp == 190 && p.substep == 6 &&
          p.vx == 163840 && !p.vy && p.hurt.count == 2 &&
          p.hurt.factories[0].blueprint == 0x60000U && p.hurt.factories[0].source.x == p.x.value &&
          p.hurt.factories[0].source.y == p.y.value && p.hit_effect_ticks == 6);
    p = Player(dra);
    p.step = SOTN_PLAYER_STUCK;
    CHECK(SotnPlayer_ContactDamage(&p, 10, 120, 0, 3, 0) && p.substep == 7 &&
          p.animation.id == 0x23 && p.vx == -163840 && !p.vy && p.hurt.count == 2 &&
          !p.hurt.factories[0].blueprint);
    p = Player(dra);
    p.step = SOTN_PLAYER_JUMP;
    CHECK(SotnPlayer_ContactDamage(&p, 10, 80, 0, 3, 0) && p.substep == 1 && p.vx == 109226 &&
          p.vy == -3 * 65536 && p.hurt.count == 1 && p.hurt.factories[0].blueprint == 0x58002cU);
    p = Player(dra);
    p.y.value = 135 * 65536;
    CHECK(SotnPlayer_ContactDamage(&p, 30, 80, 0, 5, 0) && p.hp == 170 && p.substep == 15 &&
          p.vx == 8 * 65536 && p.vy == -32768 && p.facing_left && p.y.value == 135 * 65536 - 1 &&
          p.hurt.damage_taken == 30 && !p.damage_random);
    for (Uint16 kind = 16; kind <= 18; ++kind)
    {
        p = Player(dra);
        p.step = SOTN_PLAYER_CROUCH;
        CHECK(SotnPlayer_ContactDamage(&p, 10, 80, 0, kind, 0) && p.substep == 1 &&
              p.vy == -4 * 65536 && p.vx == (kind == 18 ? -109226 : 109226) && p.hurt.count == 1);
    }
    p = Player(dra);
    p.poison_ticks = 100;
    p.curse_ticks = 200;
    p.tight_space = 4;
    CHECK(SotnPlayer_ContactDamage(&p, 10, 80, 0, 3, 0) && !p.vy && p.hurt.count == 3 &&
          p.hurt.factories[0].blueprint == 0x17002cU && p.hurt.factories[1].blueprint == 0x16002cU);
    p = Player(dra);
    const SotnPlayer before = p;
    CHECK(!SotnPlayer_ContactDamage(&p, 10, 80, 0, 19, 0) && !SDL_memcmp(&before, &p, sizeof(p)));
    p = Player(dra);
    p.step = SOTN_PLAYER_HIGH_JUMP;
    p.gravity.upside_down = true;
    p.facing_left = true;
    CHECK(SotnPlayer_ContactDamage(&p, 10, 80, 0, 3, 0) && p.facing_left &&
          !p.gravity.upside_down && p.hurt.factories[0].source.facing_left);
    return true;
}
static bool Palettes(Uint8 *dra)
{
    const struct
    {
        Uint16 effects, palette, ticks;
    } cases[] = {{0, 0x8166, 6},       {0x20, 0x8166, 6},    {0x400, 0x8166, 6},
                 {0x100, 0x8165, 6},   {0x80, 0x8164, 6},    {0x8000, 0x8160, 16},
                 {0x4000, 0x8102, 24}, {0x2000, 0x8169, 12}, {0x1000, 0x8168, 8},
                 {0x800, 0x8164, 16},  {0x40, 0x8166, 16},   {0x180, 0x8165, 6},
                 {0x8080, 0x8164, 6},  {0xc000, 0x8160, 16}, {0x6000, 0x8102, 24},
                 {0x3000, 0x8169, 12}, {0x1800, 0x8164, 16}, {0x1840, 0x8166, 16}};
    for (Uint16 i = 0; i < SDL_arraysize(cases); ++i)
    {
        SotnPlayer p = Player(dra);
        CHECK(SotnPlayer_ContactDamage(&p, 10, 80, 0, 2, cases[i].effects) &&
              p.damage_palette == cases[i].palette && p.hit_effect_ticks == cases[i].ticks &&
              p.palette == 0x8100);
    }
    for (Uint16 kind = 1; kind <= 2; ++kind)
    {
        SotnPlayer p = Player(dra);
        p.poison_ticks = p.curse_ticks = 100;
        CHECK(SotnPlayer_ContactDamage(&p, 10, 80, 0, 2, kind == 1 ? 0x80 : 0x180) &&
              p.hurt.count == 5 && p.hurt.factories[0].blueprint == 0x17002cU &&
              p.hurt.factories[1].blueprint == 0x16002cU &&
              p.hurt.factories[2].blueprint == 0x60000U &&
              p.hurt.factories[kind == 1 ? 3 : 4].blueprint ==
                  (kind == 1 ? 0x16002cU : 0x17002cU) &&
              p.hurt.factories[kind == 1 ? 4 : 3].blueprint == ((Uint32)kind << 16 | 47U));
    }
    SotnPlayer p = Player(dra);
    p.hurt.count = 8;
    const SotnPlayer before = p;
    CHECK(!SotnPlayer_ContactDamage(&p, 10, 80, 0, 2, 0x100) &&
          !SDL_memcmp(&p, &before, sizeof(p)));
    p = Player(dra);
    CHECK(SotnPlayer_ContactDamage(&p, 10, 80, 0, 2, 0x40) && p.hurt.count == 3 &&
          p.hurt.factories[0].blueprint == 0x60000U && p.hurt.factories[1].blueprint == 0x42002cU &&
          p.hurt.factories[2].blueprint == 0x50014U && p.extra_sound == 0x62e &&
          p.hit_effect_ticks == 16);
    p = Player(dra);
    const Sint32 y = p.y.value;
    CHECK(SotnPlayer_ContactDamage(&p, 10, 80, 0, 5, 0x40) && p.hurt.count == 2 &&
          p.hurt.factories[0].source.y == y - 1 && p.hurt.factories[1].source.y == y - 1 &&
          p.hurt.factories[0].source.facing_left == p.facing_left);
    p = Player(dra);
    p.poison_ticks = p.curse_ticks = 100;
    CHECK(SotnPlayer_ContactDamage(&p, 10, 80, 0, 2, 0x1040) && p.hurt.count == 7 &&
          p.hurt.factories[0].blueprint == 0x17002cU &&
          p.hurt.factories[1].blueprint == 0x16002cU && p.hurt.factories[2].blueprint == 0x60000U &&
          p.hurt.factories[3].blueprint == 119 && p.hurt.factories[4].blueprint == 0x68002cU &&
          p.hurt.factories[5].blueprint == 0x42002cU && p.hurt.factories[6].blueprint == 0x50014U &&
          p.damage_palette == 0x8166);
    p = Player(dra);
    p.poison_ticks = p.curse_ticks = 100;
    CHECK(SotnPlayer_ContactDamage(&p, 10, 80, 0, 2, 0x1840) && p.hurt.count == 9 &&
          p.hurt.factories[3].blueprint == 119 && p.hurt.factories[4].blueprint == 0x68002cU &&
          p.hurt.factories[5].blueprint == 113 && p.hurt.factories[6].blueprint == 0x62002cU &&
          p.hurt.factories[7].blueprint == 0x42002cU && p.hurt.factories[8].blueprint == 0x50014U);
    p = Player(dra);
    p.poison_ticks = p.curse_ticks = 100;
    CHECK(SotnPlayer_ContactDamage(&p, 10, 80, 0, 2, 0xf840) && p.hurt.count == 6 &&
          p.hurt.factories[3].blueprint == 0x45002cU && p.hurt.factories[4].blueprint == 0x10011U &&
          p.hurt.factories[5].blueprint == 18 && p.damage_palette == 0x8160 &&
          p.hit_effect_ticks == 16 && p.extra_sound == 0x690);
    p = Player(dra);
    p.poison_ticks = p.curse_ticks = 100;
    CHECK(SotnPlayer_ContactDamage(&p, 10, 80, 0, 2, 0x7840) && p.hurt.count == 5 &&
          p.hurt.factories[3].blueprint == 45 && p.hurt.factories[4].blueprint == 0x46002cU &&
          p.damage_palette == 0x8102 && p.hit_effect_ticks == 24 && !p.extra_sound);
    return true;
}
static bool Recovery(Uint8 *dra)
{
    const Sint32 velocities[] = {-8192, -8191, 24575, 24576};
    for (Uint16 i = 0; i < SDL_arraysize(velocities); ++i)
    {
        SotnPlayer p = Player(dra);
        p.step = SOTN_PLAYER_HIT;
        p.substep = 1;
        p.vy = velocities[i];
        p.water.feet = 41;
        p.animation.remaining = 1;
        SotnPlayer_HurtTick(&p);
        CHECK(!p.failed && p.vy == velocities[i] + (i == 1 || i == 2 ? 1408 : 2816));
    }
    SotnPlayer p = Player(dra);
    p.step = SOTN_PLAYER_HIT;
    p.substep = 1;
    p.contact = 1;
    p.vx = -109226;
    SotnPlayer_HurtTick(&p);
    CHECK(p.step == SOTN_PLAYER_CROUCH && p.substep == 3 && p.vx == -109226 && p.sound == 0x647 &&
          p.movement_effects.smoke && !p.movement_effects.smoke_blueprint && p.invincibility == 12);
    p = Player(dra);
    p.step = SOTN_PLAYER_HIT;
    p.substep = 2;
    p.contact = 2;
    p.vx = 8 * 65536;
    SotnPlayer_HurtTick(&p);
    CHECK(p.substep == 5 && p.impact_ticks == 24 && p.vx == 4 * 65536 && !p.vy &&
          p.hurt.count == 1 && p.hurt.factories[0].blueprint == 0x10004U &&
          p.hurt.factories[0].source.x == p.x.value + 3 * 65536 &&
          p.hurt.factories[0].source.y == p.y.value - 22 * 65536 && p.hurt.shake_requested &&
          p.hurt.shake == 3 && p.sound == 0x644);
    p.hurt.count = 0;
    p.hurt.shake_requested = false;
    p.damage_clock = 4;
    SotnPlayer_HurtTick(&p);
    CHECK(p.substep == 5 && p.impact_ticks == 24 && p.hurt.count == 1 && !p.hurt.shake_requested &&
          p.vx == 4 * 65536 - 8192);
    p.hurt.count = 0;
    p.impact_ticks = 0;
    p.contact = 4;
    SotnPlayer_HurtTick(&p);
    CHECK(p.substep == 5 && p.vy == 6144 && p.hurt.count == 1 &&
          p.hurt.factories[0].blueprint == 0x4000cU);
    p.hurt.count = 0;
    p.contact = 1;
    SotnPlayer_HurtTick(&p);
    CHECK(p.substep == 1 && !p.animation.pose && !p.animation.remaining);
    return true;
}
static bool Walls(Uint8 *dra)
{
    const Uint16 stages[] = {0, 18, 24, 56};
    for (Uint16 i = 0; i < SDL_arraysize(stages); ++i)
    {
        SotnPlayer p = Player(dra);
        p.step = SOTN_PLAYER_HIT;
        p.substep = 2;
        p.contact = 4;
        p.vx = 8 * 65536;
        p.hurt.knock_right = true;
        CHECK(SotnPlayer_SetStageContext(&p, stages[i]));
        SotnPlayer_HurtTick(&p);
        if (!i)
            CHECK(p.substep == 1 && p.vy == -4 * 65536 && p.vx == 81920 &&
                  p.hurt.factories[0].blueprint == 0x90004U && p.hurt.shake == 2);
        else
        {
            CHECK(p.substep == 14 && p.impact_ticks == 48 && p.form_rotated &&
                  p.bat.rotation == 1024 && p.draw_pivot_x == 16 && p.draw_pivot_y == 4 &&
                  p.hurt.factories[0].blueprint == 0x8001fU);
            p.hurt.count = 0;
            p.impact_ticks = 65535;
            SotnPlayer_HurtTick(&p);
            CHECK(p.step == SOTN_PLAYER_JUMP && p.substep == 1 && !p.form_rotated &&
                  p.bat.rotation == 2048 && p.vx == -2 * 65536);
        }
    }
    SotnPlayer p = Player(dra);
    p.step = SOTN_PLAYER_HIT;
    p.substep = 2;
    p.contact = 0x8004;
    p.wall[2].effects = SOTN_EFFECT_SIDE;
    SotnPlayer_HurtTick(&p);
    CHECK(p.substep == 14 && p.hurt.count == 2 && p.hurt.factories[0].blueprint == 0xa0045U &&
          p.hurt.factories[1].blueprint == 0x8001fU);
    p = Player(dra);
    p.step = SOTN_PLAYER_HIT;
    p.substep = 2;
    p.contact = 4;
    p.animation.pose = 5;
    p.animation.remaining = 7;
    SotnPlayer_HurtTick(&p);
    CHECK(p.animation.id == 0x2f && p.animation.pose == 5 && p.animation.remaining == 7 &&
          p.hurt.factories[0].source.x == p.x.value + 3 * 65536 && p.hurt.shake == 2);
    return true;
}
static bool Secondary(Uint8 *dra)
{
    SotnPlayer p = Player(dra);
    p.step = SOTN_PLAYER_HIT;
    p.substep = 15;
    p.contact = 4;
    p.wall[2].effects = SOTN_EFFECT_SIDE;
    p.hurt.damage_taken = 30;
    p.attributes.defense = 6;
    p.poison_ticks = 100;
    p.timer_status = 0x4000; // Actual prior status supplied to this direct controller fixture.
    p.cape = 54;
    p.hearts_max = 50;
    SotnPlayer_HurtTick(&p);
    CHECK(!p.failed && p.hp == 182 && p.substep == 3 && p.impact_ticks == 8 &&
          p.damage_number.applied && p.damage_number.amount == 18 &&
          !p.damage_number.neutral_hurt && p.heart_gain.requested && p.heart_gain.amount == 18 &&
          p.hit_effect_ticks == 4 && p.damage_palette == 0x8166 && p.sound == 0x644 &&
          p.extra_sound == 0x6e3);
    p.hurt.count = 0;
    p.substep = 15;
    p.hp = 18;
    p.heart_gain.requested = false;
    SotnPlayer_HurtTick(&p);
    CHECK(!p.hp && p.step == SOTN_PLAYER_KILL && p.damage_number.response == 4 &&
          !p.heart_gain.requested && p.sound == 0x644 && p.extra_sound == 0x6f6);
    p = Player(dra);
    p.step = SOTN_PLAYER_HIT;
    p.substep = 15;
    p.contact = 2;
    p.hurt.damage_taken = 30;
    p.talismans = 1;
    p.attributes.total[3] = 511;
    SotnPlayer_HurtTick(&p);
    Uint32 rng = 0;
    (void)SotnCombat_Random(&rng);
    (void)SotnCombat_Random(&rng);
    CHECK(p.hp == 200 && p.substep == 5 && p.damage_number.amount == 15 && p.damage_random == rng &&
          p.hit_effect_ticks == 4);
    p = Player(dra);
    p.step = SOTN_PLAYER_HIT;
    p.substep = 15;
    p.contact = 4;
    p.hurt.damage_taken = 65535;
    SotnPlayer_HurtTick(&p);
    CHECK(p.hp == 199 && p.damage_number.amount == 1);
    p = Player(dra);
    p.step = SOTN_PLAYER_HIT;
    p.substep = 15;
    p.contact = 2;
    p.hurt.damage_taken = 30;
    p.timer_status = 0x4000;
    // The common timer pass already expired poison; the prior status still doubles this hit.
    p.poison_ticks = 0;
    SotnPlayer_HurtTick(&p);
    CHECK(p.hp == 170 && p.damage_number.amount == 30 && p.damage_number.secondary_impact &&
          !p.damage_number.boss_grab && p.hud_hp_hold);
    for (Uint16 zero = 0; zero < 2; ++zero)
    {
        p = Player(dra);
        p.step = SOTN_PLAYER_HIT;
        p.substep = 15;
        p.contact = 2;
        p.hurt.damage_taken = 30;
        p.attributes.defense = zero ? 15 : 100;
        p.talismans = 1;
        p.attributes.total[3] = 511;
        SotnPlayer_HurtTick(&p);
        CHECK(p.hp == 200 && p.damage_number.response == 2 &&
              p.damage_number.amount == (zero ? 0 : 65451) && p.damage_number.secondary_impact &&
              p.hud_hp_hold && !p.guard.applied && !p.dodge.applied);
    }
    return true;
}
static bool HoldsAndRebase(Uint8 *dra)
{
    Uint8 tiles[16 * 16 * 2] = {0}, types[1] = {0};
    const SotnCollisionMap map = {{tiles, sizeof(tiles)}, {types, sizeof(types)}, 16, 16};
    SotnPlayer p = Player(dra);
    p.map = map;
    p.step = SOTN_PLAYER_HIT;
    p.substep = 2;
    p.contact = 0x8002;
    SotnPlayer_HurtTick(&p);
    CHECK(p.hurt.count == 2 && p.substep == 5 && p.impact_ticks == 24);
    const SotnPlayerHurt before = p.hurt;
    CHECK(SotnPlayer_ChangeRoom(&p, map, 8, -4) && p.impact_ticks == 24 && p.substep == 5);
    for (Uint16 i = 0; i < p.hurt.count; ++i)
        CHECK(p.hurt.factories[i].source.x == before.factories[i].source.x + 8 * 65536 &&
              p.hurt.factories[i].source.y == before.factories[i].source.y - 4 * 65536);
    Uint16 timers[SOTN_PLAYER_TIMERS];
    SDL_memcpy(timers, p.timers, sizeof(timers));
    CHECK(SotnPlayer_TickSpellFreeze(&p, SOTN_BUTTON_RIGHT, false) &&
          !SDL_memcmp(timers, p.timers, sizeof(timers)) && p.substep == 5 && !p.hurt.count &&
          !p.hurt.shake_requested && !p.hurt.started);
    timers[SOTN_PLAYER_TIMER_INVINCIBLE_CONSUMABLES] = 48;
    CHECK(SotnPlayer_EndSpellFreeze(&p) && !SDL_memcmp(timers, p.timers, sizeof(timers)));
    p.hurt.count = 1;
    p.hurt.factories[0].source.x = SDL_MAX_SINT32;
    const SotnPlayer held = p;
    CHECK(!SotnPlayer_ChangeRoom(&p, map, 8, -4) && !SDL_memcmp(&held, &p, sizeof(p)));
    CHECK(!SotnPlayer_SetStageContext(&p, 256) && !SDL_memcmp(&held, &p, sizeof(p)));
    return true;
}
int main(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000);
    if (!dra)
        return 1;
    for (Uint16 i = 0; i < 8; ++i)
    {
        dra[0xcf84 + i * 2U] = (Uint8)(0xe0U + i);
        dra[0xcf85 + i * 2U] = 6;
    }
    const bool ok = Births(dra) && Palettes(dra) && Recovery(dra) && Walls(dra) && Secondary(dra) &&
                    HoldsAndRebase(dra);
    SDL_free(dra);
    return ok ? 0 : 1;
}
