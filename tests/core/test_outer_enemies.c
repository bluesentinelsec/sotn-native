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
    Uint8 dra[0x1400], stage[0x4000], exe[0x23220], tiles[32 * 16 * 2], types[2];
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
    SDL_memcpy(f->dra + 0x228, f->dra + 0x200, 40);
    Put16(f->dra + 0x22c, 0x7fff);
    Put16(f->stage + 0x968 + 8, 1);
    Put16(f->dra + 0x200 + 94 * 40 + 6, 18);
    Put16(f->dra + 0x200 + 94 * 40 + 8, 32);
    Put16(f->dra + 0x200 + 95 * 40 + 6, 24);
    Put16(f->dra + 0x200 + 95 * 40 + 8, 32);
    Put16(f->dra + 0x200 + 2 * 40 + 6, 9);
    Put32(f->dra + 0x200 + 2 * 40 + 36, 0x88000000);
    Put16(f->stage + 0xb0c + 8, 2);
    const size_t spear_move[] = {0x2f8c, 0x2f98, 0x2fa0, 0x2fa8};
    for (Uint32 i = 0; i < 4; ++i)
    {
        Animation(f, spear_move[i], 1);
        Put32(f->stage + 0x3054 + i * 4, 0x80180000 + (Uint32)spear_move[i]);
        Put16(f->stage + 0x2f74 + i * 2, 2);
        Put32(f->stage + 0x2f7c + i * 4, (Uint32)-2048);
    }
    const size_t spear_attacks[] = {0x2fb0, 0x2fbc, 0x2fdc, 0x2ff4, 0x3008, 0x3018, 0x3034};
    for (Uint32 i = 0; i < SDL_arraysize(spear_attacks); ++i)
        Animation(f, spear_attacks[i], 1);
    Animation(f, 0x2ff4, 7);
    Put16(f->stage + 0x306e, 24);
    Put16(f->stage + 0x3066, 24);
    Put16(f->stage + 0x3068, 8);
    for (Uint32 i = 0; i < 8; ++i)
        f->stage[0x2f04 + i] = 6;
    for (Uint32 i = 0; i < 4; ++i)
    {
        f->stage[0x2f0c + i] = 32;
        f->stage[0x2f10 + i] = 32;
    }
    for (Uint32 i = 0; i < 16; ++i)
        f->stage[0x2f14 + i] = (Uint8)(63 - i * 4);
    for (Uint32 i = 0; i < 43; ++i)
    {
        Put16(f->stage + 0x307c + i * 4, 4);
        Put16(f->stage + 0x3128 + i * 4, 8);
        Put16(f->stage + 0x312a + i * 4, 8);
    }
    for (Uint32 i = 0; i <= 1024; ++i)
        Put16(f->exe + 0x22a10 + i * 2, (Uint16)(i / 2));
    Put16(f->stage + 0x2a8a, 30);
    Put16(f->stage + 0x2a82, 30);
    Put16(f->stage + 0x2a84, 24);
    const size_t sword_anims[] = {0x2a9c, 0x2aac, 0x2ad4, 0x2b00, 0x2b10, 0x2b1c};
    for (Uint32 i = 0; i < SDL_arraysize(sword_anims); ++i)
        Animation(f, sword_anims[i], 1);
    f->stage[0x2b01] = 25;
    f->stage[0x2b11] = 29;
    for (Uint32 i = 0; i < 4; ++i)
        f->stage[0x2a98 + i] = 3;
    for (Uint32 i = 0; i < 6; ++i)
        Put32(f->stage + 0x2b34 + i * 4, 4096);
    for (Uint32 i = 0; i < 21; ++i)
    {
        f->stage[0x2b64 + i * 4] = 12;
        f->stage[0x2b65 + i * 4] = (Uint8)-4;
        f->stage[0x2b66 + i * 4] = 8;
        f->stage[0x2b67 + i * 4] = 8;
    }
    for (Uint32 i = 0; i < 30; ++i)
        f->stage[0x2bb8 + i] = 1;
    for (Uint32 i = 0; i < 6; ++i)
    {
        Put16(f->stage + 0x239c + i * 2, (Uint16)(32 + i * 16));
        Put16(f->stage + 0x23ac + i * 2, (Uint16)(48 + i * 16));
        Put32(f->stage + 0x23bc + i * 4, 2048);
        Put32(f->stage + 0x23d4 + i * 4, 2048);
    }
    for (Uint32 i = 0; i < 4; ++i)
    {
        f->stage[0x23ec + i] = 1;
        Put16(f->stage + 0x23f0 + i * 2, 8);
    }
    Put16(f->stage + 0x245a, 32);
    Put16(f->stage + 0x2452, 32);
    Put16(f->stage + 0x2454, 8);
    const size_t blue_anims[] = {0x2488, 0x2498, 0x24b0, 0x24c4, 0x24d0, 0x24e0,
                                 0x24f0, 0x2504, 0x2514, 0x252c, 0x2538, 0x2400};
    for (Uint32 i = 0; i < SDL_arraysize(blue_anims); ++i)
        Animation(f, blue_anims[i], 1);
    Animation(f, 0x2498, 8);
    Animation(f, 0x24b0, 7);
    Put16(f->stage + 0x25f4, 2);
    Put32(f->stage + 0x25f4, 2 * 65536);
    Put32(f->stage + 0x25f8, 2 * 65536);
    Put32(f->stage + 0x25fc, 65536);
    Put32(f->stage + 0x2608, (Uint32)(-4 * 65536));
    for (Uint32 i = 0; i < 48; ++i)
        f->stage[0x2554 + i] = 1;
    f->stage[0x254a] = 8;
    f->stage[0x254b] = 26;
    for (Uint32 i = 0; i < 56; ++i)
        f->stage[0x25bc + i] = 1;
    f->stage[0x2588] = 12;
    f->stage[0x2589] = (Uint8)-4;
    f->stage[0x258a] = f->stage[0x258b] = 8;
    Put16(f->stage + 0xae8 + 8, 1);
    SDL_memcpy(f->dra + 0x278, f->dra + 0x228, 40);
    Put16(f->dra + 0x27e, 30);
    Put16(f->dra + 0x280, 0x8001);
    Put16(f->stage + 0xaf4 + 8, 3);
    Put16(f->stage + 0x2d5a, 40);
    Put16(f->stage + 0x2d62, (Uint16)-4);
    Put16(f->stage + 0x2d6a, 40);
    Put16(f->stage + 0x2d6c, 8);
    for (Uint32 i = 0; i < 8; ++i)
        f->stage[0x2d50 + i] = 5;
    for (Uint32 i = 0; i < 4; ++i)
        Put16(f->stage + 0x2d70 + i * 2, 4);
    const size_t armor_anims[] = {0x2d78, 0x2d84, 0x2d9c, 0x2da4, 0x2dac, 0x2dc4, 0x2de0, 0x2dec};
    for (Uint32 i = 0; i < SDL_arraysize(armor_anims); ++i)
        Animation(f, armor_anims[i], 1);
    Animation(f, 0x2dc4, 8);
    f->stage[0x2dc5] = 16;
    for (Uint32 i = 1; i < 27; ++i)
    {
        Put16(f->stage + 0x2e2c + i * 4, 8);
        Put16(f->stage + 0x2e2e + i * 4, 12);
        Put16(f->stage + 0x2e98 + i * 4, 32);
        Put16(f->stage + 0x2e9a + i * 4, (Uint16)-8);
    }
    f->types[1] = 3;
    for (Uint32 y = 12; y < 16; ++y)
        for (Uint32 x = 0; x < 32; ++x)
            Put16(f->tiles + (y * 32 + x) * 2, 1);
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 1, s));
    CHECK(SotnActors_SetServices(*s, (SotnDataView){f->exe, sizeof(f->exe)}, 0, true));
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
static bool Spear(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *root = &s->slots[24];
    *root = (Actor){.state = {.id = 0x4f, .generation = 700, .x = 200 * 65536, .y = 168 * 65536},
                    .slot = 24,
                    .dynamic = true};
    const SotnPlayerState player = {
        .x = 320 * 65536, .y = 168 * 65536, .hp = 70, .invulnerable = true};
    CHECK(SotnActors_Tick(s, player, 0, 0));
    Actor *block = &s->slots[25];
    CHECK(block->state.id == 0x50 && block->state.hit_points == 0x7fff);
    CHECK(SotnActors_Tick(s, player, 0, 0) && root->state.step == 2);
    CHECK(SotnActors_Tick(s, player, 0, 0) && root->state.step == 4);
    root->spear.timer = 0;
    CHECK(SotnActors_Tick(s, player, 0, 0) && root->spear.timer == 255);
    root->spear.timer = 7;
    SotnPlayerState casting = player;
    casting.subweapon_starting = true;
    CHECK(SotnActors_Tick(s, casting, 0, 0) && root->spear.timer == 7);
    SetStep(root, 9);
    CHECK(SotnActors_Tick(s, player, 0, 0) && (block->state.hitbox_state & 2) &&
          (block->state.flags & 0xc000) == 0xc000);
    block->hit_flags = 2;
    const Uint16 sounds = s->combat.sound_count;
    CHECK(ActorSpearGuard(s, block) && s->combat.sound_count == sounds + 1);
    Animation(f, 0x2fb0, 1);
    f->stage[0x2fb1] = 7;
    SetStep(root, 6);
    root->state.facing_left = true;
    CHECK(SotnActors_Tick(s, player, 0, 0) && block->attack == 18 && root->vx == 245760);
    SetStep(root, 11);
    root->vx = 0;
    bool thrown = false;
    for (Uint32 i = 0; i < 8; ++i)
    {
        CHECK(SotnActors_Tick(s, player, 0, 0));
        thrown = thrown || s->slots[96].state.id == 0x58;
    }
    CHECK(thrown && s->slots[96].state.step == 2);
    CHECK(SotnActors_Tick(s, player, 0, 0) && s->slots[96].state.step == 3 &&
          s->slots[96].vx == 6 * 65536);
    CHECK(SotnActors_Tick(s, player, 0, 0) && s->slots[96].vy == 1024);
    root->state.flags |= 0x10c100;
    CHECK(SotnActors_Tick(s, player, 0, 0) && root->state.step == 12 && !block->state.generation);
    CHECK(root->state.opacity == 125 && root->state.opacity_set);
    for (Uint32 i = 0; i < 63; ++i)
        CHECK(SotnActors_Tick(s, player, 0, 0));
    CHECK(!root->state.generation);
    Actor orphan = {.state = {.id = 0x50, .generation = 701}};
    CHECK(ActorSpearGuard(s, &orphan) && orphan.remove);
    Actor invalid = {.state = {.id = 0x4f, .generation = 702}, .slot = 191};
    CHECK(!ActorSpearGuard(s, &invalid));
    SotnActors_Destroy(s);
    SotnActorLayout_Close(f->layout);
    return true;
}
static bool Sword(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *root = &s->slots[32];
    *root = (Actor){.state = {.id = 0x48, .generation = 800, .x = 200 * 65536, .y = 162 * 65536},
                    .slot = 32,
                    .dynamic = true};
    const SotnPlayerState player = {
        .x = 128 * 65536, .y = 168 * 65536, .hp = 70, .invulnerable = true};
    CHECK(SotnActors_Tick(s, player, 0, 0));
    Actor *weapon = &s->slots[33];
    CHECK(weapon->state.id == 0x49 && weapon->state.hitbox_x == 12 && weapon->state.hitbox_y == -4);
    CHECK(SotnActors_Tick(s, player, 0, 0) && root->state.step == 2);
    CHECK(SotnActors_Tick(s, player, 0, 0) && root->state.step == 3);
    SetStep(root, 4);
    f->stage[0x2aad] = 9;
    CHECK(SotnActors_Tick(s, player, 0, 0) && root->vx == -4 * 65536);
    SetStep(root, 4);
    f->stage[0x2aad] = 13;
    root->sword.smashed = false;
    CHECK(SotnActors_Tick(s, player, 0, 0) && root->sword.smashed && s->slots[96].state.id == 0x14);
    root->stun = 20;
    for (Uint32 t = 0; t < 12; ++t)
        CHECK(SotnActors_Tick(s, player, 0, 0));
    bool dust = false;
    for (Uint32 i = 96; i < 128; ++i)
        dust = dust || (s->slots[i].state.id == 0x14 && s->slots[i].state.generation);
    CHECK(!dust);
    root->state.flags |= 0x10c100;
    bool pulse = false, parts = false, first_fade = false;
    for (Uint32 t = 0; t < 500; ++t)
    {
        CHECK(SotnActors_Tick(s, player, 0, 0));
        if (root->state.primitive_count)
        {
            ActorPrimitive *effect = ActorPrimitiveGet(s, root, 0);
            CHECK(effect != NULL);
            pulse = pulse || effect->state.active;
        }
        for (Uint32 i = 160; i < 192; ++i)
        {
            const Actor *part = &s->slots[i];
            parts = parts || (part->state.id == 0x48 && part->state.parameters > 0);
            first_fade = first_fade || (part->state.id == 0x48 && part->state.step == 9 &&
                                        part->state.opacity_set && part->state.opacity == 160);
        }
    }
    CHECK(pulse && parts && first_fade && !root->state.generation && !weapon->state.generation);
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        CHECK(!s->primitives[i].owner);
    Actor invalid = {.state = {.id = 0x48, .step = 7, .parameters = 6, .generation = 801}};
    CHECK(!ActorSwordLord(s, &invalid));
    Actor origin = {.state = {.generation = 802, .x = 128 * 65536, .y = 128 * 65536}};
    CHECK(!ActorOuterDust(s, &origin, 7, 1, 0, 0, 0, 1));
    Actor intense = {.state = {.id = 6, .generation = 803}};
    CHECK(ActorOuterEffect(s, &intense) && intense.state.animation_set == 5);
    for (Uint32 t = 0; t < 37; ++t)
        CHECK(ActorOuterEffect(s, &intense));
    CHECK(intense.remove);
    SotnActors_Destroy(s);
    SotnActorLayout_Close(f->layout);
    return true;
}
static bool Blue(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *root = &s->slots[40];
    *root = (Actor){.state = {.id = 0x20, .generation = 900, .x = 200 * 65536, .y = 160 * 65536},
                    .slot = 40,
                    .dynamic = true};
    SotnPlayerState player = {.x = 320 * 65536, .y = 160 * 65536, .hp = 70, .invulnerable = true};
    CHECK(SotnActors_Tick(s, player, 0, 0));
    Actor *weapon = &s->slots[41];
    CHECK(root->blue.armored && root->blue.delay == 512 && weapon->state.id == 0x22 &&
          weapon->state.hitbox_x == 12 && weapon->state.hitbox_y == -4 &&
          (weapon->state.flags & 0x01000000));
    CHECK(SotnActors_Tick(s, player, 0, 0) && root->state.step == 2);
    root->blue.delay = 2;
    CHECK(SotnActors_Tick(s, player, 0, 0) && root->blue.delay == -1);
    player.y = 64 * 65536;
    CHECK(SotnActors_Tick(s, player, 0, 0) && root->state.step == 7);
    bool thrown = false;
    for (Uint32 t = 0; t < 10; ++t)
    {
        CHECK(SotnActors_Tick(s, player, 0, 0));
        thrown = thrown || (s->slots[96].state.id == 0x21 && s->slots[96].state.parameters == 2);
    }
    CHECK(thrown && s->slots[96].state.step == 2);
    Actor axe = {.state = {.id = 0x21, .generation = 901, .facing_left = true}};
    CHECK(ActorBlueKnight(s, &axe) && axe.vx == 2 * 65536 && axe.blue.delay == -64);
    for (Uint32 t = 0; t < 96; ++t)
        CHECK(ActorBlueKnight(s, &axe));
    CHECK(axe.vx == -2 * 65536 && axe.state.rotation == 0);
    axe.blue.delay = SDL_MAX_SINT16;
    CHECK(ActorBlueKnight(s, &axe) && axe.blue.delay == SDL_MIN_SINT16);
    axe.state.flags |= 0x100;
    CHECK(ActorBlueKnight(s, &axe) && axe.remove);
    SetStep(root, 6);
    player.x = 400 * 65536;
    player.y = 160 * 65536;
    for (Uint32 t = 0; t < 35; ++t)
        CHECK(SotnActors_Tick(s, player, 0, 0));
    CHECK(root->substep == 2 && root->vx == 163840);
    SetStep(root, 2);
    root->state.hit_points = 7;
    CHECK(SotnActors_Tick(s, player, 0, 0) && !root->blue.armored);
    Actor first_death = {.state = {.id = 0x20, .generation = 905, .step = 8}, .timer = 65};
    CHECK(ActorBlueKnight(s, &first_death) && first_death.timer == 64);
    // Exercise the complete alternate death, including hidden companion allocation.
    SetStep(root, 8);
    root->state.hitbox_state = 0;
    root->substep = 16;
    root->state.flags |= 0x10c100;
    bool pulse = false;
    for (Uint32 t = 0; t < 100; ++t)
    {
        CHECK(SotnActors_Tick(s, player, 0, 0));
        pulse = pulse || root->state.primitive_count == 2;
    }
    CHECK(pulse && !root->state.generation && !weapon->state.generation);
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        CHECK(!s->primitives[i].owner);
    Actor invalid = {.state = {.id = 0x21, .parameters = 3, .generation = 902}};
    CHECK(!ActorBlueKnight(s, &invalid));
    invalid = (Actor){.state = {.id = 0x20, .generation = 903}, .slot = 191};
    CHECK(!ActorBlueKnight(s, &invalid));
    // Original shatter allocation failure finishes death rather than stopping gameplay.
    Actor dying = {.state = {.id = 0x20, .generation = 904}, .substep = 17};
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        s->primitives[i].owner = 999;
    bool done = false;
    CHECK(ActorBlueDeath(s, &dying, &done) && done && !dying.state.primitive_count);
    SotnActors_Destroy(s);
    SotnActorLayout_Close(f->layout);
    return true;
}
static bool Armor(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *root = &s->slots[16];
    *root = (Actor){.state = {.id = 0x4a, .generation = 100, .x = 140 * 65536, .y = 150 * 65536},
                    .slot = 16,
                    .dynamic = true};
    const SotnPlayerState player = {
        .x = 210 * 65536, .y = 170 * 65536, .hp = 70, .invulnerable = true};
    CHECK(SotnActors_Tick(s, player, 0, 0));
    Actor *weapon = &s->slots[17];
    CHECK(root->state.step == 1 && weapon->state.id == 0x4b &&
          weapon->enemy_state.owner_generation == 100);
    root->state.x = 160 * 65536 + 123;
    CHECK(SotnActors_Tick(s, player, 0, 0) && root->state.x == 144 * 65536 + 123);
    root->state.step = 3;
    root->substep = 0;
    for (Uint32 i = 0; i < 4; ++i)
        CHECK(SotnActors_Tick(s, player, 0, 0));
    CHECK(root->state.step == 5);
    // Armor Lord observes weapon startup, independently of subweapon startup.
    root->state.step = 3;
    root->substep = 1;
    root->timer = 99;
    s->random = 1;
    SotnPlayerState startup = player;
    startup.subweapon_starting = true;
    CHECK(SotnActors_Tick(s, startup, 0, 0) && root->state.step == 3 && !root->armor.did_shield);
    startup.subweapon_starting = false;
    startup.weapon_starting = true;
    s->random = 1;
    CHECK(SotnActors_Tick(s, startup, 0, 0) && root->state.step == 7 && root->armor.did_shield);
    // The sword follows its owner's attack frame and retains signed table offsets.
    root->state.step = 6;
    root->state.frame = 16;
    root->pose = 1;
    root->remaining = 10;
    weapon->state.step = 1;
    CHECK(SotnActors_Tick(s, player, 0, 0));
    CHECK(weapon->state.step == 2 && weapon->state.frame == 32 && weapon->state.scale_x == 456 &&
          weapon->state.hitbox_width == 8 && weapon->state.hitbox_y == -8);
    Put16(f->stage + 0x2e2c + 12 * 4, 65);
    CHECK(!ActorArmorLord(s, weapon));
    Put16(f->stage + 0x2e2c + 12 * 4, 8);
    SDL_ClearError();
    // A fire wave owns paired primitives; damage actors expire independently.
    root->state.frame = 1;
    root->pose = 6;
    root->substep = 0;
    root->remaining = 10;
    CHECK(SotnActors_Tick(s, player, 0, 0));
    Actor *wave = &s->slots[160];
    CHECK(wave->state.id == 0x4c && wave->state.primitive_count == 26);
    bool damaging = false, initialized_damage = false, smoke = false;
    SotnPlayerState nearby = player;
    nearby.x = 170 * 65536;
    for (Uint32 t = 0; t < 260; ++t)
    {
        // Keep the body idle while the independently owned wave finishes.
        root->state.step = 2;
        nearby.x = (170 + (Sint32)(t < 24 ? t : 24)) * 65536;
        CHECK(SotnActors_Tick(s, nearby, 0, 0));
        for (Uint32 slot = 96; slot < 128; ++slot)
        {
            const Actor *hit = &s->slots[slot];
            damaging = damaging || hit->state.id == 0x4d;
            initialized_damage =
                initialized_damage || (hit->state.generation && hit->state.id == 0x4d &&
                                       hit->state.step >= 2 && hit->attack_element == 0x8001 &&
                                       hit->enemy_state.owner_generation == wave->state.generation);
        }
        for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
            smoke = smoke || s->primitives[i].armor_wave.smoke;
    }
    CHECK(damaging && initialized_damage && smoke && !wave->state.generation);
    // Killed ownership removes the separate weapon and all death primitives.
    root->state.flags |= 0x100;
    for (Uint32 i = 0; i < 600 && root->state.generation; ++i)
        CHECK(SotnActors_Tick(s, player, 0, 0));
    CHECK(s->armor_lord_killed && !root->state.generation && !weapon->state.generation);
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        CHECK(!s->primitives[i].owner);
    // Per-overlay defeat state survives room entry, without becoming a save flag.
    CHECK(SotnActors_Enter(
        s, f->layout,
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 16}, 0,
        0));
    Actor suppressed = {.state = {.id = 0x4a, .generation = 901}, .slot = 16};
    CHECK(ActorArmorLord(s, &suppressed) && suppressed.remove);
    // An old damage actor must never release a new wave's latch after slot reuse.
    *wave = (Actor){.state = {.id = 0x4c, .generation = 902}, .slot = 160};
    Actor stale = {.state = {.id = 0x4d, .generation = 903},
                   .enemy_state = {.owner_slot = 160, .owner_generation = 901}};
    CHECK(ActorArmorLord(s, &stale) && stale.remove);
    // Exhausted optional effect storage follows the original nonfatal fallback.
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        s->primitives[i].owner = 999;
    CHECK(ActorArmorLord(s, wave) && wave->remove && !wave->state.primitive_count);
    Actor fading = {.state = {.id = 0x4a, .generation = 904, .step = 9}};
    CHECK(ActorArmorLord(s, &fading) && fading.remove);
    SotnActors_Destroy(s);
    SotnActorLayout_Close(f->layout);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok = Musket(f) && Archer(f) && Spear(f) && Sword(f) && Blue(f) && Armor(f);
    SDL_free(f);
    return ok ? 0 : 1;
}
