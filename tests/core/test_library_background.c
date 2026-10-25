#include "actors_internal.h"
#include "boss_projection.h"
#include <SDL3/SDL.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d %s: %s", __LINE__, #v, SDL_GetError());                                \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)
static void Put16(Uint8 *p, Uint16 v)
{
    p[0] = (Uint8)v;
    p[1] = (Uint8)(v >> 8);
}
static void Put32(Uint8 *p, Uint32 value)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(value >> (i * 8));
}
static int Actors(void)
{
    Uint8 dra[0x400] = {0}, lib[0x1000] = {0};
    Put32(dra + 0x98, 0x800a0200);
    SotnActors *s = NULL;
    CHECK(SotnActors_CreateStage((SotnDataView){dra, sizeof(dra)}, (SotnDataView){lib, sizeof(lib)},
                                 2, &s));
    Uint16 backdrop = 123;
    CHECK(SotnActors_GetBackdrop(s, &backdrop) && !backdrop);
    CHECK(!SotnActors_GetBackdrop(NULL, &backdrop));
    CHECK(!SotnActors_GetBackdrop(s, NULL));
    for (Uint16 kind = 0; kind < 3; ++kind)
    {
        Actor a = {.slot = 128,
                   .state = {.id = (Uint16)(0x16U + kind),
                             .generation = (Uint32)(kind + 1U),
                             .x = 100 * 65536,
                             .y = 200 * 65536}};
        CHECK(ActorLibrary(s, &a) && a.state.step == 1 &&
              a.state.primitive_count == (kind == 2 ? 10 : 4));
        const ActorPrimitive *first = ActorPrimitiveGet(s, &a, 0);
        CHECK(first && first->state.screen_space && first->state.priority_set);
        if (kind == 0)
            CHECK(first->state.offset_x[0] == -148 && first->state.offset_x[1] == -24 &&
                  first->state.offset_y[0] == 124 && first->state.page == 15 &&
                  first->state.palette == 0x35 && first->state.u[0] == 130 &&
                  s->backdrop_color == 0x842);
        if (kind == 1)
            CHECK(first->state.offset_x[0] == -140 && first->state.offset_x[1] == -20 &&
                  first->state.offset_y[0] == 172 && first->state.palette == 0x34);
        if (kind == 2)
        {
            CHECK(!first->state.textured && first->state.blend && first->state.offset_x[0] == -68);
            const ActorPrimitive *last = ActorPrimitiveGet(s, &a, 9),
                                 *hidden = ActorPrimitiveGet(s, &a, 6);
            CHECK(last && hidden && last->state.active && !hidden->state.active &&
                  last->state.offset_x[0] == 0 && last->state.offset_x[1] == 255 &&
                  last->state.offset_y[0] == 88 && last->state.offset_y[2] == 136);
        }
        ActorPrimitivesClear(s, &a);
    }
    Put16(lib + 0xdf4, 1024);
    Put16(lib + 0xe78, 200);
    Put16(lib + 0xe7a, 300);
    Put32(lib + 0xe68, 0x80180df8);
    for (Uint16 i = 0; i < 25; ++i)
        lib[0xdf8 + i] = (Uint8)(64U + i);
    s->camera_x = 100;
    s->camera_y = 200;
    Actor far = {.slot = 128, .state = {.id = 0x19, .generation = 7, .x = 999 * 65536 + 123}};
    CHECK(ActorLibrary(s, &far) && far.state.x == 200 * 65536 + 123 && far.state.y == 300 * 65536 &&
          far.state.primitive_count == 16 && s->backdrop_color == 0x14e8);
    const ActorPrimitive *face = ActorPrimitiveGet(s, &far, 0);
    CHECK(face && face->state.gouraud && face->state.modulate && face->state.u[0] == 254 &&
          face->state.u[1] == 136 && face->state.vertex_red[0] == 64 &&
          face->state.vertex_green[1] == 65 && face->state.vertex_blue[2] == 69 &&
          face->state.vertex_red[3] == 70);
    const Sint16 before = face->state.offset_x[0];
    ++s->camera_x;
    CHECK(ActorLibrary(s, &far) && face->state.offset_x[0] == before - 1);
    Put32(lib + 0xe68, 0x80180f00);
    CHECK(!ActorLibrary(s, &far));
    ActorPrimitivesClear(s, &far);
    far = (Actor){.slot = 128, .state = {.id = 0x19, .generation = 8, .parameters = 4}};
    CHECK(!ActorLibrary(s, &far));
    Actor boundary = {.slot = 128, .state = {.id = 0x16, .generation = 9, .y = -32767 * 65536}};
    CHECK(!ActorLibrary(s, &boundary));
    SotnActors_Destroy(s);
    return 0;
}
int main(void)
{
    Uint8 bytes[0xf00] = {0};
    const SotnDataView lib = {bytes, sizeof(bytes)};
    SotnProjectedPoint p = {0};
    for (Uint16 kind = 0; kind < 4; ++kind)
    {
        CHECK(SotnLibrary_ProjectBackground(lib, kind, 192, 80, &p));
        CHECK(p.x == 192 && p.y == 80);
    }
    Put16(bytes + 0xdf4, 1024);
    CHECK(SotnLibrary_ProjectBackground(lib, 3, 1, 3, &p) && p.x == 64 && p.y == 65);
    CHECK(SotnLibrary_ProjectBackground(lib, 3, 129, 127, &p) && p.x == 128 && p.y == 127);
    CHECK(SotnLibrary_ProjectBackground(lib, 0, 32767, -32767, &p) && p.x == 1023 && p.y == -1024);
    const SotnProjectedPoint before = p;
    CHECK(!SotnLibrary_ProjectBackground(lib, 4, 0, 0, &p) && p.x == before.x && p.y == before.y);
    CHECK(!SotnLibrary_ProjectBackground(lib, 0, 32768, 0, &p));
    CHECK(!SotnLibrary_ProjectBackground(lib, 0, 0, 0, NULL));
    CHECK(!SotnLibrary_ProjectBackground((SotnDataView){bytes, 0xdc4}, 0, 0, 0, &p));
    Put16(bytes + 0xdc8, 1);
    CHECK(!SotnLibrary_ProjectBackground(lib, 0, 0, 0, &p));
    Put16(bytes + 0xdc8, 0);
    Put16(bytes + 0xdc0, 1);
    CHECK(!SotnLibrary_ProjectBackground(lib, 0, 0, 0, &p));
    Put16(bytes + 0xdc0, 0);
    Put16(bytes + 0xdc4, 0xffff);
    CHECK(!SotnLibrary_ProjectBackground(lib, 0, 0, 0, &p));
    CHECK(Actors() == 0);
    SDL_Log("SOTN_LIBRARY_BACKGROUND_TEST: PASS");
    return 0;
}
