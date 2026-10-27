#include "olrox_internal.h"
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d %s: %s", __LINE__, #v, SDL_GetError());                                \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
typedef struct Fixture
{
    Uint8 dra[0x1000], stage[0x4000], exe[0x24000], tiles[32 * 32 * 2], types[2];
    SotnActorLayout *layout;
} Fixture;
static void Put16(Uint8 *p, Sint32 n)
{
    p[0] = (Uint8)n;
    p[1] = (Uint8)((Uint32)n >> 8);
}
static void Put32(Uint8 *p, Uint32 n)
{
    for (Uint32 i = 0; i < 4; ++i)
        p[i] = (Uint8)(n >> (i * 8));
}
static bool Open(Fixture *f, SotnActors **out)
{
    SDL_zero(*f);
    Put32(f->dra + 0x98, 0x800a0200);
    Put16(f->dra + 0x204, 12);
    Put16(f->dra + 0x20c, 3);
    const size_t inits[] = {0x6cc, 0x6d8, 0x6e4};
    for (Uint16 i = 0; i < SDL_arraysize(inits); ++i)
    {
        Put16(f->stage + inits[i], 0x8002);
        Put16(f->stage + inits[i] + 2, 3);
        Put16(f->stage + inits[i] + 6, 2);
    }
    const size_t streams[] = {0xd38, 0xd68, 0xd74, 0xd7c};
    for (Uint16 i = 0; i < SDL_arraysize(streams); ++i)
    {
        for (Uint16 j = 0; j < 4; ++j)
        {
            f->stage[streams[i] + j * 2U] = 2;
            f->stage[streams[i] + j * 2U + 1] = (Uint8)(3 + j);
        }
        f->stage[streams[i] + 8] = 0;
    }
    // Authored triangle-wave and linear SDK fixtures, rather than copied game tables.
    for (Uint16 i = 0; i < 256; ++i)
        Put16(f->stage + 0x187c + i * 2U, i < 64    ? i * 64
                                          : i < 192 ? 8192 - i * 64
                                                    : i * 64 - 16384);
    for (Uint16 i = 0; i <= 1024; ++i)
    {
        Put16(f->exe + 0x1dbcc + i * 2U, i * 4);
        Put16(f->exe + 0x22a10 + i * 2U, i / 2);
    }
    for (Uint16 i = 0; i < 192; ++i)
        Put16(f->exe + 0x1e3e0 + i * 2U, 4096);
    Put32(f->stage + 28, 0x80180040);
    for (Uint16 i = 0; i < 106; ++i)
        Put32(f->stage + 0x40 + i * 4U, 0x80180300);
    Put16(f->stage + 0x300, 0xfffe);
    Put16(f->stage + 0x302, 0xfffe);
    Put16(f->stage + 0x30a, 0xffff);
    Put16(f->stage + 0x30c, 0xffff);
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 30, out));
    CHECK(SotnActorLayout_OpenStage((SotnDataView){f->stage, sizeof(f->stage)}, 30, 0, &f->layout));
    CHECK(SotnActors_Enter(
        *out, f->layout,
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 32}, 0,
        0));
    (*out)->executable = (SotnDataView){f->exe, sizeof(f->exe)};
    (*out)->slots[32] = (Actor){
        .slot = 32,
        .state = {
            .id = 0x2b, .generation = 3, .x = 128 * 65536, .y = 128 * 65536, .priority = 136}};
    (*out)->player = (SotnPlayerState){.x = 160 * 65536, .y = 128 * 65536};
    return true;
}
static void Close(Fixture *f, SotnActors *s)
{
    SotnActors_Destroy(s);
    SotnActorLayout_Close(f->layout);
    f->layout = NULL;
}
static bool Orb(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *root = &s->slots[32], *a = NULL;
    CHECK(OlroxReserved(s, root, 1, 0x2d, 0, &a));
    a->state.y -= 32 * 65536;
    CHECK(ActorOlroxOrbs(s, a) && a->state.step == 1 && !a->state.hitbox_state &&
          a->olrox.timer == 14 && a->state.scale_set && a->state.palette == 0x219);
    CHECK(ActorOlroxOrbs(s, a) && a->state.step == 2 && a->state.hitbox_state == 3 &&
          !a->state.scale_set && a->state.transform_flags_set && !a->state.transform_flags &&
          a->state.palette == 0x209);
    for (Uint16 i = 0; i < 20; ++i)
        CHECK(ActorOlroxOrbs(s, a));
    CHECK(a->olrox.timer == 14 && a->state.step == 2);
    root->olrox.release_orbs = 1;
    for (Uint16 i = 0; i < 14; ++i)
        CHECK(ActorOlroxOrbs(s, a));
    CHECK(a->state.step == 3 && a->olrox.timer == 0 && (a->vx != 0 || a->vy != 0));
    a->state.x = 513 * 65536;
    a->vx = a->vy = 0;
    CHECK(ActorOlroxOrbs(s, a) && a->remove);
    a->remove = false;
    a->state.flags |= 0x100;
    a->state.x = a->state.y = 200 * 65536;
    CHECK(ActorOlroxOrbs(s, a) && a->remove && s->slots[160].state.id == 2 &&
          s->slots[160].state.parameters == 2 && s->combat.sounds[0] == 0x64e &&
          s->combat.pans[0] == -3);
    Close(f, s);
    return true;
}
static bool PortalAndBats(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *root = &s->slots[32], *portal = NULL;
    CHECK(OlroxReserved(s, root, 10, 0x2e, 0, &portal));
    CHECK(ActorOlroxOrbs(s, portal) && portal->state.step == 1 && portal->state.scale_set &&
          !portal->state.scale_x && !portal->state.hitbox_state);
    for (Uint16 i = 0; i < 32; ++i)
        CHECK(ActorOlroxOrbs(s, portal));
    CHECK(portal->state.step == 1 && portal->state.scale_x == 256);
    CHECK(ActorOlroxOrbs(s, portal) && portal->state.step == 2);
    s->ticks = 60;
    CHECK(ActorOlroxOrbs(s, portal) && portal->state.palette == 0x8215 &&
          s->slots[96].state.id == 0x2f && s->combat.sounds[0] == 0x691);
    Actor *bat = &s->slots[96];
    CHECK(ActorOlroxOrbs(s, bat) && bat->state.step == 1 && bat->state.opacity_set &&
          !bat->state.hitbox_state && s->slots[160].state.id == 0x2f &&
          s->slots[160].state.parameters == 1);
    Actor *shadow = &s->slots[160];
    CHECK(ActorOlroxOrbs(s, shadow) && shadow->state.step == 2 && !shadow->state.hitbox_state &&
          shadow->state.blend_mode == 0x30);
    for (Uint16 i = 0; i < 32; ++i)
    {
        CHECK(ActorOlroxOrbs(s, bat) && !bat->state.hitbox_state);
        CHECK(ActorOlroxOrbs(s, shadow) && Pixel(shadow->state.x) == Pixel(bat->state.x) &&
              Pixel(shadow->state.y) == Pixel(bat->state.y) &&
              shadow->state.opacity == bat->state.opacity);
    }
    CHECK(bat->state.opacity == 128 && ActorOlroxOrbs(s, bat) && bat->state.hitbox_state == 3);
    root->olrox.close_portal = 1;
    s->ticks = 61;
    CHECK(ActorOlroxOrbs(s, portal) && portal->state.step == 3 && portal->state.palette == 0x8213);
    for (Uint16 i = 0; i < 32; ++i)
        CHECK(ActorOlroxOrbs(s, portal) && !portal->remove);
    CHECK(!portal->state.scale_x && ActorOlroxOrbs(s, portal) && portal->remove);
    ++bat->state.generation;
    CHECK(ActorOlroxOrbs(s, shadow) && shadow->remove);
    s->olrox_flags = 1;
    CHECK(ActorOlroxOrbs(s, bat) && bat->remove && s->slots[161].state.id == 0x47 &&
          s->slots[161].state.parameters == 2);
    Close(f, s);
    return true;
}
static bool OwnershipAndFailure(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *root = &s->slots[32], *a = NULL;
    CHECK(OlroxReserved(s, root, 1, 0x2d, 9, &a));
    CHECK(!ActorOlroxOrbs(s, a));
    a->state.parameters = 0;
    CHECK(ActorOlroxOrbs(s, a));
    a->state.step = 4;
    CHECK(!ActorOlroxOrbs(s, a));
    a->state.step = 2;
    Put16(f->stage + 0x187c, 4097);
    a->olrox.angle = 0;
    // Truncated SDK input fails before advancing a direction through corrupted data.
    s->executable = (SotnDataView){0};
    CHECK(!ActorOlroxOrbs(s, a));
    s->executable = (SotnDataView){f->exe, sizeof(f->exe)};
    for (Uint16 i = 0; i < 256; ++i)
        Put16(f->stage + 0x187c + i * 2U, 4097);
    CHECK(!ActorOlroxOrbs(s, a) && SDL_strstr(SDL_GetError(), "amplitude"));
    ++root->state.generation;
    const Uint32 random = s->random;
    CHECK(ActorOlroxOrbs(s, a) && a->remove && s->random == random &&
          !s->slots[160].state.generation);
    CHECK(!ActorOlroxOrbs(NULL, a) && !ActorOlroxOrbs(s, NULL));
    Close(f, s);
    CHECK(Open(f, &s));
    root = &s->slots[32];
    CHECK(OlroxReserved(s, root, 10, 0x2e, 0, &a) && ActorOlroxOrbs(s, a));
    a->state.step = 2;
    a->state.scale_x = a->state.scale_y = 256;
    for (Uint16 i = 96; i < 104; ++i)
        s->slots[i] = (Actor){.state = {.generation = 5, .id = 3}};
    s->ticks = 12;
    CHECK(ActorOlroxOrbs(s, a) && !s->combat.sound_count && a->state.step == 2);
    root->state.flags |= 0x100;
    CHECK(ActorOlroxOrbs(s, a) && a->state.step == 3 && a->state.scale_x == 248);
    Close(f, s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok = Orb(f) && PortalAndBats(f) && OwnershipAndFailure(f);
    SDL_free(f);
    if (!ok)
        return 1;
    SDL_Log("OLROX_ORBS_PORTALS: PASS");
    return 0;
}
