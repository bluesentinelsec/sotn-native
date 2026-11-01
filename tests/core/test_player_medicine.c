#include "character_fixture.h"
#include "player_internal.h"
#include <sotn/bat_runtime.h>
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
static bool Factory(void *context, SotnPlayerFactorySource source, Uint32 packed, Uint16 item,
                    bool *allocated)
{
    Fixture *f = context;
    ++f->requests;
    return SotnBatRuntime_RequestMedicine(&f->runtime, source, packed, item, allocated);
}
static bool Consume(void *context, Uint16 hand, Uint16 item, Uint16 *equipped)
{
    Fixture *f = context;
    if (f->reject)
        return SDL_SetError("Authored consumption rejection");
    if (!SotnCharacter_UseMedicine(f->character, hand, item, equipped))
        return false;
    ++f->uses;
    return true;
}
static bool Create(Fixture *f, Uint16 item, Uint16 hand, Uint16 spare, Uint16 duplicator)
{
    SotnPlayer_Close(f->p);
    SotnCharacter_Destroy(f->character);
    SDL_zero(*f);
    TestCharacter_Fill(f->dra);
    f->dra[0x500 + 18] = 0x24;
    for (Uint16 id = 144; id <= 162; ++id)
    {
        Uint8 *e = f->dra + 0x500 + id * 52;
        e[15] = 255;
        e[17] = 131;
        e[18] = 0xbe;
        e[19] = 55;
        e[20] = (Uint8)(id < 157 ? id - 144 : id - 143);
        e[25] = 1;
    }
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
        CHECK(SotnCharacter_Collect(f->character, false, item));
    CHECK(SotnCharacter_Equip(f->character, hand, item));
    if (duplicator)
        CHECK(SotnCharacter_Collect(f->character, true, 84) &&
              SotnCharacter_Equip(f->character, duplicator, 84));
    CHECK(SotnPlayer_Create((SotnDataView){f->dra, sizeof(f->dra)},
                            (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, 2}, 64, 16},
                            256, 135, &f->p));
    CHECK(SotnPlayer_SetEquipment(f->p, (SotnDataView){0}, hand ? 0 : item, hand ? item : 0) &&
          SotnPlayer_SetMedicineContext(f->p, Factory, Consume, f, false));
    return true;
}
static bool Commands(Fixture *f)
{
    for (Uint16 item = 144; item <= 162; ++item)
        for (Uint16 hand = 0; hand < 2; ++hand)
            for (Uint16 spare = 0; spare < 2; ++spare)
                for (Uint16 dup = 0; dup < 3; ++dup)
                {
                    if (item == 146 || item == 147)
                        continue;
                    CHECK(Create(f, item, hand, spare, dup ? dup + 4 : 0));
                    f->p->curse_ticks = 30;
                    const Uint16 button = hand ? SOTN_BUTTON_RIGHT_HAND : SOTN_BUTTON_LEFT_HAND;
                    CHECK(SotnPlayer_Tick(f->p, button) && f->uses == 1 && f->requests == 1 &&
                          f->p->medicine.used && !f->p->curse_failure && !f->p->attack_mode &&
                          !f->p->weapon_startup && f->p->animation.id == 0xbe);
                    const SotnPlayerFactory *root = &f->runtime.factories[0].cursor;
                    CHECK(root->blueprint == 55 && root->argument == item &&
                          root->parameter ==
                              (Uint16)(((item < 157 ? item - 144 : item - 143) + hand * 128) << 8));
                    SotnCharacterState c = {0};
                    CHECK(SotnCharacter_Get(f->character, &c, NULL));
                    CHECK(c.hand_count[item] == (dup ? spare : 0) &&
                          c.equipment[hand] == (dup || spare ? item : 0));
                    for (Uint16 i = 0; i < 3; ++i)
                        CHECK(SotnPlayer_Tick(f->p, button) && f->uses == 1 &&
                              !f->p->medicine.used);
                }
    for (Uint16 state = 1; state <= 4; ++state)
    {
        CHECK(Create(f, 159, 0, 0, 0));
        f->p->step = (SotnPlayerStep)state;
        f->p->contact = state >= 3 ? 0 : 1;
        f->p->vx = state == 1 ? 98304 : 0;
        f->p->vy = state == 3 ? -65536 : state == 4 ? 65536 : 0;
        f->p->x.value += 123;
        f->p->y.value += 456;
        const Sint32 origin_x = f->p->x.value, origin_y = f->p->y.value;
        CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_LEFT_HAND | (state == 1 ? SOTN_BUTTON_RIGHT : 0)) &&
              f->p->medicine.used && f->uses == 1 && f->p->step == (SotnPlayerStep)state &&
              !f->p->attack_mode && f->runtime.factories[0].cursor.source.x == origin_x &&
              f->runtime.factories[0].cursor.source.y == origin_y);
    }
    return true;
}
static bool Boundaries(Fixture *f)
{
    CHECK(Create(f, 159, 0, 0, 0));
    SotnCharacterState owned = {0}, rejected = {0};
    CHECK(SotnCharacter_Get(f->character, &owned, NULL));
    owned.hand_count[146] = owned.hand_count[147] = 1;
    CHECK(SotnCharacter_Restore(f->character, &owned));
    for (Uint16 item = 146; item <= 147; ++item)
        for (Uint16 hand = 0; hand < 2; ++hand)
            CHECK(!SotnCharacter_Equip(f->character, hand, item) &&
                  SotnCharacter_Get(f->character, &rejected, NULL) &&
                  !SDL_memcmp(&owned, &rejected, sizeof(owned)));
    for (Uint16 item = 146; item <= 147; ++item)
        for (Uint16 hand = 0; hand < 2; ++hand)
        {
            CHECK(SotnPlayer_SetEquipment(f->p, (SotnDataView){0}, hand ? 0 : item,
                                          hand ? item : 0) &&
                  SotnPlayer_Tick(f->p, 0) &&
                  SotnPlayer_Tick(f->p, hand ? SOTN_BUTTON_RIGHT_HAND : SOTN_BUTTON_LEFT_HAND) &&
                  !f->uses && !f->requests && !f->p->medicine.used);
        }
    CHECK(Create(f, 159, 0, 0, 0));
    for (Uint16 i = 0; i < 8; ++i)
        CHECK(SotnPlayerFactory_Begin(&f->runtime.factories[i].cursor, (SotnPlayerFactorySource){0},
                                      0, 0));
    CHECK(!SotnBatRuntime_MedicinePresent(&f->runtime) &&
          SotnPlayer_Tick(f->p, SOTN_BUTTON_LEFT_HAND) && !f->uses && !f->p->medicine.used);
    SDL_zero(f->runtime.factories[3]);
    CHECK(SotnPlayer_Tick(f->p, 0) && SotnPlayer_Tick(f->p, SOTN_BUTTON_LEFT_HAND) && f->uses == 1);
    CHECK(Create(f, 159, 0, 0, 0));
    f->runtime.children[16].kind = 39;
    CHECK(SotnBatRuntime_MedicinePresent(&f->runtime) &&
          SotnPlayer_SetMedicineContext(f->p, Factory, Consume, f, true) &&
          SotnPlayer_Tick(f->p, SOTN_BUTTON_LEFT_HAND) && !f->uses && !f->requests);
    CHECK(Create(f, 159, 0, 0, 0));
    f->reject = true;
    const SotnPlayer old = *f->p;
    SotnCharacterState before = {0}, after = {0};
    CHECK(SotnCharacter_Get(f->character, &before, NULL) &&
          !SotnPlayer_Tick(f->p, SOTN_BUTTON_LEFT_HAND) && !SDL_memcmp(&old, f->p, sizeof(old)) &&
          !f->uses && SotnCharacter_Get(f->character, &after, NULL) &&
          !SDL_memcmp(&before, &after, sizeof(before)));
    CHECK(Create(f, 159, 0, 0, 0));
    TestCharacter_Put32(f->dra + 0x101b8 + 0xbe * 4U, 0x800bffff);
    CHECK(!SotnPlayer_Tick(f->p, SOTN_BUTTON_LEFT_HAND) && !f->uses && f->requests == 1);
    return true;
}
static bool FamiliarFactory(Fixture *f)
{
    CHECK(Create(f, 159, 0, 1, 5));
    SotnCharacterState before = {0}, after = {0};
    CHECK(SotnCharacter_Get(f->character, &before, NULL));
    for (Uint16 profile = 0; profile < 20; ++profile)
    {
        SDL_zero(f->runtime);
        const SotnPlayerFactorySource source = {.x = SDL_MIN_SINT32 + 0xabcd,
                                                .y = SDL_MAX_SINT32 - 0x1234,
                                                .flags = 0x0c030000,
                                                .priority = 511,
                                                .facing_left = (profile & 1U) != 0};
        const Uint32 packed = 55U | (Uint32)profile << 16;
        bool allocated = false;
        for (Uint16 slot = 0; slot < 8; ++slot)
        {
            CHECK(SotnBatRuntime_RequestFamiliarMedicine(&f->runtime, source, packed, &allocated) &&
                  allocated);
            const SotnPlayerFactory root = f->runtime.factories[slot].cursor;
            CHECK(root.blueprint == 55 && root.entity_id == 1 && !root.argument &&
                  root.parameter == profile * 256 && root.source.x == source.x &&
                  root.source.y == source.y && root.source.priority == 511 &&
                  root.source.flags == 0x10000 && root.source.facing_left == source.facing_left &&
                  !root.initialized);
        }
        SotnPlayerFactory held[8] = {0};
        for (Uint16 i = 0; i < 8; ++i)
            held[i] = f->runtime.factories[i].cursor;
        CHECK(SotnBatRuntime_RequestFamiliarMedicine(&f->runtime, source, packed, &allocated) &&
              !allocated);
        const Uint32 invalid[] = {54, 55U | 20U << 16, 55U | 128U << 16, 0x1000037};
        for (Uint16 i = 0; i < SDL_arraysize(invalid); ++i)
        {
            allocated = true;
            CHECK(!SotnBatRuntime_RequestFamiliarMedicine(&f->runtime, source, invalid[i],
                                                          &allocated) &&
                  allocated);
        }
        for (Uint16 i = 0; i < 8; ++i)
            CHECK(!SDL_memcmp(&held[i], &f->runtime.factories[i].cursor, sizeof(held[i])));
    }
    CHECK(SotnCharacter_Get(f->character, &after, NULL) &&
          !SDL_memcmp(&before, &after, sizeof(before)));
    bool allocated = true;
    CHECK(!SotnBatRuntime_RequestMedicine(&f->runtime, (SotnPlayerFactorySource){0}, 55U | 3U << 16,
                                          147, &allocated) &&
          allocated);
    return true;
}
static bool Resources(Fixture *f)
{
    CHECK(Create(f, 159, 0, 0, 0));
    f->p->hp = 10;
    f->p->mp = 3;
    f->p->healing.pending = 8;
    f->p->healing.pending_kind = 2;
    f->p->invincibility = 100;
    const SotnMedicineEffectEvents e = {
        .heal = true, .heal_amount = 50, .cure_mask = 3, .protection = 64, .mp_full = true};
    CHECK(SotnPlayer_CommitMedicine(f->p, &e, 20) && f->p->hp == 10 &&
          f->p->healing.pending == 50 && f->p->healing.pending_kind == 1 && f->p->mp == 20 &&
          f->p->timers[0] == 2 && f->p->timers[1] == 2 && f->p->invincibility == 100 &&
          SotnPlayer_Tick(f->p, 0) && f->p->hp == 60 && f->p->healing.applied_amount == 50);
    f->p->step = SOTN_PLAYER_KILL;
    f->p->hp = 0;
    f->p->mp = 3;
    CHECK(SotnPlayer_CommitMedicine(f->p, &e, 20) && !f->p->hp && f->p->step == SOTN_PLAYER_KILL &&
          f->p->healing.pending == 50 && f->p->mp == 20);
    CHECK(Create(f, 159, 0, 0, 0));
    f->p->step = SOTN_PLAYER_STONE;
    f->p->substep = 2;
    f->p->stone.active = true;
    f->p->stone.shakes = 5;
    const SotnMedicineEffectEvents hammer = {.hammer = true};
    CHECK(SotnPlayer_CommitMedicine(f->p, &hammer, 20) && f->p->hammer_timer == 64 &&
          f->p->stone.shakes == 1);
    CHECK(SotnPlayer_Tick(f->p, 0) && f->p->hammer_timer == 63 && f->p->step == SOTN_PLAYER_HIT &&
          f->p->substep == 6);
    CHECK(SotnPlayer_RestoreHammer(f->p, 32));
    const SotnPlayer held = *f->p;
    CHECK(!SotnPlayer_RestoreHammer(f->p, 65) && !SDL_memcmp(&held, f->p, sizeof(held)));
    CHECK(!SotnPlayer_CommitMedicine(f->p, &hammer, 20) && !SDL_memcmp(&held, f->p, sizeof(held)));
    return true;
}
static bool Cursor(Fixture *f)
{
    Uint8 *r = f->dra + 0xdef0;
    SDL_memset(r, 0, 28);
    r[0] = 2;
    r[1] = 3;
    r[2] = 255;
    r[8] = 53;
    r[9] = 40;
    r[24] = 0x84;
    SotnMedicineEffect n = {0};
    SotnMedicineEffectEvents e = {0};
    const SotnMedicineEffectInput in = {
        .dra = {f->dra, sizeof(f->dra)}, .hp_max = 70, .enhanced_potion = true};
    CHECK(SotnMedicineEffect_Tick(&n, in, &e) && n.timer == 2 && e.protection == 64 &&
          e.sound_count == 1);
    CHECK(SotnMedicineEffect_Tick(&n, in, &e) && n.timer == 1 && !e.factory_count);
    CHECK(SotnMedicineEffect_Tick(&n, in, &e) && e.factory_count == 1 && e.factories[0] == 53 &&
          n.timer == 3);
    CHECK(SotnMedicineEffect_Tick(&n, in, &e) && SotnMedicineEffect_Tick(&n, in, &e) && !e.heal);
    CHECK(SotnMedicineEffect_Tick(&n, in, &e) && n.phase == 2 && e.heal && e.heal_amount == 75 &&
          e.sounds[0] == 0x67d);
    CHECK(SotnMedicineEffect_Tick(&n, in, &e) && n.removed && !e.heal && !e.sound_count);
    n = (SotnMedicineEffect){.phase = 1, .timer = SDL_MIN_SINT16};
    CHECK(SotnMedicineEffect_Tick(&n, in, &e) && n.timer == SDL_MAX_SINT16 && !e.heal &&
          !e.factory_count);
    const SotnMedicineEffect held = n;
    CHECK(!SotnMedicineEffect_Tick(&n, (SotnMedicineEffectInput){0}, &e) &&
          !SDL_memcmp(&held, &n, sizeof(n)));
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok = Commands(f) && Boundaries(f) && FamiliarFactory(f) && Resources(f) && Cursor(f);
    SotnPlayer_Close(f->p);
    SotnCharacter_Destroy(f->character);
    SDL_free(f);
    return ok ? 0 : 1;
}
