#include "actors_internal.h"
#include "character_fixture.h"
#include "player_internal.h"
#include <sotn/combat.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s: %s", __LINE__, #v, SDL_GetError());                               \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
typedef struct Fixture
{
    Uint8 dra[0x11000], tiles[32 * 16 * 2], types[2];
    SotnPlayer *player;
    SotnCharacter *character;
} Fixture;
static bool Capture(void *context, Uint32 generation)
{
    (void)context;
    (void)generation;
    return true;
}
static bool Decay(void *context, Uint32 generation, bool *done)
{
    (void)context;
    (void)generation;
    *done = false;
    return true;
}
static bool Create(Fixture *f)
{
    SDL_memset(f, 0, sizeof(*f));
    TestCharacter_Fill(f->dra);
    for (Uint16 i = 0; i < 247; ++i)
        TestCharacter_Put32(f->dra + 0x101b8 + i * 4U, 0x800a0200);
    for (Uint16 i = 0; i < 32; ++i)
        TestCharacter_Put32(f->dra + 0x200 + i * 4U, 3U | ((Uint32)i + 1U) << 16);
    TestCharacter_Put32(f->dra + 0x280, 65535);
    f->dra[0x3400 + 28 * 32U + 28] = 0;
    for (Uint16 i = 38; i <= 43; ++i)
        f->dra[0x3400 + i * 32U + 28] = 0;
    f->dra[0x3400 + 54 * 32U + 28] = 2;
    CHECK(SotnCharacter_Create((SotnDataView){f->dra, sizeof(f->dra)}, &f->character));
    for (Uint16 id = 38; id <= 43; ++id)
        CHECK(SotnCharacter_Collect(f->character, true, id) &&
              SotnCharacter_Equip(f->character, SOTN_EQUIP_HEAD, id));
    SotnCharacterState profile = {0};
    SotnCharacterStats attributes = {0};
    CHECK(SotnCharacter_Get(f->character, &profile, &attributes));
    profile.hp = 40;
    CHECK(SotnCharacter_Restore(f->character, &profile));
    f->types[1] = 3;
    for (Uint16 y = 10; y < 16; ++y)
        for (Uint16 x = 0; x < 32; ++x)
            f->tiles[(y * 32U + x) * 2U] = 1;
    CHECK(SotnPlayer_Create(
        (SotnDataView){f->dra, sizeof(f->dra)},
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 16}, 100,
        135, &f->player));
    f->player->fixed_stage = true;
    CHECK(SotnPlayer_SetCharacter(f->player, &profile, attributes) && f->player->head == 43);
    CHECK(SotnPlayer_SetDeathCapture(f->player, Capture, Decay, f));
    f->player->head = 26;
    f->player->attributes = (SotnCharacterStats){0};
    f->player->hp = 70;
    return true;
}
static void Close(Fixture *f)
{
    SotnPlayer_Close(f->player);
    SotnCharacter_Destroy(f->character);
}
static bool Contact(SotnPlayer *p, Uint32 clock, Uint16 attack, Uint16 parameters)
{
    return SotnPlayer_QueueContact(
               p, (SotnPlayerContact){.attack = attack, .parameters = parameters}) &&
           SotnPlayer_SetDamageContext(p, clock) && SotnPlayer_Tick(p, 0);
}
static bool OwnedAndOrdering(Fixture *f, SotnDataView exe)
{
    CHECK(Create(f) && SotnPlayer_SetScriptContext(f->player, exe));
    SotnPlayer *p = f->player;
    SotnCharacterState c = {0};
    SotnCharacterStats stats = {0};
    CHECK(!SotnCharacter_Equip(f->character, SOTN_EQUIP_HEAD, 28) &&
          SotnCharacter_Collect(f->character, true, 28) &&
          SotnCharacter_Equip(f->character, SOTN_EQUIP_HEAD, 28) &&
          SotnCharacter_Get(f->character, &c, &stats) && SotnPlayer_SetCharacter(p, &c, stats) &&
          p->head == 28);
    p->hp = 70;
    p->attributes = (SotnCharacterStats){0};
    const SotnPlayer base = *p;
    // Individual contact bits use a fixed attack5: selected bits reduce it to4,
    // while the nonselected bits preserve5. No private item/profile data is copied.
    const Uint16 reduced[] = {0x80, 0x100, 0x800, 0x1000, 0x2000, 0x4000, 0x8000};
    for (Uint16 i = 0; i < SDL_arraysize(reduced); ++i)
    {
        *p = base;
        CHECK(Contact(p, 1, 5, reduced[i] | 1U) && p->hp == 66);
    }
    const Uint16 ordinary[] = {0, 0x20, 0x40, 0x400};
    for (Uint16 i = 0; i < SDL_arraysize(ordinary); ++i)
    {
        *p = base;
        CHECK(Contact(p, 1, 5, ordinary[i] | 1U) && p->hp == 65);
    }
    const Uint16 attack[] = {1, 4, 5, 6, 9, 10, 11, 19, 20},
                 hp[] = {69, 66, 66, 65, 62, 62, 61, 54, 54};
    for (Uint16 i = 0; i < SDL_arraysize(attack); ++i)
    {
        *p = base;
        CHECK(Contact(p, 1, attack[i], 0x8001) && p->hp == hp[i]);
    }
    *p = base;
    CHECK(Contact(p, 1, 10, 0xf981) && p->hp == 62); // Multiple bits still reduce once.
    *p = base;
    p->hp = 5;
    CHECK(Contact(p, 1, 5, 0x8001) && p->hp == 1 && p->step != SOTN_PLAYER_KILL);
    *p = base;
    p->attributes.weakness = 0x8000;
    CHECK(Contact(p, 1, 3, 0x8001) && p->hp == 65);
    *p = base;
    p->attributes.resistance = 0x8000;
    CHECK(Contact(p, 1, 8, 0x8001) && p->hp == 66);
    *p = base;
    p->attributes.defense = 3;
    p->poison_ticks = 40;
    CHECK(Contact(p, 1, 11, 0x8001) && p->hp == 58 && p->poison_ticks == 39);
    *p = base;
    p->attributes.immunity = 0x8000;
    CHECK(Contact(p, 1, 10, 0x8001) && p->hp == 70 && p->immunity.applied);
    *p = base;
    p->hp = 60;
    p->attributes.absorption = p->attributes.weakness = 0x8000;
    CHECK(Contact(p, 1, 5, 0x8001) && p->hp == 70 && p->absorption.amount == 10);
    *p = base;
    p->attributes.immunity = 0x200;
    CHECK(Contact(p, 1, 10, 0x8201) && p->hp == 62 && p->step != SOTN_PLAYER_STONE);
    *p = base;
    p->attributes.defense = 3;
    CHECK(Contact(p, 1, 10, 0x8201) && p->hp == 68 && p->step == SOTN_PLAYER_STONE);
    *p = base;
    p->step = SOTN_PLAYER_STONE;
    p->stone.active = true;
    bool born = false;
    CHECK(SotnPlayer_QueueContact(p, (SotnPlayerContact){.attack = 10, .parameters = 0x8001}) &&
          SotnPlayer_SetDamageContext(p, 1) && SotnPlayer_ContactTick(p, &born) && p->hp == 62);
    *p = base;
    CHECK(Contact(p, 1, 5, 0x8010) && p->hp == 62);
    *p = base;
    CHECK(SotnPlayer_SetWaterContext(p, (SotnPlayerWater){41, 25, 0}) &&
          Contact(p, 10, 5, 0x8000) && p->hp == 69 && p->water_pain.events.response == 9);
    *p = base;
    p->attributes.defense = 100;
    CHECK(Contact(p, 1, 101, 0x8001) && p->hp == 69 && p->guard.applied);
    *p = base;
    p->head = 26;
    p->attributes.defense = 100;
    CHECK(Contact(p, 1, 101, 0x8001) && p->hp == 69 && !p->guard.applied);
    *p = base;
    p->talismans = 1;
    p->damage_random = 0;
    CHECK(Contact(p, 1, 9999, 0x8001) && p->hp == 70 && p->dodge.applied &&
          p->damage_random == 0x3039);
    Close(f);
    return true;
}
static bool CommitAndPriority(Fixture *f, SotnDataView exe)
{
    CHECK(Create(f) && SotnPlayer_SetScriptContext(f->player, exe));
    SotnPlayer *p = f->player;
    p->head = 28;
    CHECK(SotnPlayer_QueueContact(p, (SotnPlayerContact){.attack = 10, .parameters = 0x8001}) &&
          SotnPlayer_QueueContact(p, (SotnPlayerContact){.attack = 9, .parameters = 1}) &&
          Contact(p, 1, 10, 1) && p->hp == 62);
    Close(f);
    CHECK(Create(f) && SotnPlayer_SetScriptContext(f->player, exe));
    p = f->player;
    p->head = 28;
    p->hp = 60;
    p->armor = 22;
    p->cape = 54;
    p->x.parts.low = 123;
    p->y.parts.low = 456;
    SotnStatBuffs buffs = {0};
    CHECK(SotnPlayer_SetStatBuffContext(p, &buffs, p->attributes) &&
          SotnPlayer_QueueBloodHealing(p, 8, 1));
    const Sint32 x = p->x.value, y = p->y.value;
    CHECK(SotnPlayer_QueueContact(p, (SotnPlayerContact){.attack = 5, .parameters = 0x8001}) &&
          SotnPlayer_SetDamageContext(p, 1) && SotnPlayer_TickSpellFreeze(p, 0, false) &&
          p->hp == 60 && p->healing.pending == 8 && p->queued_contact.attack == 5);
    CHECK(SotnPlayer_TickSpellFreeze(p, 0, true) && p->hp == 60 && p->healing.pending == 8 &&
          p->queued_contact.attack == 5);
    Uint8 animations[247 * 4] = {0};
    SDL_memcpy(animations, f->dra + 0x101b8, sizeof(animations));
    for (Uint16 i = 0; i < 247; ++i)
        TestCharacter_Put32(f->dra + 0x101b8 + i * 4U, 0x801bffff);
    const SotnPlayer before = *p;
    CHECK(!SotnPlayer_Tick(p, 0) && !SDL_memcmp(&before, p, sizeof(*p)));
    SDL_memcpy(f->dra + 0x101b8, animations, sizeof(animations));
    CHECK(SotnPlayer_Tick(p, 0) && p->hp == 64 && p->healing.applied_amount == 8 &&
          p->heart_gain.requested && p->heart_gain.amount == 4 && p->heart_gain.source.x == x &&
          p->heart_gain.source.y == y && p->defense_buff_minimum == 512 &&
          !p->queued_contact.attack && !buffs.timers[0]);
    Close(f);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    Uint8 *exe = SDL_calloc(1, 0x24000);
    if (!f || !exe)
        return 1;
    for (Uint16 i = 0; i <= 1024; ++i)
    {
        const Uint16 value = i * 4U;
        exe[0x1dbcc + i * 2U] = (Uint8)value;
        exe[0x1dbcd + i * 2U] = (Uint8)(value >> 8);
    }
    for (Uint16 i = 0; i < 192; ++i)
        exe[0x1e3e1 + i * 2U] = 16;
    const SotnDataView data = {exe, 0x24000};
    const bool ok = OwnedAndOrdering(f, data) && CommitAndPriority(f, data);
    SDL_free(exe);
    SDL_free(f);
    return ok ? 0 : 1;
}
