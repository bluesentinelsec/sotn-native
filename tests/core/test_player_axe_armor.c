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
    Uint8 dra[0x11000], exe[0x24000], tiles[64 * 16 * 2], types[2];
    Uint8 *weapons;
    SotnCharacter *character;
    SotnPlayer *p;
    Uint32 clock;
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
    TestCharacter_Put32(f->dra + 0x200, 2U | (1U << 16));
    TestCharacter_Put32(f->dra + 0x204, 2U | (2U << 16));
    TestCharacter_Put32(f->dra + 0x208, 65535);
    Uint8 *virtual = f->dra + 0x500 + 216U * 52U;
    virtual[15] = 45;
    virtual[17] = 131;
    virtual[18] = 0xbe;
    virtual[19] = 55;
    virtual[20] = 14;
    f->dra[0x3400 + 25U * 32U + 28] = 1;
    f->weapons = SDL_calloc(46U, 0x7000);
    CHECK(f->weapons &&
          SotnCharacter_Create((SotnDataView){f->dra, sizeof(f->dra)}, &f->character));
    f->types[1] = 3;
    for (Uint16 y = 10; y < 16; ++y)
        for (Uint16 x = 0; x < 64; ++x)
            f->tiles[(y * 64U + x) * 2U] = 1;
    CHECK(SotnPlayer_Create(
        (SotnDataView){f->dra, sizeof(f->dra)},
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 64, 16}, 256,
        135, &f->p));
    CHECK(SotnPlayer_SetScriptContext(f->p, (SotnDataView){f->exe, sizeof(f->exe)}) &&
          SotnPlayer_SetDeathCapture(f->p, Capture, Decay, f) &&
          SotnPlayer_SetAxeContext(f->p, (SotnDataView){f->weapons, 46U * 0x7000}, false));
    return true;
}
static void Close(Fixture *f)
{
    SotnPlayer_Close(f->p);
    SotnCharacter_Destroy(f->character);
    SDL_free(f->weapons);
}
static bool Armor(Fixture *f, Uint16 id)
{
    SotnCharacterState profile = {0};
    SotnCharacterStats stats = {0};
    CHECK(SotnCharacter_Get(f->character, &profile, NULL));
    profile.hp = f->p->hp;
    CHECK(SotnCharacter_Restore(f->character, &profile) &&
          SotnCharacter_Collect(f->character, true, id) &&
          SotnCharacter_Equip(f->character, SOTN_EQUIP_ARMOR, id) &&
          SotnCharacter_Get(f->character, &profile, &stats) &&
          SotnPlayer_SetCharacter(f->p, &profile, stats));
    return true;
}
static bool Tick(Fixture *f, Uint16 buttons)
{
    CHECK(SotnPlayer_SetDamageContext(f->p, ++f->clock) && SotnPlayer_Tick(f->p, buttons));
    return true;
}
static bool Player(Fixture *f)
{
    CHECK(Create(f));
    TestCharacter_Put32(f->weapons + 45U * 0x7000 + 0x4000 + 0x13b8, 17);
    TestCharacter_Put32(f->weapons + 45U * 0x7000 + 0x4000 + 0x13bc, 0x12345678);
    CHECK(Armor(f, 25) && Tick(f, SOTN_BUTTON_RIGHT));
    SotnPlayer *p = f->p;
    CHECK(p->axe_armor.active && p->axe_armor.entered && p->animation_bank == 16 &&
          p->palette == 0x110 && p->step == SOTN_PLAYER_AXE_STAND && p->sound == 0x6f2 &&
          p->simulated_pad.remaining == 31 && p->hp == 70 && p->mp == 20 &&
          p->axe_armor.cursor.angle == 0x12345678 && p->axe_armor.cursor.recovery == 17);
    for (Uint16 i = 0; i < 31; ++i)
        CHECK(Tick(f, SOTN_BUTTON_RIGHT) && p->step == SOTN_PLAYER_AXE_STAND &&
              !p->axe_armor.entered);
    CHECK(Tick(f, SOTN_BUTTON_RIGHT) && p->step == SOTN_PLAYER_AXE_WALK && p->vx == 163840);
    CHECK(Tick(f, 0) && Tick(f, SOTN_BUTTON_LEFT_HAND) && p->animation.id == 0xce &&
          p->axe_armor.events.factory_count == 1 && p->axe_armor.events.factories[0] == 87);
    const SotnPlayer active = *p;
    CHECK(SotnPlayer_TickSpellFreeze(p, 0, false) && !p->axe_armor.events.factory_count &&
          !SDL_memcmp(&active.axe_armor.cursor, &p->axe_armor.cursor, sizeof(SotnAxeArmor)) &&
          p->animation.remaining == active.animation.remaining);
    CHECK(SotnPlayer_SetSimulatedPad(p, (SotnPlayerSimulatedPad){.mode = 2, .remaining = 1}) &&
          Tick(f, 0) && p->step == SOTN_PLAYER_AXE_SCRIPT && !p->vx && !p->vy);
    p->axe_armor.draw_flags = 12;
    p->timers[2] = 1;
    p->damage_palette = 0x8166;
    p->palette = 0x8166;
    CHECK(Tick(f, SOTN_BUTTON_RIGHT) && p->step == SOTN_PLAYER_AXE_SCRIPT && p->palette == 0x8100);
    CHECK(SotnPlayer_SetSimulatedPad(p, (SotnPlayerSimulatedPad){0}) && Tick(f, 0) &&
          p->step == SOTN_PLAYER_AXE_STAND && p->animation.id == 0xcf && p->palette == 0x8100);
    CHECK(Tick(f, 0) && p->palette == 0x110);
    CHECK(Armor(f, 0) && Tick(f, 0) && !p->axe_armor.active && p->axe_armor.exited &&
          p->animation_bank == 1 && p->palette == 0x8100 && p->step == SOTN_PLAYER_STAND);
    CHECK(Armor(f, 25) &&
          SotnPlayer_SetAxeContext(p, (SotnDataView){f->weapons, 46U * 0x7000}, true) &&
          p->axe_armor.active && p->animation.id == 0xcf && !p->simulated_pad.remaining);
    const SotnPlayer rebound = *p;
    CHECK(SotnPlayer_SetAxeContext(p, (SotnDataView){f->weapons, 46U * 0x7000}, true) &&
          !SDL_memcmp(&rebound, p, sizeof(*p)));
    p->axe_armor.cursor.angle = 1234;
    CHECK(SotnPlayer_SetSimulatedPad(p, (SotnPlayerSimulatedPad){.mode = 1, .remaining = 1}) &&
          Tick(f, 0) && p->step == SOTN_PLAYER_SCRIPT_REACTION && !p->axe_armor.active &&
          Tick(f, 0) && p->step == SOTN_PLAYER_AXE_STAND && p->axe_armor.active &&
          p->axe_armor.cursor.angle == 1234);
    p->simulated_pad = (SotnPlayerSimulatedPad){0};
    // End the script fixture's retained protection before testing new raw contacts.
    p->invincibility = 0;
    const SotnPlayer base = *p;
    const Uint16 effects[] = {0x20, 0x80, 0x100, 0x200};
    for (Uint16 i = 0; i < SDL_arraysize(effects); ++i)
    {
        *p = base;
        p->attributes.defense = 10;
        CHECK(SotnPlayer_QueueContact(
                  p, (SotnPlayerContact){.attack = 1, .parameters = effects[i] | 1}) &&
              Tick(f, 0) && p->step == SOTN_PLAYER_AXE_HIT && p->animation.id == 0xd1 &&
              p->axe_armor.events.factories[0] == 85 && p->damage_number.applied &&
              p->damage_number.response == 3 && !p->damage_number.neutral_hurt &&
              p->damage_number.amount == (i == 3 ? 0 : 1) && !p->poison_ticks && !p->curse_ticks &&
              p->invincibility == 20 && p->sound == 0x6ed);
    }
    *p = base;
    p->attributes.defense = 100;
    CHECK(SotnPlayer_QueueContact(p, (SotnPlayerContact){.attack = 1, .parameters = 1}) &&
          Tick(f, 0) && p->step == SOTN_PLAYER_AXE_HIT && !p->guard.applied &&
          p->damage_number.response == 3 && p->damage_number.amount == 1);
    for (Uint16 kind = 16; kind <= 18; ++kind)
    {
        *p = base;
        CHECK(SotnPlayer_QueueContact(p, (SotnPlayerContact){.attack = 1,
                                                             .parameters = kind,
                                                             .source_x = p->x.parts.high + 100}) &&
              Tick(f, 0) && p->step == SOTN_PLAYER_AXE_HIT &&
              p->axe_armor.knock_right == (kind == 17) && p->vx == (kind == 17 ? 81920 : -81920));
        *p = base;
        CHECK(SotnPlayer_QueueContact(p, (SotnPlayerContact){.attack = 1,
                                                             .parameters = kind | 0x100U,
                                                             .source_x = p->x.parts.high + 100}) &&
              Tick(f, 0) && !p->axe_armor.knock_right && !p->curse_ticks);
    }
    *p = base;
    p->hp = 1;
    CHECK(SotnPlayer_QueueContact(p, (SotnPlayerContact){.attack = 9999, .parameters = 1}) &&
          Tick(f, 0) && !p->hp && p->step == SOTN_PLAYER_KILL && !p->axe_armor.active &&
          p->animation_bank == 1 && p->damage_number.response == 4);
    *p = base;
    CHECK(Armor(f, 0) && Tick(f, 0));
    p->step = SOTN_PLAYER_HIGH_JUMP;
    p->gravity.upside_down = true;
    p->draw_pivot_y = 2;
    p->facing_left = true;
    CHECK(Armor(f, 25) && Tick(f, 0) && p->axe_armor.active && p->facing_left &&
          !p->gravity.upside_down && !p->axe_armor.draw_flags && p->axe_armor.rotation == 2048 &&
          p->draw_pivot_y == 2);
    // A failed entry animation leaves equipment-triggered control/timers/requests uncommitted.
    *p = base;
    p->axe_armor.active = false;
    p->step = SOTN_PLAYER_STAND;
    TestCharacter_Put32(f->dra + 0x101b8 + 0xcfU * 4U, 0x801bffff);
    CHECK(SotnPlayer_SetDamageContext(p, ++f->clock));
    const SotnPlayer before = *p;
    CHECK(!SotnPlayer_Tick(p, 0) && !SDL_memcmp(&before, p, sizeof(*p)));
    Close(f);
    return true;
}
static bool Shared(Fixture *f)
{
    CHECK(Create(f) && Armor(f, 25));
    Uint8 *bp = f->dra + 0xd1d4 + 87U * 6U;
    bp[0] = 50;
    bp[1] = 1;
    bp[2] = 1U | 64U | 128U;
    bp[4] = 3;
    f->dra[0xd4b8 + 3U * 2U] = 16;
    f->dra[0xd4b9 + 3U * 2U] = 16;
    Uint8 *spell = f->dra + 0x8410 + 11U * 28U;
    spell[13] = 12;
    spell[16] = 2;
    spell[18] = 1;
    spell[22] = 0x20;
    spell[24] = 30;
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    CHECK(r);
    SotnBatRuntimeInput in = {
        .dra = {f->dra, sizeof(f->dra)},
        .player = {
            .hp = 70,
            .hp_max = 70,
            .step = SOTN_PLAYER_AXE_STAND,
            .status = 0x29100000,
            .animation = {.sprite = 5},
            .axe_armor = {.active = true, .events = {.factories = {87}, .factory_count = 1}}}};
    CHECK(SotnBatRuntime_Tick(r, f->character, in) && r->children[0].kind == 50 &&
          r->children[0].initialized && r->children[0].group == 1 && r->groups[1] == 1 &&
          r->axe_mask == 1 && !r->children[0].primitive_count &&
          r->children[0].state.attack.width == 12 && r->children[0].state.attack.height == 32 &&
          r->children[0].state.attack.offset_y == -12 && r->children[0].state.attack.power == 31);
    SotnCharacterState profile = {0};
    CHECK(SotnCharacter_Get(f->character, &profile, NULL));
    const Uint32 random = profile.random, generation = r->children[0].generation;
    in.player.axe_armor.events.factory_count = 0;
    in.player.animation.sprite = 6;
    in.player.facing_left = true;
    CHECK(SotnBatRuntime_Tick(r, f->character, in) && r->children[0].generation == generation &&
          !r->children[0].source.facing_left && r->children[0].state.attack.width == 10 &&
          r->children[0].state.attack.offset_y == 16 &&
          SotnCharacter_Get(f->character, &profile, NULL) && profile.random == random);
    in.player.animation.sprite = 1;
    CHECK(SotnBatRuntime_Tick(r, f->character, in) && !r->children[0].kind && !r->axe_mask &&
          !r->groups[1]);
    in.player.animation.sprite = 5;
    in.player.axe_armor.events.factory_count = 1;
    in.other_occupied = (Uint64)1 << 16;
    const SotnBatRuntime before = *r;
    CHECK(!SotnBatRuntime_Tick(r, f->character, in) && !SDL_memcmp(&before, r, sizeof(*r)));
    in.other_occupied = 0;
    // Eight actual delayed factory records exhaust the shared root allocator before spell RNG.
    bp = f->dra + 0xd1d4;
    bp[0] = 2;
    bp[1] = bp[2] = 1;
    bp[5] = 200;
    f->dra[0xd4b8] = 32;
    f->dra[0xd4b9] = 63;
    for (Uint16 i = 0; i < 8; ++i)
        CHECK(SotnPlayerFactory_Begin(&r->factories[i].cursor, (SotnPlayerFactorySource){0}, 0, 0));
    CHECK(SotnBatRuntime_Tick(r, f->character, in) && !r->axe_mask &&
          SotnCharacter_Get(f->character, &profile, NULL) && profile.random == random);
    for (Uint16 i = 0; i < 8; ++i)
        CHECK(r->factories[i].cursor.entity_id && r->factories[i].cursor.delay == 199);
    SDL_free(r);
    Close(f);
    return true;
}
static bool Shadow(Fixture *f)
{
    CHECK(Create(f));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    CHECK(r);
    r->children[16] =
        (SotnBatRuntimeChild){.kind = 49, .generation = 1, .source = {.x = 0x1234, .y = 0xabcd}};
    SotnBatRuntimeInput in = {.dra = {f->dra, sizeof(f->dra)},
                              .game_timer = 2,
                              .player = {.hp = 70,
                                         .hp_max = 70,
                                         .x = 80 * 65536 + 1,
                                         .y = 100 * 65536 + 2,
                                         .step = SOTN_PLAYER_AXE_HIT,
                                         .priority = 148,
                                         .animation_bank = 16,
                                         .palette = 0x110,
                                         .facing_left = true,
                                         .axe_armor = {.active = true,
                                                       .rotation = -512,
                                                       .draw_flags = 12,
                                                       .opacity = 231,
                                                       .knock_right = true}}};
    CHECK(SotnBatRuntime_Tick(r, f->character, in));
    const SotnBatRuntimeChild *c = &r->children[16];
    CHECK(c->initialized && !c->primitive_count && c->source.x == 96 * 65536 + 0x1234 &&
          c->source.y == 110 * 65536 + 0xabcd && c->source.facing_left &&
          c->state.axe_shadow.bank == 16 && c->state.axe_shadow.palette == 0x110 &&
          c->state.axe_shadow.texture == 0x64 && c->state.axe_shadow.frame == 7 &&
          c->state.axe_shadow.draw_flags == 8 && c->state.axe_shadow.opacity == 231);
    const SotnBatRuntimeChild held = *c;
    in.freeze = 3;
    in.player.x += 65536;
    in.player.axe_armor.rotation = 896;
    CHECK(SotnBatRuntime_Tick(r, f->character, in) && !SDL_memcmp(&held, c, sizeof(held)));
    in.freeze = 0;
    in.entering = true;
    in.room_shift_x = -256;
    CHECK(SotnBatRuntime_Tick(r, f->character, in) && c->source.x == held.source.x - 256 * 65536 &&
          c->state.axe_shadow.opacity == held.state.axe_shadow.opacity);
    in.entering = false;
    in.room_shift_x = 0;
    in.player.axe_armor.knock_right = false;
    in.player.facing_left = false;
    CHECK(SotnBatRuntime_Tick(r, f->character, in) && c->source.facing_left &&
          c->source.x == 57 * 65536 + 0x1234 && c->source.y == 117 * 65536 + 0xabcd);
    in.player.step = SOTN_PLAYER_AXE_STAND;
    CHECK(SotnBatRuntime_Tick(r, f->character, in) && !c->kind);
    SDL_free(r);
    Close(f);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok = Player(f) && Shared(f) && Shadow(f);
    SDL_free(f);
    return ok ? 0 : 1;
}
