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
    Uint8 dra[0x1000], stage[0x4000], tiles[32 * 16 * 2], types[8], executable[0x24000];
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
static void Animation(Fixture *f, size_t at, Uint8 frame)
{
    f->stage[at] = 1;
    f->stage[at + 1] = frame;
    f->stage[at + 2] = 255;
}
static bool Open(Fixture *f, SotnActors **out)
{
    SDL_memset(f, 0, sizeof(*f));
    Put32(f->dra + 0x98, 0x800a0200);
    Put16(f->dra + 0x204, 20);
    f->dra[0x222] = 8;
    f->dra[0x223] = 16;
    const Uint16 inits[] = {0x820, 0x82c, 0x838, 0x844, 0x850, 0x85c,
                            0x868, 0x874, 0x88c, 0x898, 0x8e0};
    for (Uint32 i = 0; i < SDL_arraysize(inits); ++i)
    {
        Put16(f->stage + inits[i], 0x8007);
        Put16(f->stage + inits[i] + 2, 1);
        Put16(f->stage + inits[i] + 4, 91);
        Put16(f->stage + inits[i] + 6, 0x251);
    }
    for (Uint16 i = 0; i < 10; ++i)
    {
        Put32(f->stage + 0xac8 + i * 4U, 0x80183200U + i * 8U);
        Animation(f, 0x3200 + i * 8U, (Uint8)(3 + i));
        f->stage[0xaf0 + i] = (Uint8)(8 + i);
        f->stage[0xafc + i] = 0;
        Put16(f->stage + 0xb08 + i * 2U, 0x251);
        Put16(f->stage + 0xb1c + i * 2U, 0x8007);
        Put16(f->stage + 0xb30 + i * 2U, 91);
        f->stage[0xb44 + i] = 0x10;
        Put16(f->stage + 0xb50 + i * 2U, -2);
    }
    for (Uint16 i = 0; i < 10; ++i)
        Put16(f->stage + 0xb68 + i * 2U, i % 4);
    for (Uint16 i = 0; i < 6; ++i)
    {
        Uint8 *r = f->stage + 0x1fb0 + i * 20U;
        Put16(r, 6);
        Put16(r + 2, 123 + i);
        Put16(r + 4, 17);
        Put16(r + 6, 5);
        Put32(r + 16, 0x80183300);
    }
    SDL_memcpy(f->stage + 0x998, f->stage + 0x1fb0, 20);
    Animation(f, 0x3300, 7);
    for (Uint16 i = 0; i < 256; ++i)
        Put16(f->stage + 0x142c + i * 2U, (Sint32)i * 16 - 2048);
    for (Uint32 i = 0; i <= 1024; ++i)
        Put16(f->executable + 0x22a10 + i * 2U,
              (Sint32)SDL_round(SDL_atan(i / 1024.0) * 2048 / SDL_PI_D));
    const Uint16 streams[] = {0x1a6c, 0x1a80, 0x1a98, 0x1aa4, 0x1ad8, 0x1adc, 0x1af0, 0x1b04,
                              0x1b10, 0x1b1c, 0x1b3c, 0x1b4c, 0x1b70, 0x1b84, 0x1bcc};
    for (Uint16 i = 0; i < SDL_arraysize(streams); ++i)
        Animation(f, streams[i], (Uint8)(2 + i));
    for (Uint32 i = 32 * 12; i < 32 * 16; ++i)
        f->tiles[i * 2] = 1;
    f->types[1] = 3;
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 4, out));
    CHECK(SotnActors_SetServices(*out, (SotnDataView){f->executable, sizeof(f->executable)}, 0,
                                 false));
    (*out)->map =
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 16};
    (*out)->player = (SotnPlayerState){.x = 200 * 65536, .y = 100 * 65536};
    return true;
}
static bool Metadata(Fixture *f)
{
    const SotnStageProfile *p = SotnStageProfile_Get(4);
    CHECK(p && !p->connected && p->actor_limit == 0x42 && p->sprite_banks == 24 &&
          p->prize_count == 13 && p->prize_bit == 0x20 && p->door_types == 4 &&
          p->camera_count == 7);
    CHECK(SotnStageProfile_SaveRoom(4, 16) && !SotnStageProfile_SaveRoom(4, 17) &&
          !SotnStageProfile_SaveRoom(4, SDL_MAX_UINT16));
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    CHECK(ActorStageOffset(s, 0xbd4) == 0x82c && ActorStageOffset(s, 0x1d9c) == 0x1850 &&
          ActorStageOffset(s, 0x22ec) == 0x1b3c && ActorStageOffset(s, 0x237c) == 0x1bcc);
    CHECK(ActorStageOffset(s, 0x23dc) == s->nz0.size);
    Actor a = {.state = {.id = 0x29, .generation = 1}};
    CHECK(ActorOlroxQuarter(s, &a) && a.state.unsupported && !a.state.step);
    CHECK(!ActorOlroxQuarter(NULL, &a) && !ActorOlroxQuarter(s, NULL));
    SotnActors_Destroy(s);
    return true;
}
static bool Breakables(Fixture *f)
{
    for (Uint16 type = 0; type < 10; ++type)
    {
        SotnActors *s = NULL;
        CHECK(Open(f, &s));
        s->world->relics[SOTN_RELIC_CUBE] = 3;
        Actor a = {.state = {.id = 1,
                             .generation = 1,
                             .parameters = (Uint16)(type * 4096 + 4),
                             .x = 128 * 65536,
                             .y = 128 * 65536}};
        CHECK(ActorOlroxQuarter(s, &a) && a.state.step == 1 && a.state.frame == 3 + type &&
              a.state.animation_set == 0x8007 && a.state.palette == 0x251 &&
              a.state.texture_offset == 91 && a.state.hitbox_height == 8 + type &&
              a.state.hitbox_y == -2);
        a.hit_params = 1;
        CHECK(ActorOlroxQuarter(s, &a));
        Uint16 debris = 0;
        for (Uint16 slot = 0; slot < SOTN_STAGE_ACTOR_SLOTS; ++slot)
            if (s->slots[slot].state.generation && s->slots[slot].state.id == 0x41)
                ++debris;
        CHECK(debris == (type == 2 ? 4 : type == 3 ? 3 : type == 9 ? 1 : 0));
        if (type == 7 || type == 8)
            CHECK(a.remove && a.permanent && s->slots[96].state.id == 0xc &&
                  s->slots[96].state.parameters == 4);
        else
            CHECK(a.state.id == 3 && a.state.parameters == 4 && a.permanent && !a.remove);
        bool sound = false;
        for (Uint16 i = 0; i < s->combat.sound_count; ++i)
            if (s->combat.sounds[i] == (type >= 7                  ? 0x61d
                                        : (type == 2 || type == 3) ? 0x67f
                                                                   : 0x634))
                sound = true;
        CHECK(sound);
        if (type == 2)
        {
            Actor *piece = NULL;
            for (Uint16 slot = 160; slot < 192; ++slot)
                if (s->slots[slot].state.id == 0x41)
                {
                    piece = &s->slots[slot];
                    break;
                }
            CHECK(piece && ActorOlroxQuarter(s, piece) && piece->state.primitive_count == 2);
            ActorPrimitive *p = ActorPrimitiveGet(s, piece, 0);
            CHECK(p && p->state.palette == 0x255 && p->state.page == 22);
        }
        SotnActors_Destroy(s);
    }
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor a = {.state = {.id = 1, .generation = 1, .parameters = 0x700d}};
    CHECK(!ActorOlroxQuarter(s, &a) && !a.state.step);
    a.state.parameters = 0xa000;
    CHECK(!ActorOlroxQuarter(s, &a));
    a.state.parameters = 4;
    s->world->relics[SOTN_RELIC_CUBE] = 0;
    a.state.x = a.state.y = 128 * 65536;
    CHECK(ActorOlroxQuarter(s, &a));
    a.hit_params = 1;
    CHECK(ActorOlroxQuarter(s, &a) && a.remove && a.permanent);
    SotnActors_Destroy(s);
    return true;
}
static bool Decorations(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor a = {.state = {.id = 8, .generation = 1, .parameters = 5}};
    CHECK(ActorOlroxQuarter(s, &a) && a.state.priority == 128 && a.state.frame == 7 &&
          a.state.texture_offset == 17 && a.state.rotated && a.state.rotation == 2048);
    a = (Actor){.state = {.id = 0x11, .generation = 2}};
    CHECK(ActorOlroxQuarter(s, &a) && a.state.priority == 123 && a.state.frame == 7 &&
          !a.state.rotated);
    a = (Actor){.state = {.id = 8, .generation = 3, .parameters = 6}};
    CHECK(!ActorOlroxQuarter(s, &a));
    a.state.id = 0x11;
    a.state.parameters = 1;
    CHECK(!ActorOlroxQuarter(s, &a));
    SotnActors_Destroy(s);
    return true;
}
static bool Enemies(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *body = &s->slots[32];
    *body = (Actor){.state = {.id = 0x16, .generation = 1, .x = 128 * 65536, .y = 100 * 65536},
                    .slot = 32};
    CHECK(ActorOlroxQuarter(s, body) && body->state.step == 1 && s->slots[33].state.id == 0x18 &&
          s->slots[34].state.id == 0x17 && s->slots[35].state.id == 0x17);
    Actor *boom = &s->slots[34];
    CHECK(ActorOlroxQuarter(s, boom) && boom->state.step == 1);
    boom->state.step = 2;
    boom->state.x = 120 * 65536;
    boom->state.y = 90 * 65536;
    boom->table.timer = 10;
    boom->table.angle = 20;
    CHECK(ActorOlroxQuarter(s, boom));
    CHECK(boom->vx % 48 == 0 && boom->vy % 48 == 0);
    CHECK(boom->vx == Signed16(f->stage + 0x142c + (Uint8)boom->table.angle * 2U) * 48 &&
          boom->vy == Signed16(f->stage + 0x142c + (Uint8)(boom->table.angle - 64) * 2U) * 48);
    Actor zombie = {.state = {.id = 0x2b, .generation = 2, .x = 128 * 65536, .y = 100 * 65536}};
    CHECK(ActorOlroxQuarter(s, &zombie) && zombie.state.step == 2 && zombie.state.palette == 0x251);
    zombie.state.step = 3;
    CHECK(ActorOlroxQuarter(s, &zombie) && zombie.state.frame == 16 && zombie.state.step == 3);
    zombie.hit_params = 1;
    CHECK(ActorOlroxQuarter(s, &zombie) && zombie.state.step == 6 && zombie.state.frame == 14);
    SotnActors_Destroy(s);
    return true;
}
static bool Blood(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    for (Uint16 i = 0; i < 2; ++i)
    {
        const size_t at = i ? 0x1bf4 : 0x1bdc;
        f->stage[at] = 1;
        Put16(f->stage + at + 1, 0x1008);
        Put16(f->stage + at + 3, 0x0808);
        f->stage[at + 5] = 255;
    }
    Actor *a = &s->slots[64];
    *a = (Actor){.state = {.id = 0x2d, .generation = 1, .x = 128 * 65536, .y = 128 * 65536},
                 .slot = 64};
    CHECK(ActorOlroxQuarter(s, a) && a->state.step == 1);
    CHECK(ActorOlroxQuarter(s, a) && a->state.step == 2 && a->state.primitive_count == 8);
    CHECK(ActorOlroxQuarter(s, a) && a->state.step == 3);
    CHECK(ActorOlroxQuarter(s, a));
    ActorPrimitive *small = ActorPrimitiveGet(s, a, 0), *big = ActorPrimitiveGet(s, a, 2);
    CHECK(small && big && small->state.active && big->state.active && small->state.u[0] == 8 &&
          small->state.v[0] == 16);
    CHECK(ActorOlroxQuarter(s, a) && !small->state.active && !big->state.active);
    ActorPrimitivesClear(s, a);
    *a = (Actor){.state = {.id = 0x2c, .generation = 2, .x = 128 * 65536, .y = 128 * 65536},
                 .slot = 64};
    Put16(f->stage + 0x1c14, 255);
    CHECK(ActorOlroxQuarter(s, a) && a->state.step == 1 && a->state.primitive_count == 1);
    CHECK(ActorOlroxQuarter(s, a) && a->vy == 6144 && a->state.y == 128 * 65536 + 6144);
    ActorPrimitivesClear(s, a);
    *a = (Actor){.state = {.id = 0x1d, .generation = 3}};
    CHECK(!ActorLibraryBlood(s, a));
    SotnActors_Destroy(s);
    return true;
}
static bool Explosion(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Put32(f->stage + 0x1908, 0x80183400);
    Put32(f->stage + 0x185c, (Uint32)-16384);
    Animation(f, 0x3400, 5);
    Actor a = {.state = {.id = 2, .generation = 1, .x = 100 * 65536, .y = 100 * 65536}};
    CHECK(ActorExplosionTick(s, &a) && a.state.step == 1 && a.state.animation_set == 2);
    CHECK(ActorExplosionTick(s, &a) && a.state.y == 100 * 65536 - 16384 && a.state.frame == 5);
    a.state.y = SDL_MAX_SINT32;
    Put32(f->stage + 0x185c, 1);
    CHECK(!ActorExplosionTick(s, &a) && a.state.y == SDL_MAX_SINT32);
    a.state.y = SDL_MIN_SINT32;
    Put32(f->stage + 0x185c, SDL_MAX_UINT32);
    CHECK(!ActorExplosionTick(s, &a) && a.state.y == SDL_MIN_SINT32);
    SotnActors_Destroy(s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    bool ok =
        Metadata(f) && Breakables(f) && Decorations(f) && Enemies(f) && Explosion(f) && Blood(f);
    SDL_free(f);
    if (!ok)
        return 1;
    SDL_Log("OLROX_QUARTER_ACTORS: PASS");
    return 0;
}
