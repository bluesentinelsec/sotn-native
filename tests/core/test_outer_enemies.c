#include "actors_internal.h"
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
    Uint8 dra[0x400], stage[0x4000], tiles[32 * 16 * 2], types[2];
    SotnActorLayout *layout;
} Fixture;
static void Put16(Uint8 *p, Uint16 v)
{
    p[0] = (Uint8)v;
    p[1] = (Uint8)(v >> 8);
}
static void Put32(Uint8 *p, Uint32 v)
{
    for (Uint32 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (i * 8));
}
static void Animation(Fixture *f, size_t at, Uint16 frames)
{
    for (Uint16 i = 0; i < frames; ++i)
    {
        f->stage[at + i * 2] = 1;
        f->stage[at + i * 2 + 1] = (Uint8)(i + 1);
    }
    f->stage[at + frames * 2] = 255;
}
static bool Open(Fixture *f, SotnActors **s)
{
    SDL_memset(f, 0, sizeof(*f));
    Put32(f->dra + 0x98, 0x800a0200);
    Put16(f->dra + 0x204, 40);
    Put16(f->dra + 0x206, 6);
    Put16(f->dra + 0x20c, 19);
    Put32(f->dra + 0x224, 0x08401000);
    f->dra[0x222] = 8;
    f->dra[0x223] = 16;
    Put32(f->stage + 28, 0x80180040);
    for (Uint32 i = 0; i < 106; ++i)
        Put32(f->stage + 0x40 + i * 4, 0x80180300);
    Put16(f->stage + 0x300, 0xfffe);
    Put16(f->stage + 0x302, 0xfffe);
    Put16(f->stage + 0x30a, 0xffff);
    Put16(f->stage + 0x30c, 0xffff);
    for (Uint32 i = 0; i < 5; ++i)
        Put32(f->stage + 0x2324 + i * 4, 0x80180380);
    Animation(f, 0x380, 1);
    Put16(f->stage + 0x299a, 16);
    Put16(f->stage + 0x29aa, 16);
    Put16(f->stage + 0x29ac, 12);
    Animation(f, 0x29b0, 6);
    Animation(f, 0x29c8, 4);
    Animation(f, 0x29e0, 1);
    Animation(f, 0x2a24, 1);
    Animation(f, 0x2a2c, 1);
    Animation(f, 0x2a38, 1);
    Animation(f, 0x2a40, 4);
    Put16(f->stage + 0x2a4e, 20);
    Put16(f->stage + 0x2a50, 40);
    Put32(f->stage + 0x2a54, 1);
    Put32(f->stage + 0x2a58, 1);
    Put32(f->stage + 0x2a5c, (Uint32)-2);
    f->stage[0x2a60] = 3;
    f->stage[0x2a61] = 1;
    f->stage[0x2a62] = 7;
    f->stage[0x2a64] = 5;
    f->stage[0x2a65] = 1;
    for (Uint32 i = 0; i < 3; ++i)
    {
        Put16(f->stage + 0x2a6c + i * 8, 8);
        Put16(f->stage + 0x2a6e + i * 8, 16);
    }
    Put16(f->stage + 0x2bda, 16);
    Put16(f->stage + 0x2bea, 16);
    const size_t archer_anims[] = {0x2bf8, 0x2c04, 0x2c0c, 0x2c14, 0x2c40, 0x2c64, 0x2ca8};
    for (Uint32 i = 0; i < SDL_arraysize(archer_anims); ++i)
        Animation(f, archer_anims[i], 1);
    for (Uint32 i = 0; i < 4; ++i)
    {
        Put32(f->stage + 0x2cd4 + i * 12, 0x80182c14);
        f->stage[0x2cd8 + i * 12] = 1;
        Put16(f->stage + 0x2cda + i * 12, 6);
        Put16(f->stage + 0x2cdc + i * 12, (Uint16)-4);
    }
    for (Uint32 i = 0; i < 8; ++i)
        f->stage[0x2d44 + i] = (Uint8)(8 + i);
    f->types[1] = 3;
    for (Uint32 y = 12; y < 16; ++y)
        for (Uint32 x = 0; x < 32; ++x)
            Put16(f->tiles + (y * 32 + x) * 2, 1);
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 1, s));
    CHECK(SotnActorLayout_OpenStage((*s)->nz0, 1, 0, &f->layout));
    CHECK(SotnActors_Enter(
        *s, f->layout,
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 16}, 0,
        0));
    return true;
}
static bool Musket(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *root = &s->slots[16];
    *root = (Actor){.state = {.id = 0x46, .generation = 100, .x = 200 * 65536, .y = 176 * 65536},
                    .slot = 16,
                    .dynamic = true};
    const SotnPlayerState player = {
        .x = 128 * 65536, .y = 170 * 65536, .hp = 70, .invulnerable = true};
    CHECK(SotnActors_Tick(s, player, 0, 0));
    CHECK(root->state.step == 2 && s->slots[17].state.parameters == 1 &&
          s->slots[18].state.parameters == 2);
    CHECK(root->musket.neighbor == 17 && s->slots[18].musket.neighbor == 16);
    CHECK(SotnActors_Tick(s, player, 0, 0));
    CHECK(root->state.step == 3 && s->slots[17].state.step == 7 && s->slots[18].state.step == 5);
    Uint32 shots = 0, last_generation = 0;
    for (Uint32 t = 0; t < 1200; ++t)
    {
        CHECK(SotnActors_Tick(s, player, 0, 0));
        const Actor *bullet = &s->slots[96];
        if (bullet->state.id == 0x47 && bullet->state.generation != last_generation)
        {
            last_generation = bullet->state.generation;
            ++shots;
        }
    }
    CHECK(shots > 3);
    for (Uint32 i = 16; i < 19; ++i)
    {
        CHECK(s->slots[i].state.id == 0x46);
        s->slots[i].state.flags |= 0x10c100;
    }
    for (Uint32 t = 0; t < 3; ++t)
        CHECK(SotnActors_Tick(s, player, 0, 0));
    for (Uint32 i = 16; i < 19; ++i)
        CHECK(!s->slots[i].state.generation);
    Actor bullet = {.state = {.id = 0x47, .generation = 300, .x = 180 * 65536, .y = 176 * 65536}};
    CHECK(ActorMusket(s, &bullet) && bullet.state.step == 2 && bullet.state.hitbox_state == 4);
    bullet.hit_params = 1;
    CHECK(ActorMusket(s, &bullet) && bullet.state.step == 3 && !bullet.state.hitbox_state);
    CHECK(ActorMusket(s, &bullet));
    ActorPrimitive *line = ActorPrimitiveGet(s, &bullet, 0);
    CHECK(line != NULL && !line->state.active);
    ActorPrimitivesClear(s, &bullet);
    bullet = (Actor){.state = {.id = 0x47, .generation = 301, .x = 180 * 65536, .y = 176 * 65536}};
    CHECK(ActorMusket(s, &bullet) && ActorMusket(s, &bullet) && bullet.state.hitbox_state == 9);
    ActorPrimitivesClear(s, &bullet);
    Actor invalid = {.state = {.id = 0x46, .parameters = 3, .generation = 302}};
    CHECK(!ActorMusket(s, &invalid));
    invalid = (Actor){.state = {.id = 0x46, .generation = 303, .x = 128 * 65536, .y = 176 * 65536},
                      .slot = 24};
    Put32(f->stage + 0x2a54, SDL_MAX_SINT32);
    CHECK(!ActorMusket(s, &invalid));
    SotnActors_Destroy(s);
    SotnActorLayout_Close(f->layout);
    return true;
}
static bool Archer(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    s->player = (SotnPlayerState){.x = 128 * 65536, .y = 170 * 65536, .hp = 70};
    Actor archer = {.state = {.id = 0x44, .generation = 500, .x = 200 * 65536, .y = 176 * 65536}};
    CHECK(ActorArcher(s, &archer));
    SetStep(&archer, 7);
    archer.substep = 3;
    CHECK(ActorArcher(s, &archer));
    CHECK(s->slots[96].state.id == 0x45 && s->slots[96].state.x == 206 * 65536 &&
          s->slots[96].state.y == 172 * 65536);
    CHECK(!s->slots[96].state.facing_left);
    Actor arrow = {.state = {.id = 0x45, .generation = 501, .x = 128 * 65536, .y = 170 * 65536}};
    CHECK(ActorArcher(s, &arrow) && arrow.state.step == 1 && arrow.vx == -4 * 65536);
    arrow.hit_flags = 0x80;
    CHECK(ActorArcher(s, &arrow) && arrow.state.step == 2 && arrow.timer == 32 &&
          !arrow.state.hitbox_state);
    const Sint32 x = arrow.state.x;
    s->player.x += 10 * 65536;
    CHECK(ActorArcher(s, &arrow) && arrow.state.x == x + 10 * 65536);
    for (Uint32 t = 0; t < 31; ++t)
        CHECK(ActorArcher(s, &arrow));
    CHECK(arrow.remove);
    Actor wrapped = {.state = {.id = 0x45, .step = 2, .generation = 502}, .timer = 2};
    wrapped.archer.x = -1;
    s->player.x = SDL_MAX_SINT32;
    CHECK(ActorArcher(s, &wrapped) && wrapped.state.x == SDL_MIN_SINT32);
    s->player.x = 128 * 65536;

    archer.state.flags |= 0x100;
    CHECK(ActorArcher(s, &archer) && archer.remove);
    for (Uint32 i = 160; i < 168; ++i)
    {
        Actor *part = &s->slots[i];
        CHECK(part->state.id == 0x44 && ActorArcher(s, part) && part->state.step == 12 &&
              !part->state.hitbox_state);
        const Uint16 life = part->timer;
        for (Uint32 t = 0; t < life; ++t)
            CHECK(ActorArcher(s, part));
        CHECK(part->remove);
    }
    CHECK(SotnActors_Enter(
        s, f->layout,
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 16}, 0,
        0));
    // A lower-damage arrow still attaches when another enemy wins contact-damage selection.
    s->slots[0] = (Actor){.state = {.id = 0x3e,
                                    .step = 1,
                                    .generation = 600,
                                    .x = 128 * 65536,
                                    .y = 170 * 65536,
                                    .hitbox_state = 1,
                                    .hitbox_width = 8,
                                    .hitbox_height = 20},
                          .attack = 50,
                          .stun = 1,
                          .slot = 0,
                          .dynamic = true};
    s->slots[1] = (Actor){.state = {.id = 0x45,
                                    .step = 1,
                                    .generation = 601,
                                    .x = 128 * 65536,
                                    .y = 170 * 65536,
                                    .hitbox_state = 1,
                                    .hitbox_width = 8,
                                    .hitbox_height = 1},
                          .attack = 6,
                          .slot = 1,
                          .dynamic = true};
    CHECK(SotnActors_Tick(s,
                          (SotnPlayerState){.x = 128 * 65536,
                                            .y = 170 * 65536,
                                            .hp = 70,
                                            .animation = {.hitbox_width = 8, .hitbox_height = 20}},
                          0, 0));
    CHECK(s->combat.contact_damage == 50 && s->slots[1].state.step == 2 && s->slots[1].timer == 32);
    Actor invalid = {.state = {.id = 0x45, .parameters = 3, .generation = 602}};
    CHECK(!ActorArcher(s, &invalid));
    invalid = (Actor){.state = {.id = 0x44, .parameters = 9, .generation = 603}};
    CHECK(!ActorArcher(s, &invalid));
    SotnActors_Destroy(s);
    SotnActorLayout_Close(f->layout);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok = Musket(f) && Archer(f);
    SDL_free(f);
    return ok ? 0 : 1;
}
