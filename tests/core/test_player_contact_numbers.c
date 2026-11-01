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
    f->player->attributes = (SotnCharacterStats){0};
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
static bool Producers(Fixture *f, SotnDataView exe)
{
    CHECK(Create(f) && SotnPlayer_SetScriptContext(f->player, exe));
    SotnPlayer *p = f->player;
    const SotnPlayer base = *p;
    p->x.parts.low = 123;
    p->y.parts.low = 456;
    const Sint32 x = p->x.value, y = p->y.value;
    CHECK(SotnPlayer_QueueMailHealing(p) && Contact(p, 1, 7, 1) && p->hp == 34 &&
          p->damage_number.applied && p->damage_number.amount == 7 &&
          p->damage_number.response == 3 && p->damage_number.source.x == x &&
          p->damage_number.source.y == y && p->healing.applied_amount == 1);
    CHECK(SotnPlayer_ChangeRoom(p, p->map, 1, -1) && p->damage_number.source.x == x + 65536 &&
          p->damage_number.source.y == y - 65536);
    CHECK(SotnPlayer_TickSpellFreeze(p, 0, false) && !p->damage_number.applied && p->hp == 34);
    *p = base;
    p->hp = 1;
    CHECK(Contact(p, 1, 100, 1) && !p->hp && p->damage_number.applied &&
          p->damage_number.response == 4 && p->damage_number.amount == 100);
    *p = base;
    p->attributes.weakness = 0x20;
    p->poison_ticks = 100;
    CHECK(Contact(p, 1, 9999, 0x21) && !p->hp && p->damage_number.response == 4 &&
          p->damage_number.amount == 39996);
    *p = base;
    p->attributes.defense = 10;
    CHECK(Contact(p, 1, 5, 0x201) && p->hp == 40 && p->damage_number.applied &&
          !p->damage_number.amount && p->damage_number.response == 7 &&
          p->step == SOTN_PLAYER_STONE);
    *p = base;
    CHECK(Contact(p, 1, 100, 0x201) && !p->hp && p->damage_number.response == 7 &&
          p->damage_number.amount == 100 && p->step == SOTN_PLAYER_STONE);
    *p = base;
    p->step = SOTN_PLAYER_STONE;
    p->stone.active = true;
    p->substep = 2;
    p->stone.shakes = 3;
    CHECK(Contact(p, 1, 1, 1) && p->hp == 32 && p->damage_number.response == 8 &&
          p->damage_number.amount == 8);
    *p = base;
    p->hp = 5;
    p->hp_max = 7;
    p->step = SOTN_PLAYER_STONE;
    p->stone.active = true;
    p->substep = 2;
    p->stone.shakes = 3;
    CHECK(Contact(p, 1, 1, 1) && p->hp == 5 && p->damage_number.applied &&
          p->damage_number.response == 8 && !p->damage_number.amount);
    *p = base;
    p->step = SOTN_PLAYER_STONE;
    p->stone.active = true;
    p->substep = 2;
    p->stone.shakes = 1;
    CHECK(Contact(p, 1, 1, 1) && p->hp == 32 && p->damage_number.response == 8 &&
          p->step == SOTN_PLAYER_HIT && !p->stone.active);
    *p = base;
    p->hp = 1;
    p->step = SOTN_PLAYER_STONE;
    p->stone.active = true;
    p->substep = 2;
    CHECK(Contact(p, 1, 1, 1) && !p->hp && p->damage_number.response == 4 &&
          p->damage_number.amount == 8);
    // A late animation failure cannot commit the accepted number or replay a heal.
    *p = base;
    CHECK(SotnPlayer_QueueMailHealing(p) &&
          SotnPlayer_QueueContact(p, (SotnPlayerContact){.attack = 7, .parameters = 1}) &&
          SotnPlayer_SetDamageContext(p, 1));
    Uint8 animations[247 * 4] = {0};
    SDL_memcpy(animations, f->dra + 0x101b8, sizeof(animations));
    for (Uint16 i = 0; i < 247; ++i)
        TestCharacter_Put32(f->dra + 0x101b8 + i * 4U, 0x801bffff);
    const SotnPlayer before = *p;
    CHECK(!SotnPlayer_Tick(p, 0) && !SDL_memcmp(&before, p, sizeof(*p)));
    SDL_memcpy(f->dra + 0x101b8, animations, sizeof(animations));
    CHECK(SotnPlayer_Tick(p, 0) && p->damage_number.applied && p->damage_number.amount == 7 &&
          p->hp == 34 && !p->healing.pending);
    Close(f);
    return true;
}
static bool HurtFeedback(Fixture *f, SotnDataView exe)
{
    CHECK(Create(f) && SotnPlayer_SetScriptContext(f->player, exe));
    SotnPlayer *p = f->player;
    const SotnPlayer base = *p;
    // A neutral accepted contact starts timer2 after its common pass, retaining birth palette.
    CHECK(Contact(p, 1, 1, 0x21) && p->damage_number.neutral_hurt && p->timers[2] == 6 &&
          p->damage_palette == 0x8166 && p->palette == 0x8100 && p->hp == 39);
    const SotnPlayer birth = *p;
    CHECK(SotnPlayer_TickSpellFreeze(p, 0, false) && !p->damage_number.applied &&
          !p->damage_number.neutral_hurt && p->timers[2] == 6 && p->palette == 0x8100);
    CHECK(SotnPlayer_TickSpellFreeze(p, 0, true) && p->timers[2] == 6 && p->palette == 0x8100);
    *p = birth;
    for (Uint16 i = 1; i <= 6; ++i)
        CHECK(Contact(p, i + 1, 0, 0) && !p->damage_number.applied &&
              p->palette == (i < 6 ? 0x8166 : 0x8100) && p->timers[2] == 6 - i);
    Uint16 updates = 0;
    while (p->step == SOTN_PLAYER_HIT && updates < 150)
        CHECK(Contact(p, 8 + updates++, 0, 0));
    CHECK(updates < 150 && p->step == SOTN_PLAYER_STAND && p->timers[4] == 12 &&
          p->afterimage_timer15 == 4 && p->invincibility == 12 && !p->body_color_blend &&
          p->palette == 0x8100 && p->hp == 39 && p->damage_random == birth.damage_random);
    for (Uint16 i = 1; i <= 12; ++i)
        CHECK(Contact(p, 200 + i, 0, 0) && p->timers[4] == 12 - i &&
              p->body_color_blend == (i < 12) && p->invincibility == 12 - i);
    // Retained ailments suppress exit color/timer15 restoration, while protection still applies.
    for (Uint16 ailment = 1; ailment <= 2; ++ailment)
    {
        *p = base;
        p->poison_ticks = ailment == 1 ? 100 : 0;
        p->curse_ticks = ailment == 2 ? 100 : 0;
        CHECK(Contact(p, 1, 1, 0x21) && p->damage_number.neutral_hurt);
        p->animation.remaining = -1;
        CHECK(Contact(p, 2, 0, 0) && p->step == SOTN_PLAYER_STAND && !p->timers[4] &&
              !p->afterimage_timer15 && p->invincibility == 12);
    }
    // Elemental/ailment producers retain their separate, incomplete presentation owners.
    const Uint16 excluded[] = {0x40, 0x80, 0x100, 0x200, 0x800, 0x1000, 0x2000, 0x4000, 0x8000};
    for (Uint16 i = 0; i < SDL_arraysize(excluded); ++i)
    {
        *p = base;
        CHECK(Contact(p, 1, 1, excluded[i] | 1) && !p->damage_number.neutral_hurt);
    }
    *p = base;
    p->hp = 1;
    CHECK(Contact(p, 1, 1, 0x21) && !p->hp && !p->damage_number.neutral_hurt);
    // Failed animation cannot publish Blink or palette/timer state; retry commits once.
    *p = base;
    CHECK(SotnPlayer_QueueContact(p, (SotnPlayerContact){.attack = 1, .parameters = 0x21}) &&
          SotnPlayer_SetDamageContext(p, 1));
    const SotnPlayer before = *p;
    Uint8 saved[247 * 4];
    SDL_memcpy(saved, f->dra + 0x101b8, sizeof(saved));
    for (Uint16 i = 0; i < 247; ++i)
        TestCharacter_Put32(f->dra + 0x101b8 + i * 4U, 0x801bffff);
    CHECK(!SotnPlayer_Tick(p, 0) && !SDL_memcmp(&before, p, sizeof(*p)));
    SDL_memcpy(f->dra + 0x101b8, saved, sizeof(saved));
    CHECK(SotnPlayer_Tick(p, 0) && p->damage_number.neutral_hurt && p->timers[2] == 6);
    Close(f);
    return true;
}
static bool Exclusions(Fixture *f, SotnDataView exe)
{
    CHECK(Create(f) && SotnPlayer_SetScriptContext(f->player, exe));
    SotnPlayer *p = f->player;
    const SotnPlayer base = *p;
    p->attributes.immunity = 0x8000;
    CHECK(Contact(p, 1, 7, 0x8001) && p->immunity.applied && !p->damage_number.applied);
    *p = base;
    p->attributes.absorption = 0x8000;
    CHECK(Contact(p, 1, 7, 0x8001) && p->absorption.applied && !p->damage_number.applied);
    *p = base;
    p->attributes.defense = 100;
    CHECK(Contact(p, 1, 7, 1) && p->guard.applied && !p->damage_number.applied);
    *p = base;
    p->talismans = 1;
    p->damage_random = 0;
    CHECK(Contact(p, 1, 100, 1) && p->dodge.applied && !p->damage_number.applied);
    *p = base;
    CHECK(SotnPlayer_SetWaterContext(p, (SotnPlayerWater){41, 25, 0}) &&
          Contact(p, 10, 100, 0x8000) && p->water_pain.events.response == 9 &&
          !p->damage_number.applied);
    *p = base;
    p->hp = 1;
    CHECK(SotnPlayer_SetWaterContext(p, (SotnPlayerWater){41, 25, 0}) &&
          Contact(p, 10, 100, 0x8000) && p->water_death.active && !p->damage_number.applied);
    *p = base;
    p->invincibility = 2;
    CHECK(Contact(p, 1, 100, 1) && p->hp == 40 && !p->damage_number.applied);
    *p = base;
    p->step = SOTN_PLAYER_HIT;
    p->substep = 6;
    CHECK(Contact(p, 1, 1, 1) && p->hp == 40 && !p->damage_number.applied);
    Close(f);
    return true;
}
static bool Shared(Fixture *f, SotnDataView exe)
{
    CHECK(Create(f));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r)), *saved = SDL_malloc(sizeof(*saved));
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    CHECK(r && saved && actors);
    SotnBatRuntimeInput in = {
        .dra = {f->dra, sizeof(f->dra)},
        .executable = exe,
        .player = {.hp = 33,
                   .hp_max = 70,
                   .step = SOTN_PLAYER_HIT,
                   .healing = {.applied = true, .applied_kind = 2, .applied_amount = 1},
                   .damage_number = {.applied = true,
                                     .amount = 7,
                                     .response = 3,
                                     .source = {.x = 100 * 65536 + 123, .y = 135 * 65536 + 456}}}};
    CHECK(SotnBatRuntime_Tick(r, f->character, in) && r->children[40].kind == 19 &&
          !r->children[40].state.healing_number.damage && r->children[41].kind == 19 &&
          r->children[41].state.healing_number.damage &&
          r->children[41].state.healing_number.number == 7 &&
          r->children[41].state.healing_number.x == 100 * 65536 + 123 &&
          r->children[41].primitive_count == 20 && !r->sound_count &&
          SotnActors_CommitBatPrimitives(actors, r));
    const SotnHealingNumber before_rebase = r->children[41].state.healing_number;
    in.entering = true;
    in.screen_shift_x = in.room_shift_x = -2;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, f->character, in) &&
          r->children[41].state.healing_number.x == before_rebase.x - 2 * 65536 &&
          r->children[41].state.healing_number.phase == before_rebase.phase &&
          r->children[41].state.healing_number.timer == before_rebase.timer &&
          SotnActors_CommitBatPrimitives(actors, r));
    in.entering = false;
    in.screen_shift_x = in.room_shift_x = 0;
    in.player.healing.applied = in.player.damage_number.applied = false;
    in.freeze = 3;
    for (Uint16 t = 0; t < 120; ++t)
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, f->character, in) &&
              SotnActors_CommitBatPrimitives(actors, r));
    CHECK(!SotnBatRuntime_Occupied(r));
    for (Uint16 row = 0; row < SOTN_PRIMITIVE_SLOTS; ++row)
        CHECK(!actors->primitives[row].owner);
    in.freeze = 0;
    in.player.healing.applied = false;
    in.player.damage_number = (SotnPlayerDamageNumber){.applied = true, .response = 7};
    in.player.hp = 40;
    in.player.step = SOTN_PLAYER_STONE;
    CHECK(SotnBatRuntime_Tick(r, f->character, in) &&
          !r->children[40].state.healing_number.number &&
          r->children[40].state.healing_number.digits[0].palette == 0x1b4);
    *r = (SotnBatRuntime){0};
    in.player.hp = 0;
    in.player.damage_number.response = 4;
    in.player.damage_number.amount = 39996;
    in.player.step = SOTN_PLAYER_KILL;
    CHECK(SotnBatRuntime_Tick(r, f->character, in) &&
          r->children[40].state.healing_number.number == 39996 &&
          r->children[40].state.healing_number.digits[0].palette == 0x1b2);
    *saved = *r;
    in.player.damage_number.neutral_hurt = true;
    CHECK(!SotnBatRuntime_Tick(r, f->character, in) && !SDL_memcmp(saved, r, sizeof(*r)));
    in.player.damage_number.neutral_hurt = false;
    in.player.damage_number.amount = 0;
    CHECK(!SotnBatRuntime_Tick(r, f->character, in) && !SDL_memcmp(saved, r, sizeof(*r)));
    *r = (SotnBatRuntime){0};
    in.player.damage_number.amount = 100;
    in.other_occupied = 0xff00000000000000ULL;
    CHECK(SotnBatRuntime_Tick(r, f->character, in) && !SotnBatRuntime_Occupied(r));
    in.other_occupied = 0;
    SDL_memset(&in.primitive_slots, 255, sizeof(in.primitive_slots));
    CHECK(SotnBatRuntime_Tick(r, f->character, in) && !SotnBatRuntime_Occupied(r));
    *r = (SotnBatRuntime){0};
    in.primitive_slots = (SotnPrimitiveSlots){0};
    in.player.damage_number = (SotnPlayerDamageNumber){.applied = true, .response = 8, .amount = 8};
    in.player.hp = 32;
    in.player.step = SOTN_PLAYER_HIT;
    CHECK(SotnBatRuntime_Tick(r, f->character, in) && r->children[40].kind == 19 &&
          r->children[40].state.healing_number.number == 8);
    SDL_free(actors);
    SDL_free(saved);
    SDL_free(r);
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
        exe[0x22a10 + i * 2U] = (Uint8)(i / 2U);
        exe[0x22a11 + i * 2U] = (Uint8)((i / 2U) >> 8);
    }
    for (Uint16 i = 0; i < 192; ++i)
        exe[0x1e3e1 + i * 2U] = 16;
    const SotnDataView data = {exe, 0x24000};
    const bool ok =
        Producers(f, data) && HurtFeedback(f, data) && Exclusions(f, data) && Shared(f, data);
    SDL_free(exe);
    SDL_free(f);
    return ok ? 0 : 1;
}
