#include "character_fixture.h"
#include "player_internal.h"
#include <sotn/bat_runtime.h>
#include <sotn/stage_route.h>
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
    Uint8 dra[0x11000], tiles[64 * 16 * 2], types[2];
    SotnPlayer *p;
    SotnCharacter *character;
    SotnBatRuntime runtime;
    Uint16 uses, requests;
    bool reject;
} Fixture;
static bool Factory(void *context, SotnPlayerFactorySource source, Uint32 packed, bool *allocated)
{
    Fixture *f = context;
    ++f->requests;
    return SotnBatRuntime_RequestTeleport(&f->runtime, source, packed, allocated);
}
static bool Use(void *context, Uint16 hand, Uint16 *equipped)
{
    Fixture *f = context;
    if (f->reject)
        return SDL_SetError("Supplied actual consumption failure");
    if (!SotnCharacter_UseLibraryCard(f->character, hand, equipped))
        return false;
    ++f->uses;
    return true;
}
static bool Create(Fixture *f, Uint16 hand, Uint16 spare, Uint16 duplicator)
{
    SotnPlayer_Close(f->p);
    SotnCharacter_Destroy(f->character);
    SDL_memset(f, 0, sizeof(*f));
    TestCharacter_Fill(f->dra);
    f->dra[0x500 + 18] = 0x24;
    Uint8 *e = f->dra + 0x500 + 166 * 52;
    e[15] = 255;
    e[17] = 131;
    e[18] = 0xbe;
    e[19] = 55;
    e[20] = 20;
    e[25] = 1;
    f->dra[0x3400 + 84 * 32 + 28] = 3;
    for (Uint16 i = 0; i < 247; ++i)
        TestCharacter_Put32(f->dra + 0x101b8 + i * 4U, 0x800a7000);
    for (Uint16 i = 0; i < 20; ++i)
        TestCharacter_Put32(f->dra + 0x7000 + i * 4U, 0x00010002);
    TestCharacter_Put32(f->dra + 0x7050, 0xffff);
    f->types[1] = 3;
    for (Uint16 y = 10; y < 16; ++y)
        for (Uint16 x = 0; x < 64; ++x)
            f->tiles[(y * 64U + x) * 2U] = 1;
    CHECK(SotnCharacter_Create((SotnDataView){f->dra, sizeof(f->dra)}, &f->character));
    for (Uint16 i = 0; i <= spare; ++i)
        CHECK(SotnCharacter_Collect(f->character, false, 166));
    CHECK(SotnCharacter_Equip(f->character, hand, 166));
    if (duplicator)
        CHECK(SotnCharacter_Collect(f->character, true, 84) &&
              SotnCharacter_Equip(f->character, duplicator, 84));
    CHECK(SotnPlayer_Create((SotnDataView){f->dra, sizeof(f->dra)},
                            (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, 2}, 64, 16},
                            256, 135, &f->p));
    CHECK(SotnPlayer_SetEquipment(f->p, (SotnDataView){0}, hand ? 0 : 166, hand ? 166 : 0));
    CHECK(SotnPlayer_SetTeleportContext(f->p, Factory, Use, f, 0));
    return true;
}
static bool Consumption(Fixture *f)
{
    for (Uint16 hand = 0; hand < 2; ++hand)
        for (Uint16 spare = 0; spare < 3; ++spare)
            for (Uint16 a = 0; a < 3; ++a)
            {
                CHECK(Create(f, hand, spare, a ? (Uint16)(a + 4) : 0));
                f->p->curse_ticks = 30;
                CHECK(SotnPlayer_Tick(f->p, hand ? SOTN_BUTTON_RIGHT_HAND : SOTN_BUTTON_LEFT_HAND));
                SotnCharacterState profile = {0};
                CHECK(SotnCharacter_Get(f->character, &profile, NULL));
                CHECK(f->p->step == SOTN_PLAYER_TELEPORT && f->p->animation.id == 0xf4 &&
                      f->p->teleport.used && f->uses == 1 && !f->requests &&
                      profile.hand_count[166] == (a       ? spare
                                                  : spare ? spare - 1
                                                          : 0) &&
                      profile.equipment[hand] == (a || spare ? 166 : 0));
                for (Uint16 i = 0; i < 8; ++i)
                    CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_JUMP | SOTN_BUTTON_RIGHT));
                CHECK(f->uses == 1 && !f->p->teleport.used);
            }
    return true;
}
static bool Boundaries(Fixture *f)
{
    CHECK(Create(f, 0, 0, 0));
    f->p->step = SOTN_PLAYER_WALK;
    f->p->vx = 98304;
    const Sint32 x = f->p->x.value;
    CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_LEFT_HAND | SOTN_BUTTON_RIGHT));
    CHECK(f->p->step == SOTN_PLAYER_TELEPORT && f->p->vx == 98304 && f->p->x.value == x + 98304);
    for (Uint16 i = 0; i < SOTN_PLAYER_TIMERS; ++i)
        f->p->timers[i] = (Uint16)(100 + i);
    f->p->hp = 50;
    f->p->healing.pending = 8;
    f->p->healing.pending_kind = 2;
    const Uint16 buttons = f->p->buttons, previous = f->p->previous_buttons;
    Uint16 timers[SOTN_PLAYER_TIMERS];
    SDL_memcpy(timers, f->p->timers, sizeof(timers));
    CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_JUMP | SOTN_BUTTON_RIGHT));
    CHECK(!f->p->vx && !f->p->vy && !SDL_memcmp(timers, f->p->timers, sizeof(timers)) &&
          f->p->hp == 50 && f->p->healing.pending == 8 && !f->p->healing.applied &&
          f->p->buttons == buttons && f->p->previous_buttons == previous &&
          f->p->simulated_pad.remaining == 4 && !f->p->simulated_pad.buttons);
    for (Uint16 i = 0; i < 8; ++i)
        CHECK(SotnPlayerFactory_Begin(&f->runtime.factories[i].cursor, (SotnPlayerFactorySource){0},
                                      0, 0));
    f->p->animation.pose = 8;
    f->p->animation.remaining = 1;
    CHECK(SotnPlayer_Tick(f->p, 0) && f->requests == 1 && f->p->animation.pose == 8 &&
          f->p->animation.remaining == 1);
    SDL_zero(f->runtime.factories[3]);
    CHECK(SotnPlayer_Tick(f->p, 0) && f->requests == 2 && f->p->animation.pose == 9 &&
          f->runtime.factories[3].cursor.blueprint == 121 && f->uses == 1);
    CHECK(Create(f, 0, 0, 0));
    f->reject = true;
    const SotnPlayer before = *f->p;
    SotnCharacterState old = {0}, after = {0};
    CHECK(SotnCharacter_Get(f->character, &old, NULL));
    CHECK(!SotnPlayer_Tick(f->p, SOTN_BUTTON_LEFT_HAND) &&
          !SDL_memcmp(&before, f->p, sizeof(before)) && !f->uses &&
          SotnCharacter_Get(f->character, &after, NULL) && !SDL_memcmp(&old, &after, sizeof(old)));
    f->reject = false;
    TestCharacter_Put32(f->dra + 0x101b8 + 0xf4 * 4U, 0x800bffff);
    CHECK(!SotnPlayer_Tick(f->p, SOTN_BUTTON_LEFT_HAND) && !f->uses);
    CHECK(Create(f, 0, 0, 0));
    f->p->step = SOTN_PLAYER_JUMP;
    CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_LEFT_HAND) && !f->uses &&
          f->p->step != SOTN_PLAYER_TELEPORT);
    for (Uint16 substep = 1; substep < 6; substep += 2)
    {
        CHECK(Create(f, 0, 0, 0) && SotnPlayer_BeginTeleportArrival(f->p, substep));
        CHECK(f->p->animation.id == 0xf5 && f->p->substep == substep);
        CHECK(SotnPlayer_Tick(f->p, 0) && f->p->step == SOTN_PLAYER_TELEPORT && !f->requests);
        CHECK(SotnPlayer_CommitTeleport(f->p, 0, true) && SotnPlayer_Tick(f->p, 0) &&
              f->p->step == SOTN_PLAYER_STAND && f->p->simulated_pad.remaining == 4);
    }
    return true;
}
static void Put16(Uint8 *p, Uint16 v)
{
    p[0] = (Uint8)v;
    p[1] = (Uint8)(v >> 8);
}
static bool Routes(Fixture *f)
{
    for (Uint8 mode = 4; mode <= 6; ++mode)
    {
        Uint8 *p = f->dra + 0x245c + (37U + mode - 4U) * 10U;
        Put16(p, 31);
        Put16(p + 2, 123);
        Put16(p + 4, 40);
        Put16(p + 6, 31);
        Put16(p + 8, mode == 4 ? 43 : mode == 5 ? 11 : 2);
        SotnStageRoute r = {0};
        CHECK(SotnStageRoute_Teleport((SotnDataView){f->dra, sizeof(f->dra)}, mode, &r) &&
              r.source == 31 && r.room == 5 && r.x == 31 && r.y == 123);
        const SotnStageRoute old = r;
        Put16(p + 8, 12);
        CHECK(!SotnStageRoute_Teleport((SotnDataView){f->dra, sizeof(f->dra)}, mode, &r) &&
              !SDL_memcmp(&old, &r, sizeof(r)));
    }
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok = Consumption(f) && Boundaries(f) && Routes(f);
    SotnPlayer_Close(f->p);
    SotnCharacter_Destroy(f->character);
    SDL_free(f);
    return ok ? 0 : 1;
}
