#include "actors_internal.h"
#include "boss_projection.h"
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
    Uint8 dra[0x1000], nz0[0x3000], main[0x24000], tiles[64 * 32 * 2], types[2048];
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
static bool Open(Fixture *f, SotnActors **s, SotnActorLayout **layout)
{
    Put32(f->dra + 0x98, 0x800a0200);
    for (Uint32 i = 0; i < 7; ++i)
    {
        Uint8 *p = f->dra + 0x200 + i * 40;
        Put16(p + 4, (Uint16)(i == 2 ? 32767 : i == 1 || i == 4 ? 100 : 1));
        Put16(p + 6, 8);
        Put16(p + 12, 3);
        p[34] = 8;
        p[35] = 16;
        Put32(p + 36, i == 1 || i == 4 ? 0x08403000 : 0x88002400);
    }
    Put32(f->dra + 0x224, 0x08000000);
    Put16(f->dra + 0x20c, 0);
    const size_t initializers[] = {0xd00, 0xd0c, 0xd18, 0xd24, 0xd30, 0xd3c, 0xd48};
    for (Uint32 i = 0; i < 7; ++i)
    {
        Put16(f->nz0 + initializers[i], 0x8001);
        Put16(f->nz0 + initializers[i] + 2, 1);
        Put16(f->nz0 + initializers[i] + 8, (Uint16)i);
    }
    Put32(f->nz0 + 28, 0x80180040);
    for (Uint32 i = 0; i < 106; ++i)
        Put32(f->nz0 + 0x40 + i * 4, 0x80180300);
    Put16(f->nz0 + 0x300, 0xfffe);
    Put16(f->nz0 + 0x302, 0xfffe);
    Put16(f->nz0 + 0x30a, 128);
    Put16(f->nz0 + 0x30c, 256);
    Put16(f->nz0 + 0x30e, 0x3e);
    Put16(f->nz0 + 0x310, 144);
    Put16(f->nz0 + 0x314, 0xffff);
    Put16(f->nz0 + 0x316, 0xffff);
    for (Uint32 i = 0; i < 12; ++i)
    {
        Put16(f->nz0 + 0x1014 + i * 6, (Uint16)(i < 4 ? -8 : 1032));
        Put16(f->nz0 + 0x1016 + i * 6, (Uint16)(104 + (i % 4) * 16 + (i >= 8 ? 256 : 0)));
        Put16(f->nz0 + 0x1018 + i * 6, (Uint16)(i >= 4));
    }
    const size_t streams[] = {0x700,  0x1074, 0x1080, 0x108c, 0x109c, 0x10a8, 0x10b4, 0x10d4,
                              0x10e8, 0x10fc, 0x1108, 0x1114, 0x1128, 0x1140, 0x1150, 0x1158,
                              0x1160, 0x1170, 0x1250, 0x1264, 0x1298, 0x12ac, 0x12c0, 0x12cc,
                              0x12dc, 0x12f0, 0x12fc, 0x1304, 0x136c, 0x1378, 0x1388};
    for (Uint32 i = 0; i < SDL_arraysize(streams); ++i)
    {
        Uint8 *p = f->nz0 + streams[i];
        p[0] = p[2] = 1;
        p[1] = p[3] = 1;
        p[4] = 255;
    }
    const size_t boxes[] = {0x1178, 0x11e0, 0x1310};
    for (Uint32 i = 0; i < 3; ++i)
    {
        Put16(f->nz0 + boxes[i], 0);
        f->nz0[boxes[i] + 2] = 8;
        f->nz0[boxes[i] + 3] = 16;
    }
    Put16(f->nz0 + 0x1242, 28);
    Put16(f->nz0 + 0x1246, 4);
    Put16(f->nz0 + 0x1248, 4);
    Put16(f->nz0 + 0x124a, (Uint16)-4);
    Put16(f->nz0 + 0x124c, (Uint16)-8);
    Put16(f->nz0 + 0x106e, 32);
    Put16(f->nz0 + 0x1070, 16);
    for (Uint32 i = 0; i < 5; ++i)
        Put32(f->nz0 + 0x1e54 + i * 4, 0x80180700);
    Put16(f->nz0 + 0xfe0, 14);
    Put16(f->nz0 + 0xfe2, 0x79);
    Put32(f->nz0 + 0xfe8, 0x80180700);
    Put16(f->nz0 + 0x285a, 23);
    for (Uint32 i = 0; i <= 1024; ++i)
        Put16(f->main + 0x22a10 + i * 2, (Uint16)(i / 2));
    f->types[1] = 3;
    for (Uint32 y = 28; y < 32; ++y)
        for (Uint32 x = 0; x < 64; ++x)
            Put16(f->tiles + (y * 64 + x) * 2, 1);
    CHECK(SotnActorLayout_Open((SotnDataView){f->nz0, sizeof(f->nz0)}, 0, layout));
    CHECK(SotnActors_Create((SotnDataView){f->dra, sizeof(f->dra)},
                            (SotnDataView){f->nz0, sizeof(f->nz0)}, s));
    CHECK(SotnActors_Enter(
        *s, *layout,
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 64, 32}, 0,
        0));
    return true;
}
static bool Project(void)
{
    SotnProjectedRect r = {0};
    CHECK(SotnBoss_ProjectReward((SotnRewardProjection){0, 0, 0, 128, 4096, 128, 100}, &r));
    CHECK(r.left == 248 && r.right == 264 && r.top == 92 && r.bottom == 108);
    CHECK(SotnBoss_ProjectReward((SotnRewardProjection){0, 0, 1024, 128, 4096, 128, 100}, &r));
    CHECK(r.left == 120 && r.right == 136 && r.top == 220 && r.bottom == 236);
    CHECK(SotnBoss_ProjectReward((SotnRewardProjection){0, 0, 0, 128, -4096, 128, 100}, &r));
    CHECK(r.left == 264 && r.right == 248 && r.top == 108 && r.bottom == 92);
    CHECK(!SotnBoss_ProjectReward((SotnRewardProjection){0, 0, 0, 129, 4096, 128, 100}, &r));
    CHECK(r.left == 264);
    return true;
}
static bool Tick(SotnActors *s, Fixture *f, SotnPlayerState p, Uint32 tick)
{
    CHECK(SotnActors_SetServices(s, (SotnDataView){f->main, sizeof(f->main)}, tick, false));
    CHECK(SotnActors_Tick(s, p, 472, 260));
    return true;
}
static bool Encounter(Fixture *f)
{
    SotnActors *s = NULL;
    SotnActorLayout *layout = NULL;
    CHECK(Open(f, &s, &layout));
    SotnPlayerState p = {.x = 600 * 65536,
                         .y = 423 * 65536,
                         .hp = 70,
                         .hp_max = 70,
                         .solid_width = 7,
                         .solid_height = 24,
                         .invulnerable = true};
    CHECK(Tick(s, f, p, 0));
    CHECK((s->boss_flags & 1) && s->slots[16].state.id == 0x40 && s->slots[24].state.id == 0x43);
    bool projectile = false, flight = false, ground = false;
    for (Uint32 t = 1; t < 600; ++t)
    {
        CHECK(Tick(s, f, p, t));
        flight |= s->slots[24].state.step == 4;
        ground |= s->slots[24].state.step == 6;
        for (Uint32 i = 96; i < 128; ++i)
            projectile |= s->slots[i].state.id == 0x42 || s->slots[i].state.id == 0x45;
    }
    CHECK(flight && ground && projectile && s->world->laboratory_boss_visited);
    CHECK(s->slots[17].enemy_state.owner_generation == s->slots[16].state.generation);
    // A hit launches Slogra into Gaibon's pickup envelope, then their shared plunge state.
    Actor *slogra = &s->slots[16], *gaibon = &s->slots[24];
    SetStep(slogra, 8);
    slogra->substep = 1;
    slogra->state.x = 600 * 65536;
    slogra->state.y = 400 * 65536;
    SetStep(gaibon, 3);
    gaibon->state.x = 600 * 65536;
    gaibon->state.y = 372 * 65536;
    CHECK(Tick(s, f, p, 600));
    CHECK(gaibon->state.step == 9 && gaibon->boss.ascending);
    CHECK(Tick(s, f, p, 601));
    CHECK(slogra->state.step == 13);
    for (Uint32 t = 602; t < 900; ++t)
        CHECK(Tick(s, f, p, t));
    slogra->state.hit_points = 20;
    SetStep(slogra, 9);
    gaibon->state.hit_points = 40;
    bool transformed = false, big = false;
    for (Uint32 t = 900; t < 1400; ++t)
    {
        CHECK(Tick(s, f, p, t));
        transformed |= gaibon->boss.near_death;
        for (Uint32 i = 96; i < 192; ++i)
            big |= s->slots[i].state.id == 0x46;
    }
    CHECK(slogra->boss.near_death && transformed && big);
    p.attack = (SotnPlayerAttack){.active = true,
                                  .power = 999,
                                  .width = 255,
                                  .height = 255,
                                  .hit_type = 2,
                                  .hit_effect = 1,
                                  .invincibility_frames = 1};
    bool reward = false, drop = false;
    Uint32 maximum_parts = 0;
    for (Uint32 t = 1400; t < 3000; ++t)
    {
        CHECK(Tick(s, f, p, t));
        for (Uint32 i = 96; i < 192; ++i)
        {
            const Actor *a = &s->slots[i];
            reward |= a->state.id == 0x49;
            maximum_parts = SDL_max(maximum_parts, a->state.primitive_count);
            drop |= a->state.id == 3 && a->state.parameters == 0x8017;
        }
        if (drop)
            break;
    }
    SDL_Log("Boss result: kills%u flags%x record%u permanent%u reward%d drop%d parts%u; Slogra "
            "id%x step%u HP%d; Gaibon id%x step%u HP%d",
            s->combat.kills, s->boss_flags, s->world->laboratory_boss_time,
            s->world->flags[SOTN_LAB_BOSSES], reward, drop, maximum_parts, slogra->state.id,
            slogra->state.step, slogra->state.hit_points, gaibon->state.id, gaibon->state.step,
            gaibon->state.hit_points);
    CHECK(s->combat.kills == 2 && s->world->flags[SOTN_LAB_BOSSES] == 1 &&
          s->world->laboratory_boss_time != 0);
    CHECK((s->boss_flags & 26) == 26 && reward && drop && maximum_parts == 193);
    Uint32 owners = 0;
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        owners += s->primitives[i].owner != 0;
    CHECK(owners == 0);
    CHECK(SotnActors_Enter(s, layout, s->map, 0, 0));
    CHECK(Tick(s, f, p, 3000));
    CHECK(s->slots[16].state.generation == 0 && s->slots[24].state.generation == 0);
    SotnActors_Destroy(s);
    SotnActorLayout_Close(layout);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    const bool ok = f != NULL && Project() && Encounter(f);
    SDL_free(f);
    return ok ? 0 : 1;
}
