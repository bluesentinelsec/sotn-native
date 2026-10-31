#include "player_internal.h"
#include <sotn/death_image.h>
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
    SotnDeathImage image;
} Fixture;
static void P32(Uint8 *p, Uint32 v)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (i * 8U));
}
static bool Capture(void *context, Uint32 generation)
{
    Fixture *f = context;
    Uint8 b[5120] = {0};
    (void)generation;
    SDL_memset(b, 0xab, sizeof(b));
    return SotnDeathImage_Begin(&f->image, (SotnDataView){b, sizeof(b)});
}
static bool Decay(void *context, Uint32 generation, bool *done)
{
    Fixture *f = context;
    (void)generation;
    if (!SotnDeathImage_Tick(&f->image))
        return false;
    *done = !f->image.remaining;
    return true;
}
static bool Create(Fixture *f, SotnPlayer **p)
{
    SDL_memset(f, 0, sizeof(*f));
    for (Uint16 i = 0; i < 247; ++i)
        P32(f->dra + 0x101b8 + i * 4U, 0x800a0100);
    for (Uint16 i = 0; i < 32; ++i)
        P32(f->dra + 0x100 + i * 4U, 3U | ((Uint32)i + 1U) << 16);
    P32(f->dra + 0x180, 65535);
    P32(f->dra + 0x101b8 + 0xc1U * 4U, 0x800a0200);
    for (Uint16 i = 0; i < 20; ++i)
        P32(f->dra + 0x200 + i * 4U, 1U | ((Uint32)i + 1U) << 16);
    P32(f->dra + 0x250, 65535);
    f->types[1] = 3;
    for (Uint16 y = 10; y < 16; ++y)
        for (Uint16 x = 0; x < 32; ++x)
            f->tiles[(y * 32 + x) * 2] = 1;
    CHECK(SotnPlayer_Create(
        (SotnDataView){f->dra, sizeof(f->dra)},
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 16}, 100,
        135, p));
    (*p)->fixed_stage = true;
    (*p)->hp = 10;
    CHECK(SotnPlayer_SetDeathCapture(*p, Capture, Decay, f));
    return true;
}
static bool Tick(SotnPlayer *p, Uint32 clock)
{
    return SotnPlayer_SetDamageContext(p, clock) && SotnPlayer_Tick(p, 0);
}
static bool Water(Fixture *f)
{
    SotnPlayer *p = NULL;
    CHECK(Create(f, &p));
    CHECK(SotnPlayer_SetWaterContext(p, (SotnPlayerWater){41, 25, 0}));
    CHECK(Tick(p, 10) && p->hp == 9 && p->water_pain.events.response == 9 &&
          p->water_pain.events.blink == 0x4e && p->water_pain.events.voice &&
          p->water_pain.events.scrape && p->water_damage.pain_timer == 32);
    const SotnPlayer consumed = *p;
    CHECK(!SotnPlayer_Tick(p, 0) && !SDL_memcmp(p, &consumed, sizeof(*p)));
    CHECK(!SotnPlayer_SetDamageContext(p, 10) && !SDL_memcmp(p, &consumed, sizeof(*p)));
    CHECK(SotnPlayer_QueueContact(p, (SotnPlayerContact){.attack = 99, .parameters = 0x2000}));
    CHECK(Tick(p, 11) && p->hp == 9 && !p->water_pain.events.voice &&
          !p->water_pain.events.scrape && p->water_pain.events.hit_params == 6 &&
          p->water_damage.pain_timer == 32);
    p->hp = 1;
    CHECK(SotnPlayer_QueueBloodHealing(p, 8, 1) && Tick(p, 20) && p->hp == 8 &&
          p->healing.applied && !p->healing.pending && !p->water_death.active);
    p->relics[14] = 3;
    CHECK(Tick(p, 30) && p->hp == 8 && !p->water_pain.events.response &&
          p->water_damage.pain_timer == 31);
    p->relics[14] = 1;
    CHECK(Tick(p, 40) && p->hp == 7 && p->water_damage.pain_timer == 32);
    p->invincibility = 2;
    CHECK(Tick(p, 50) && p->hp == 7 && !p->water_pain.events.response &&
          p->water_damage.pain_timer == 31);
    CHECK(Tick(p, 60) && p->hp == 6 && p->water_pain.events.response == 9);
    CHECK(Tick(p, SDL_MAX_UINT32) && p->hp == 6 && !p->water_pain.events.scrape && Tick(p, 0) &&
          p->hp == 5 && p->water_pain.events.scrape);
    CHECK(SotnPlayer_SetWaterContext(p, (SotnPlayerWater){0}) &&
          SotnPlayer_SetWaterPainTimer(p, 1) && Tick(p, 1) && !p->water_damage.pain_timer);
    CHECK(SotnPlayer_SetWaterContext(p, (SotnPlayerWater){41, 25, 0}) && Tick(p, 2) &&
          p->water_pain.events.voice && !p->water_pain.events.scrape);
    const SotnPlayer before = *p;
    CHECK(!SotnPlayer_SetWaterPainTimer(p, 33) && !SDL_memcmp(p, &before, sizeof(*p)));
    SotnPlayer_Close(p);
    return true;
}
static bool StoneAndProtection(Fixture *f)
{
    SotnPlayer *p = NULL;
    CHECK(Create(f, &p));
    CHECK(SotnPlayer_SetWaterContext(p, (SotnPlayerWater){41, 25, 0}));
    p->hp = 30;
    CHECK(SotnPlayer_Petrify(p, 0, 10, 0));
    p->invincibility = 0;
    p->poison_ticks = 100;
    CHECK(Tick(p, 10) && p->hp == 22 && p->step == SOTN_PLAYER_STONE &&
          !p->water_pain.events.response);
    p->invincibility = 2;
    p->hp = 0;
    CHECK(Tick(p, 20) && !p->hp && p->step == SOTN_PLAYER_STONE);
    SotnPlayer_Close(p);
    p = NULL;
    CHECK(Create(f, &p));
    p->armor = 22;
    const SotnPlayer unsupported = *p;
    CHECK(!SotnPlayer_SetDamageContext(p, 10) && !SDL_memcmp(p, &unsupported, sizeof(*p)));
    p->armor = 0;
    CHECK(SotnPlayer_SetWaterContext(p, (SotnPlayerWater){41, 25, 0}) &&
          SotnPlayer_SetScriptHold(p, true) && Tick(p, 10) && p->hp == 10 &&
          !p->water_pain.events.response);
    SotnPlayer_Close(p);
    return true;
}
static bool Contacts(Fixture *f)
{
    SotnPlayer *p = NULL;
    CHECK(Create(f, &p));
    CHECK(SotnPlayer_SetWaterContext(p, (SotnPlayerWater){41, 25, 0}));
    p->attributes.defense = 1;
    CHECK(SotnPlayer_QueueContact(
        p, (SotnPlayerContact){.attack = 3, .parameters = 1, .source_x = 10}));
    CHECK(SotnPlayer_QueueContact(
              p, (SotnPlayerContact){.attack = 3, .parameters = 0x81, .source_x = 20}) &&
          p->queued_contact.source_x == 10);
    CHECK(Tick(p, 10) && p->hp == 8 && p->step == SOTN_PLAYER_HIT &&
          !p->water_pain.events.response && !p->queued_contact.attack && !p->contact_dispatch &&
          p->animation.remaining == 3);
    const Uint32 random = p->damage_random;
    CHECK(Tick(p, 20) && p->hp == 8 && !p->water_pain.events.response &&
          p->damage_random == random);
    p->step = SOTN_PLAYER_STAND;
    p->invincibility = 0;
    p->attributes.absorption = 0x8000;
    CHECK(SotnPlayer_QueueContact(p, (SotnPlayerContact){.attack = 3, .parameters = 0x8001}) &&
          Tick(p, 30) && p->hp == 11 && !p->water_pain.events.response);
    p->attributes.absorption = 0;
    p->invincibility = 0; // Independent immunity fixture, outside absorption's protection window.
    p->attributes.immunity = 0x8000;
    CHECK(SotnPlayer_QueueContact(p, (SotnPlayerContact){.attack = 3, .parameters = 0x8001}) &&
          Tick(p, 40) && p->hp == 11 && !p->water_pain.events.response);
    p->attributes.immunity = 0;
    p->poison_ticks = 2;
    CHECK(SotnPlayer_QueueContact(p, (SotnPlayerContact){.attack = 3, .parameters = 1}) &&
          Tick(p, 50) && p->hp == 7 && p->poison_ticks == 1);
    const SotnPlayer before = *p;
    CHECK(!SotnPlayer_QueueContact(p, (SotnPlayerContact){.attack = 10000}) &&
          !SDL_memcmp(p, &before, sizeof(*p)));
    SotnPlayer_Close(p);
    return true;
}
static bool DeathAndRollback(Fixture *f)
{
    SotnPlayer *p = NULL;
    CHECK(Create(f, &p));
    CHECK(SotnPlayer_SetWaterContext(p, (SotnPlayerWater){41, 25, 0}));
    p->hp = 10;
    p->animation.id = 247;
    CHECK(SotnPlayer_SetDamageContext(p, 10));
    const SotnPlayer before = *p;
    CHECK(!SotnPlayer_Tick(p, 0) && !SDL_memcmp(p, &before, sizeof(*p)) && !f->image.captured);
    p->animation.id = 1;
    p->hp = 1;
    CHECK(SotnPlayer_Tick(p, 0) && !p->hp && p->step == SOTN_PLAYER_KILL_WATER &&
          p->water_death.started && p->sound == 0x6f6 && !p->water_pain.events.response);
    p->curse_ticks = 10;
    p->poison_ticks = 10;
    p->dark.ticks = 10;
    p->spell_use_ticks = 10;
    p->timers[0] = 10;
    CHECK(SotnPlayer_QueueBloodHealing(p, 8, 1));
    const Uint16 pressed = p->previous_buttons;
    CHECK(Tick(p, 11) && p->healing.pending == 8 && !p->hp && p->curse_ticks == 10 &&
          p->poison_ticks == 10 && p->dark.ticks == 10 && p->spell_use_ticks == 10 &&
          p->timers[0] == 10 && p->previous_buttons == pressed);
    CHECK(SotnPlayer_TickSpellFreeze(p, 0, true) && !p->water_death.started &&
          !p->water_pain.events.response);
    for (Uint32 clock = 12; clock < 220 && p->death.phase != 128; ++clock)
        CHECK(Tick(p, clock));
    CHECK(p->death.phase == 128 && !p->hp && p->healing.pending == 8 && !f->image.remaining);
    SotnPlayer_Close(p);
    return true;
}
static bool BloodCloak(Fixture *f)
{
    SotnPlayer *p = NULL;
    CHECK(Create(f, &p));
    p->cape = 54;
    p->hearts = 10;
    p->hearts_max = 50;
    p->hp = 50;
    p->attributes.defense = 1;
    SotnCharacterState bound = {.hp = 50, .hp_max = 70};
    bound.equipment[4] = 54;
    const SotnPlayer unbound = *p;
    CHECK(!SotnPlayer_SetCharacter(p, &bound, p->attributes) &&
          !SDL_memcmp(&unbound, p, sizeof(*p)));
    bound.hearts = 10;
    bound.hearts_max = 50;
    CHECK(SotnPlayer_SetCharacter(p, &bound, p->attributes) && p->cape == 54 && p->hearts == 10 &&
          p->hearts_max == 50);
    const Sint32 x = p->x.value, y = p->y.value;
    CHECK(SotnPlayer_QueueContact(p, (SotnPlayerContact){.attack = 4, .parameters = 1}) &&
          Tick(p, 1) && p->hp == 47 && p->heart_gain.requested && p->heart_gain.amount == 3 &&
          p->heart_gain.source.x == x && p->heart_gain.source.y == y);
    const SotnCollisionMap map = p->map;
    CHECK(SotnPlayer_ChangeRoom(p, map, 1, 2) && p->heart_gain.source.x == x + 65536 &&
          p->heart_gain.source.y == y + 2 * 65536 && p->heart_gain.amount == 3 && p->hearts == 10);
    CHECK(Tick(p, 2) && !p->heart_gain.requested && p->hp == 47);
    p->step = SOTN_PLAYER_STAND;
    p->invincibility = 0;
    p->poison_ticks = 10;
    CHECK(SotnPlayer_QueueContact(p, (SotnPlayerContact){.attack = 4, .parameters = 1}) &&
          Tick(p, 3) && p->hp == 41 && p->heart_gain.amount == 6);
    CHECK(SotnPlayer_TickSpellFreeze(p, 0, false) && !p->heart_gain.requested);
    p->step = SOTN_PLAYER_STAND;
    p->invincibility = 0;
    p->attributes.immunity = 0x8000;
    CHECK(SotnPlayer_QueueContact(p, (SotnPlayerContact){.attack = 4, .parameters = 0x8001}) &&
          Tick(p, 4) && !p->heart_gain.requested && p->hp == 41);
    p->attributes.immunity = 0;
    p->attributes.absorption = 0x8000;
    CHECK(SotnPlayer_QueueContact(p, (SotnPlayerContact){.attack = 4, .parameters = 0x8001}) &&
          Tick(p, 5) && !p->heart_gain.requested && p->hp == 45);
    p->attributes.absorption = 0;
    p->invincibility = 0; // Subsequent independent ordinary-water fixture.
    CHECK(SotnPlayer_SetWaterContext(p, (SotnPlayerWater){41, 25, 0}) && Tick(p, 10) &&
          p->hp == 44 && !p->heart_gain.requested);
    CHECK(SotnPlayer_SetWaterContext(p, (SotnPlayerWater){0}));
    p->poison_ticks = 0;
    CHECK(SotnPlayer_QueueContact(p, (SotnPlayerContact){.attack = 1, .parameters = 0x202}) &&
          Tick(p, 11) && p->step == SOTN_PLAYER_STONE && p->hp == 44 && p->heart_gain.requested &&
          !p->heart_gain.amount);
    p->invincibility = 0;
    CHECK(SotnPlayer_SetWaterContext(p, (SotnPlayerWater){41, 25, 0}) && Tick(p, 20) &&
          p->hp == 36 && p->heart_gain.requested && p->heart_gain.amount == 8);
    CHECK(SotnPlayer_SetWaterContext(p, (SotnPlayerWater){0}));
    p->step = SOTN_PLAYER_STAND;
    p->invincibility = 0;
    for (Uint16 id = 0x31; id <= 0x34; ++id)
        P32(f->dra + 0x101b8 + id * 4U, 0x801bffff);
    CHECK(SotnPlayer_QueueContact(p, (SotnPlayerContact){.attack = 2, .parameters = 1}) &&
          SotnPlayer_SetDamageContext(p, 21));
    const SotnPlayer before = *p;
    CHECK(!SotnPlayer_Tick(p, 0) && !SDL_memcmp(&before, p, sizeof(*p)));
    for (Uint16 id = 0x31; id <= 0x34; ++id)
        P32(f->dra + 0x101b8 + id * 4U, 0x800a0100);
    CHECK(SotnPlayer_Tick(p, 0) && p->hp == 35 && p->heart_gain.requested &&
          p->heart_gain.amount == 1);
    p->step = SOTN_PLAYER_STAND;
    p->invincibility = 0;
    p->hearts = p->hearts_max;
    CHECK(SotnPlayer_QueueContact(p, (SotnPlayerContact){.attack = 2, .parameters = 1}) &&
          Tick(p, 22) && p->hp == 34 && !p->heart_gain.requested);
    p->step = SOTN_PLAYER_STAND;
    p->invincibility = 0;
    p->hp = 1;
    CHECK(SotnPlayer_QueueContact(p, (SotnPlayerContact){.attack = 2, .parameters = 1}) &&
          Tick(p, 23) && !p->hp && !p->heart_gain.requested);
    SotnPlayer_Close(p);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    bool ok =
        Water(f) && Contacts(f) && StoneAndProtection(f) && DeathAndRollback(f) && BloodCloak(f);
    SDL_free(f);
    return ok ? 0 : 1;
}
