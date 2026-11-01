#include "actors_internal.h"
#include "character_fixture.h"
#include "player_internal.h"
#include <sotn/bat_runtime.h>
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
    f->player->head = 26;
    f->player->attributes = (SotnCharacterStats){.defense = 100};
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
static bool Ownership(Fixture *f, SotnDataView exe)
{
    CHECK(Create(f) && SotnPlayer_SetScriptContext(f->player, exe));
    SotnPlayer *p = f->player;
    p->x.parts.low = 123;
    p->y.parts.low = 456;
    const Sint32 x = p->x.value, y = p->y.value;
    const Uint32 random = p->damage_random;
    CHECK(SotnPlayer_QueueMailHealing(p) && Contact(p, 1, 100, 1) && p->hp == 40 &&
          p->guard.applied && p->guard.source.x == x && p->guard.source.y == y &&
          p->step == SOTN_PLAYER_STAND && p->guard.sound == 0x6e7 && !p->sound && !p->extra_sound &&
          p->invincibility == 12 && p->timers[3] == 6 && p->gravity.timer == 0x8166 &&
          p->damage_random == random && p->palette == 0x8100 && p->healing.applied_amount == 1 &&
          !p->immunity.applied && !p->absorption.applied);
    CHECK(SotnPlayer_ChangeRoom(p, p->map, 1, -1) && p->guard.source.x == x + 65536 &&
          p->guard.source.y == y - 65536);
    CHECK(SotnPlayer_TickSpellFreeze(p, 0, false) && !p->guard.applied && p->timers[3] == 6 &&
          p->invincibility == 12 && p->palette == 0x8100);
    for (Uint32 t = 2; t <= 6; ++t)
        CHECK(SotnPlayer_SetDamageContext(p, t) && SotnPlayer_Tick(p, 0) && p->palette == 0x8166 &&
              p->timers[3] == 7 - t && !p->body_color_blend && !p->guard.applied &&
              p->damage_random == random);
    CHECK(SotnPlayer_SetDamageContext(p, 7) && SotnPlayer_Tick(p, 0) && !p->timers[3] &&
          p->timers[4] == 11 && p->palette == 0x8100 && p->body_color_blend &&
          p->invincibility == 11 && p->afterimage_timer15 == 11);
    for (Uint32 t = 8; t <= 18; ++t)
        CHECK(SotnPlayer_SetDamageContext(p, t) && SotnPlayer_Tick(p, 0));
    CHECK(!p->timers[4] && !p->body_color_blend && !p->invincibility);
    CHECK(SotnPlayer_QueueContact(p, (SotnPlayerContact){.attack = 99, .parameters = 1}) &&
          SotnPlayer_SetDamageContext(p, 19));
    TestCharacter_Put32(f->dra + 0x101b8 + p->animation.id * 4U, 0x801bffff);
    const SotnPlayer before = *p;
    CHECK(!SotnPlayer_Tick(p, 0) && !SDL_memcmp(&before, p, sizeof(*p)));
    TestCharacter_Put32(f->dra + 0x101b8 + p->animation.id * 4U, 0x800a0200);
    p->script_executable = (SotnDataView){0};
    const SotnPlayer unbound = *p;
    CHECK(!SotnPlayer_Tick(p, 0) && !SDL_memcmp(&unbound, p, sizeof(*p)));
    CHECK(SotnPlayer_SetScriptContext(p, exe) && SotnPlayer_Tick(p, 0) && p->hp == 39 &&
          p->guard.applied);
    Close(f);
    return true;
}
static bool Priority(Fixture *f, SotnDataView exe)
{
    CHECK(Create(f) && SotnPlayer_SetScriptContext(f->player, exe));
    SotnPlayer *p = f->player;
    p->attributes.immunity = 0x8000;
    CHECK(Contact(p, 1, 5, 0x8001) && p->immunity.applied && !p->guard.applied && p->hp == 40);
    p->attributes.immunity = 0;
    p->attributes.absorption = 0x8000;
    p->invincibility = 0;
    CHECK(Contact(p, 2, 5, 0x8001) && p->absorption.applied && !p->guard.applied && p->hp == 45);
    p->attributes.absorption = 0;
    p->invincibility = 0;
    p->poison_ticks = 20;
    p->cape = 54;
    p->armor = 22;
    SotnStatBuffs buffs = {0};
    CHECK(SotnPlayer_SetStatBuffContext(p, &buffs, p->attributes));
    CHECK(Contact(p, 3, 5, 1) && p->hp == 44 && p->guard.applied && p->poison_ticks == 19 &&
          p->heart_gain.requested && p->heart_gain.amount == 1 && p->defense_buff_minimum == 512);
    p->invincibility = 0;
    p->hearts = p->hearts_max;
    CHECK(Contact(p, 4, 5, 1) && p->hp == 43 && p->guard.applied && !p->heart_gain.requested);
    // Poison/curse-bearing contacts do not qualify; existing poison alone still permits guard.
    p->armor = p->cape = 0;
    p->invincibility = 0;
    CHECK(Contact(p, 5, 5, 0x81) && !p->guard.applied && p->step == SOTN_PLAYER_HIT);
    Close(f);
    CHECK(Create(f) && SotnPlayer_SetScriptContext(f->player, exe));
    p = f->player;
    CHECK(SotnPlayer_QueueContact(p, (SotnPlayerContact){.attack = 5, .parameters = 1}) &&
          SotnPlayer_SetDamageContext(p, 1) && SotnPlayer_Tick(p, SOTN_BUTTON_CANCEL) &&
          p->guard.applied && p->guard.sound == 0x6e7 && p->sound == 0x617 &&
          p->step == SOTN_PLAYER_STAND && p->substep == 3);
    Close(f);
    CHECK(Create(f) && SotnPlayer_SetScriptContext(f->player, exe));
    p = f->player;
    p->attributes.defense = 99;
    CHECK(Contact(p, 1, 5, 1) && !p->guard.applied && p->step == SOTN_PLAYER_HIT);
    Close(f);
    CHECK(Create(f) && SotnPlayer_SetScriptContext(f->player, exe));
    p = f->player;
    CHECK(Contact(p, 1, 101, 1) && !p->guard.applied && p->step == SOTN_PLAYER_HIT);
    Close(f);
    CHECK(Create(f) && SotnPlayer_SetScriptContext(f->player, exe));
    p = f->player;
    CHECK(SotnPlayer_SetWaterContext(p, (SotnPlayerWater){41, 25, 0}) &&
          Contact(p, 10, 5, 0x8000) && p->hp == 39 && !p->guard.applied &&
          p->water_pain.events.response == 9);
    Close(f);
    CHECK(Create(f) && SotnPlayer_SetScriptContext(f->player, exe));
    p = f->player;
    p->hp = 1;
    CHECK(Contact(p, 1, 5, 1) && !p->guard.applied && !p->hp && p->step == SOTN_PLAYER_KILL);
    Close(f);
    return true;
}
static bool Timers(SotnDataView exe)
{
    for (Uint16 ailments = 0; ailments < 8; ++ailments)
    {
        SotnPlayer p = {.script_executable = exe,
                        .timers = {[3] = 1},
                        .gravity = {.timer = 0x8166},
                        .timer_status = (ailments & 1 ? 0x80U : 0) | (ailments & 2 ? 0x4000U : 0) |
                                        (ailments & 4 ? 0x8000U : 0)};
        CHECK(SotnPlayer_TimersTick(&p) && SotnPlayer_AfterimageBegin(&p) && p.palette == 0x8100 &&
              p.afterimage_timer15 == 11);
        CHECK(p.timers[4] == (ailments ? 0 : 11) && p.invincibility == (ailments ? 0 : 11) &&
              p.body_color_blend == !ailments);
    }
    return true;
}
static bool Numbers(SotnDataView exe)
{
    SotnHealingNumber n = {.number = 1, .damage = true};
    CHECK(SotnHealingNumber_Tick(&n, exe, (SotnPlayerFactorySource){.x = 100 * 65536}, true) &&
          n.digits[0].palette == 0x1b2 && !n.extra_digits);
    SotnHealingNumber zero = {.damage = true};
    CHECK(SotnHealingNumber_Tick(&zero, exe, (SotnPlayerFactorySource){0}, true) &&
          zero.digits[0].palette == 0x1b4);
    Uint8 *dra = SDL_calloc(1, 0x11000);
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    CHECK(dra && r && actors);
    TestCharacter_Fill(dra);
    // Authored additive gear proves owned selection without copying private item definitions.
    dra[0x3400 + 24 * 32U + 10] = 100;
    dra[0x3400 + 87 * 32U + 28] = 3;
    SotnCharacter *c = NULL;
    SotnCharacterStats stats = {0};
    CHECK(SotnCharacter_Create((SotnDataView){dra, 0x11000}, &c) &&
          SotnCharacter_Collect(c, true, 24) && SotnCharacter_Collect(c, true, 87) &&
          SotnCharacter_Equip(c, SOTN_EQUIP_ARMOR, 24) &&
          SotnCharacter_Equip(c, SOTN_EQUIP_RING1, 87) && SotnCharacter_Get(c, NULL, &stats) &&
          stats.defense == 100);
    SotnBatRuntimeInput in = {
        .dra = {dra, 0x11000},
        .executable = exe,
        .player = {.hp = 39,
                   .hp_max = 70,
                   .guard = {.applied = true},
                   .healing = {.applied = true, .applied_kind = 2, .applied_amount = 1}}};
    CHECK(SotnBatRuntime_Tick(r, c, in) && r->children[40].kind == 19 &&
          !r->children[40].state.healing_number.damage && r->children[41].kind == 19 &&
          r->children[41].state.healing_number.damage &&
          r->children[41].state.healing_number.number == 1 && !r->sound_count &&
          SotnActors_CommitBatPrimitives(actors, r));
    in.player.healing.applied = in.player.guard.applied = false;
    in.freeze = 3;
    for (Uint16 t = 0; t < 120; ++t)
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, c, in) && SotnActors_CommitBatPrimitives(actors, r));
    CHECK(!SotnBatRuntime_Occupied(r));
    in.freeze = 0;
    in.player.guard.applied = true;
    in.other_occupied = 0xff00000000000000ULL;
    CHECK(SotnBatRuntime_Tick(r, c, in) && !SotnBatRuntime_Occupied(r));
    in.other_occupied = 0;
    SDL_memset(&in.primitive_slots, 255, sizeof(in.primitive_slots));
    CHECK(SotnBatRuntime_Tick(r, c, in) && !SotnBatRuntime_Occupied(r));
    SotnCharacter_Destroy(c);
    SDL_free(dra);
    SDL_free(r);
    SDL_free(actors);
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
        exe[0x22a10 + i * 2U] = (Uint8)(i / 2U);
        exe[0x22a11 + i * 2U] = (Uint8)((i / 2U) >> 8);
    }
    for (Uint16 i = 0; i < 192; ++i)
        exe[0x1e3e1 + i * 2U] = 16;
    const SotnDataView data = {exe, 0x24000};
    const bool ok = Ownership(f, data) && Priority(f, data) && Timers(data) && Numbers(data);
    SDL_free(f);
    SDL_free(exe);
    return ok ? 0 : 1;
}
