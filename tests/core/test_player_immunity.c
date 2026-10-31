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
static bool ContactOwnership(Fixture *f)
{
    CHECK(Create(f));
    SotnPlayer *p = f->player;
    f->dra[0x3400 + 44 * 32U + 28] = 0;
    f->dra[0x3400 + 44 * 32U + 21] = 1;
    SotnCharacterState profile = {0};
    SotnCharacterStats attributes = {0};
    CHECK(SotnCharacter_Collect(f->character, true, 44) &&
          SotnCharacter_Equip(f->character, SOTN_EQUIP_HEAD, 44) &&
          SotnCharacter_Get(f->character, &profile, &attributes) &&
          SotnPlayer_SetCharacter(p, &profile, attributes) && p->head == 44 &&
          p->attributes.immunity == 0x100);
    p->attributes.absorption = 0x100;
    p->attributes.weakness = p->attributes.resistance = 0x100;
    p->poison_ticks = 40;
    p->x.parts.low = 123;
    p->y.parts.low = 456;
    const Sint32 x = p->x.value, y = p->y.value;
    CHECK(SotnPlayer_QueueMailHealing(p) && Contact(p, 1, 9999, 0x101) && p->hp == 41 &&
          p->immunity.applied && !p->absorption.applied && p->invincibility == 12 &&
          p->poison_ticks == 39 && !p->heart_gain.requested && !p->defense_buff_minimum &&
          !p->sound && p->step != SOTN_PLAYER_HIT && p->healing.applied_amount == 1 &&
          p->immunity.source.x == x && p->immunity.source.y == y);
    SotnPlayerState state = {0};
    CHECK(SotnPlayer_GetState(p, &state) && state.immunity.applied && state.previous_contacts == 1);
    CHECK(SotnPlayer_ChangeRoom(p, p->map, 2, -1) && p->immunity.source.x == x + 2 * 65536 &&
          p->immunity.source.y == y - 65536);
    CHECK(SotnPlayer_TickSpellFreeze(p, 0, false) && !p->immunity.applied &&
          p->invincibility == 12);
    for (Uint32 t = 2; t < 14; ++t)
        CHECK(Contact(p, t, 9, 0x101) && p->hp == 41 && p->immunity.applied == (t == 13) &&
              p->invincibility == (t == 13 ? 12 : 13 - t));
    p->invincibility = 0;
    CHECK(SotnPlayer_QueueContact(p, (SotnPlayerContact){.attack = 9, .parameters = 0x101}) &&
          SotnPlayer_SetDamageContext(p, 14));
    TestCharacter_Put32(f->dra + 0x101b8 + p->animation.id * 4U, 0x801bffff);
    const SotnPlayer before = *p;
    CHECK(!SotnPlayer_Tick(p, 0) && !SDL_memcmp(&before, p, sizeof(*p)));
    TestCharacter_Put32(f->dra + 0x101b8 + p->animation.id * 4U, 0x800a0200);
    CHECK(SotnPlayer_Tick(p, 0) && p->hp == 41 && p->immunity.applied && !p->queued_contact.attack);
    Close(f);
    return true;
}
static void Put16(Uint8 *p, Uint16 value)
{
    p[0] = (Uint8)value;
    p[1] = (Uint8)(value >> 8);
}
static bool Shared(SotnDataView exe)
{
    Uint8 *dra = SDL_calloc(1, 0x11000);
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r)), *saved = SDL_malloc(sizeof(*saved));
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    CHECK(dra && r && saved && actors);
    TestCharacter_Fill(dra);
    for (Uint16 k = 0; k < 2; ++k)
    {
        Uint8 *b = dra + 0xd1d4 + (k ? 47 : 44) * 6;
        b[0] = k ? 37 : 11;
        b[1] = 1;
        b[2] = 0x81;
    }
    dra[0xd4b8] = 32;
    dra[0xd4b9] = 63;
    dra[0xcfec] = 1;
    dra[0xcfed] = 2;
    dra[0xcfee] = 25;
    dra[0xcfef] = 10;
    Put16(dra + 0xcff0, 0x1b4);
    Put16(dra + 0xcff2, 1);
    const Uint16 params[] = {0x43, 0x51};
    for (Uint16 i = 0; i < 2; ++i)
    {
        Uint8 *p = dra + 0xd670 + (params[i] & 63U) * 20;
        Put16(p + 6, 0x100);
        Put16(p + 8, 256);
        Put16(p + 10, 256);
        Put16(p + 12, 256);
        Put16(p + 14, 2);
        Put16(p + 16, 0x31);
        Put16(p + 18, 512);
    }
    SotnCharacter *character = NULL;
    CHECK(SotnCharacter_Create((SotnDataView){dra, 0x11000}, &character));
    SotnBatRuntimeInput in = {.dra = {dra, 0x11000},
                              .executable = exe,
                              .image = {12, 24, -6, -12},
                              .player = {.animation = {.sprite = 1},
                                         .hp = 70,
                                         .hp_max = 70,
                                         .priority = 148,
                                         .x = 100 * 65536 + 123,
                                         .y = 140 * 65536 + 456,
                                         .immunity = {.applied = true}}};
    CHECK(SotnBatRuntime_Tick(r, character, in));
    CHECK(!r->sound_count && r->children[47].kind == 37 &&
          r->children[47].state.immunity_text.phase == 1 && !r->children[47].primitive_count &&
          r->children[46].parameter == 0x4300 && r->children[45].parameter == 0x5100 &&
          r->children[40].kind == 19 && r->children[40].state.healing_number.damage &&
          r->children[40].state.healing_number.digits[0].palette == 0x1b4);
    CHECK(SotnActors_CommitBatPrimitives(actors, r));
    in.player.immunity.applied = false;
    for (Uint16 t = 0; t < 3; ++t)
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && !r->children[47].primitive_count &&
              r->children[47].state.immunity_text.phase == 1 &&
              SotnActors_CommitBatPrimitives(actors, r));
    in.player.previous_contacts = 1;
    CHECK(SotnBatRuntime_Tick(r, character, in) && r->children[47].state.immunity_text.phase == 2);
    *saved = *r;
    dra[0xcff2] = 2;
    CHECK(!SotnBatRuntime_Tick(r, character, in) && !SDL_memcmp(saved, r, sizeof(*r)));
    dra[0xcff2] = 1;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
    CHECK(r->children[47].primitive_count == 18 && r->children[47].state.immunity_text.phase == 3);
    for (Uint16 row = 0; row < 18; ++row)
        CHECK(actors->primitives[r->children[47].primitives[row]].ability_kind == 37);
    const SotnImmunityText prior = r->children[47].state.immunity_text;
    in.entering = true;
    in.screen_shift_x = -2;
    CHECK(SotnBatRuntime_Tick(r, character, in) &&
          r->children[47].state.immunity_text.x == prior.x - 2 * 65536 &&
          r->children[47].state.immunity_text.timer == prior.timer);
    in.entering = false;
    in.screen_shift_x = 0;
    in.freeze = 3;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) &&
          r->children[47].state.immunity_text.timer == prior.timer - 1 &&
          SotnActors_CommitBatPrimitives(actors, r));
    for (Uint16 t = 0; t < 160; ++t)
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
    CHECK(!SotnBatRuntime_Occupied(r));
    for (Uint16 row = 0; row < SOTN_PRIMITIVE_SLOTS; ++row)
        CHECK(!actors->primitives[row].owner);
    in.freeze = 0;
    in.player.immunity.applied = true;
    in.other_occupied = ~((Uint64)255 << 8);
    CHECK(SotnBatRuntime_Tick(r, character, in) && !SotnBatRuntime_Occupied(r));
    in.other_occupied = 0;
    SDL_memset(&in.primitive_slots, 255, sizeof(in.primitive_slots));
    CHECK(SotnBatRuntime_Tick(r, character, in));
    in.player.immunity.applied = false;
    CHECK(!r->children[40].kind);
    CHECK(SotnBatRuntime_Tick(r, character, in) && SotnBatRuntime_Tick(r, character, in) &&
          !r->children[47].kind);
    SotnCharacter_Destroy(character);
    SDL_free(actors);
    SDL_free(saved);
    SDL_free(r);
    SDL_free(dra);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    const size_t size = 0x22a10 + 2050;
    Uint8 *exe = SDL_calloc(1, size);
    if (!f || !exe)
        return 1;
    for (Uint16 i = 0; i <= 1024; ++i)
    {
        Put16(exe + 0x1dbcc + i * 2U, i * 4U);
        Put16(exe + 0x22a10 + i * 2U, i / 2U);
    }
    for (Uint16 i = 0; i < 192; ++i)
        Put16(exe + 0x1e3e0 + i * 2U, 4096);
    const bool ok = ContactOwnership(f) && Shared((SotnDataView){exe, size});
    SDL_free(f);
    SDL_free(exe);
    return ok ? 0 : 1;
}
