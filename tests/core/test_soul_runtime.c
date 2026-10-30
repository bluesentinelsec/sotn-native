#include "actors_internal.h"
#include "character_fixture.h"
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
typedef struct Fixture
{
    Uint8 dra[0x11000];
    SotnCharacter *c;
    SotnBatRuntime r;
    SotnBatRuntimeInput in;
} Fixture;
static bool Create(Fixture *f)
{
    SotnCharacter_Destroy(f->c);
    SDL_zero(*f);
    TestCharacter_Fill(f->dra);
    Uint8 *b = f->dra + 0xd1d4 + 112 * 6;
    b[0] = 53;
    b[1] = 1;
    b[2] = 193;
    b[4] = 9;
    f->dra[0xd4b8 + 18] = 47;
    f->dra[0xd4b8 + 19] = 47;
    Uint8 *s = f->dra + 0x8410 + 5 * 28;
    s[13] = 10;
    Put16(s + 14, 8);
    Put16(s + 16, 0x82);
    Put16(s + 18, 2);
    Put16(s + 24, 15);
    CHECK(SotnCharacter_Create((SotnDataView){f->dra, sizeof(f->dra)}, &f->c));
    f->in = (SotnBatRuntimeInput){.dra = {f->dra, sizeof(f->dra)},
                                  .player = {.x = 128 * 65536 + 123,
                                             .y = 140 * 65536 + 456,
                                             .priority = 148,
                                             .step = SOTN_PLAYER_SOUL}};
    CHECK(SotnPlayerFactory_Begin(
        &f->r.factories[0].cursor,
        (SotnPlayerFactorySource){.x = f->in.player.x, .y = f->in.player.y, .priority = 148}, 112,
        0));
    return true;
}
static bool Runtime(Fixture *f)
{
    CHECK(Create(f));
    for (Uint16 i = 0; i < 16; ++i)
        f->in.primitive_slots.occupied[i] = SDL_MAX_UINT64;
    CHECK(SotnBatRuntime_Tick(&f->r, f->c, f->in) && f->r.soul_attack &&
          f->r.children[31].kind == 53 && f->r.children[31].initialized &&
          f->r.children[31].group == 7 && f->r.groups[7] == 1 &&
          !f->r.children[31].primitive_count);
    const SotnPlayerAttack attack = f->r.children[31].state.attack;
    CHECK(attack.active && attack.hit_type == 0x82 && attack.hit_effect == 2 && !attack.width &&
          !attack.height && attack.power >= 15);
    SotnCharacterState before = {0}, after = {0};
    CHECK(SotnCharacter_Get(f->c, &before, NULL));
    f->in.player.x += 16 * 65536;
    f->in.player.y -= 8 * 65536;
    CHECK(SotnBatRuntime_Tick(&f->r, f->c, f->in) && f->r.soul_attack &&
          SotnCharacter_Get(f->c, &after, NULL) && before.random == after.random &&
          !SDL_memcmp(&f->r.children[31].state.attack, &attack, sizeof(attack)));
    f->in.player.step = SOTN_PLAYER_STAND;
    CHECK(SotnBatRuntime_Tick(&f->r, f->c, f->in) && !f->r.soul_attack && !f->r.children[31].kind &&
          !f->r.groups[7]);
    CHECK(Create(f));
    f->in.other_occupied = (Uint64)1 << 47;
    CHECK(SotnCharacter_Get(f->c, &before, NULL) && SotnBatRuntime_Tick(&f->r, f->c, f->in) &&
          SotnCharacter_Get(f->c, &after, NULL) && !f->r.soul_attack && !f->r.children[31].kind &&
          before.random == after.random && !f->r.factories[0].cursor.entity_id);
    CHECK(Create(f));
    f->dra[0x8410 + 5 * 28 + 16] = 2;
    const SotnBatRuntime retained = f->r;
    CHECK(SotnCharacter_Get(f->c, &before, NULL) && !SotnBatRuntime_Tick(&f->r, f->c, f->in) &&
          !SDL_memcmp(&f->r, &retained, sizeof(retained)) &&
          SotnCharacter_Get(f->c, &after, NULL) && before.random == after.random);
    return true;
}
static bool Contacts(Fixture *f)
{
    Uint8 *stage = SDL_calloc(1, 0x2100), *exe = SDL_calloc(1, 0x24000);
    CHECK(stage && exe);
    Uint8 tiles[32 * 16 * 2] = {0}, types[1] = {0};
    TestCharacter_Put32(stage + 28, 0x80180040);
    for (Uint16 i = 0; i < 106; ++i)
        TestCharacter_Put32(stage + 0x40 + i * 4U, 0x80180300);
    Put16(stage + 0x300, 0xfffe);
    Put16(stage + 0x302, 0xfffe);
    Put16(stage + 0x30a, 0xffff);
    Put16(stage + 0x30c, 0xffff);
    Uint8 *shape = stage + 0x1f90;
    Put16(shape + 4, 8);
    Put16(shape + 6, 8);
    Put16(shape + 12, 8);
    Put16(shape + 14, 8);
    stage[0x1ff0] = 2;
    stage[0x1ff1] = 1;
    SotnActorLayout *layout = NULL;
    CHECK(SotnActorLayout_Open((SotnDataView){stage, 0x2100}, 0, &layout));
    for (Uint16 mode = 0; mode < 5; ++mode)
    {
        CHECK(Create(f) && SotnBatRuntime_Tick(&f->r, f->c, f->in));
        TestCharacter_Put32(f->dra + 0x98, 0x800a0100);
        f->dra[0x122] = 2;
        f->dra[0x123] = 3;
        SotnActors *s = NULL;
        CHECK(SotnActors_Create((SotnDataView){f->dra, sizeof(f->dra)},
                                (SotnDataView){stage, 0x2100}, &s) &&
              SotnActors_Enter(s, layout,
                               (SotnCollisionMap){{tiles, sizeof(tiles)}, {types, 1}, 32, 16}, 0,
                               0) &&
              SotnActors_SetServices(s, (SotnDataView){exe, 0x24000}, 0, false) &&
              SotnActors_SetBatRuntime(s, &f->r));
        s->slots[10] = (Actor){.state = {.id = 0x26,
                                         .generation = 0x80000010U,
                                         .step = 1,
                                         .x = (mode == 3 ? 320 : 220) * 65536,
                                         .y = 140 * 65536,
                                         .hit_points = mode == 4 ? 0x7fff : 100,
                                         .hitbox_state = 2,
                                         .hitbox_width = 10,
                                         .hitbox_height = 15},
                               .stun = 200,
                               .slot = 10,
                               .dynamic = true};
        if (mode == 1)
        {
            s->slots[10].enemy.immunes = 1;
            f->r.children[31].state.attack.element = 1;
        }
        if (mode == 2)
            for (Uint16 i = 96; i < 128; ++i)
                s->slots[i] = (Actor){.state = {.id = 2, .generation = 0x80000100U + i, .step = 1},
                                      .stun = 200,
                                      .slot = i,
                                      .dynamic = true};
        const Sint16 hp = s->slots[10].state.hit_points;
        CHECK(SotnActors_Tick(s, f->in.player, 0, 0));
        Uint16 orbs = 0;
        for (Uint16 i = 96; i < 128; ++i)
            orbs += (Uint16)(s->slots[i].state.id == 7);
        CHECK(orbs == ((mode == 0 || mode == 1 || mode == 4) ? 1 : 0));
        CHECK(s->slots[10].state.hit_points == ((mode == 1 || mode == 3 || mode == 4)
                                                    ? hp
                                                    : hp - f->r.children[31].state.attack.power));
        if (mode == 0 || mode == 1 || mode == 4)
        {
            CHECK(s->slots[96].state.id == 7 && s->slots[96].state.step == 1 &&
                  s->slots[96].state.primitive_count == 1);
        }
        if (mode == 2)
            for (Uint16 i = 96; i < 128; ++i)
                CHECK(s->slots[i].state.generation == 0x80000100U + i && s->slots[i].state.id == 2);
        CHECK(SotnActors_Tick(s, f->in.player, 0, 0));
        orbs = 0;
        for (Uint16 i = 96; i < 128; ++i)
            orbs += (Uint16)(s->slots[i].state.id == 7);
        CHECK(orbs == ((mode == 0 || mode == 1 || mode == 4) ? 1 : 0));
        SotnActors_Destroy(s);
    }
    SotnActorLayout_Close(layout);
    SDL_free(stage);
    SDL_free(exe);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    bool ok = Runtime(f) && Contacts(f);
    SotnCharacter_Destroy(f->c);
    SDL_free(f);
    if (ok)
        SDL_Log(
            "SOUL_RUNTIME: PASS actual slot/group/global-contact/orb-capacity/exhaustion contexts");
    return ok ? 0 : 1;
}
