#include "chapel_confessional_internal.h"
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
    Uint8 dra[0x1000], stage[0x2000], tiles[16 * 16 * 2], types[2];
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
static bool Open(Fixture *f, SotnActors **out)
{
    SDL_zero(*f);
    Put32(f->dra + 0x98, 0x800a0200);
    Put16(f->dra + 0x204, 12);
    Put16(f->dra + 0x206, 7);
    Put16(f->dra + 0x20c, 3);
    f->dra[0x222] = f->dra[0x223] = 6;
    const Uint16 init[] = {0x968, 0x98c, 0x9bc, 0x9d4};
    for (Uint16 i = 0; i < SDL_arraysize(init); ++i)
    {
        Put16(f->stage + init[i], 0x8001);
        Put16(f->stage + init[i] + 2, 7);
    }
    // Synthetic curtain/animation/position data, not original runtime arrays.
    for (Uint16 side = 0; side < 2; ++side)
    {
        Uint8 *p = f->stage + 0xe40 + side * 16;
        Put16(p, 90);
        Put16(p + 2, 100);
        Put16(p + 4, side ? 92 : 108);
        Put16(p + 6, side ? 16384 : -16384);
        Put16(p + 8, side ? 80 : 160);
        Put16(p + 10, side ? -4 : 4);
        Put32(p + 12, 0x80180e20U + side * 16U);
        for (Uint16 i = 0; i < 4; ++i)
        {
            Put32(f->stage + 0xe20 + side * 16 + i * 4, 0x80180d54U + i * 8U);
            f->stage[0xd54 + i * 8] = 2;
            f->stage[0xd55 + i * 8] = (Uint8)(i + 1);
            f->stage[0xd56 + i * 8] = (Uint8)(i == 3 ? 255 : 0);
        }
    }
    for (Uint16 row = 0; row < 5; ++row)
    {
        Put16(f->stage + 0xe00 + row * 6, 110);
        Put16(f->stage + 0xe02 + row * 6, 130 + row);
        Put16(f->stage + 0xe04 + row * 6, row == 4 ? 120 : 100);
    }
    f->types[1] = 3;
    for (Uint16 x = 0; x < 16; ++x)
        Put16(f->tiles + (10 * 16 + x) * 2, 1);
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 6, out));
    (*out)->map = (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, 2}, 16, 16};
    (*out)->player = (SotnPlayerState){.hp = 70, .step = SOTN_PLAYER_STAND, .substep = 4};
    return true;
}
static bool Chairs(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor chair = {.slot = 16,
                   .state = {.id = 0x22,
                             .generation = 1,
                             .parameters = 0x104,
                             .x = 100 * 65536 + 123,
                             .y = 140 * 65536}};
    CHECK(ActorChapel(s, &chair) && chair.state.step == 1);
    Actor *ghost = &s->slots[17];
    CHECK(ghost->state.id == 0x23 && ghost->enemy_state.owner_generation == 1 &&
          ghost->state.x == 176 * 65536 && ghost->state.y == 128 * 65536);
    CHECK(ActorChapel(s, ghost) && ghost->state.step == 1 && s->control.block_stopwatch);
    s->player.x = 97 * 65536 + 456;
    s->player.y = chair.state.y;
    s->buttons = SOTN_BUTTON_UP | SOTN_BUTTON_RIGHT;
    CHECK(ActorChapel(s, &chair) && chair.state.step == 1);
    s->buttons = SOTN_BUTTON_UP;
    CHECK(ActorChapel(s, &chair) && chair.state.step == 2 && s->motion.chair_kind == 4 &&
          s->control.timer == 10 && s->control.buttons == SOTN_BUTTON_UP);
    for (Uint16 i = 0; i < 4; ++i)
    {
        s->motion.x = 0;
        CHECK(ActorChapel(s, &chair));
    }
    CHECK(chair.state.step == 3 && s->player.x == 100 * 65536 + 456);
    s->player.animation.remaining = -1;
    CHECK(ActorChapel(s, &chair) && chair.state.step == 4 && ghost->state.step == 2);
    s->player.substep = 0;
    CHECK(ActorChapel(s, &chair) && chair.state.step == 1);
    // Optional primitive exhaustion may remove the ghost; sitting itself still works.
    chair.state.step = 3;
    s->player.substep = 4;
    ActorPrimitivesClear(s, ghost);
    *ghost = (Actor){0};
    CHECK(ActorChapel(s, &chair) && chair.state.step == 4);
    chair.state.step = 3;
    ghost->state.generation = 2;
    ghost->state.id = 0x23;
    CHECK(!ActorChapel(s, &chair));
    chair.state.step = 0;
    CHECK(!ActorChapel(s, &chair));
    *ghost = (Actor){0};
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        s->primitives[i].owner = 2;
    *ghost = (Actor){.slot = 17, .state = {.id = 0x23, .generation = 3}};
    CHECK(ActorChapel(s, ghost) && ghost->remove);
    chair.state.parameters = 0x300;
    CHECK(!ActorChapel(s, &chair));
    SotnActors_Destroy(s);
    return true;
}
static bool Ghosts(Fixture *f)
{
    for (Uint16 variant = 0; variant < 4; ++variant)
    {
        SotnActors *s = NULL;
        CHECK(Open(f, &s));
        const bool parish = (variant & 1U) != 0, bad = variant > 1;
        Actor *g = &s->slots[20];
        *g = (Actor){.slot = 20,
                     .state = {.id = 0x23,
                               .generation = 9,
                               .parameters = (Uint16)((parish ? 1 : 0) | (bad ? 0x100 : 0)),
                               .x = (parish ? 79 : 161) * 65536,
                               .y = 160 * 65536}};
        CHECK(ActorChapel(s, g) && g->state.opacity_set && g->state.opacity == 0 &&
              g->state.palette == (bad ? 0xce : 0xc9));
        CHECK(s->confessional_chime == parish && s->combat.sequence_change == parish);
        if (parish)
            CHECK(s->combat.sequence == 0x204 && s->combat.cd_command == 0x90);
        g->state.step = 2;
        CHECK(ActorChapel(s, g) && g->state.step == 3);
        for (Uint16 i = 0; i < 4 && g->state.step == 3; ++i)
            CHECK(ActorChapel(s, g));
        CHECK(g->state.step == (bad ? 6 : 4));
        g->state.opacity = 192;
        if (!bad)
        {
            CHECK(ActorChapel(s, g));
            if (parish)
                s->player.substep = 0;
            else
            {
                CHECK(g->timer == 256);
                for (Uint16 i = 0; i < 256; ++i)
                    CHECK(ActorChapel(s, g));
            }
            if (parish)
                CHECK(ActorChapel(s, g));
            CHECK(g->state.step == 5);
            // Find an owned RNG state whose next original draw selects the reward.
            if (!parish)
            {
                for (Uint32 seed = 0;; ++seed)
                {
                    s->random = seed;
                    if (!(ActorRandom(s) & 3U))
                    {
                        s->random = seed;
                        break;
                    }
                }
            }
            for (Uint16 i = 0; i < 192; ++i)
                CHECK(ActorChapel(s, g));
            CHECK(g->state.step == 128 && !g->state.opacity);
            if (!parish)
                CHECK(s->slots[96].state.id == 0xa && s->slots[96].state.parameters == 56);
        }
        else
        {
            for (Uint16 i = 0; i < 4 && g->state.step == 6; ++i)
                CHECK(ActorChapel(s, g));
            CHECK(g->state.step == 7 && s->combat.sounds[0] == 0x7bb);
            CHECK(ActorChapel(s, g) && ActorChapel(s, g) && g->state.step == 8);
            for (Uint16 i = 0; i < 16; ++i)
                CHECK(ActorChapel(s, g));
            CHECK(g->state.step == (parish ? 10 : 9));
            for (Uint16 i = 0; i < (parish ? 32 : 80); ++i)
                CHECK(ActorChapel(s, g));
            CHECK(g->substep == 2 && g->timer == 128);
            for (Uint16 i = 0; i < (parish ? 1 : 4); ++i)
                CHECK(s->slots[96 + i].state.id == 0x24 &&
                      s->slots[96 + i].state.parameters == (parish ? 4 : i));
            for (Uint16 i = 0; i < 128; ++i)
                CHECK(ActorChapel(s, g));
            CHECK(g->state.step == 11);
            CHECK(ActorChapel(s, g) && ActorChapel(s, g) && g->state.step == 14);
        }
        CHECK(ActorChapel(s, g) && g->state.frame == 0);
        SotnActors_Destroy(s);
    }
    return true;
}
static bool Blades(Fixture *f)
{
    for (Uint16 row = 0; row < 5; ++row)
    {
        SotnActors *s = NULL;
        CHECK(Open(f, &s));
        Actor a = {.slot = 96, .state = {.id = 0x24, .generation = 1, .parameters = row}};
        CHECK(ActorChapel(s, &a) && a.state.frame == 7 + row && a.state.priority == 158 &&
              a.state.x == 110 * 65536 && s->combat.sounds[0] == 0x655);
        for (Uint16 i = 0; i < 4; ++i)
        {
            Actor *d = &s->slots[160 + i];
            CHECK(d->state.id == 0x25 && ActorChapel(s, d) && d->state.rotated && d->timer >= 15 &&
                  d->timer <= 78 && d->state.frame == 42);
        }
        const Uint16 moving = row == 4 ? 3 : 1;
        for (Uint16 i = 0; i < 16 && a.state.step == moving; ++i)
            CHECK(ActorChapel(s, &a));
        CHECK(a.state.step == moving + 1 && a.timer == 96);
        const Sint32 edge = a.state.x;
        for (Uint16 i = 0; i < 96; ++i)
            CHECK(ActorChapel(s, &a) && a.state.x == edge);
        for (Uint16 i = 0; i < 16 && !a.remove; ++i)
            CHECK(ActorChapel(s, &a));
        CHECK(a.remove);
        a.remove = false;
        a.state.flags |= 0x100U;
        CHECK(ActorChapel(s, &a) && a.remove && s->slots[164].state.id == 2);
        a.state.parameters = 5;
        CHECK(!ActorChapel(s, &a));
        SotnActors_Destroy(s);
    }
    return true;
}
static bool Interrupted(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor g = {.slot = 20, .state = {.id = 0x23, .generation = 1}};
    CHECK(ActorChapel(s, &g));
    g.state.step = 3;
    g.state.x = 164 * 65536;
    g.state.y = 160 * 65536;
    s->player.substep = 0;
    CHECK(ActorChapel(s, &g) && g.state.step == 13 && g.state.opacity == 4);
    CHECK(ActorChapel(s, &g) && g.state.step == 13 && g.state.opacity == 0);
    CHECK(ActorChapel(s, &g) && g.state.step == 14 && g.state.opacity == 252 && !g.state.frame);
    ConfessionalStep(&g, 4);
    g.state.opacity = 192;
    CHECK(ActorChapel(s, &g) && g.state.step == 5 && g.timer == 1);
    for (Uint16 i = 0; i < 192; ++i)
        CHECK(ActorChapel(s, &g));
    CHECK(g.state.step == 128 && !s->slots[96].state.generation);
    ConfessionalStep(&g, 8);
    g.timer = 0;
    CHECK(!ActorChapel(s, &g));
    g.state.step = 9;
    g.substep = 3;
    g.timer = 32;
    CHECK(!ActorChapel(s, &g));
    ActorPrimitivesClear(s, &g);
    SotnActors_Destroy(s);
    return true;
}
static bool ChimeAndBounds(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor end = {.state = {.id = 0x2d, .generation = 1}};
    CHECK(ActorChapel(s, &end) && end.remove);
    end.remove = false;
    s->confessional_chime = true;
    CHECK(ActorChapel(s, &end) && end.state.step == 2 && s->combat.sequence_change &&
          !s->combat.sequence);
    CHECK(ActorChapel(s, &end) && end.state.step == 3 && s->combat.restart_stage_music &&
          !s->confessional_chime);
    end.state.step = 0;
    s->confessional_chime = true;
    s->music_playing = true;
    s->combat.restart_stage_music = false;
    CHECK(ActorChapel(s, &end) && end.state.step == 3 && s->combat.cd_command == 0x80 &&
          !s->combat.restart_stage_music && s->confessional_chime);
    ConfessionalProfile p = {0};
    CHECK(!ConfessionalRead(s, 2, &p));
    Put32(f->stage + 0xe4c, 0x80180000);
    CHECK(!ConfessionalRead(s, 0, &p));
    Put32(f->stage + 0xe4c, 0x80180e20);
    Put16(f->stage + 0xe46, 0);
    CHECK(!ConfessionalRead(s, 0, &p));
    CHECK(!ActorChapelConfessional(NULL, &end));
    // Empty synthetic placement lists let the real room-entry and hit pipeline run.
    Put32(f->stage + 28, 0x80180040);
    for (Uint16 i = 0; i < 106; ++i)
        Put32(f->stage + 0x40 + i * 4U, 0x80180300);
    Put16(f->stage + 0x300, -2);
    Put16(f->stage + 0x302, -2);
    Put16(f->stage + 0x30a, -1);
    Put16(f->stage + 0x30c, -1);
    SotnActorLayout *layout = NULL;
    CHECK(SotnActorLayout_OpenStage(s->nz0, 6, 0, &layout));
    s->control.block_stopwatch = true;
    s->confessional_chime = true;
    CHECK(SotnActors_Enter(s, layout, s->map, 0, 0) && !s->control.block_stopwatch &&
          s->confessional_chime);
    Actor source = {.state = {.x = 110 * 65536, .y = 130 * 65536, .generation = 1}};
    Actor *blade = NULL;
    CHECK(ActorSpawn(s, &source, 0x24, 0, 96, 128, &blade) && blade && ActorChapel(s, blade));
    const SotnPlayerState attacking = {
        .x = blade->state.x,
        .y = blade->state.y,
        .hp = 70,
        .attack = {.active = true, .power = 100, .hit_type = 2, .width = 16, .height = 16}};
    CHECK(SotnActors_Tick(s, attacking, 0, 0) && !blade->state.generation &&
          s->slots[164].state.id == 2 && !s->slots[164].state.parameters &&
          !s->combat.restart_stage_music);
    SotnActorLayout_Close(layout);
    SotnActors_Destroy(s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok = Chairs(f) && Ghosts(f) && Blades(f) && Interrupted(f) && ChimeAndBounds(f);
    SDL_free(f);
    return ok ? 0 : 1;
}
