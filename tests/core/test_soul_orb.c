#include "actors_internal.h"
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
static void Tables(Uint8 *p)
{
    for (Uint16 i = 0; i < 8; ++i)
        Put16(p + i * 2U, (Uint16)(i * 16U));
    for (Uint16 i = 0; i < 6; ++i)
    {
        Uint8 *s = p + 16U + i * 16U;
        Put16(s, (Uint16)-2);
        Put16(s + 2, (Uint16)-4);
        Put16(s + 4, 10);
        Put16(s + 6, 12);
        Put16(s + 8, (Uint16)(10 + i));
        Put16(s + 10, 30);
        Put16(s + 12, (Uint16)(20 + i));
        Put16(s + 14, 42);
        p[112 + i * 2U] = 2;
        p[113 + i * 2U] = (Uint8)(i + 1);
    }
}
static bool Component(SotnDataView exe)
{
    Uint8 tables[126] = {0};
    Tables(tables);
    SotnSoulOrbInput in = {.executable = exe,
                           .tables = {tables, sizeof(tables)},
                           .x = 100 * 65536 + 123,
                           .y = 140 * 65536 + 456,
                           .player_x = 100,
                           .player_y = 140,
                           .allocated = true};
    SotnSoulOrb s = {0};
    SotnSoulOrbEvents out = {0};
    Uint32 rng = 1;
    CHECK(SotnSoulOrb_Tick(&s, in, &rng, &out) && s.phase == 1 && s.speed == 0x400 && !s.hitbox &&
          !out.draw && rng == 0x01010102U);
    CHECK(s.angle == (Uint16)-16 && s.x == in.x && s.y == in.y);
    for (Uint16 t = 1; t <= 20; ++t)
    {
        CHECK(SotnSoulOrb_Tick(&s, in, &rng, &out) && s.age == t && s.hitbox == (t >= 16) &&
              out.draw && !out.contact);
        CHECK(s.turn == t * 2U && s.speed == 0x400 + t * 4U &&
              s.scale_x == (t < 16 ? t * 16U : 256U));
        CHECK(out.x[0] == 98 && out.y[0] == 136 && out.x[3] == 108 && out.y[3] == 148 &&
              out.v[3] == 42);
    }
    const SotnSoulOrb saved = s;
    CHECK(SotnSoulOrb_Translate(&s, -32, 16) && s.x == saved.x - 32 * 65536 &&
          s.y == saved.y + 16 * 65536 && s.age == saved.age);
    CHECK(SotnSoulOrb_Translate(&s, 32, -16) && !SDL_memcmp(&s, &saved, sizeof(s)));
    CHECK(!SotnSoulOrb_Translate(&s, 32768, 0) && !SDL_memcmp(&s, &saved, sizeof(s)));
    in.hit = true;
    CHECK(SotnSoulOrb_Tick(&s, in, &rng, &out) && s.removed && out.contact && out.heal == 8 &&
          !out.draw);
    s = saved;
    in.heal_pending = true;
    CHECK(SotnSoulOrb_Tick(&s, in, &rng, &out) && s.removed && out.contact && !out.heal);
    s = saved;
    in.hit = false;
    in.tables.size = 125;
    out.heal = 99;
    const Uint32 prior = rng;
    CHECK(!SotnSoulOrb_Tick(&s, in, &rng, &out) && !SDL_memcmp(&s, &saved, sizeof(s)) &&
          rng == prior && out.heal == 99);
    in.tables.size = 126;
    tables[113] = 7;
    s = (SotnSoulOrb){.phase = 1};
    const SotnSoulOrb malformed = s;
    CHECK(!SotnSoulOrb_Tick(&s, in, &rng, &out) && !SDL_memcmp(&s, &malformed, sizeof(s)) &&
          rng == prior && out.heal == 99);
    s = (SotnSoulOrb){0};
    CHECK(SotnSoulOrb_Tick(&s, (SotnSoulOrbInput){0}, &rng, &out) && s.removed && rng == prior &&
          !out.contact);
    return true;
}
static bool Owner(SotnDataView exe)
{
    SotnActors *s = SDL_calloc(1, sizeof(*s));
    CHECK(s);
    Uint8 dra[0x300] = {0}, stage[0x2100] = {0};
    Put32(dra + 0x98, 0x800a0200);
    dra[0x222] = 5;
    dra[0x223] = 7;
    Tables(stage + 0x1f80);
    s->dra = (SotnDataView){dra, sizeof(dra)};
    s->nz0 = (SotnDataView){stage, sizeof(stage)};
    s->executable = exe;
    s->stage_id = 12;
    s->random = 1;
    Actor a = {
        .state = {.id = 7, .x = 100 * 65536 + 123, .y = 140 * 65536 + 456, .generation = 123}};
    SotnPlayerState p = {.x = 100 * 65536, .y = 140 * 65536, .invulnerable = true};
    CHECK(ActorSoulOrb(s, &a, p) && a.state.step == 1 && a.state.primitive_count == 1 &&
          s->primitives[0].owner == 123 && !s->primitives[0].state.active &&
          a.state.hitbox_width == 5 && a.state.hitbox_height == 7);
    for (Uint16 t = 1; t <= 16; ++t)
        CHECK(ActorSoulOrb(s, &a, p));
    CHECK(a.state.hitbox_state == 1 && s->primitives[0].state.active &&
          s->primitives[0].state.page == 0x18 && s->primitives[0].state.palette == 0x194 &&
          !s->primitives[0].state.blend && !s->primitives[0].state.modulate);
    s->camera_x = 32;
    CHECK(ActorSoulOrb(s, &a, p) && a.state.x == 100 * 65536 + 123 &&
          s->primitives[0].state.offset_x[0] == 98);
    a.hit_flags = 0x80;
    CHECK(ActorSoulOrb(s, &a, p) && a.remove && s->combat.blood_heal == 8 &&
          s->combat.blood_contacts == 1);
    ActorPrimitivesClear(s, &a);
    for (Uint16 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        s->primitives[i].owner = 999;
    const Uint32 rng = s->random;
    a = (Actor){.state = {.id = 7, .generation = 124}};
    CHECK(ActorSoulOrb(s, &a, p) && a.remove && !a.state.step && s->random == rng);
    for (Uint16 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        CHECK(s->primitives[i].owner == 999);
    SDL_free(s);
    return true;
}
static bool Contacts(SotnDataView exe)
{
    Uint8 dra[0x300] = {0}, stage[0x2100] = {0}, tiles[32 * 16 * 2] = {0}, types[1] = {0};
    Put32(dra + 0x98, 0x800a0200);
    dra[0x222] = 5;
    dra[0x223] = 7;
    Tables(stage + 0x1f80);
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
        Put16(r + 4, 7);
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
        SotnPlayerState player = {.x = 100 * 65536,
                                  .y = 140 * 65536,
                                  .status = 0,
                                  .invulnerable = true,
                                  .healing = {.pending = pending ? 8 : 0}};
        player.animation.hitbox_width = 8;
        player.animation.hitbox_height = 16;
        SotnActorCombat combat = {0};
        Uint16 contacts = 0;
        for (Uint16 t = 0; t < 22; ++t)
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

int main(void)
{
    Uint8 *exe = SDL_calloc(1, 0x24000);
    if (!exe)
        return 1;
    bool ok = Component((SotnDataView){exe, 0x24000}) && Owner((SotnDataView){exe, 0x24000}) &&
              Contacts((SotnDataView){exe, 0x24000});
    SDL_free(exe);
    if (ok)
        SDL_Log("SOUL_ORB: PASS authored homing/animation/healing/row ownership/failure contexts");
    return ok ? 0 : 1;
}
