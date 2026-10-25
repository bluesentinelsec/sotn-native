#include "actors_internal.h"
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
    Uint8 dra[0x1000], stage[0x2400], exe[0x23220], tiles[32 * 32 * 2], types[2];
} Fixture;
static void Put16(Uint8 *p, Sint32 v)
{
    p[0] = (Uint8)(Uint16)v;
    p[1] = (Uint8)((Uint16)v >> 8);
}
static void Put32(Uint8 *p, Uint32 v)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (i * 8U));
}
static void Animation(Fixture *f, size_t at, Uint8 frame, bool loop)
{
    f->stage[at] = 1;
    f->stage[at + 1] = frame;
    f->stage[at + 2] = (Uint8)(loop ? 0 : 255);
}
static bool Open(Fixture *f, SotnActors **out)
{
    SDL_zero(*f);
    Put32(f->dra + 0x98, 0x800a0200);
    Put16(f->dra + 0x204, 20);
    Put16(f->dra + 0x20c, 3);
    f->dra[0x222] = 8;
    f->dra[0x223] = 16;
    const size_t profiles[] = {0xa64, 0xa70, 0xa7c, 0xa88};
    for (Uint16 i = 0; i < SDL_arraysize(profiles); ++i)
        Put16(f->stage + profiles[i], 0x800a);
    Animation(f, 0x1f58, 1, true);
    Animation(f, 0x1f68, 4, true);
    Animation(f, 0x1f74, 6, true);
    Animation(f, 0x1f80, 7, false);
    Animation(f, 0x1f88, 11, true);
    Animation(f, 0x1f90, 13, true);
    Animation(f, 0x1f9c, 16, true);
    for (Uint16 i = 0; i < 17; ++i)
    {
        Put16(f->stage + 0x1fa8 + i * 8U, (i % 3) - 1);
        Put16(f->stage + 0x1faa + i * 8U, (i % 5) - 2);
        Put16(f->stage + 0x1fac + i * 8U, 4 + i % 7);
        Put16(f->stage + 0x1fae + i * 8U, 5 + i % 5);
    }
    Put16(f->stage + 0x2032, 24);
    Put16(f->stage + 0x2036, 4);
    Put16(f->stage + 0x2038, 8);
    Put16(f->stage + 0x203a, -4);
    Put16(f->stage + 0x203c, -16);
    Animation(f, 0x2064, 1, false);
    Animation(f, 0x2078, 9, true);
    Animation(f, 0x209c, 30, false);
    Animation(f, 0x20d0, 13, false);
    Animation(f, 0x20d4, 41, false);
    Animation(f, 0x20e8, 10, false);
    Animation(f, 0x20fc, 42, true);
    // Deliberately non-sinusoidal samples catch accidental host-math substitution.
    for (Uint32 i = 0; i < 256; ++i)
        Put16(f->stage + 0x1474 + i * 2, (Sint32)i * 16 - 2048);
    for (Uint32 i = 0; i <= 1024; ++i)
        Put16(f->exe + 0x22a10 + i * 2, (Sint32)SDL_round(SDL_atan(i / 1024.0) * 2048 / SDL_PI_D));
    for (Uint32 i = 32 * 12; i < 32 * 32; ++i)
        f->tiles[i * 2] = 1;
    f->types[1] = 3;
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 6, out));
    CHECK(SotnActors_SetServices(*out, (SotnDataView){f->exe, sizeof(f->exe)}, 0, false));
    (*out)->map =
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 32};
    (*out)->player = (SotnPlayerState){.x = 192 * 65536, .y = 128 * 65536};
    return true;
}
static bool Crow(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor a = {.state = {.id = 0x38, .generation = 1, .x = 128 * 65536, .y = 96 * 65536}};
    CHECK(ActorChapel(s, &a) && a.state.step == 1 && a.state.frame == 1 &&
          a.state.priority == 176 && a.crow.reference_y == 96 * 65536 && a.state.hitbox_width == 5);
    CHECK(ActorChapel(s, &a) && a.state.step == 3);
    CHECK(ActorChapel(s, &a) && a.substep == 1 && a.vx == 98304 && a.vy == 262144 &&
          s->combat.sounds[0] == 0x6d7);
    Uint16 ticks = 0;
    for (; ticks < 300 && a.state.step != 2; ++ticks)
        CHECK(ActorChapel(s, &a));
    CHECK(ticks > 64 && ticks < 300 && a.crow.hover_timer == 128 && !a.vx);
    for (Uint16 i = 0; i < 128; ++i)
        CHECK(ActorChapel(s, &a));
    CHECK(a.state.step == 2 && !a.crow.hover_timer);
    CHECK(ActorChapel(s, &a) && a.state.step == 3);
    a.state.flags |= 0x100U;
    CHECK(ActorChapel(s, &a) && a.state.step == 4 && a.state.rotated && a.state.rotation == 64 &&
          a.vy == 6144 && !a.state.hitbox_state);
    CHECK(ActorChapel(s, &a) && a.remove && s->slots[160].state.id == 2 &&
          s->combat.sounds[1] == 0x6d6);
    SotnActors_Destroy(s);
    return true;
}
static bool Raven(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor a = {.state = {.id = 0x39, .generation = 2, .x = 128 * 65536, .y = 96 * 65536}};
    CHECK(ActorChapel(s, &a) && a.state.frame == 11 && a.state.priority == 176);
    CHECK(ActorChapel(s, &a) && a.state.step == 2);
    CHECK(ActorChapel(s, &a) && a.state.step == 3 && a.vy == 65536);
    for (Uint16 i = 0; i < 40 && a.state.step == 3; ++i)
        CHECK(ActorChapel(s, &a));
    CHECK(a.state.step == 4 && a.crow.hover_timer == 32);
    for (Uint16 i = 0; i < 32; ++i)
        CHECK(ActorChapel(s, &a));
    CHECK(a.state.step == 5 && a.vx == 131072 && s->combat.sounds[0] == 0x6d7);
    const Sint32 x = a.state.x;
    CHECK(ActorChapel(s, &a) && a.state.x == x + 131072 && a.vx == 135168 && a.vy == 63488);
    SetStep(&a, 3);
    a.state.flags |= 0x100U;
    a.state.y = s->player.y - 20 * 65536;
    a.vy = 65536;
    CHECK(ActorChapel(s, &a) && a.state.step == 3 && !a.remove);
    a.state.y = s->player.y - 7 * 65536;
    CHECK(ActorChapel(s, &a) && a.state.step == 4 && !a.remove);
    CHECK(ActorChapel(s, &a) && a.remove && s->slots[160].state.id == 2);
    a = (Actor){.state = {.id = 0x39, .step = 2, .y = 128 * 65536 + 1}};
    s->player.y = 128 * 65536 + 2;
    CHECK(ActorChapel(s, &a) && a.vy == -65536);
    SotnActors_Destroy(s);
    return true;
}
static bool Skelerang(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *root = &s->slots[16];
    *root = (Actor){
        .slot = 16,
        .state = {
            .id = 0x3a, .generation = 80, .parameters = 1, .x = 128 * 65536, .y = 168 * 65536}};
    s->player = (SotnPlayerState){.x = 240 * 65536, .y = 168 * 65536};
    CHECK(ActorChapel(s, root));
    for (Uint16 i = 17; i <= 19; ++i)
        CHECK(ActorChapel(s, &s->slots[i]));
    CHECK(s->slots[17].state.id == 0x3c && s->slots[18].state.id == 0x3b);
    CHECK(ActorChapel(s, &s->slots[18]) && ActorChapel(s, &s->slots[19]));
    CHECK(ActorChapel(s, root) && root->state.step == 3);
    SetStep(root, 4);
    CHECK(ActorChapel(s, root) && s->slots[18].state.step == 2);
    Actor *boom = &s->slots[18];
    CHECK(ActorChapel(s, boom) &&
          boom->vx == Signed16(f->stage + 0x1474 + (Uint8)boom->table.angle * 2U) * 48);
    root->state.flags |= 0x100U;
    root->state.frame = 1;
    CHECK(ActorChapel(s, root) && root->remove && s->slots[17].remove &&
          s->slots[19].state.step == 4);
    for (Uint16 i = 160; i < 168; ++i)
    {
        Actor *part = &s->slots[i];
        CHECK(part->state.id == 0x3a && part->state.step == 14);
        for (Uint16 t = 0; t < 16; ++t)
            CHECK(ActorChapel(s, part));
        CHECK(part->remove);
    }
    root->state.generation = 81;
    boom->state.step = 2;
    CHECK(ActorChapel(s, boom) && boom->remove);
    root->state.flags = 0;
    root->state.step = 0;
    root->remove = false;
    CHECK(ActorChapel(s, root) && s->slots[18].enemy_state.owner_generation == 81);
    root->state.flags |= 0x100U;
    root->state.frame = 41;
    SetStep(root, 8);
    CHECK(ActorChapel(s, root) && root->remove);
    bool cower_explosion = false;
    for (Uint16 i = 160; i < 192; ++i)
        cower_explosion |= s->slots[i].state.id == 2 && s->slots[i].state.parameters == 2 &&
                           s->slots[i].state.y == root->state.y + 24 * 65536;
    CHECK(cower_explosion);
    const Uint32 saved = s->slots[17].state.generation;
    s->slots[18].state.id = 0x35;
    root->state.flags = 0;
    root->remove = false;
    root->state.step = 0;
    CHECK(!ActorChapel(s, root) && s->slots[17].state.generation == saved);
    SotnActors_Destroy(s);
    return true;
}
static bool Bounds(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor a = {.state = {.id = 0x38, .step = 4, .x = SDL_MAX_SINT32}, .vx = 1};
    CHECK(!ActorChapel(s, &a) && a.state.x == SDL_MAX_SINT32);
    a = (Actor){.state = {.id = 0x38, .step = 1}};
    f->stage[0x1f59] = 17;
    CHECK(!ActorChapel(s, &a));
    CHECK(!ActorChapelCrow(NULL, &a) && !ActorChapelCrow(s, NULL));
    CHECK(!ActorSkelerang(NULL, &a) && !ActorSkelerang(s, &a));
    s->slots[16] = (Actor){.slot = 16, .state = {.id = 0x3a, .generation = 3}};
    for (Uint32 i = 0; i < 256; ++i)
        Put16(f->stage + 0x1474 + i * 2, 5000);
    a = (Actor){.state = {.id = 0x3b, .step = 2},
                .enemy_state = {.owner_slot = 16, .owner_generation = 3}};
    a.table.timer = 48;
    CHECK(!ActorChapel(s, &a));
    SotnActors_Destroy(s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    const bool ok = f && Crow(f) && Raven(f) && Skelerang(f) && Bounds(f);
    SDL_free(f);
    if (!ok)
        return 1;
    SDL_Log("SOTN_CHAPEL_ENEMIES: PASS");
    return 0;
}
