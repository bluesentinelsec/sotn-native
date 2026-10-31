#include "actors_internal.h"
#include "player_internal.h"
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
static void Put32(Uint8 *p, Uint32 v)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (i * 8U));
}
static bool Component(SotnDataView exe)
{
    for (Uint16 face = 0; face < 2; ++face)
    {
        SotnEnemyBlood b = {0};
        Uint32 rng = 1;
        SotnEnemyBloodEvents out = {0};
        SotnEnemyBloodInput in = {.executable = exe,
                                  .x = 100 * 65536 + 123,
                                  .y = 140 * 65536 + 456,
                                  .allocated = true,
                                  .absorbing = true,
                                  .facing_left = face != 0};
        CHECK(SotnEnemyBlood_Tick(&b, in, &rng, &out) && b.phase == 1 && b.timer == 48 &&
              b.hitbox && b.hitbox_height == 8 && b.velocity_x == (face ? 81920 : -81920));
        Uint32 expected = 1;
        for (Uint16 i = 0; i < 48; ++i)
            expected = expected * 0x01010101U + 1U;
        CHECK(rng == expected && !out.contact);
        for (Uint16 i = 0; i < 12; ++i)
            CHECK(b.particles[i].width == 4 && b.particles[i].fixed_x == 0 &&
                  b.particles[i].interval >= 7 && b.particles[i].interval <= 14);
        const SotnEnemyBlood before = b;
        CHECK(SotnEnemyBlood_Translate(&b, -32, 16) && b.x == before.x - 32 * 65536 &&
              b.particles[0].x == before.particles[0].x - 32 &&
              b.particles[0].fixed_x == before.particles[0].fixed_x && b.timer == before.timer);
        CHECK(SotnEnemyBlood_Translate(&b, 32, -16) && !SDL_memcmp(&b, &before, sizeof(b)));
        CHECK(!SotnEnemyBlood_Translate(&b, 32768, 0) && !SDL_memcmp(&b, &before, sizeof(b)));
        in.hit = true;
        CHECK(SotnEnemyBlood_Tick(&b, in, &rng, &out) && out.contact && out.heal == 8 &&
              !b.hitbox && rng == expected);
        CHECK(SotnEnemyBlood_Tick(&b, in, &rng, &out) && !out.contact && !out.heal);
        while (!b.removed)
            CHECK(SotnEnemyBlood_Tick(&b, in, &rng, &out));
        b = before;
        b.timer = 17;
        in.bloodstone = true;
        CHECK(SotnEnemyBlood_Tick(&b, in, &rng, &out) && b.timer == 16 && out.contact &&
              out.heal == 16 && !b.hitbox);
        b = before;
        in.heal_pending = true;
        CHECK(SotnEnemyBlood_Tick(&b, in, &rng, &out) && out.contact && !out.heal);
        b = before;
        in.absorbing = false;
        CHECK(SotnEnemyBlood_Tick(&b, in, &rng, &out) && !b.hitbox && !out.contact &&
              b.x == before.x);
        in.absorbing = true;
        CHECK(SotnEnemyBlood_Tick(&b, in, &rng, &out) && !b.hitbox && !out.contact);
    }
    SotnEnemyBlood b = {0};
    Uint32 rng = 17;
    SotnEnemyBloodEvents out = {.heal = 99, .contact = true};
    CHECK(!SotnEnemyBlood_Tick(&b, (SotnEnemyBloodInput){.allocated = true}, &rng, &out) &&
          !b.phase && rng == 17 && out.heal == 99);
    CHECK(SotnEnemyBlood_Tick(&b, (SotnEnemyBloodInput){0}, &rng, &out) && b.removed && rng == 17 &&
          !out.contact);
    return true;
}
static bool Owner(SotnDataView exe)
{
    SotnActors *s = SDL_calloc(1, sizeof(*s));
    Uint8 dra[0x300] = {0}, stage[0xc00] = {0};
    CHECK(s);
    Put32(dra + 0x98, 0x800a0200);
    s->dra = (SotnDataView){dra, sizeof(dra)};
    s->nz0 = (SotnDataView){stage, sizeof(stage)};
    s->executable = exe;
    s->random = 17;
    Actor a = {
        .state = {.id = 0xd, .x = 100 * 65536 + 123, .y = 140 * 65536 + 456, .generation = 123}};
    SotnPlayerState player = {.status = 0x2000000U};
    CHECK(ActorEnemyBlood(s, &a, player) && a.state.primitive_count == 12 && a.state.step == 1 &&
          a.state.hitbox_state == 1);
    for (Uint16 i = 0; i < 12; ++i)
        CHECK(a.primitive[i] == i && s->primitives[i].owner == 123 &&
              s->primitives[i].state.active);
    CHECK(!s->primitives[12].owner);
    const SotnEnemyBlood before = a.enemy_blood;
    s->camera_x = 32;
    CHECK(ActorEnemyBlood(s, &a, player) && a.enemy_blood.timer == 47 &&
          a.enemy_blood.particles[0].fixed_x != 0 &&
          a.enemy_blood.x == before.x - 32 * 65536 + a.enemy_blood.velocity_x);
    a.hit_flags = 0x80;
    CHECK(ActorEnemyBlood(s, &a, player) && s->combat.blood_contacts == 1 &&
          s->combat.blood_heal == 8 && !a.state.hitbox_state);
    CHECK(ActorEnemyBlood(s, &a, player) && s->combat.blood_contacts == 1);
    ActorPrimitivesClear(s, &a);
    for (Uint16 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        s->primitives[i].owner = (i % 12 == 11) ? 999 : 0;
    a = (Actor){.state = {.id = 0xd, .generation = 124}};
    const Uint32 random = s->random;
    CHECK(ActorEnemyBlood(s, &a, player) && a.remove && !a.state.step && !a.state.primitive_count &&
          s->random == random);
    for (Uint16 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        CHECK(s->primitives[i].owner == ((i % 12 == 11) ? 999U : 0U));
    SDL_free(s);
    return true;
}
static bool Contacts(SotnDataView exe)
{
    Uint8 dra[0x300] = {0}, stage[0xc00] = {0}, tiles[32 * 16 * 2] = {0}, types[1] = {0};
    Put32(dra + 0x98, 0x800a0200);
    Put32(stage + 28, 0x80180040);
    for (Uint16 i = 0; i < 106; ++i)
        Put32(stage + 0x40 + i * 4U, 0x80180300);
    Put16(stage + 0x300, 0xfffe);
    Put16(stage + 0x302, 0xfffe);
    for (Uint16 i = 0; i < 2; ++i)
    {
        Uint8 *r = stage + 0x30a + i * 10U;
        Put16(r, 100);
        Put16(r + 2, 140);
        Put16(r + 4, 0xd);
        Put16(r + 6, i);
        Put16(r + 8, 0);
    }
    Put16(stage + 0x31e, 0xffff);
    Put16(stage + 0x320, 0xffff);
    SotnActorLayout *layout = NULL;
    CHECK(SotnActorLayout_Open((SotnDataView){stage, sizeof(stage)}, 0, &layout));
    for (Uint16 pending = 0; pending < 2; ++pending)
    {
        SotnActors *s = NULL;
        CHECK(SotnActors_Create((SotnDataView){dra, sizeof(dra)},
                                (SotnDataView){stage, sizeof(stage)}, &s));
        CHECK(SotnActors_Enter(s, layout,
                               (SotnCollisionMap){{tiles, sizeof(tiles)}, {types, 1}, 32, 16}, 0,
                               0) &&
              SotnActors_SetServices(s, exe, 0, false));
        SotnPlayerState player = {
            .x = 100 * 65536,
            .y = 140 * 65536,
            .status = 0x2000000U,
            .invulnerable = true,
            .healing = {.pending = pending ? 8 : 0, .pending_kind = pending ? 1 : 0}};
        player.animation.hitbox_width = 8;
        player.animation.hitbox_height = 16;
        SotnActorCombat combat = {0};
        Uint16 contacts = 0;
        for (Uint16 t = 0; t < 6; ++t)
        {
            CHECK(SotnActors_Tick(s, player, 0, 0) && SotnActors_GetCombat(s, &combat));
            if (combat.blood_contacts)
                CHECK(combat.blood_contacts == 2 && combat.blood_heal == (pending ? 0 : 8));
            contacts = (Uint16)(contacts + combat.blood_contacts);
        }
        CHECK(contacts == 2 && !combat.blood_contacts && !combat.blood_heal);
        SotnActors_Destroy(s);
    }
    SotnActorLayout_Close(layout);
    return true;
}
static bool Healing(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000);
    Uint8 tiles[32 * 16 * 2] = {0}, types[2] = {0, 3};
    CHECK(dra);
    for (Uint16 i = 0; i < 247; ++i)
        Put32(dra + 0x101b8 + i * 4U, 0x800a0100);
    Put32(dra + 0x100, 2U | 1U << 16);
    Put32(dra + 0x104, 0);
    SDL_memset(dra + 0xcf18, 3, 20);
    SDL_memset(dra + 0xcf2c, 5, 16);
    SDL_memset(dra + 0xcf3c, 28, 16);
    for (Uint16 y = 10; y < 16; ++y)
        for (Uint16 x = 0; x < 32; ++x)
            tiles[(y * 32U + x) * 2U] = 1;
    SotnCollisionMap map = {{tiles, sizeof(tiles)}, {types, 2}, 32, 16};
    SotnPlayer *p = NULL;
    CHECK(SotnPlayer_Create((SotnDataView){dra, 0x11000}, map, 128, 135, &p));
    p->hp = 50;
    p->hp_max = 70;
    CHECK(!SotnPlayer_QueueBloodHealing(p, 9, 1) && !SotnPlayer_QueueBloodHealing(p, 8, 33));
    CHECK(SotnPlayer_QueueBloodHealing(p, 8, 1) && SotnPlayer_QueueBloodHealing(p, 16, 2) &&
          p->hp == 50 && p->healing.pending == 8 && p->healing.contacts == 3);
    CHECK(SotnPlayer_Tick(p, 0) && p->hp == 58 && !p->healing.pending && p->healing.applied &&
          p->healing.effects && p->healing.applied_amount == 8 &&
          p->healing.source.x == 128 * 65536);
    const SotnPlayerHealing h = p->healing;
    CHECK(SotnPlayer_ChangeRoom(p, map, 16, 0) && p->healing.source.x == h.source.x + 16 * 65536 &&
          p->healing.contacts == h.contacts);
    CHECK(SotnPlayer_Tick(p, 0) && p->hp == 58 && !p->healing.applied && !p->healing.effects);
    CHECK(SotnPlayer_QueueBloodHealing(p, 16, 1) && SotnPlayer_Tick(p, 0) && p->hp == 70 &&
          p->healing.applied_amount == 16);
    CHECK(SotnPlayer_QueueBloodHealing(p, 8, 1));
    const SotnPlayer before = *p;
    Put32(dra + 0x101b8 + p->animation.id * 4U, 0x800b1000);
    CHECK(!SotnPlayer_Tick(p, 0) && !SDL_memcmp(p, &before, sizeof(before)));
    SotnPlayer_Close(p);
    SDL_free(dra);
    return true;
}
int main(void)
{
    Uint8 *exe = SDL_calloc(1, 0x24000);
    if (!exe)
        return 1;
    /* Original test data, deliberately linear rather than copyrighted SDK samples. */
    for (Uint16 i = 0; i < 1025; ++i)
        Put16(exe + 0x1dbcc + i * 2U, (Uint16)(i * 4U));
    bool ok = Component((SotnDataView){exe, 0x24000}) && Owner((SotnDataView){exe, 0x24000}) &&
              Contacts((SotnDataView){exe, 0x24000}) && Healing();
    SDL_free(exe);
    return ok ? 0 : 1;
}
