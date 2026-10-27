#include "player_internal.h"
#include <SDL3/SDL.h>
#include <sotn/player.h>

#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
typedef struct Fixture
{
    Uint8 dra[0x11000], exe[0x24000], tiles[32 * 16 * 2], types[3];
    SotnPlayer *player;
    SotnMistEffect effect;
    SotnPlayerState state;
    SotnCharacterState character;
    Uint32 generation, tick;
    bool present;
} Fixture;
static void Put16(Uint8 *p, Uint16 n)
{
    p[0] = (Uint8)n;
    p[1] = (Uint8)(n >> 8);
}
static void Put32(Uint8 *p, Uint32 n)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(n >> (i * 8U));
}
static bool Create(Fixture *f)
{
    SDL_memset(f, 0, sizeof(*f));
    for (Uint16 i = 0; i < 247; ++i)
        Put32(f->dra + 0x101b8 + i * 4U, 0x800a0100);
    for (Uint16 i = 0; i < 6; ++i)
        Put32(f->dra + 0x100 + i * 4U, 2U | ((Uint32)i + 1U) << 16);
    Put32(f->dra + 0x118, 0);
    for (Uint16 i = 0; i < 1025; ++i)
    {
        Put16(f->exe + 0x1dbcc + i * 2U, (Uint16)(i * 4U));
        Put16(f->exe + 0x22a10 + i * 2U, (Uint16)(i / 2U));
    }
    for (Uint16 i = 0; i < 192; ++i)
        Put16(f->exe + 0x1e3e0 + i * 2U, 4096);
    for (Uint16 i = 0; i < 32; ++i)
        Put16(f->dra + 0xe1b0 + i * 2U, 64);
    for (Uint16 i = 0; i < 8; ++i)
        f->dra[0xe230 + i * 4U] = f->dra[0xe250 + i * 4U] = (Uint8)i;
    f->types[1] = 3;
    f->types[2] = 0xf8;
    for (Uint16 y = 10; y < 16; ++y)
        for (Uint16 x = 0; x < 32; ++x)
            f->tiles[(y * 32U + x) * 2U] = 1;
    f->character = (SotnCharacterState){.hp = 70, .hp_max = 70, .mp = 20, .mp_max = 20};
    SotnCollisionMap map = {{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 16};
    CHECK(SotnPlayer_Create((SotnDataView){f->dra, sizeof(f->dra)}, map, 135, 135, &f->player));
    for (Uint16 i = 0; i < 17; ++i)
        CHECK(SotnPlayer_Tick(f->player, 0));
    return true;
}
static void Capture(SotnMistWedge out[8], SotnPlayerState state)
{
    const Sint32 x = state.x / 65536, y = state.y / 65536;
    for (Uint16 i = 0; i < 8; ++i)
        out[i] = (SotnMistWedge){.x = {(Sint16)(x - 8), (Sint16)(x + 8), (Sint16)x},
                                 .y = {(Sint16)(y - 16), (Sint16)(y - 16), (Sint16)(y - 4)},
                                 .u = {4, 20, 12},
                                 .v = {1, 1, 13}};
}
static bool Tick(Fixture *f, Uint16 buttons, bool water)
{
    const SotnCharacterStats stats = {.total = {6, 6, 6, 6}, .attack = {6, 6}, .unarmed_attack = 6};
    CHECK(SotnPlayer_SetCharacter(f->player, &f->character, stats));
    CHECK(SotnPlayer_SetMistContext(f->player, f->present ? &f->effect : NULL, f->generation,
                                    f->tick, water));
    CHECK(SotnPlayer_Tick(f->player, buttons) && SotnPlayer_GetState(f->player, &f->state));
    CHECK(f->character.mp >= f->state.mp_spent);
    f->character.mp = (Uint16)(f->character.mp - f->state.mp_spent);
    if (f->state.mist.started)
    {
        f->effect = (SotnMistEffect){.x = f->state.x, .y = f->state.y};
        f->generation = f->state.mist.generation;
        f->present = true;
    }
    if (f->state.mist.request_restore)
    {
        CHECK(f->present && f->generation == f->state.mist.generation);
        f->effect.phase = 3;
    }
    if (f->state.mist.retire)
    {
        CHECK(f->present && f->effect.phase == 5);
        f->present = false;
        f->generation = 0;
    }
    if (f->present)
    {
        SotnMistWedge wedges[8] = {0};
        Capture(wedges, f->state);
        SotnMistEffectInput input = {.dra = {f->dra, sizeof(f->dra)},
                                     .executable = {f->exe, sizeof(f->exe)},
                                     .capture = wedges,
                                     .x = (Sint16)(f->state.x / 65536),
                                     .y = (Sint16)(f->state.y / 65536),
                                     .vx = f->state.velocity_x,
                                     .vy = f->state.velocity_y,
                                     .game_timer = f->tick,
                                     .priority = 148,
                                     .facing_left = f->state.facing_left};
        SotnMistEffectEvents events = {0};
        CHECK(SotnMistEffect_Tick(&f->effect, input, &events));
    }
    ++f->tick;
    return true;
}
static bool Progress(Fixture *f)
{
    CHECK(Create(f));
    SotnWorldState world = {0};
    world.relics[SOTN_RELIC_MIST] = 1;
    world.relics[SOTN_RELIC_POWER_MIST] = 3;
    CHECK(SotnPlayer_SetRelics(f->player, &world));
    CHECK(Tick(f, SOTN_BUTTON_MIST, false) && !f->state.mist.active);
    CHECK(Tick(f, 0, false));
    world.relics[SOTN_RELIC_MIST] = 3;
    CHECK(SotnPlayer_SetRelics(f->player, &world));
    f->character.mp = 2;
    CHECK(Tick(f, SOTN_BUTTON_MIST, false) && !f->state.mist.active);
    CHECK(Tick(f, 0, false));
    f->character.mp = 20;
    CHECK(Tick(f, SOTN_BUTTON_MIST, false) && f->state.step == SOTN_PLAYER_MIST &&
          f->state.mist.started);
    CHECK(f->state.invulnerable && (f->state.status & 2U) && f->state.solid_height == 7 &&
          f->present);
    const Sint32 start = f->state.x;
    for (Uint16 i = 0; i < 60; ++i)
        CHECK(Tick(f, SOTN_BUTTON_RIGHT, false));
    CHECK(f->state.x > start + 50 * 65536 && f->character.mp == 16 && f->state.mist.active);
    CHECK(!SotnPlayer_SetMistContext(f->player, NULL, 0, f->tick, false));
    CHECK(!SotnPlayer_SetMistContext(f->player, &f->effect, f->generation + 1U, f->tick, false));
    CHECK(Tick(f, SOTN_BUTTON_MIST, false) && f->state.step == SOTN_PLAYER_UNMORPH_MIST);
    Uint16 waited = 0;
    while (f->state.mist.active && waited < 100)
    {
        CHECK(Tick(f, 0, false));
        ++waited;
    }
    CHECK(waited >= 47 && waited < 80 && f->state.mist.retire && !f->present &&
          !(f->state.status & 2U));
    const Uint32 previous = f->state.mist.generation;
    CHECK(Tick(f, SOTN_BUTTON_MIST, false) && !f->state.mist.active &&
          f->state.mist.generation == previous);
    for (Uint16 i = 0; i < 25; ++i)
        CHECK(Tick(f, 0, false));
    CHECK(Tick(f, SOTN_BUTTON_MIST, false) && f->state.mist.started &&
          f->state.mist.generation == previous + 1U);
    CHECK(Tick(f, 0, true) && f->state.step == SOTN_PLAYER_UNMORPH_MIST);
    SotnPlayer_Close(f->player);
    return true;
}
static bool Barrier(Fixture *f)
{
    CHECK(Create(f));
    for (Uint16 y = 0; y < 10; ++y)
        f->tiles[(y * 32U + 10U) * 2U] = 2;
    SotnWorldState world = {0};
    world.relics[SOTN_RELIC_MIST] = world.relics[SOTN_RELIC_POWER_MIST] = 3;
    CHECK(SotnPlayer_SetRelics(f->player, &world) && Tick(f, SOTN_BUTTON_MIST, false));
    for (Uint16 i = 0; i < 33; ++i)
        CHECK(Tick(f, SOTN_BUTTON_RIGHT, false));
    CHECK(f->state.x >= 167 * 65536 && f->state.x < 176 * 65536);
    CHECK(Tick(f, SOTN_BUTTON_MIST, false));
    Uint16 waited = 0;
    while (f->state.mist.active && waited < 120)
    {
        CHECK(Tick(f, 0, false));
        ++waited;
    }
    CHECK(!f->state.mist.active && waited < 120 && f->state.x >= 183 * 65536);
    SotnPlayer_Close(f->player);
    return true;
}
static bool Cramped(Fixture *f)
{
    CHECK(Create(f));
    SotnWorldState world = {0};
    world.relics[SOTN_RELIC_MIST] = world.relics[SOTN_RELIC_POWER_MIST] = 3;
    CHECK(SotnPlayer_SetRelics(f->player, &world) && Tick(f, SOTN_BUTTON_MIST, false));
    for (Uint16 x = 0; x < 32; ++x)
        f->tiles[(7U * 32U + x) * 2U] = 1;
    CHECK(Tick(f, 0, false) && Tick(f, SOTN_BUTTON_MIST, false));
    Uint16 waited = 0;
    while (f->state.mist.active && waited < 100)
    {
        CHECK(Tick(f, 0, false));
        ++waited;
    }
    CHECK(!f->state.mist.active && f->state.step == SOTN_PLAYER_STUCK && f->state.sound == 0x6f2);
    CHECK(Tick(f, SOTN_BUTTON_LEFT, false));
    SotnPlayer_Close(f->player);
    return true;
}
static bool SlopeMask(Fixture *f)
{
    for (Uint16 pass = 0; pass < 2; ++pass)
    {
        CHECK(Create(f));
        SotnPlayer *p = f->player;
        p->step = SOTN_PLAYER_UNMORPH_MIST;
        p->vx = p->vy = 0;
        p->contact = pass ? 0x8000 : 0x4000;
        p->mist = (SotnPlayerMist){.active = true, .restoring = true, .generation = 1};
        p->mist_visual_present = true;
        p->mist_visual_generation = 1;
        p->mist_visual_phase = 1;
        for (Uint16 i = 0; i < 4; ++i)
        {
            p->floor_sensors[i].y = 8;
            p->ceiling_sensors[i].y = -8;
        }
        const Sint16 y = p->y.parts.high;
        SotnPlayer_UnmorphMistTick(p);
        CHECK(!p->failed && p->y.parts.high == y - pass);
        SotnPlayer_Close(p);
    }
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    bool ok = Progress(f) && Barrier(f) && Cramped(f) && SlopeMask(f);
    SDL_free(f);
    return ok ? 0 : 1;
}
