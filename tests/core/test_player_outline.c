#include <SDL3/SDL.h>
#include <sotn/player_outline.h>
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
static bool Lifecycle(void)
{
    Uint8 *dra = SDL_calloc(1, 0xdaa8);
    CHECK(dra);
    for (Uint16 id = 0; id < 24; ++id)
    {
        for (Uint16 i = 0; i < 3; ++i)
            Put16(dra + 0xd9b8 + id * 10U + i * 2U, (Uint16)(40U + i * 40U));
        Put16(dra + 0xd9b8 + id * 10U + 6U, 0x103);
        Put16(dra + 0xd9b8 + id * 10U + 8U, 49);
    }
    SotnPlayerOutlineInput in = {.dra = {dra, 0xdaa8},
                                 .image = {32, 48, -16, -24},
                                 .x = 128,
                                 .y = 120,
                                 .scale_x = 256,
                                 .scale_y = 256,
                                 .bank = 1,
                                 .frame = 6,
                                 .priority = 148};
    for (Uint16 id = 0; id < 24; ++id)
        for (Uint16 face = 0; face < 2; ++face)
        {
            SotnPlayerOutline e = {.parameter = (Uint16)(id << 8)};
            SotnPlayerOutlineEvents out = {0};
            in.facing_left = face != 0;
            CHECK(SotnPlayerOutline_Tick(&e, in, &out) && e.phase == 1 && e.timer == 8 &&
                  !e.removed && out.reset_afterimages && e.quad.page == 0x18 &&
                  e.quad.palette == 0x103 && e.quad.priority == 150 && e.quad.draw_mode == 0x335 &&
                  e.brightness == (id == 1   ? 256
                                   : id == 2 ? 192
                                             : 128));
            const SotnPlayerOutline before = e;
            CHECK(SotnPlayerOutline_Translate(&e, -32, 16) &&
                  e.quad.x[0] == before.quad.x[0] - 32 && e.y == before.y + 16 &&
                  e.timer == before.timer && e.scale_x == before.scale_x &&
                  e.brightness == before.brightness);
            CHECK(SotnPlayerOutline_Translate(&e, 32, -16) && !SDL_memcmp(&e, &before, sizeof(e)));
            for (Uint16 t = 0; t < 8; ++t)
                CHECK(SotnPlayerOutline_Tick(&e, in, &out) && out.reset_afterimages && !e.removed);
            CHECK(e.phase == 2 && !e.timer);
            Uint16 ticks = 9;
            while (!e.removed && ticks < 64)
            {
                CHECK(SotnPlayerOutline_Tick(&e, in, &out) && out.reset_afterimages == !e.removed);
                ++ticks;
            }
            CHECK(e.removed && ticks < 64 && !out.reset_afterimages);
        }
    SotnPlayerOutline e = {.parameter = 0x1100};
    SotnPlayerOutlineEvents out = {0};
    in.facing_left = false;
    in.pivot_x = 3;
    in.pivot_y = 7;
    CHECK(SotnPlayerOutline_Tick(&e, in, &out));
    in.pivot_x = 99;
    in.pivot_y = -99;
    in.x += 20;
    in.image.pivot_x -= 2;
    CHECK(SotnPlayerOutline_Tick(&e, in, &out) && e.pivot_x == 3 && e.pivot_y == 7 &&
          e.quad.x[0] == 130);
    const Uint32 retirement[] = {1, 2, 4, 0x80, 0x40000, 0x1000000};
    for (Uint16 i = 0; i < SDL_arraysize(retirement); ++i)
    {
        e = (SotnPlayerOutline){0};
        in.status = retirement[i];
        CHECK(SotnPlayerOutline_Tick(&e, in, &out) && e.removed && !out.reset_afterimages);
    }
    in.status = 0;
    in.allocation_failed = true;
    e = (SotnPlayerOutline){0};
    CHECK(SotnPlayerOutline_Tick(&e, in, &out) && e.removed && !out.reset_afterimages);
    in.allocation_failed = false;
    e = (SotnPlayerOutline){.parameter = 0x1100, .draw_flags = 4};
    const SotnPlayerOutline before = e;
    out.reset_afterimages = true;
    CHECK(!SotnPlayerOutline_Tick(&e, in, &out) && !SDL_memcmp(&e, &before, sizeof(e)) &&
          out.reset_afterimages);
    e = (SotnPlayerOutline){.phase = 1};
    CHECK(!SotnPlayerOutline_Tick(&e, in, &out));
    e = (SotnPlayerOutline){.parameter = 0x1800};
    CHECK(!SotnPlayerOutline_Tick(&e, in, &out));
    CHECK(!SotnPlayerOutline_Translate(&e, 32768, 0));
    SDL_free(dra);
    return true;
}
int main(void)
{
    return Lifecycle() ? 0 : 1;
}
