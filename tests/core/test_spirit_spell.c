#include <SDL3/SDL.h>
#include <sotn/spell_target.h>
#include <sotn/spirit_spell.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s: %s", __LINE__, #v, SDL_GetError());                               \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static void Put16(Uint8 *p, Uint16 v)
{
    p[0] = (Uint8)v;
    p[1] = (Uint8)(v >> 8);
}
static bool Targets(void)
{
    SotnSpellTarget slots[128] = {0};
    Uint16 cursor = 120, selected = 99;
    CHECK(SotnSpellTarget_Select(slots, &cursor, &selected) && selected == SDL_MAX_UINT16 &&
          cursor == 120);
    for (Uint16 i = 0; i < 128; ++i)
        slots[i] = (SotnSpellTarget){.identity = 1, .hitbox = 2, .hp = 10, .x = 100, .y = 120};
    slots[0].identity = 0;
    slots[1].hitbox = 0;
    slots[2].flags = 0x200000;
    slots[3].x = -17;
    slots[4].x = 273;
    slots[5].y = -1;
    slots[6].y = 241;
    slots[7].hp = 0x7000;
    CHECK(SotnSpellTarget_Select(slots, &cursor, &selected) && selected == 8 && cursor == 120 &&
          slots[8].flags == 0x80000);
    CHECK(SotnSpellTarget_Select(slots, &cursor, &selected) && selected == 9 && cursor == 120);
    for (Uint16 i = 0; i < 128; ++i)
        slots[i].flags |= 0x80000;
    slots[120].x = -16;
    slots[120].y = 240;
    slots[120].hp = -1;
    slots[120].flags |= 0x100;
    CHECK(SotnSpellTarget_Select(slots, &cursor, &selected) && selected == 120 && cursor == 121);
    cursor = 127;
    CHECK(SotnSpellTarget_Select(slots, &cursor, &selected) && selected == 127 && !cursor);
    CHECK(SotnSpellTarget_Select(slots, &cursor, &selected) && selected == 8 && cursor == 9);
    cursor = 128;
    selected = 17;
    const SotnSpellTarget before = slots[8];
    CHECK(!SotnSpellTarget_Select(slots, &cursor, &selected) && cursor == 128 && selected == 17 &&
          !SDL_memcmp(&before, &slots[8], sizeof(before)));
    return true;
}
static bool Motion(SotnDataView dra, SotnDataView exe)
{
    for (Uint16 param = 0; param < 4; ++param)
        for (Uint16 face = 0; face < 2; ++face)
        {
            SotnSpirit s = {.parameter = (Uint16)(param | 0x100U)};
            SotnSpiritEvents out = {0};
            SotnSpiritInput in = {.dra = dra,
                                  .executable = exe,
                                  .x = 100 * 65536 + 123,
                                  .y = -33 * 65536 + 456,
                                  .allocated = true,
                                  .player_facing_left = face != 0};
            CHECK(SotnSpirit_Tick(&s, in, &out) && s.phase == 1 && !s.removed &&
                  out.initialize_attack && out.spell == 3 && s.facing_left == (face == 0) &&
                  s.cursor == 1);
            CHECK(s.trails[0].state == 2 && s.trails[0].scale == 256 &&
                  s.trails[0].quad.u[0] == 17 && s.trails[0].quad.v[0] == 33 &&
                  s.trails[0].quad.green[3] == 128 && !(s.trails[0].quad.draw_mode & 8));
            for (Uint16 i = 1; i < 8; ++i)
                CHECK(!s.trails[i].state && (s.trails[i].quad.draw_mode & 8));
            const SotnSpirit born = s;
            CHECK(SotnSpirit_Translate(&s, -32, 16) && s.x == born.x - 32 * 65536 &&
                  s.trails[0].x == born.trails[0].x - 32 &&
                  s.trails[0].scale == born.trails[0].scale && s.cursor == born.cursor);
            CHECK(SotnSpirit_Translate(&s, 32, -16) && !SDL_memcmp(&s, &born, sizeof(s)));
            CHECK(!SotnSpirit_Translate(&s, 32768, 0) && !SDL_memcmp(&s, &born, sizeof(s)));
            in.frame = 1;
            CHECK(SotnSpirit_Tick(&s, in, &out) && s.phase == 2 && out.select_target &&
                  !out.initialize_attack && !s.age && s.angle == 0xc00);
            CHECK(s.trails[0].scale == 240 && s.trails[0].quad.green[3] == 116 &&
                  s.trails[1].scale == 256);
            CHECK(s.facing_left == ((face == 0) != (param >= 2)));
            in.target_bound = in.target_alive = true;
            in.target_x = 100;
            in.target_y = 100;
            for (Uint16 t = 0; t < 97; ++t)
                CHECK(SotnSpirit_Tick(&s, in, &out) && !out.select_target);
            CHECK(s.phase == 3 && s.age == 97 && s.flags == 0xc930000U);
            CHECK(SotnSpirit_Tick(&s, in, &out) && s.flags == 0x8910000U);
            s = born;
            CHECK(SotnSpirit_Tick(&s, in, &out) && out.select_target);
            in.target_alive = false;
            const Sint16 age = s.age;
            CHECK(SotnSpirit_Tick(&s, in, &out) && s.phase == 3 && s.age == age &&
                  s.flags == 0xc930000U);
        }
    SotnSpirit s = {0}, before = s;
    SotnSpiritEvents out = {.spell = 99};
    CHECK(!SotnSpirit_Tick(&s, (SotnSpiritInput){.allocated = true}, &out) &&
          !SDL_memcmp(&s, &before, sizeof(s)) && out.spell == 99);
    CHECK(!SotnSpirit_Tick(&s, (SotnSpiritInput){.dra = dra, .allocated = true}, &out) &&
          !SDL_memcmp(&s, &before, sizeof(s)) && out.spell == 99);
    CHECK(SotnSpirit_Tick(&s, (SotnSpiritInput){0}, &out) && s.removed && !out.initialize_attack);
    s = (SotnSpirit){.parameter = 4};
    CHECK(!SotnSpirit_Tick(&s, (SotnSpiritInput){0}, &out));
    return true;
}
static bool Portal(SotnDataView dra, SotnDataView exe)
{
    for (Uint16 tetra = 0; tetra < 2; ++tetra)
    {
        SotnSpiritPortal p = {.parameter = (Uint16)(tetra << 8)};
        SotnSpiritPortalEvents out = {0};
        SotnSpiritPortalInput in = {.dra = dra,
                                    .executable = exe,
                                    .x = 100 * 65536 + 123,
                                    .y = 100 * 65536 + 456,
                                    .player_y = 160,
                                    .allocated = true};
        CHECK(SotnSpiritPortal_Tick(&p, in, &out) && p.phase == 1 && p.delay == 10 &&
              out.freeze == 3 && out.flash == 13 && !out.capture);
        for (Uint16 t = 0; t < 9; ++t)
            CHECK(SotnSpiritPortal_Tick(&p, in, &out) && p.phase == 1 && !out.freeze &&
                  !out.capture);
        const SotnSpiritPortal waiting = p;
        CHECK(SotnSpiritPortal_Tick(&p, in, &out) && p.phase == 2 && p.x == 128 * 65536 + 123 &&
              p.y == 112 * 65536 + 456 && out.capture && out.capture_x == 96 &&
              out.capture_y == 80);
        CHECK(p.quads[0].x[1] == 143 && p.quads[0].y[1] == 96 && p.quads[0].u[1] == 47 &&
              p.quads[0].v[1] == 208 && p.quads[0].x[0] == 128 && p.quads[0].y[0] == 112 &&
              p.quads[0].priority == 449 && p.quads[4].priority == 452);
        for (Uint16 t = 0; t < 11; ++t)
        {
            CHECK(SotnSpiritPortal_Tick(&p, in, &out));
            if (t < 10)
                CHECK(!out.factories[0] && !out.sound);
        }
        CHECK(p.phase == 3 && p.timer == 11 && out.factories[0] == (tetra ? 118U : 116U) &&
              out.factories[1] == 0x67002cU && out.sound == 0x67d);
        for (Uint16 t = 0; t < 11; ++t)
            CHECK(SotnSpiritPortal_Tick(&p, in, &out) && !p.removed && !out.release_freeze &&
                  !out.factories[0]);
        CHECK(SotnSpiritPortal_Tick(&p, in, &out) && p.removed && out.release_freeze);
        p = waiting;
        in.executable = (SotnDataView){0};
        out.flash = 99;
        CHECK(!SotnSpiritPortal_Tick(&p, in, &out) && !SDL_memcmp(&p, &waiting, sizeof(p)) &&
              out.flash == 99);
        p = waiting;
        in.allocated = false;
        in.dra = in.executable = (SotnDataView){0};
        CHECK(SotnSpiritPortal_Tick(&p, in, &out) && p.removed && !out.release_freeze &&
              !out.capture);
        p = waiting;
        in.allocated = true;
        out.flash = 99;
        CHECK(!SotnSpiritPortal_Tick(&p, in, &out) && !SDL_memcmp(&p, &waiting, sizeof(p)) &&
              out.flash == 99);
    }
    return true;
}
int main(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *exe = SDL_calloc(1, 0x24000);
    if (!dra || !exe)
        return 1;
    /* Authored geometry, UV neighborhood and linear SDK fixtures, not game artwork/tables. */
    const Sint16 shape[18] = {15, 0,   15, -16, 0,  -16, -16, -16, -16,
                              0,  -16, 15, 0,   15, 15,  15,  15,  0};
    for (Uint16 i = 0; i < 18; ++i)
        Put16(dra + 0x10860 + i * 2U, (Uint16)shape[i]);
    for (Uint16 i = 0; i < 18; ++i)
        dra[0x10846 + i] = (Uint8)(17U + i * 16U);
    for (Uint16 i = 0; i < 4; ++i)
        Put16(dra + 0x10858 + i * 2U, (Uint16)(512U + i * 512U));
    for (Uint16 i = 0; i < 1025; ++i)
    {
        Put16(exe + 0x1dbcc + i * 2U, (Uint16)(i * 4U));
        Put16(exe + 0x22a10 + i * 2U, (Uint16)(i / 2U));
    }
    bool ok = Targets() && Motion((SotnDataView){dra, 0x11000}, (SotnDataView){exe, 0x24000}) &&
              Portal((SotnDataView){dra, 0x11000}, (SotnDataView){exe, 0x24000});
    SDL_free(dra);
    SDL_free(exe);
    return ok ? 0 : 1;
}
