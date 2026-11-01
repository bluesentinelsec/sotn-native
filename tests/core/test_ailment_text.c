#include "actors_internal.h"
#include "character_fixture.h"
#include <sotn/bat_runtime.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s: %s", __LINE__, #v, SDL_GetError());                               \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static void Put16(Uint8 *p, Uint16 value)
{
    p[0] = (Uint8)value;
    p[1] = (Uint8)(value >> 8);
}
static void Profiles(Uint8 *dra)
{
    TestCharacter_Fill(dra);
    for (Uint16 i = 0; i < 4; ++i)
    {
        Uint8 *p = dra + 0xcfec + i * 8U;
        p[0] = 1;
        p[1] = 2;
        p[2] = (Uint8)(25 + i * 4U);
        p[3] = 10;
        Put16(p + 4, 0x1b4);
        Put16(p + 6, 1);
    }
    for (Uint16 i = 0; i < 2; ++i)
    {
        Uint8 *b = dra + 0xd1d4 + (i ? 47U : 44U) * 6U;
        b[0] = i ? 37 : 11;
        b[1] = 1;
        b[2] = 0x81;
        Uint8 *p = dra + 0xd670 + (0x16U + i) * 20U;
        Put16(p + 6, 0x100);
        for (Uint16 k = 8; k <= 12; k += 2)
            Put16(p + k, 256);
        Put16(p + 14, i ? 0x7000 : 0x7002);
        Put16(p + 16, 0x31);
        Put16(p + 18, 512);
    }
    dra[0xd4b8] = 32;
    dra[0xd4b9] = 63;
}
static bool Controller(SotnDataView dra, SotnDataView exe)
{
    for (Uint16 kind = 0; kind < 4; ++kind)
    {
        SotnImmunityText n = {0};
        const Uint16 parameter = (Uint16)(kind << 8);
        for (Uint16 t = 0; t < 6; ++t)
            CHECK(SotnImmunityText_TickParameter(&n, dra, exe, 100 * 65536 + 123, 200 * 65536 + 456,
                                                 false, false, parameter) &&
                  n.phase == 1 && !n.removed && !n.sound);
        CHECK(SotnImmunityText_TickParameter(&n, dra, exe, 0, 0, true, false, parameter) &&
              n.phase == 2);
        SotnImmunityText lost = n;
        CHECK(SotnImmunityText_TickParameter(&lost, dra, exe, 0, 0, true, false, parameter) &&
              lost.removed);
        CHECK(SotnImmunityText_TickParameter(&n, dra, exe, 100 * 65536 + 123, 200 * 65536 + 456,
                                             true, true, parameter) &&
              n.phase == 3 && n.y == 216 * 65536 && n.target == 120 * 65536 + 12 + kind * 2);
        const SotnImmunityText before = n;
        CHECK(!SotnImmunityText_TickParameter(&n, (SotnDataView){0}, exe, 0, 0, true, true,
                                              parameter) &&
              !SDL_memcmp(&n, &before, sizeof(n)));
        CHECK(!SotnImmunityText_TickParameter(&n, dra, exe, 0, 0, true, true, 0x400) &&
              !SDL_memcmp(&n, &before, sizeof(n)));
        Uint16 sounds = 0, hold = 0, tiles = 0;
        for (Uint16 t = 0; t < 300 && !n.removed; ++t)
        {
            const Uint8 phase = n.phase;
            CHECK(SotnImmunityText_TickParameter(&n, dra, exe, 0, 0, false, true, parameter));
            sounds += n.sound != 0;
            hold += phase == 6;
            if (n.sound)
                CHECK(n.sound == 0x6af && n.phase == 6 && n.timer == 96 && n.x == n.target &&
                      n.half_width == 12 + kind * 2);
            for (Uint16 i = 0; i < 16; ++i)
                tiles += n.tiles[i].active;
        }
        CHECK(n.removed && sounds == (kind ? 1 : 0) && hold == (kind ? 96 : 0) &&
              (tiles != 0) == (kind != 0));
    }
    SotnImmunityText n = {.phase = 6, .timer = 2, .x = 120 * 65536 + 12, .y = 32 * 65536};
    n.tiles[0].x = n.tiles[1].x = 100;
    n.tiles[0].y = n.tiles[1].y = 32767;
    n.tiles[0].active = n.tiles[1].active = true;
    n.tiles[0].timer = n.tiles[1].timer = 1;
    n.tiles[0].draw_mode = 2;
    n.tiles[1].draw_mode = 0;
    CHECK(SotnImmunityText_Rebase(&n, -17, 5) && n.x == 120 * 65536 + 12 && n.tiles[0].x == 83 &&
          n.tiles[1].x == 100 && n.tiles[0].y == -32764);
    CHECK(SotnImmunityText_TickParameter(&n, dra, exe, 0, 0, true, true, 0x100) &&
          !n.tiles[0].active && n.tiles[0].draw_mode == 8 && n.tiles[0].y == -32763 &&
          n.tiles[1].y == -32768);
    return true;
}
static bool Shared(Uint8 *dra, SotnDataView exe)
{
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r)), *saved = SDL_malloc(sizeof(*saved));
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnCharacter *character = NULL;
    CHECK(r && saved && actors && SotnCharacter_Create((SotnDataView){dra, 0x11000}, &character));
    for (Uint16 kind = 1; kind <= 3; ++kind)
    {
        SDL_zero(*r);
        SotnBatRuntimeInput in = {.dra = {dra, 0x11000},
                                  .executable = exe,
                                  .image = {12, 24, -6, -12},
                                  .player = {.x = 100 * 65536 + 123,
                                             .y = 140 * 65536 + 456,
                                             .step = SOTN_PLAYER_HIT,
                                             .substep = 6,
                                             .hp = 70,
                                             .hp_max = 70,
                                             .mp = 20,
                                             .priority = 148,
                                             .contacts = 1,
                                             .poison_ticks = 3999,
                                             .curse_ticks = 1000,
                                             .animation = {.sprite = 1},
                                             .hurt = {.count = 2, .started = true}}};
        const Uint16 text = kind == 1 ? 46 : 47, blink = kind == 1 ? 47 : 46;
        in.player.hurt.factories[kind == 1 ? 0 : 1].blueprint = kind == 1 ? 0x16002cU : 0x17002cU;
        in.player.hurt.factories[kind == 1 ? 1 : 0].blueprint = (Uint32)kind << 16 | 47U;
        CHECK(SotnBatRuntime_Tick(r, character, in) && r->children[text].kind == 37 &&
              r->children[text].parameter == kind << 8 && r->children[blink].kind == 11 &&
              r->children[blink].primitive_count == 8 && SotnActors_CommitBatPrimitives(actors, r));
        in.player.hurt.count = 0;
        in.player.hurt.started = false;
        in.freeze = 3;
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) &&
              r->children[text].state.immunity_text.phase == 2);
        *saved = *r;
        dra[0xcfec + kind * 8U + 6] = 2;
        CHECK(!SotnBatRuntime_Tick(r, character, in) && !SDL_memcmp(saved, r, sizeof(*r)));
        dra[0xcfec + kind * 8U + 6] = 1;
        CHECK(SotnBatRuntime_Tick(r, character, in) && r->children[text].primitive_count == 18 &&
              SotnActors_CommitBatPrimitives(actors, r));
        for (Uint16 row = 0; row < 18; ++row)
            CHECK(actors->primitives[r->children[text].primitives[row]].ability_kind == 37);
        const SotnImmunityText prior = r->children[text].state.immunity_text;
        in.entering = true;
        in.screen_shift_x = -17;
        CHECK(SotnBatRuntime_Tick(r, character, in) &&
              r->children[text].state.immunity_text.x == prior.x &&
              r->children[text].state.immunity_text.quad.x[0] == prior.quad.x[0] - 17 &&
              r->children[text].state.immunity_text.timer == prior.timer);
        in.entering = false;
        in.screen_shift_x = 0;
        Uint16 sounds = 0;
        for (Uint16 t = 0; t < 300; ++t)
        {
            if (t == 100)
                in.player.poison_ticks = in.player.curse_ticks = 0;
            CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
                  SotnBatRuntime_Tick(r, character, in) &&
                  SotnActors_CommitBatPrimitives(actors, r));
            for (Uint16 i = 0; i < r->sound_count; ++i)
            {
                CHECK(r->sounds[i] == 0x6af);
                ++sounds;
            }
        }
        CHECK(sounds == 1 && !SotnBatRuntime_Occupied(r));
        for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
            CHECK(!actors->primitives[i].owner);
        SotnCharacterState before = {0}, after = {0};
        CHECK(SotnCharacter_Get(character, &before, NULL));
        in.freeze = 0;
        in.player.hurt.started = true;
        in.player.hurt.count = 1;
        in.player.hurt.factories[0].blueprint = (Uint32)kind << 16 | 47U;
        SDL_memset(&in.primitive_slots, 255, sizeof(in.primitive_slots));
        CHECK(SotnBatRuntime_Tick(r, character, in) && r->children[47].kind == 37);
        in.player.hurt.count = 0;
        in.player.hurt.started = false;
        CHECK(SotnBatRuntime_Tick(r, character, in) &&
              r->children[47].state.immunity_text.phase == 2 &&
              SotnBatRuntime_Tick(r, character, in) && !SotnBatRuntime_Occupied(r) &&
              SotnActors_CommitBatPrimitives(actors, r) &&
              SotnCharacter_Get(character, &after, NULL) && after.hp == before.hp &&
              after.mp == before.mp && after.random == before.random);
    }
    SotnCharacter_Destroy(character);
    SDL_free(actors);
    SDL_free(saved);
    SDL_free(r);
    return true;
}
int main(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *exe = SDL_calloc(1, 0x24000);
    if (!dra || !exe)
        return 1;
    Profiles(dra);
    for (Uint16 i = 0; i <= 1024; ++i)
    {
        Put16(exe + 0x1dbcc + i * 2U, i * 4U);
        Put16(exe + 0x22a10 + i * 2U, i / 2U);
    }
    const bool ok = Controller((SotnDataView){dra, 0x11000}, (SotnDataView){exe, 0x24000}) &&
                    Shared(dra, (SotnDataView){exe, 0x24000});
    SDL_free(dra);
    SDL_free(exe);
    return ok ? 0 : 1;
}
