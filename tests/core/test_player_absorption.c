#include "character_fixture.h"
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
static bool ProtectionAndOrdering(Fixture *f)
{
    CHECK(Create(f));
    SotnPlayer *p = f->player;
    CHECK(SotnCharacter_Collect(f->character, true, 22) &&
          SotnCharacter_Equip(f->character, SOTN_EQUIP_ARMOR, 22) &&
          SotnCharacter_Collect(f->character, true, 54) &&
          SotnCharacter_Equip(f->character, SOTN_EQUIP_CAPE, 54));
    SotnCharacterState profile = {0};
    SotnCharacterStats attributes = {0};
    SotnStatBuffs buffs = {0};
    CHECK(SotnCharacter_Get(f->character, &profile, &attributes) &&
          SotnPlayer_SetCharacter(p, &profile, attributes) &&
          SotnPlayer_SetStatBuffContext(p, &buffs, attributes) && p->armor == 22 && p->cape == 54);
    p->x.parts.low = 123;
    p->y.parts.low = 456;
    const Sint32 x = p->x.value, y = p->y.value;
    const Uint32 random = p->damage_random;
    CHECK(Contact(p, 1, 3, 7) && p->hp == 46 && p->absorption.applied &&
          p->absorption.amount == 6 && p->invincibility == 12 && p->damage_random == random &&
          p->absorption.source.x == x && p->absorption.source.y == y && !p->sound &&
          !p->extra_sound && p->step != SOTN_PLAYER_HIT && !p->heart_gain.requested &&
          !p->defense_buff_minimum);
    const SotnCollisionMap map = p->map;
    CHECK(SotnPlayer_ChangeRoom(p, map, 1, 2) && p->absorption.source.x == x + 65536 &&
          p->absorption.source.y == y + 2 * 65536 && p->absorption.amount == 6);
    CHECK(SotnPlayer_QueueContact(p, (SotnPlayerContact){.attack = 3, .parameters = 7}));
    for (Uint16 i = 0; i < 3; ++i)
        CHECK(SotnPlayer_TickSpellFreeze(p, 0, false) && p->hp == 46 && p->invincibility == 12 &&
              !p->absorption.applied && p->queued_contact.attack == 3);
    for (Uint16 clock = 2; clock <= 12; ++clock)
        CHECK(Contact(p, clock, 3, 7) && p->hp == 46 && !p->absorption.applied &&
              p->invincibility == 13 - clock && !p->queued_contact.attack);
    CHECK(Contact(p, 13, 3, 7) && p->hp == 52 && p->absorption.applied && p->invincibility == 12);
    p->invincibility = 0;
    p->hp = 40;
    CHECK(SotnPlayer_QueueBloodHealing(p, 8, 1) && Contact(p, 14, 3, 7) && p->hp == 54 &&
          p->healing.applied && p->healing.applied_amount == 8 && p->absorption.applied &&
          p->absorption.amount == 6);
    p->invincibility = 0;
    CHECK(SotnPlayer_QueueMailHealing(p) && Contact(p, 15, 3, 7) && p->hp == 61 &&
          p->healing.applied_kind == 2 && p->healing.applied_amount == 1 &&
          p->absorption.amount == 6);
    p->invincibility = 0;
    p->hp = p->hp_max;
    CHECK(Contact(p, 16, 3, 7) && p->hp == p->hp_max && p->absorption.applied &&
          p->absorption.amount == 6 && p->invincibility == 12);
    Close(f);
    return true;
}
static bool PriorityAndRollback(Fixture *f)
{
    CHECK(Create(f));
    SotnPlayer *p = f->player;
    p->attributes.weakness = 0x8000;
    CHECK(Contact(p, 1, 9999, 0x8007) && p->hp == 70 && p->absorption.amount == 39996);
    p->hp = 40;
    p->invincibility = 0;
    p->attributes.absorption = 0x8000;
    CHECK(Contact(p, 2, 3, 0x8007) && p->hp == 46 && p->absorption.amount == 6);
    p->attributes.weakness = 0;
    p->attributes.immunity = 0x8000;
    p->invincibility = 0;
    CHECK(Contact(p, 3, 3, 0x8007) && p->hp == 46 && !p->absorption.applied && !p->invincibility);
    p->attributes.immunity = 0x200;
    CHECK(Contact(p, 4, 3, 0x8207) && p->hp == 49 && p->absorption.amount == 3 &&
          p->step != SOTN_PLAYER_STONE);
    p->attributes.absorption = p->attributes.immunity = 0;
    p->attributes.resistance = 0x8000;
    p->invincibility = 0;
    CHECK(Contact(p, 5, 1, 0x8007) && p->hp == 50 && p->absorption.amount == 1);
    p->attributes.resistance = 0;
    p->invincibility = 0;
    p->hp = 40;
    CHECK(SotnPlayer_QueueBloodHealing(p, 8, 1) &&
          SotnPlayer_QueueContact(p, (SotnPlayerContact){.attack = 3, .parameters = 7}) &&
          SotnPlayer_SetDamageContext(p, 6));
    TestCharacter_Put32(f->dra + 0x101b8 + p->animation.id * 4U, 0x801bffff);
    const SotnPlayer before = *p;
    CHECK(!SotnPlayer_Tick(p, 0) && !SDL_memcmp(&before, p, sizeof(*p)));
    TestCharacter_Put32(f->dra + 0x101b8 + p->animation.id * 4U, 0x800a0200);
    CHECK(SotnPlayer_Tick(p, 0) && p->hp == 54 && p->absorption.applied &&
          p->absorption.amount == 6);
    p->invincibility = 0;
    p->head = 26;
    p->attributes.defense = 0;
    CHECK(Contact(p, 7, 1, 7) && p->hp == 53 && !p->absorption.applied);
    Close(f);
    CHECK(Create(f));
    p = f->player;
    p->head = 26;
    p->attributes.absorption = 0x80;
    p->poison_ticks = 20;
    p->curse_ticks = 20;
    CHECK(Contact(p, 1, 3, 0x81) && p->hp == 43 && p->absorption.amount == 3 &&
          p->poison_ticks == 19 && p->curse_ticks == 19 && !p->healing.applied);
    p->invincibility = 0;
    p->step = SOTN_PLAYER_STONE;
    p->absorption = (SotnPlayerAbsorption){0};
    p->queued_contact = (SotnPlayerContact){.attack = 3, .parameters = 0x81};
    p->damage_pending = true;
    bool born = false;
    CHECK(SotnPlayer_ContactTick(p, &born) && p->hp == 46 && p->step == SOTN_PLAYER_STONE &&
          p->absorption.applied && !born && p->invincibility == 12);
    p->step = SOTN_PLAYER_KILL;
    p->hp = 0;
    p->absorption = (SotnPlayerAbsorption){0};
    p->queued_contact = (SotnPlayerContact){.attack = 3, .parameters = 0x81};
    CHECK(SotnPlayer_ContactTick(p, &born) && !p->hp && !p->absorption.applied &&
          !p->queued_contact.attack);
    Close(f);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok = ProtectionAndOrdering(f) && PriorityAndRollback(f);
    SDL_free(f);
    return ok ? 0 : 1;
}
