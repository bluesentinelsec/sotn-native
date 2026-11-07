#include "actors_internal.h"
#include "character_fixture.h"
#include "player_internal.h"
#include <sotn/bat_runtime.h>
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
    for (Uint16 i = 38; i <= 43; ++i)
        f->dra[0x3400 + i * 32U + 28] = 0;
    f->dra[0x3400 + 54 * 32U + 28] = 2;
    f->dra[0x3400 + 83 * 32U + 28] = 3;
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
static Uint32 Seed(Uint16 roll)
{
    for (Uint32 seed = 0;; ++seed)
    {
        Uint32 next = seed;
        if ((SotnCombat_Random(&next) & 511U) == roll)
            return seed;
    }
}
static bool Rolls(Fixture *f, SotnDataView exe)
{
    CHECK(Create(f) && SotnPlayer_SetScriptContext(f->player, exe));
    const SotnPlayer base = *f->player;
    const Uint16 lucks[] = {0, 1, 17, 99, 198};
    for (Uint16 count = 0; count <= 2; ++count)
        for (Uint16 l = 0; l < SDL_arraysize(lucks); ++l)
            for (Uint16 roll = 0; roll < 512; ++roll)
            {
                SotnPlayer p = base;
                p.talismans = count;
                p.attributes.total[3] = lucks[l];
                p.damage_random = Seed(roll);
                Uint32 random = p.damage_random;
                if (count)
                    (void)SotnCombat_Random(&random);
                const bool dodge = count && lucks[l] * count >= roll;
                CHECK(Contact(&p, 1, 1, 1) && p.dodge.applied == dodge &&
                      p.guard.applied == !dodge && p.hp == (dodge ? 40 : 39) &&
                      p.damage_random == random && p.step == SOTN_PLAYER_STAND &&
                      p.invincibility == 12 && p.timers[3] == 6 && !p.heart_gain.requested);
            }
    Close(f);
    return true;
}
static bool Ownership(Fixture *f, SotnDataView exe)
{
    CHECK(Create(f) && SotnPlayer_SetScriptContext(f->player, exe));
    SotnPlayer *p = f->player;
    SotnCharacterState profile = {0};
    SotnCharacterStats stats = {0};
    CHECK(SotnCharacter_Collect(f->character, true, 83) &&
          SotnCharacter_Collect(f->character, true, 83) &&
          SotnCharacter_Equip(f->character, SOTN_EQUIP_RING1, 83) &&
          SotnCharacter_Equip(f->character, SOTN_EQUIP_RING2, 83) &&
          SotnCharacter_Get(f->character, &profile, &stats) &&
          SotnPlayer_SetCharacter(p, &profile, stats) && p->talismans == 2);
    p->hp = 1;
    p->damage_random = Seed(0);
    CHECK(Contact(p, 1, 9999, 1) && p->hp == 1 && p->dodge.applied && p->step == SOTN_PLAYER_STAND);
    p->invincibility = 0;
    p->x.parts.low = 123;
    p->y.parts.low = 456;
    p->damage_random = Seed(0);
    const Sint32 x = p->x.value, y = p->y.value;
    p->poison_ticks = p->curse_ticks = 40;
    p->cape = 54;
    p->armor = 22;
    SotnStatBuffs buffs = {0};
    CHECK(SotnPlayer_SetStatBuffContext(p, &buffs, p->attributes));
    CHECK(SotnPlayer_QueueMailHealing(p) && Contact(p, 2, 9999, 0x181) && p->hp == 2 &&
          p->dodge.applied && p->dodge.effects == 0x180 && !p->guard.applied &&
          p->step == SOTN_PLAYER_STAND && p->dodge.source.x == x && p->dodge.source.y == y &&
          p->poison_ticks == 39 && p->curse_ticks == 39 && !p->sound && !p->extra_sound &&
          !p->heart_gain.requested && !p->defense_buff_minimum && p->healing.applied_amount == 1);
    CHECK(SotnPlayer_ChangeRoom(p, p->map, 2, -1) && p->dodge.source.x == x + 131072 &&
          p->dodge.source.y == y - 65536);
    const Uint32 random = p->damage_random;
    CHECK(SotnPlayer_TickSpellFreeze(p, 0, false) && !p->dodge.applied && p->invincibility == 12 &&
          p->timers[3] == 6 && p->damage_random == random);
    p->invincibility = 0;
    p->damage_random = Seed(0);
    CHECK(SotnPlayer_QueueContact(p, (SotnPlayerContact){.attack = 9999, .parameters = 1}) &&
          SotnPlayer_SetDamageContext(p, 3));
    TestCharacter_Put32(f->dra + 0x101b8 + p->animation.id * 4U, 0x801bffff);
    const SotnPlayer before = *p;
    CHECK(!SotnPlayer_Tick(p, 0) && !SDL_memcmp(p, &before, sizeof(*p)));
    TestCharacter_Put32(f->dra + 0x101b8 + p->animation.id * 4U, 0x800a0200);
    p->script_executable = (SotnDataView){0};
    const SotnPlayer unbound = *p;
    CHECK(!SotnPlayer_Tick(p, 0) && !SDL_memcmp(p, &unbound, sizeof(*p)));
    CHECK(SotnPlayer_SetScriptContext(p, exe));
    CHECK(SotnPlayer_Tick(p, 0) && p->dodge.applied && p->hp == 2);
    Close(f);
    // Every earlier response and invincibility bypasses the luck roll.
    for (Uint16 gate = 0; gate < 6; ++gate)
    {
        CHECK(Create(f) && SotnPlayer_SetScriptContext(f->player, exe));
        p = f->player;
        p->talismans = 2;
        p->damage_random = Seed(0);
        Uint32 seed = p->damage_random;
        if (gate == 5)
        {
            (void)SotnCombat_Random(&seed);
            (void)SotnCombat_Random(&seed);
        }
        if (gate == 0)
            p->attributes.immunity = 0x8000;
        if (gate == 1)
            p->attributes.absorption = 0x8000;
        if (gate == 2)
            p->head = 43;
        if (gate == 3)
            p->invincibility = 2;
        if (gate == 4)
            CHECK(SotnPlayer_SetWaterContext(p, (SotnPlayerWater){41, 25, 0}));
        bool born = false;
        CHECK(SotnPlayer_QueueContact(p, (SotnPlayerContact){.attack = 1,
                                                             .parameters = gate == 2   ? 7
                                                                           : gate == 5 ? 0x201
                                                                           : gate == 4 ? 0x8000
                                                                                       : 0x8001}) &&
              SotnPlayer_SetDamageContext(p, 10) && SotnPlayer_ContactTick(p, &born) &&
              !p->dodge.applied && p->damage_random == seed);
        Close(f);
    }
    return true;
}
static void Put16(Uint8 *p, Uint16 value)
{
    p[0] = (Uint8)value;
    p[1] = (Uint8)(value >> 8);
}
static bool Shared(Fixture *f, SotnDataView exe)
{
    CHECK(Create(f));
    Uint8 *dra = f->dra;
    // Authored factory schedule and profiles; no original game table is included.
    const Uint16 ids[] = {44, 115};
    for (Uint16 k = 0; k < 2; ++k)
    {
        Uint8 *b = dra + 0xd1d4 + ids[k] * 6U;
        b[0] = k ? 46 : 11;
        b[1] = k ? 2 : 1;
        b[2] = k ? 0xc2 : 0x81;
    }
    dra[0xd4b8] = 32;
    dra[0xd4b9] = 63;
    for (Uint16 k = 0; k < 2; ++k)
    {
        Uint8 *p = dra + 0xd670 + (k ? 0x18 : 0x26) * 20U;
        Put16(p + 6, 0x103);
        Put16(p + 8, 256);
        Put16(p + 10, 256);
        Put16(p + 12, 256);
        Put16(p + 14, k ? 0x7007 : 16);
        Put16(p + 18, 512);
    }
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r)), *saved = SDL_malloc(sizeof(*saved));
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    CHECK(r && saved && actors);
    SotnBatRuntimeInput in = {
        .dra = {dra, sizeof(f->dra)},
        .executable = exe,
        .image = {12, 24, -6, -12},
        .talisman_image = {16, 32, -8, -16},
        .player = {.hp = 40,
                   .hp_max = 70,
                   .priority = 148,
                   .x = 100 * 65536,
                   .y = 140 * 65536,
                   .dodge = {.applied = true,
                             .source = {.x = 100 * 65536 + 123, .y = 140 * 65536 + 456}}}};
    CHECK(SotnBatRuntime_Tick(r, f->character, in) && SotnActors_CommitBatPrimitives(actors, r));
    CHECK(r->children[47].kind == 46 && r->children[46].kind == 46 && r->children[45].kind == 11 &&
          r->children[47].primitive_count == 8 && r->children[46].parameter == 1 &&
          !r->sound_count && r->children[47].state.talisman.x == 100 * 65536 + 123 &&
          r->children[47].state.talisman.velocity_y == -3 * 65536 && r->afterimage_reset);
    *saved = *r;
    r->children[0] = r->children[47];
    r->children[47] = (SotnBatRuntimeChild){0};
    CHECK(!SotnBatRuntime_Tick(r, f->character, in) && !SotnActors_CommitBatPrimitives(actors, r));
    *r = *saved;
    const SotnPlayerEffectQuad *q = NULL;
    Uint16 count = 0;
    CHECK(SotnBatRuntime_Visual(r, 47, &q, &count) && count == 8 && q[0].u[0] == 132 &&
          q[0].v[0] == 129 && q[0].priority == 140 && q[0].red[0] == 128 && !q[0].green[0] &&
          q[0].draw_mode == 0x137);
    CHECK(SotnBatRuntime_Visual(r, 46, &q, &count) && count == 8 && q[0].palette == 0x100 &&
          q[0].green[0] == 128 && q[0].draw_mode == 0x117);
    in.player.dodge.applied = false;
    const Sint32 x = r->children[47].state.talisman.x;
    in.entering = true;
    in.screen_shift_x = -3;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, f->character, in) &&
          r->children[47].state.talisman.x == x - 3 * 65536 &&
          r->children[47].state.talisman.gray == 128);
    in.entering = false;
    in.screen_shift_x = 0;
    in.freeze = 3;
    for (Uint16 tick = 0; tick < 150; ++tick)
    {
        in.game_timer = tick;
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots));
        if (tick == 0)
        {
            *saved = *r;
            in.talisman_image.width = 0;
            CHECK(!SotnBatRuntime_Tick(r, f->character, in) && !SDL_memcmp(saved, r, sizeof(*r)));
            in.talisman_image.width = 16;
        }
        CHECK(SotnBatRuntime_Tick(r, f->character, in) &&
              SotnActors_CommitBatPrimitives(actors, r));
        if (tick == 0)
            CHECK(r->children[47].state.talisman.x == x - 3 * 65536);
        if (tick == 1)
            CHECK(r->children[47].state.talisman.x == x - 3 * 65536 - 0x1aaaa);
    }
    CHECK(!SotnBatRuntime_Occupied(r));
    for (Uint16 row = 0; row < SOTN_PRIMITIVE_SLOTS; ++row)
        CHECK(!actors->primitives[row].owner);
    // Actual slot order gives Blink eight rows first, then one entity46 child;
    // optional sibling exhaustion cannot replace an unrelated row.
    *r = (SotnBatRuntime){0};
    for (Uint16 row = 0; row < SOTN_PRIMITIVE_SLOTS - 16; ++row)
    {
        actors->primitives[row].owner = 2;
        actors->primitives[row].ability_kind = 0x7f; // no shared-runtime kind
    }
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots));
    in.freeze = 0;
    in.player.dodge.applied = true;
    CHECK(SotnBatRuntime_Tick(r, f->character, in) && SotnActors_CommitBatPrimitives(actors, r) &&
          !r->children[47].kind && r->children[46].kind == 46 &&
          r->children[46].primitive_count == 8 && r->children[45].kind == 11 &&
          r->children[45].initialized && actors->primitives[0].ability_kind == 0x7f);
    *r = (SotnBatRuntime){0};
    in.freeze = 0;
    in.player.dodge.applied = true;
    SDL_memset(&in.primitive_slots, 255, sizeof(in.primitive_slots));
    CHECK(SotnBatRuntime_Tick(r, f->character, in) && !r->children[47].kind &&
          !r->children[46].kind && r->children[45].kind == 11 && !r->children[45].initialized);
    SDL_free(saved);
    SDL_free(r);
    SDL_free(actors);
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
        Put16(exe + 0x1dbcc + i * 2U, i * 4U);
        Put16(exe + 0x22a10 + i * 2U, i / 2U);
    }
    for (Uint16 i = 0; i < 192; ++i)
        Put16(exe + 0x1e3e0 + i * 2U, 4096);
    const SotnDataView data = {exe, 0x24000};
    const bool ok = Rolls(f, data) && Ownership(f, data) && Shared(f, data);
    SDL_free(exe);
    SDL_free(f);
    return ok ? 0 : 1;
}
