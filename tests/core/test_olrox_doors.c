#include "actors_internal.h"
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d %s: %s", __LINE__, #v, SDL_GetError());                               \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
typedef struct Fixture
{
    Uint8 stage[0x2000], dra[0x400], exe[0x1e400], tiles[48 * 32 * 2], types[64];
    SotnActorLayout *layout;
} Fixture;
static void Put16(Uint8 *p, Uint16 value)
{
    p[0] = (Uint8)value;
    p[1] = (Uint8)(value >> 8);
}
static void Put32(Uint8 *p, Uint32 value)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(value >> (i * 8U));
}
static bool Open(Fixture *f, SotnActors **out, Uint16 params, bool near)
{
    SDL_zero(*f);
    Put32(f->dra + 0x98, 0x800a0200);
    Put32(f->stage + 28, 0x80180040);
    for (Uint16 i = 0; i < 106; ++i)
        Put32(f->stage + 0x40 + i * 4U, 0x80180300);
    Put16(f->stage + 0x300, 0xfffe);
    Put16(f->stage + 0x302, 0xfffe);
    Put16(f->stage + 0x30a, 0xffff);
    Put16(f->stage + 0x30c, 0xffff);
    // Authored texture/tile and linear quarter-wave fixtures, not original game data.
    for (Uint16 i = 0; i < 48; ++i)
        f->stage[0xf7c + i] = (Uint8)(i + 1);
    for (Uint16 i = 0; i < 32; ++i)
        Put16(f->stage + 0xfac + i * 2U, (Uint16)(10 + i));
    for (Uint16 i = 0; i <= 1024; ++i)
        Put16(f->exe + 0x1dbcc + i * 2U, (Uint16)(i * 4U));
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 4, out));
    CHECK(SotnActorLayout_OpenStage((SotnDataView){f->stage, sizeof(f->stage)}, 4, 0, &f->layout));
    CHECK(SotnActors_Enter(
        *out, f->layout,
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 48, 32}, 0,
        0));
    SotnActors *s = *out;
    s->executable = (SotnDataView){f->exe, sizeof(f->exe)};
    s->player = (SotnPlayerState){
        .x = (near ? 128 : 360) * 65536, .y = 128 * 65536, .priority = 0x80, .velocity_y = 1234};
    s->slots[16] = (Actor){.slot = 16,
                           .dynamic = true,
                           .state = {.id = 0x33,
                                     .generation = 1,
                                     .parameters = params,
                                     .x = 128 * 65536 + 123,
                                     .y = 128 * 65536 + 456}};
    return true;
}
static void Close(Fixture *f, SotnActors *s)
{
    SotnActors_Destroy(s);
    SotnActorLayout_Close(f->layout);
}
static bool Tiles(SotnActors *s, Actor *a, bool closed)
{
    const Uint32 index =
        (Uint32)Pixel(a->state.x) / 16 + (Uint32)(Pixel(a->state.y) - 24) / 16 * s->map.width;
    for (Uint16 i = 0; i < 4; ++i)
        CHECK(Read16(s->mutable_tiles + (index + i * s->map.width) * 2U) ==
              10 + (a->state.parameters & 255U) * 8U + (closed ? 0 : 4) + i);
    return true;
}
static bool Cycle(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    CHECK(f);
    for (Uint16 params = 0; params < 4; ++params)
        for (Uint16 side = 0; side < 2; ++side)
            for (Uint16 alt = 0; alt < 2; ++alt)
            {
                SotnActors *s = NULL;
                CHECK(Open(f, &s, (Uint16)(params | side * 0x100 | alt * 0x1000), false));
                Actor *a = &s->slots[16];
                CHECK(ActorOlroxQuarter(s, a) && a->state.step == 1 && a->state.frame == 1 &&
                      a->state.primitive_count == 3);
                CHECK((a->state.x & 65535) == 123 && (a->state.y & 65535) == 456 &&
                      Tiles(s, a, true));
                ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
                CHECK(p && p->state.active == (alt != 0));
                s->player.x = a->state.x - 16 * 65536;
                s->player.y = a->state.y;
                s->player.step = SOTN_PLAYER_WALK;
                s->player.facing_left = true;
                CHECK(ActorOlroxQuarter(s, a) && a->state.step == 1); // Facing away cannot open it.
                s->player.facing_left = false;
                s->combat = (SotnActorCombat){0};
                CHECK(ActorOlroxQuarter(s, a) && a->state.step == 2 && a->state.frame == 0 &&
                      Tiles(s, a, false));
                CHECK(s->combat.sound_count == 2 && s->combat.sounds[0] == 0x642 &&
                      s->combat.volumes[0] == 96 && s->combat.pans[0] == (side ? 6 : -6));
                ActorPrimitive *face = ActorPrimitiveGet(s, a, side ? 2 : 1);
                CHECK(face && face->state.vertex_red[1] == (side ? 63 : 127));

                for (Uint16 tick = 0; tick < 32; ++tick)
                    CHECK(ActorOlroxQuarter(s, a));
                CHECK(a->state.step == 3 && a->enemy_state.angle_delta == (side ? 0x800 : 0x1000));
                CHECK(p->state.page == (alt ? 0x12 : 0x1f) &&
                      p->state.palette == (alt ? 0x21b : 0x198) && p->state.gouraud &&
                      p->state.priority == 0x60);
                CHECK(p->state.vertex_red[0] == 127 && p->state.vertex_red[1] == (side ? 127 : 63));

                s->control.timer = 3;
                CHECK(ActorOlroxQuarter(s, a) &&
                      s->control.buttons == (side ? SOTN_BUTTON_RIGHT : SOTN_BUTTON_LEFT));
                // Entry from the adjoining room creates an open door without a relic gate.
                Close(f, s);
                CHECK(Open(f, &s, (Uint16)(params | side * 0x100 | alt * 0x1000), true));
                a = &s->slots[16];
                CHECK(ActorOlroxQuarter(s, a) && a->state.step == 4 && !a->state.frame &&
                      s->motion.halt_vertical && s->player.velocity_y == 0);
                s->player.x = 400 * 65536;
                s->combat = (SotnActorCombat){0};
                CHECK(ActorOlroxQuarter(s, a) && a->state.step == 5 &&
                      s->combat.sounds[0] == 0x642);
                for (Uint16 tick = 0; tick < 32; ++tick)
                    CHECK(ActorOlroxQuarter(s, a));
                CHECK(a->state.step == 1 && a->state.frame == 1 &&
                      a->enemy_state.angle_delta == 0xc00 && Tiles(s, a, true));
                CHECK(s->combat.sounds[s->combat.sound_count - 1] == 0x64f);
                Close(f, s);
            }
    SDL_free(f);
    return true;
}
static bool Invalid(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    SotnActors *s = NULL;
    CHECK(f && Open(f, &s, 4, false));
    Actor *a = &s->slots[16];
    CHECK(!ActorNo2Door(s, a) && !a->state.step);
    a->state.parameters = 0;
    a->state.x = SDL_MAX_SINT32;
    CHECK(!ActorNo2Door(s, a));
    a->state.x = 128 * 65536;
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        s->primitives[i].owner = 999;
    CHECK(ActorNo2Door(s, a) && a->remove && !a->state.primitive_count);
    Close(f, s);
    CHECK(Open(f, &s, 0, false));
    a = &s->slots[16];
    Uint8 before[sizeof(f->tiles)] = {0};
    SDL_memcpy(before, s->mutable_tiles, sizeof(before));
    Put16(f->stage + 0xfac + 6, 64);
    CHECK(!ActorNo2Door(s, a) && SDL_memcmp(before, s->mutable_tiles, sizeof(before)) == 0);
    Close(f, s);
    CHECK(Open(f, &s, 0, false));
    a = &s->slots[16];
    s->executable = (SotnDataView){0};
    CHECK(!ActorNo2Door(s, a));
    CHECK(!ActorNo2Door(NULL, a) && !ActorNo2Door(s, NULL));
    Close(f, s);
    CHECK(Open(f, &s, 0, false));
    a = &s->slots[16];
    CHECK(ActorNo2Door(s, a));
    s->player = (SotnPlayerState){.x = a->state.x + 25 * 65536,
                                  .y = a->state.y + 32 * 65536,
                                  .step = SOTN_PLAYER_WALK,
                                  .facing_left = true,
                                  .priority = 0x80};
    CHECK(ActorNo2Door(s, a) && a->state.step == 1);
    s->player.x -= 65536;
    s->player.y += 65536;
    CHECK(ActorNo2Door(s, a) && a->state.step == 1);
    s->player.y -= 65536;
    CHECK(ActorNo2Door(s, a) && a->state.step == 2);
    a->state.step = 6;
    CHECK(!ActorNo2Door(s, a));
    Close(f, s);
    SDL_free(f);
    return true;
}
int main(void)
{
    if (!Cycle() || !Invalid())
        return 1;
    SDL_Log("NO2_STONE_DOORS: PASS");
    return 0;
}
