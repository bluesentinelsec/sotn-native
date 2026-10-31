#include "character_fixture.h"
#include <SDL3/SDL.h>
#include <sotn/bat_runtime.h>
#include <sotn/combat.h>

_Static_assert(SOTN_PLAYER_BAT == 5 && SOTN_PLAYER_UNMORPH_BAT == 9 && SOTN_BUTTON_BAT == 8 &&
                   SOTN_BUTTON_START == 0x800,
               "Original Bat states and its shoulder input must retain their US identities");

#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return false;                                                                          \
        }                                                                                          \
    } while (0)

typedef struct Fixture
{
    Uint8 dra[0x11000], exe[0x24000], tiles[64 * 16 * 2], types[3];
    SotnPlayer *player;
    SotnCharacter *character;
    SotnPlayerState state;
    SotnWorldState world;
    SotnBatRuntime runtime;
    Uint64 occupied;
    SotnPrimitiveSlots primitive_slots;
    Uint32 tick, spent;
    bool wolf_owned;
} Fixture;
static void Put16(Uint8 *p, Uint16 n)
{
    p[0] = (Uint8)n;
    p[1] = (Uint8)(n >> 8);
}
static void Put32(Uint8 *p, Uint32 n)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(n >> (i * 8U));
}
static bool WolfQuery(void *context, Sint32 x, Sint32 y, SotnCollider *out)
{
    Fixture *f = context;
    const SotnCollisionMap map = {
        {f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 64, 16};
    return SotnCollision_Query(&map, x, y, out);
}
static bool Tick(Fixture *f, Uint16 buttons)
{
    SotnCharacterState progress = {0};
    SotnCharacterStats stats = {0};
    const SotnPlayerBlink *effect = NULL;
    const SotnPlayerFactory *pending = NULL;
    CHECK(SotnPlayer_GetState(f->player, &f->state));
    CHECK(SotnCharacter_Get(f->character, &progress, &stats));
    CHECK(SotnPlayer_SetCharacter(f->player, &progress, stats));
    CHECK(SotnPlayer_SetRelics(f->player, &f->world));
    CHECK(SotnPlayer_SetMistContext(f->player, NULL, 0, f->tick, false));
    CHECK(SotnBatRuntime_Context(&f->runtime, f->state.bat.generation, &effect, &pending));
    CHECK(SotnPlayer_SetBatRuntimeContext(f->player, effect, pending,
                                          effect || pending ? f->state.bat.generation : 0,
                                          SotnBatRuntime_Available(&f->runtime)));
    if (f->wolf_owned)
    {
        const SotnPlayerBlink *wolf_effect = NULL;
        const SotnPlayerFactory *wolf_factory = NULL;
        CHECK(SotnBatRuntime_Context(&f->runtime, f->state.wolf.generation, &wolf_effect,
                                     &wolf_factory));
        CHECK(SotnPlayer_SetWolfContext(f->player, &f->runtime.wolf, wolf_effect, wolf_factory,
                                        wolf_effect || wolf_factory ? f->state.wolf.generation : 0,
                                        SotnBatRuntime_Available(&f->runtime), 0, 0));
    }
    CHECK(SotnPlayer_Tick(f->player, buttons) && SotnPlayer_GetState(f->player, &f->state));
    CHECK(SotnCharacter_SpendMP(f->character, f->state.mp_spent));
    f->spent += f->state.mp_spent;
    if (f->state.bat_commands.learn_wing_smash)
    {
        CHECK(SotnCharacter_Get(f->character, &progress, NULL));
        CHECK(SotnCharacter_LearnSpell(&progress, 6));
        CHECK(SotnCharacter_Restore(f->character, &progress));
    }
    const bool bat = f->state.animation_bank == 13;
    CHECK(SotnBatRuntime_Tick(
        &f->runtime, f->character,
        (SotnBatRuntimeInput){
            .dra = {f->dra, sizeof(f->dra)},
            .executable = {f->exe, sizeof(f->exe)},
            .player = f->state,
            .image = bat ? (SotnPlayerImage){32, 48, -16, (Sint16)(-24 + f->state.bat.settle_y)}
                         : (SotnPlayerImage){40, 64, -20, -48},
            .other_occupied = f->occupied,
            .wolf_owned = f->wolf_owned,
            .wolf_query = WolfQuery,
            .wolf_context = f,
            .primitive_slots = f->primitive_slots,
            .force_echo_enabled = (f->world.relics[SOTN_RELIC_FORCE_ECHO] & 3U) == 3}));
    if (f->runtime.use_ticks)
        CHECK(SotnPlayer_SetUseTimer(f->player, f->runtime.use_ticks));
    if (f->wolf_owned && f->state.step == SOTN_PLAYER_WOLF && f->runtime.wolf.generation)
    {
        CHECK(SotnPlayer_CommitWolfBody(f->player, &f->runtime.wolf, &f->runtime.wolf_frame, 0, 0));
        f->spent += f->runtime.wolf_events.mp_spent;
        CHECK(SotnPlayer_GetState(f->player, &f->state));
    }
    ++f->tick;
    return true;
}
static bool Create(Fixture *f)
{
    SDL_memset(f, 0, sizeof(*f));
    TestCharacter_Fill(f->dra);
    for (Uint16 i = 0; i < 247; ++i)
        Put32(f->dra + 0x101b8 + i * 4U, 0x800a0100);
    for (Uint16 i = 0; i < 6; ++i)
        Put32(f->dra + 0x100 + i * 4U, 2U | ((Uint32)i + 1U) << 16);
    Put32(f->dra + 0x118, 0);
    Put32(f->dra + 0x101b8 + 0xca * 4U, 0x800afda4);
    Put16(f->dra + 0xfda4, 1);
    for (Uint16 i = 0; i < 1025; ++i)
    {
        Put16(f->exe + 0x1dbcc + i * 2U, (Uint16)(i * 4U));
        Put16(f->exe + 0x22a10 + i * 2U, (Uint16)(i / 2U));
    }
    for (Uint16 i = 0; i < 192; ++i)
        Put16(f->exe + 0x1e3e0 + i * 2U, 4096);
    const Uint16 ids[4] = {0x20, 0x21, 0x1b, 0x1c};
    for (Uint16 i = 0; i < 4; ++i)
    {
        Uint8 *p = f->dra + 0xd670 + ids[i] * 20U;
        Put16(p + 6, 0x123);
        Put16(p + 8, 256);
        Put16(p + 10, 256);
        Put16(p + 12, 256);
        Put16(p + 14, i == 2 ? 0x7008 : i == 3 ? 0x7009 : 16);
        Put16(p + 16, 0x31);
        Put16(p + 18, 512);
    }
    for (Uint16 i = 0; i < 17; ++i)
    {
        Put32(f->dra + 0x109f8 + i * 4U, 0x800b0800U + i * 16U);
        for (Uint16 j = 0; j < 8; ++j)
            Put16(f->dra + 0x10800 + i * 16U + j * 2U, 1);
    }
    for (Uint16 i = 0; i < 6; ++i)
    {
        Put16(f->dra + 0x10abc + i * 4U, i == 5 ? 65535 : 2);
        Put16(f->dra + 0x10abe + i * 4U, 72);
    }
    const Uint16 wolf_profiles[] = {0x23, 0x24, 0x25};
    for (Uint16 i = 0; i < 3; ++i)
    {
        Uint8 *profile = f->dra + 0xd670 + wolf_profiles[i] * 20U;
        Put16(profile + 6, 0x123);
        Put16(profile + 8, 256);
        Put16(profile + 10, 256);
        Put16(profile + 12, 256);
        Put16(profile + 14, i == 2 ? 0x700b : 16);
        Put16(profile + 16, 0x31);
        Put16(profile + 18, 512);
    }
    f->types[1] = 3;
    f->types[2] = 0xf8;
    for (Uint16 y = 10; y < 16; ++y)
        for (Uint16 x = 0; x < 64; ++x)
            f->tiles[(y * 64U + x) * 2U] = 1;
    // Authored descriptors exercise shared factory allocation; original tables stay private.
    const Uint8 ids_bp[] = {44, 65, 67}, kinds[] = {0, 2, 6}, children[] = {11, 35, 36};
    for (Uint16 i = 0; i < 3; ++i)
    {
        Uint8 *bp = f->dra + 0xd1d4 + ids_bp[i] * 6U;
        bp[0] = children[i];
        bp[1] = 1;
        bp[2] = 1;
        bp[4] = kinds[i];
    }
    f->dra[0xd4b8] = 32;
    f->dra[0xd4b9] = 63;
    f->dra[0xd4bc] = 32;
    f->dra[0xd4bd] = 47;
    f->dra[0xd4c4] = 17;
    f->dra[0xd4c5] = 31;
    Uint8 *spell = f->dra + 0x8410 + 6 * 28;
    spell[12] = 8;
    spell[13] = 10;
    spell[14] = 9;
    spell[16] = 2;
    spell[18] = 3;
    spell[20] = 16;
    spell[24] = 30;
    CHECK(SotnCharacter_Create((SotnDataView){f->dra, sizeof(f->dra)}, &f->character));
    f->world.relics[SOTN_RELIC_BAT] = 3;
    const SotnCollisionMap map = {
        {f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 64, 16};
    CHECK(SotnPlayer_Create((SotnDataView){f->dra, sizeof(f->dra)}, map, 135, 135, &f->player));
    for (Uint16 i = 0; i < 20; ++i)
        CHECK(Tick(f, 0));
    return true;
}
static void Close(Fixture *f)
{
    SotnPlayer_Close(f->player);
    SotnCharacter_Destroy(f->character);
}
static bool Enter(Fixture *f)
{
    CHECK(Tick(f, SOTN_BUTTON_BAT));
    for (Uint16 i = 0; i < 45; ++i)
        CHECK(Tick(f, 0));
    CHECK(f->state.step == SOTN_PLAYER_BAT && f->state.substep == 1 &&
          f->state.animation_bank == 13);
    return true;
}
static bool Command(Fixture *f)
{
    const Uint16 dirs[] = {0x1000, 0x9000, 0x8000, 0xc000, 0x4000, 0x6000, 0x2000};
    for (Uint16 i = 0; i < 7; ++i)
        CHECK(Tick(f, (Uint16)(dirs[i] | SOTN_BUTTON_JUMP)));
    CHECK(Tick(f, 0));
    CHECK(f->state.bat_commands.wing_attack && f->state.bat_commands.mp_spent == 8);
    return true;
}
static bool Wing(Fixture *f)
{
    CHECK(Create(f) && Enter(f) && Command(f));
    CHECK(f->runtime.wing_mask == 1 && f->runtime.children[1].kind == 36);
    const SotnBatRuntimeChild child = f->runtime.children[1];
    CHECK(child.initialized && child.group == 3 && child.state.attack.critical_rate == 16 &&
          child.state.attack.active && child.state.attack.width == 12 &&
          child.state.attack.offset_x == 4);
    SotnCharacterState progress = {0};
    CHECK(SotnCharacter_Get(f->character, &progress, NULL));
    CHECK(progress.spells_learned & (1U << 6));
    const Uint32 random = progress.random;
    CHECK(Tick(f, 0));
    CHECK(SotnCharacter_Get(f->character, &progress, NULL) && progress.random == random);
    bool trail = false, flash = false, returned = false, normal = false;
    for (Uint16 i = 0; i < 180; ++i)
    {
        CHECK(Tick(f, 0));
        returned |= f->state.bat.return_started;
        normal |= returned && !f->state.bat.active && f->state.animation_bank == 1;
        for (Uint16 j = 0; j < 48; ++j)
        {
            trail |= f->runtime.children[j].kind == 35;
            flash |=
                f->runtime.children[j].kind == 11 && f->runtime.children[j].parameter == 0x5c00;
        }
    }
    CHECK(trail && flash && returned && normal && !SotnBatRuntime_Occupied(&f->runtime));
    Close(f);
    CHECK(Create(f) && Enter(f) && Command(f) && Command(f));
    CHECK(f->runtime.wing_mask == 3 && f->runtime.children[1].group == 3 &&
          f->runtime.children[2].group == 4);
    Uint16 flashes = 0;
    for (Uint16 i = 0; i < 48; ++i)
        flashes +=
            (f->runtime.children[i].kind == 11 && f->runtime.children[i].parameter == 0x5c00);
    CHECK(flashes == 1);
    Close(f);
    return true;
}
static bool Fire(Fixture *f)
{
    CHECK(Create(f));
    Uint8 *bp = f->dra + 0xd1d4 + 81 * 6;
    bp[0] = 45;
    bp[1] = 1;
    bp[2] = 0x81;
    bp[4] = 2;
    Uint8 *spell = f->dra + 0x8410 + 9 * 28;
    spell[12] = 3;
    spell[13] = 10;
    spell[14] = 6;
    spell[16] = 2;
    spell[18] = 1;
    spell[20] = 16;
    spell[24] = 30;
    Put32(f->dra + 0x101b8 + 0xc9 * 4, 0x800a0200);
    Put32(f->dra + 0x200, 2U | (13U << 16));
    Put32(f->dra + 0x204, 0xffff);
    Put32(f->dra + 0x101b8 + 0xc3 * 4, 0x800a0220);
    Put32(f->dra + 0x220, 1U | (2U << 16));
    Put32(f->dra + 0x224, 1U | (3U << 16));
    Put32(f->dra + 0x228, 1U | (4U << 16));
    Put32(f->dra + 0x22c, 0xfffeU | (0xc4U << 16));
    Put32(f->dra + 0x101b8 + 0xc4 * 4, 0x800a0240);
    for (Uint16 i = 0; i < 9; ++i)
        Put32(f->dra + 0x240 + i * 4, 1U | ((Uint32)i + 1U) << 16);
    Put32(f->dra + 0x264, 0);
    for (Uint16 i = 0; i < 8; ++i)
        Put32(f->dra + 0x10798 + i * 4, 2U | ((Uint32)i + 100U) << 16);
    Put32(f->dra + 0x107b8, 0);
    SDL_memcpy(f->dra + 0xd670 + 5 * 20, f->dra + 0xd670 + 0x20 * 20, 20);
    CHECK(Enter(f));
    CHECK(Tick(f, SOTN_BUTTON_LEFT_HAND) && !f->state.bat_commands.fire_flash);
    CHECK(Tick(f, 0));
    f->world.relics[SOTN_RELIC_FIRE_BAT] = 3;
    CHECK(Tick(f, SOTN_BUTTON_LEFT_HAND) && f->state.bat_commands.fire_flash &&
          f->state.bat_commands.mp_spent == 3 && f->state.substep == 4);
    Uint16 wait = 0;
    while (!f->runtime.fire_mask && wait++ < 20)
        CHECK(Tick(f, 0));
    CHECK(f->runtime.fire_mask == ((Uint64)1 << 16));
    const SotnBatRuntimeChild child = f->runtime.children[16];
    CHECK(child.kind == 45 && child.initialized && child.group == 7 &&
          child.state.fire.frame == 100 && child.state.fire.attack.width == 4 &&
          child.state.fire.attack.height == 8 && child.state.fire.attack.critical_rate == 16 &&
          f->runtime.use_ticks == 4 && f->runtime.sound_count == 1 &&
          f->runtime.sounds[0] == 0x680);
    SotnCharacterState owned_before = {0};
    CHECK(SotnCharacter_Get(f->character, &owned_before, NULL));
    const SotnBatRuntime before_rebase = f->runtime;
    CHECK(SotnBatRuntime_Tick(
        &f->runtime, f->character,
        (SotnBatRuntimeInput){.entering = true, .room_shift_x = -16, .room_shift_y = 8}));
    CHECK(f->runtime.children[16].state.fire.motion.x == child.state.fire.motion.x - 16 * 65536 &&
          f->runtime.children[16].state.fire.motion.y == child.state.fire.motion.y + 8 * 65536 &&
          f->runtime.children[16].state.fire.frame == child.state.fire.frame &&
          f->runtime.children[16].state.fire.remaining == child.state.fire.remaining &&
          f->runtime.children[16].state.fire.motion.scale_x == child.state.fire.motion.scale_x);
    SotnCharacterState after_rebase = {0};
    CHECK(SotnCharacter_Get(f->character, &after_rebase, NULL) &&
          after_rebase.random == owned_before.random);
    f->runtime = before_rebase;
    // A malformed next private animation record cannot advance any owned state/RNG.
    f->runtime.children[16].state.fire.remaining = 1;
    const SotnBatRuntime invalid_before = f->runtime;
    Put16(f->dra + 0x10798 + 4, 0);
    SotnBatRuntimeInput malformed = {.dra = {f->dra, sizeof(f->dra)},
                                     .executable = {f->exe, sizeof(f->exe)},
                                     .player = f->state,
                                     .image = {32, 48, -16, -24}};
    malformed.player.bat_commands = (SotnBatCommandEvents){0};
    CHECK(!SotnBatRuntime_Tick(&f->runtime, f->character, malformed));
    CHECK(!SDL_memcmp(&invalid_before, &f->runtime, sizeof(invalid_before)));
    CHECK(SotnCharacter_Get(f->character, &after_rebase, NULL) &&
          after_rebase.random == owned_before.random);
    Put16(f->dra + 0x10798 + 4, 2);
    f->runtime = before_rebase;
    CHECK(!SotnBatRuntime_Hit(&f->runtime, 16, child.generation + 1, 1));
    CHECK(SotnBatRuntime_Hit(&f->runtime, 16, child.generation, 2));
    CHECK(Tick(f, 0) && !f->runtime.fire_mask && !f->runtime.children[16].kind);
    for (Uint16 i = 0; i < 35; ++i)
        CHECK(Tick(f, 0));
    CHECK(f->state.substep == 1);
    CHECK(Tick(f, SOTN_BUTTON_LEFT_HAND));
    wait = 0;
    while (!f->runtime.fire_mask && wait++ < 20)
        CHECK(Tick(f, 0));
    CHECK(f->runtime.fire_mask && f->runtime.children[16].generation > child.generation);
    CHECK(Tick(f, SOTN_BUTTON_BAT));
    CHECK(f->runtime.fire_mask); // A launched shot outlives its owner's form.
    for (Uint16 i = 0; i < 180; ++i)
        CHECK(Tick(f, 0));
    CHECK(!f->state.bat.active && !SotnBatRuntime_Occupied(&f->runtime));
    Close(f);
    return true;
}
static bool Capacity(Fixture *f)
{
    CHECK(Create(f));
    f->occupied = SDL_MAX_UINT64 << 32;
    CHECK(Tick(f, SOTN_BUTTON_BAT));
    for (Uint16 i = 0; i < 10; ++i)
        CHECK(Tick(f, 0));
    CHECK(f->state.substep == 0 && f->state.bat.signal == 1 && f->state.animation_bank == 1 &&
          SotnBatRuntime_Occupied(&f->runtime) == ((Uint64)1 << 8));
    CHECK(!SotnPlayer_SetBatRuntimeContext(f->player, NULL, NULL, 0, true));
    const SotnPlayerBlink *effect = NULL;
    const SotnPlayerFactory *pending = NULL;
    CHECK(SotnBatRuntime_Context(&f->runtime, f->state.bat.generation, &effect, &pending));
    CHECK(!effect && pending && pending->blueprint == 44);
    f->occupied &= ~((Uint64)1 << 63);
    for (Uint16 i = 0; i < 45; ++i)
        CHECK(Tick(f, 0));
    CHECK(f->state.substep == 1 && f->state.animation_bank == 13 &&
          f->runtime.children[47].kind == 11);
    CHECK(Tick(f, SOTN_BUTTON_BAT));
    bool normal = false;
    for (Uint16 i = 0; i < 150; ++i)
    {
        CHECK(Tick(f, 0));
        normal |= !f->state.bat.active;
    }
    CHECK(normal && !SotnBatRuntime_Occupied(&f->runtime));
    Close(f);
    return true;
}
static bool Wall(Fixture *f)
{
    CHECK(Create(f));
    for (Uint16 y = 0; y < 10; ++y)
        f->tiles[(y * 64U + 12U) * 2U] = 2;
    CHECK(Enter(f) && Command(f));
    bool impact = false;
    for (Uint16 i = 0; i < 30; ++i)
    {
        CHECK(Tick(f, 0));
        if (f->state.bat.return_started)
        {
            CHECK(f->state.bat_commands.wall_impact && f->state.sound == 0x644 &&
                  f->state.velocity_x == 0);
            impact = true;
            break;
        }
    }
    CHECK(impact && !f->runtime.wing_mask);
    for (Uint16 i = 0; i < 150; ++i)
        CHECK(Tick(f, 0));
    CHECK(!f->state.bat.active && !SotnBatRuntime_Occupied(&f->runtime));
    Close(f);
    return true;
}
static bool AtomicAndRebase(Fixture *f)
{
    CHECK(Create(f) && Enter(f) && Command(f));
    for (Uint16 i = 0; i < 4; ++i)
        CHECK(Tick(f, 0));
    SotnBatRuntime before = f->runtime;
    SotnCharacterState profile = {0}, after = {0};
    CHECK(SotnCharacter_Get(f->character, &profile, NULL));
    SotnBatRuntimeInput in = {.entering = true,
                              .room_shift_x = -32,
                              .room_shift_y = 16,
                              .screen_shift_x = 3,
                              .screen_shift_y = -4};
    CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in));
    for (Uint16 i = 0; i < 48; ++i)
    {
        if (!before.children[i].kind)
            continue;
        CHECK(f->runtime.children[i].source.x == before.children[i].source.x - 32 * 65536);
        if (before.children[i].kind == 35)
            CHECK(f->runtime.children[i].state.trail.x ==
                      before.children[i].state.trail.x - 32 * 65536 &&
                  f->runtime.children[i].state.trail.scale == before.children[i].state.trail.scale);
    }
    CHECK(SotnCharacter_Get(f->character, &after, NULL) && after.random == profile.random);
    // Rebase and invalid requests preserve both owners on failure.
    before = f->runtime;
    in.other_occupied = SotnBatRuntime_Occupied(&f->runtime);
    CHECK(!SotnBatRuntime_Tick(&f->runtime, f->character, in));
    CHECK(!SDL_memcmp(&before, &f->runtime, sizeof(before)));
    in.other_occupied = 0;
    in.player.bat.started = true;
    CHECK(!SotnBatRuntime_Tick(&f->runtime, f->character, in));
    CHECK(!SDL_memcmp(&before, &f->runtime, sizeof(before)));
    in.player = (SotnPlayerState){0};
    in = (SotnBatRuntimeInput){.dra = {f->dra, sizeof(f->dra)},
                               .executable = {f->exe, sizeof(f->exe)},
                               .player = f->state,
                               .image = {32, 48, -16, -24}};
    in.player.bat_commands = (SotnBatCommandEvents){.wing_attack = true};
    const size_t spell = 0x8410U + 6U * 28U;
    Put16(f->dra + spell + 24, 65535);
    CHECK(!SotnBatRuntime_Tick(&f->runtime, f->character, in));
    CHECK(!SDL_memcmp(&before, &f->runtime, sizeof(before)));
    CHECK(SotnCharacter_Get(f->character, &after, NULL) && after.random == profile.random);
    Put16(f->dra + spell + 24, 30);
    in = (SotnBatRuntimeInput){.entering = true};
    f->runtime.children[1].group = 11;
    before = f->runtime;
    CHECK(!SotnBatRuntime_Tick(&f->runtime, f->character, in));
    CHECK(!SDL_memcmp(&before, &f->runtime, sizeof(before)));
    CHECK(SotnCharacter_Get(f->character, &after, NULL) &&
          !SDL_memcmp(&profile, &after, sizeof(profile)));
    Close(f);
    return true;
}
static bool Echo(Fixture *f)
{
    CHECK(Create(f));
    Uint8 *bp = f->dra + 0xd1d4 + 103 * 6;
    bp[0] = 52;
    bp[1] = 1;
    bp[2] = 129; // Authored noncritical single-child schedule.
    bp[4] = 9;
    f->dra[0xd4ca] = f->dra[0xd4cb] = 47;
    for (Uint16 i = 0; i < 17; ++i)
    {
        Put16(f->dra + 0x10884 + i * 2, (Uint16)(i % 6));
        Put16(f->dra + 0x108a8 + i * 2, (Uint16)(i * 8));
    }
    Uint8 *spell = f->dra + 0x8410 + 10 * 28;
    spell[13] = 20;
    spell[16] = 2;
    spell[18] = 3;
    spell[20] = 16;
    spell[24] = 5;
    CHECK(Enter(f));
    CHECK(Tick(f, SOTN_BUTTON_CANCEL) && !f->state.bat_commands.echo);
    CHECK(Tick(f, 0));
    f->world.relics[SOTN_RELIC_ECHO_BAT] = 3;
    SotnCharacterState before = {0}, after = {0};
    CHECK(SotnCharacter_Get(f->character, &before, NULL));
    CHECK(Tick(f, SOTN_BUTTON_CANCEL) && f->state.bat_commands.echo);
    CHECK(f->runtime.echo_generation && f->runtime.children[31].kind == 52 &&
          !f->runtime.echo_attack && f->runtime.echo.limit == 160 && f->runtime.sound_count == 1 &&
          f->runtime.sounds[0] == 0x61f && !f->state.mp_spent);
    CHECK(SotnCharacter_Get(f->character, &after, NULL) && after.random == before.random);
    const Uint32 generation = f->runtime.echo_generation;
    // Original single-slot exhaustion cannot replace the current wave.
    CHECK(Tick(f, 0) && Tick(f, SOTN_BUTTON_CANCEL));
    CHECK(f->runtime.echo_generation == generation);
    f->world.relics[SOTN_RELIC_FORCE_ECHO] = 3;
    CHECK(Tick(f, 0) && f->runtime.echo.limit == 160 && !f->runtime.echo_attack);
    // Room/camera rebases translate packets without advancing retained driver phases.
    const Sint16 angle = f->runtime.echo.drivers[0].angle, x = f->runtime.echo.quads[3].x[0];
    CHECK(SotnBatRuntime_Tick(&f->runtime, f->character,
                              (SotnBatRuntimeInput){.entering = true, .screen_shift_x = -23}));
    CHECK(f->runtime.echo.drivers[0].angle == angle && f->runtime.echo.quads[3].x[0] == x - 23);
    for (Uint16 i = 0; i < 40; ++i)
        CHECK(Tick(f, 0));
    CHECK(!f->runtime.echo_generation);
    CHECK(SotnCharacter_Get(f->character, &before, NULL));
    CHECK(Tick(f, SOTN_BUTTON_CANCEL));
    CHECK(f->runtime.echo_generation && f->runtime.echo.limit == 256 && f->runtime.echo_attack &&
          f->runtime.children[31].group == 7 && f->runtime.children[31].state.attack.width == 40 &&
          f->runtime.children[31].state.attack.offset_x == 64);
    CHECK(SotnCharacter_Get(f->character, &after, NULL) && after.random != before.random);
    f->world.relics[SOTN_RELIC_FORCE_ECHO] = 1;
    for (Uint16 i = 0; i < 27; ++i)
        CHECK(Tick(f, 0));
    CHECK(f->runtime.echo_generation && !f->runtime.echo_attack && f->runtime.echo.limit == 256);
    for (Uint16 i = 0; i < 30; ++i)
        CHECK(Tick(f, 0));
    CHECK(!f->runtime.echo_generation);
    // A real full pool removes the requested wave after its sound, without RNG or attacks.
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_WORDS; ++i)
        f->primitive_slots.occupied[i] = SDL_MAX_UINT64;
    CHECK(SotnCharacter_Get(f->character, &before, NULL));
    CHECK(Tick(f, SOTN_BUTTON_CANCEL));
    CHECK(!f->runtime.echo_generation && !f->runtime.children[31].kind &&
          f->runtime.sound_count == 1 && !f->runtime.echo_attack);
    CHECK(SotnCharacter_Get(f->character, &after, NULL) && after.random == before.random);
    CHECK(Tick(f, 0));
    f->primitive_slots = (SotnPrimitiveSlots){0};
    f->world.relics[SOTN_RELIC_FORCE_ECHO] = 3;
    Put16(spell + 24, 65535);
    const SotnBatRuntime saved_runtime = f->runtime;
    CHECK(!Tick(f, SOTN_BUTTON_CANCEL));
    CHECK(!SDL_memcmp(&saved_runtime, &f->runtime, sizeof(saved_runtime)));
    CHECK(SotnCharacter_Get(f->character, &after, NULL) && after.random == before.random);
    Close(f);
    return true;
}
static bool PrimitivePressure(Fixture *f)
{
    CHECK(Create(f));
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_WORDS; ++i)
        f->primitive_slots.occupied[i] = SDL_MAX_UINT64;
    CHECK(Tick(f, SOTN_BUTTON_BAT));
    for (Uint16 i = 0; i < 50; ++i)
        CHECK(Tick(f, 0));
    CHECK(f->state.step == SOTN_PLAYER_BAT && !f->state.substep && f->state.bat.signal == 1);
    const SotnPlayerBlink *effect = NULL;
    const SotnPlayerFactory *pending = NULL;
    CHECK(SotnBatRuntime_Context(&f->runtime, f->state.bat.generation, &effect, &pending));
    CHECK(effect && !pending && !effect->phase && !effect->removed && !f->runtime.shared.bat_lock);
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        CHECK(!f->runtime.children[i].draw_count && !f->runtime.children[i].primitive_count);
    f->primitive_slots = (SotnPrimitiveSlots){0};
    for (Uint16 i = 0; i < 45; ++i)
        CHECK(Tick(f, 0));
    CHECK(f->state.step == SOTN_PLAYER_BAT && f->state.substep == 1);
    bool leased = false;
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        leased |= f->runtime.children[i].kind == 11 &&
                  f->runtime.children[i].primitive_count == 8 && f->runtime.children[i].initialized;
    CHECK(leased);
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if (f->runtime.children[i].draw_count)
        {
            SotnBatRuntimeChild *child = &f->runtime.children[i];
            const Uint8 count = child->primitive_count;
            child->primitive_count = 0;
            const SotnPlayerEffectQuad *quads = NULL;
            Uint16 drawn = 99;
            CHECK(!SotnBatRuntime_Visual(&f->runtime, i, &quads, &drawn) && !quads && drawn == 99);
            child->primitive_count = count;
        }
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_WORDS; ++i)
        f->primitive_slots.occupied[i] = SDL_MAX_UINT64;
    SotnPlayerState player = f->state;
    player.bat_commands = (SotnBatCommandEvents){.trail = true};
    CHECK(SotnBatRuntime_Tick(&f->runtime, f->character,
                              (SotnBatRuntimeInput){.dra = {f->dra, sizeof(f->dra)},
                                                    .executable = {f->exe, sizeof(f->exe)},
                                                    .player = player,
                                                    .image = {32, 48, -16, -24},
                                                    .primitive_slots = f->primitive_slots}));
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        CHECK(f->runtime.children[i].kind != 35);
    Close(f);
    return true;
}
static bool WolfPressure(Fixture *f)
{
    CHECK(Create(f));
    f->wolf_owned = true;
    f->world.relics[SOTN_RELIC_WOLF] = 3;
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_WORDS; ++i)
        f->primitive_slots.occupied[i] = SDL_MAX_UINT64;
    CHECK(Tick(f, SOTN_BUTTON_WOLF));
    const Uint32 generation = f->runtime.wolf.generation;
    CHECK(generation && !f->runtime.wolf.root_reserved && !f->runtime.wolf.environment_reserved);
    for (Uint16 i = 0; i < 30; ++i)
        CHECK(Tick(f, 0));
    CHECK(f->state.step == SOTN_PLAYER_WOLF && f->state.substep == 8 && f->state.wolf.signal == 1 &&
          f->runtime.wolf.generation == generation && !f->runtime.wolf.root_reserved &&
          !f->runtime.children[47].initialized);
    f->primitive_slots = (SotnPrimitiveSlots){0};
    Uint16 wait = 0;
    while (f->state.substep == 8 && wait++ < 100)
        CHECK(Tick(f, 0));
    CHECK(wait < 100 && f->state.wolf.signal == 3 && f->runtime.wolf.generation == generation &&
          f->runtime.wolf.root_reserved && f->runtime.wolf.environment_reserved);
    Close(f);
    return true;
}

static bool WolfOrder(Fixture *f)
{
    CHECK(Create(f));
    f->wolf_owned = true;
    f->world.relics[SOTN_RELIC_WOLF] = 3;
    CHECK(Tick(f, SOTN_BUTTON_WOLF) && f->state.wolf.body_started &&
          f->runtime.wolf.root_reserved && f->runtime.wolf.environment_reserved &&
          f->runtime.wolf_bound);
    CHECK(f->runtime.children[47].kind == 11 && f->runtime.children[47].parameter == 0x2300);
    CHECK(f->runtime.wolf.environment_rows[0] == 0 && f->runtime.wolf.root_rows[0] == 2 &&
          f->runtime.children[47].primitives[0] == 8);
    Uint16 wait = 0;
    while (f->runtime.children[47].state.blink.phase != 13 && wait++ < 40)
        CHECK(Tick(f, 0));
    CHECK(wait < 40);
    // A previous-frame bound cannot leak into the later actual slot-63 flash.
    f->runtime.wolf.body.capture =
        (SotnWolfCaptureBounds){.left = -200, .right = 200, .top = -200, .bottom = 200};
    CHECK(Tick(f, 0));
    const SotnPlayerEffectQuad q = f->runtime.children[47].state.blink.quads[0];
    const SotnWolfCaptureBounds bounds = f->runtime.wolf.body.capture;
    CHECK(q.x[1] - q.x[0] == bounds.right - bounds.left &&
          q.y[0] == f->state.y / 65536 + bounds.top && q.u[0] == (Uint8)(Uint32)(44 + bounds.left));
    wait = 0;
    while (f->state.substep == 8 && wait++ < 100)
        CHECK(Tick(f, 0));
    CHECK(f->state.step == SOTN_PLAYER_WOLF && f->state.wolf.signal == 3 && wait < 100);
    for (Uint16 i = 0; i < 35; ++i)
        CHECK(Tick(f, SOTN_BUTTON_RIGHT));
    // A later child's malformed private spell must roll back the already advanced body.
    SotnBatRuntime before = f->runtime;
    SotnCharacterState progress = {0}, after = {0};
    CHECK(SotnCharacter_Get(f->character, &progress, NULL));
    SotnBatRuntimeInput in = {.dra = {f->dra, sizeof(f->dra)},
                              .executable = {f->exe, sizeof(f->exe)},
                              .player = f->state,
                              .image = {40, 64, -20, -48},
                              .wolf_owned = true,
                              .wolf_query = WolfQuery,
                              .wolf_context = f,
                              .primitive_slots = f->primitive_slots};
    in.player.bat_commands.fireball = true;
    const size_t spell = 0x8410U + 9U * 28U + 24U;
    const Uint16 original = (Uint16)(f->dra[spell] | (Uint16)f->dra[spell + 1] << 8);
    Put16(f->dra + spell, 65535);
    CHECK(!SotnBatRuntime_Tick(&f->runtime, f->character, in));
    CHECK(!SDL_memcmp(&before, &f->runtime, sizeof(before)));
    CHECK(SotnCharacter_Get(f->character, &after, NULL) &&
          !SDL_memcmp(&progress, &after, sizeof(after)));
    Put16(f->dra + spell, original);
    // Camera rebases move the actual entities/packets, without advancing motion or MP.
    in.player.bat_commands = (SotnBatCommandEvents){0};
    in.entering = true;
    in.screen_shift_x = -23;
    in.screen_shift_y = 7;
    CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in));
    for (Uint16 i = 0; i < 14; ++i)
        CHECK(f->runtime.wolf.body.parts[i].x == before.wolf.body.parts[i].x - 23 * 65536 &&
              f->runtime.wolf.body.parts[i].y == before.wolf.body.parts[i].y + 7 * 65536);
    CHECK(f->runtime.wolf.body.wave == before.wolf.body.wave &&
          !SDL_memcmp(&f->runtime.wolf.body.capture, &before.wolf.body.capture,
                      sizeof(before.wolf.body.capture)));
    CHECK(SotnCharacter_Get(f->character, &after, NULL) &&
          !SDL_memcmp(&progress, &after, sizeof(after)));
    in.screen_shift_x = 23;
    in.screen_shift_y = -7;
    CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in));
    before.wolf_events = (SotnWolfRuntimeEvents){0};
    CHECK(!SDL_memcmp(&before, &f->runtime, sizeof(before)));
    CHECK(Tick(f, 0));
    CHECK(Tick(f, SOTN_BUTTON_WOLF) && f->state.wolf.return_started &&
          !f->runtime.wolf.generation && !f->runtime.wolf.root_reserved &&
          f->runtime.wolf.environment_reserved);
    wait = 0;
    while (f->state.wolf.active && wait++ < 150)
        CHECK(Tick(f, 0));
    CHECK(!f->state.wolf.active && f->state.animation_bank == 1 && wait < 150);
    Close(f);
    return true;
}

static bool GravityImpactOwnership(Fixture *f)
{
    CHECK(Create(f));
    Uint8 *bp = f->dra + 0xd1d4 + 4U * 6U;
    bp[0] = 5;
    bp[1] = bp[2] = 1;
    Put32(f->dra + 0xdb98 + 4U, 0x800b0c00);
    Uint8 *profile = f->dra + 0x10c00;
    profile[0] = 3;
    profile[1] = profile[2] = profile[3] = 80;
    profile[4] = profile[5] = 2;
    Put16(profile + 6, 2);
    Put16(profile + 10, 1);
    SotnBatRuntimeInput in = {.dra = {f->dra, sizeof(f->dra)},
                              .executable = {f->exe, sizeof(f->exe)},
                              .player = f->state,
                              .camera_x = 20,
                              .camera_y = 8};
    in.player.gravity.ceiling_impact = true;
    in.player.gravity.impact_source = (SotnPlayerFactorySource){
        .x = 10 * 65536 + 123, .y = 20 * 65536 + 456, .priority = 148, .facing_left = true};
    SotnCharacterState before = {0}, after = {0};
    CHECK(SotnCharacter_Get(f->character, &before, NULL));
    profile[0] = 17;
    const SotnBatRuntime unchanged = f->runtime;
    CHECK(!SotnBatRuntime_Tick(&f->runtime, f->character, in) &&
          !SDL_memcmp(&unchanged, &f->runtime, sizeof(unchanged)));
    CHECK(SotnCharacter_Get(f->character, &after, NULL) &&
          !SDL_memcmp(&before, &after, sizeof(before)));
    profile[0] = 3;
    CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in));
    const SotnBatRuntimeChild child = f->runtime.children[47];
    CHECK(child.kind == 5 && child.initialized && child.parameter == 0x100 &&
          child.primitive_count == 4 && child.source.facing_left &&
          child.source.x == in.player.gravity.impact_source.x &&
          child.source.y == in.player.gravity.impact_source.y &&
          child.state.particles.tiles[0].x == -10 * 65536 &&
          child.state.particles.tiles[0].y == 12 * 65536);
    Uint32 expected = before.random;
    for (Uint16 i = 0; i < 6; ++i)
        (void)SotnCombat_Random(&expected);
    CHECK(SotnCharacter_Get(f->character, &after, NULL) && after.random == expected &&
          after.mp == before.mp);
    in.entering = true;
    in.camera_x = 19;
    in.room_shift_x = -4;
    in.screen_shift_x = -3;
    CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in) &&
          f->runtime.children[47].generation == child.generation &&
          f->runtime.children[47].source.x == child.source.x - 4 * 65536 &&
          f->runtime.children[47].state.particles.tiles[0].x == -13 * 65536 &&
          !f->runtime.children[46].generation);
    CHECK(SotnCharacter_Get(f->character, &after, NULL) && after.random == expected);
    f->runtime = (SotnBatRuntime){0};
    in.entering = false;
    in.room_shift_x = in.screen_shift_x = 0;
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_WORDS; ++i)
        in.primitive_slots.occupied[i] = SDL_MAX_UINT64;
    CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in) &&
          !SotnBatRuntime_Occupied(&f->runtime));
    CHECK(SotnCharacter_Get(f->character, &after, NULL) && after.random == expected &&
          after.mp == before.mp);
    Close(f);
    return true;
}

static bool WolfEffectOwnership(Fixture *f)
{
    CHECK(Create(f));
    const Uint16 ids[] = {4, 69};
    for (Uint16 i = 0; i < 2; ++i)
    {
        Uint8 *bp = f->dra + 0xd1d4 + ids[i] * 6U;
        bp[0] = i ? 2 : 5;
        bp[1] = bp[2] = 1;
    }
    Put32(f->dra + 0xdb98 + 9 * 4U, 0x800b0c00);
    Uint8 *profile = f->dra + 0x10c00;
    profile[0] = 3;
    profile[1] = profile[2] = profile[3] = 80;
    profile[4] = profile[5] = 2;
    Put16(profile + 6, 2);
    Put16(profile + 10, 9);
    for (Uint16 i = 0; i < 24; ++i)
    {
        Put16(f->dra + 0xd57c + i * 4U, 1);
        Put16(f->dra + 0xd57e + i * 4U, (Uint16)(i + 1U));
    }
    Put16(f->dra + 0xd5dc, 65535);
    Put16(f->dra + 0xd572, 96);
    SotnBatRuntimeInput in = {.dra = {f->dra, sizeof(f->dra)},
                              .executable = {f->exe, sizeof(f->exe)},
                              .player = f->state,
                              .wolf_owned = true,
                              .wolf_query = WolfQuery,
                              .wolf_context = f,
                              .camera_x = 20,
                              .wolf_command_camera_x = 16,
                              .room_shift_x = -4};
    const SotnPlayerFactorySource source = {
        .x = 10 * 65536 + 123, .y = 20 * 65536 + 456, .priority = 148};
    in.player.wolf.commands = (SotnWolfControllerEvents){
        .count = 4,
        .requests = {{.source = source, .parameter = 0x90004, .kind = SOTN_WOLF_FACTORY},
                     {.source = source, .parameter = 0x10045, .kind = SOTN_WOLF_FACTORY},
                     {.parameter = 0x644, .kind = SOTN_WOLF_SOUND},
                     {.parameter = 2, .kind = SOTN_WOLF_SHAKE}}};
    CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in));
    CHECK(f->runtime.children[47].kind == 5 && f->runtime.children[47].initialized &&
          f->runtime.children[47].primitive_count == 4 &&
          f->runtime.children[47].state.particles.tiles[0].x == 2 * 65536 &&
          f->runtime.children[46].kind == 2 && f->runtime.children[46].initialized &&
          f->runtime.children[46].state.smoke.x == -6 * 65536 + 123 &&
          f->runtime.children[46].state.smoke.y == 44 * 65536 + 456 &&
          f->runtime.sound_count == 1 && f->runtime.sounds[0] == 0x644 &&
          f->runtime.wolf_shake_count == 1 && f->runtime.wolf_shakes[0] == 2);
    const SotnBatRuntime before = f->runtime;
    SotnCharacterState character = {0}, after = {0};
    CHECK(SotnCharacter_Get(f->character, &character, NULL));
    in.entering = true;
    in.screen_shift_x = -3;
    in.screen_shift_y = 5;
    CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in));
    CHECK(f->runtime.children[47].state.particles.tiles[0].x == -65536 &&
          f->runtime.children[46].state.smoke.x == -9 * 65536 + 123 &&
          f->runtime.children[46].state.smoke.remaining ==
              before.children[46].state.smoke.remaining);
    CHECK(SotnCharacter_Get(f->character, &after, NULL) && after.random == character.random);
    f->runtime = (SotnBatRuntime){0};
    in.entering = false;
    in.screen_shift_x = in.screen_shift_y = 0;
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_WORDS; ++i)
        in.primitive_slots.occupied[i] = SDL_MAX_UINT64;
    CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in));
    CHECK(!f->runtime.children[47].kind && f->runtime.children[46].kind == 2);
    CHECK(SotnCharacter_Get(f->character, &after, NULL) && after.random == character.random);
    // Original engine culling follows actual initialization; RNG is consumed but rows retire.
    f->runtime = (SotnBatRuntime){0};
    in.primitive_slots = (SotnPrimitiveSlots){0};
    in.player.wolf.commands.requests[0].source.x = 400 * 65536;
    in.player.wolf.commands.requests[1].source.x = 400 * 65536;
    CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in));
    CHECK(!SotnBatRuntime_Occupied(&f->runtime) && !f->runtime.children[47].primitive_count &&
          !f->runtime.children[46].kind);
    CHECK(SotnCharacter_Get(f->character, &after, NULL) && after.random != character.random);
    Close(f);
    CHECK(Create(f));
    f->wolf_owned = true;
    f->world.relics[SOTN_RELIC_WOLF] = 3;
    Uint8 *bp = f->dra + 0xd1d4 + 4 * 6U;
    bp[0] = 5;
    bp[1] = bp[2] = 1;
    Put32(f->dra + 0xdb98 + 13 * 4U, 0x800b0c00);
    profile = f->dra + 0x10c00;
    profile[0] = 3;
    profile[1] = profile[2] = profile[3] = 80;
    profile[4] = profile[5] = 2;
    Put16(profile + 6, 2);
    Put16(profile + 8, 8);
    Put16(profile + 10, 13);
    CHECK(Tick(f, SOTN_BUTTON_WOLF));
    for (Uint16 i = 0; i < 80; ++i)
        CHECK(Tick(f, 0));
    Uint16 wait = 0;
    while (f->runtime.children[47].kind == 11 && wait++ < 100)
        CHECK(Tick(f, 0));
    CHECK(wait < 100 && !f->runtime.children[47].kind);
    f->runtime.wolf.head.remaining = -1;
    f->runtime.wolf.head.breath_timer = 0;
    in = (SotnBatRuntimeInput){.dra = {f->dra, sizeof(f->dra)},
                               .executable = {f->exe, sizeof(f->exe)},
                               .player = f->state,
                               .wolf_owned = true,
                               .wolf_query = WolfQuery,
                               .wolf_context = f};
    in.player.wolf.controller.fall_water_level = 40;
    in.player.water.feet = 40;
    CHECK(SotnCharacter_Get(f->character, &character, NULL));
    CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in) && f->runtime.wolf_events.bubble);
    const SotnPlayerFactorySource bubble = f->runtime.wolf_events.bubble_source;
    CHECK(f->runtime.factories[0].cursor.entity_id &&
          f->runtime.factories[0].cursor.blueprint == 4 && !f->runtime.children[47].kind);
    CHECK(SotnCharacter_Get(f->character, &after, NULL) && after.random == character.random);
    CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in) && !f->runtime.wolf_events.bubble);
    CHECK(f->runtime.children[47].kind == 5 && f->runtime.children[47].parameter == 0xd00 &&
          f->runtime.children[47].initialized && f->runtime.children[47].primitive_count == 4 &&
          f->runtime.children[47].primitives[0] == 8 &&
          f->runtime.children[47].state.particles.tiles[0].x / 65536 == bubble.x / 65536 &&
          f->runtime.children[47].state.particles.tiles[1].timer == 4);
    Close(f);
    return true;
}

static bool WolfSpellOwnership(Fixture *f)
{
    CHECK(Create(f));
    f->wolf_owned = true;
    f->world.relics[SOTN_RELIC_WOLF] = 3;
    const Uint16 spells[] = {4, 13, 14};
    for (Uint16 i = 0; i < 3; ++i)
    {
        Uint8 *spell = f->dra + 0x8410 + spells[i] * 28U;
        spell[12] = 3;
        spell[13] = 8;
        spell[14] = 6;
        spell[16] = 2;
        spell[18] = 1;
        spell[20] = 16;
        spell[24] = 20;
    }
    CHECK(Tick(f, SOTN_BUTTON_WOLF));
    for (Uint16 i = 0; i < 80; ++i)
        CHECK(Tick(f, 0));
    CHECK(f->state.wolf.active && f->state.substep != 8);
    SotnBatRuntimeInput in = {.dra = {f->dra, sizeof(f->dra)},
                              .executable = {f->exe, sizeof(f->exe)},
                              .player = f->state,
                              .image = {40, 64, -20, -48},
                              .wolf_owned = true,
                              .wolf_query = WolfQuery,
                              .wolf_context = f};
    in.player.velocity_x = in.player.wolf.controller.motion.velocity_x = 6 * 65536;
    in.player.wolf.controller.motion.substep = 1;
    f->runtime.wolf.body.parts[3].sprite = 75;
    CHECK(SotnWolfRuntime_Hit(&f->runtime.wolf, f->runtime.wolf.children[1].generation, 1));
    f->runtime.groups[3] = 5;
    f->runtime.groups[4] = 1;
    f->runtime.groups[5] = 2;
    f->runtime.groups[6] = 0;
    SotnCharacterState before = {0}, after = {0};
    CHECK(SotnCharacter_Get(f->character, &before, NULL));
    SotnBatRuntime saved = f->runtime;
    Put16(f->dra + 0x8410 + 14 * 28 + 24, 65535);
    CHECK(!SotnBatRuntime_Tick(&f->runtime, f->character, in));
    CHECK(!SDL_memcmp(&saved, &f->runtime, sizeof(saved)));
    CHECK(SotnCharacter_Get(f->character, &after, NULL) &&
          !SDL_memcmp(&before, &after, sizeof(after)));
    Put16(f->dra + 0x8410 + 14 * 28 + 24, 20);
    CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in));
    CHECK(f->runtime.wolf_events.spell_count == 2 && f->runtime.wolf_mask == 3 &&
          f->runtime.wolf_events.spells[0].spell == 13 &&
          f->runtime.wolf_events.spells[1].spell == 14 && f->runtime.groups[3] == 2 &&
          f->runtime.groups[4] == 0 && f->runtime.groups[5] == 0 && f->runtime.groups[6] == 0);
    CHECK(SotnCharacter_Get(f->character, &after, NULL) && after.mp + 3U == before.mp &&
          f->runtime.wolf_frame.mp == after.mp && f->runtime.wolf_events.mp_spent == 3);
    Uint32 expected = before.random;
    for (Uint16 i = 0; i < 2; ++i)
        (void)SotnCombat_Random(&expected);
    CHECK(after.random == expected);
    // A supplied later Bat snapshot retires Wolf while retaining its prior count for
    // this tick's allocation. The end-of-tick scan then counts only living children.
    in.player.step = SOTN_PLAYER_BAT;
    in.player.substep = 3;
    in.player.bat_commands.wing_attack = true;
    CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in));
    CHECK(!f->runtime.wolf_mask && !f->runtime.wolf.generation &&
          f->runtime.children[15].kind == 36 && f->runtime.children[15].group == 4 &&
          f->runtime.groups[3] == 0 && f->runtime.groups[4] == 1);
    Close(f);
    return true;
}

static bool CommonPlayerLifetime(Fixture *f)
{
    CHECK(Create(f));
    f->wolf_owned = true;
    CHECK(SotnBatRuntime_InitCommon(&f->runtime, f->primitive_slots));
    CHECK(Tick(f, 0) && !SotnBatRuntime_Occupied(&f->runtime) &&
          f->runtime.common_generation == 1 && f->runtime.afterimage_rows[5] == 5 &&
          f->runtime.wolf.environment_rows[0] == 6 && f->runtime.wolf.environment_rows[1] == 7);
    f->world.relics[SOTN_RELIC_WOLF] = 3;
    CHECK(Tick(f, SOTN_BUTTON_WOLF) && f->runtime.wolf.root_reserved &&
          f->runtime.wolf.root_rows[0] == 8 && f->runtime.children[47].primitives[0] == 14);
    for (Uint16 i = 0; i < 100; ++i)
        CHECK(Tick(f, 0));
    CHECK(f->state.step == SOTN_PLAYER_WOLF && f->state.substep != 8 &&
          f->runtime.wolf.environment_generation == 1);
    CHECK(Tick(f, SOTN_BUTTON_WOLF));
    for (Uint16 i = 0; i < 100; ++i)
        CHECK(Tick(f, 0));
    CHECK(f->state.step != SOTN_PLAYER_WOLF && f->state.step != SOTN_PLAYER_UNMORPH_WOLF &&
          !SotnBatRuntime_Occupied(&f->runtime) && !f->runtime.wolf.root_reserved &&
          f->runtime.common_generation == 1 && f->runtime.afterimage_rows[0] == 0 &&
          f->runtime.afterimage_rows[5] == 5 && f->runtime.wolf.environment_rows[0] == 6 &&
          f->runtime.wolf.environment_rows[1] == 7 && f->runtime.wolf.environment_generation == 1);
    Close(f);
    return true;
}

static bool OrdinarySmokeOwnership(Fixture *f)
{
    CHECK(Create(f));
    for (Uint16 i = 0; i < 2; ++i)
    {
        Uint8 *bp = f->dra + 0xd1d4 + i * 6U;
        bp[0] = 2;
        bp[1] = bp[2] = i ? 3 : 1;
    }
    for (Uint16 i = 0; i < 24; ++i)
    {
        Put16(f->dra + 0xd57c + i * 4U, 1);
        Put16(f->dra + 0xd57e + i * 4U, (Uint16)(i + 1U));
    }
    Put16(f->dra + 0xd5dc, 65535);
    SotnBatRuntimeInput in = {.dra = {f->dra, sizeof(f->dra)},
                              .executable = {f->exe, sizeof(f->exe)},
                              .player = f->state,
                              .camera_x = 20};
    const SotnPlayerFactorySource source = {
        .x = 100 * 65536 + 123, .y = 120 * 65536 + 456, .priority = 148, .facing_left = true};
    in.player.movement_effects = (SotnPlayerMovementEffects){.smoke = true, .smoke_source = source};
    SotnCharacterState before = {0}, after = {0};
    CHECK(SotnCharacter_Get(f->character, &before, NULL));
    CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in));
    CHECK(f->runtime.children[47].kind == 2 && !f->runtime.children[47].parameter &&
          f->runtime.children[47].initialized && f->runtime.children[47].source.x == source.x &&
          f->runtime.children[47].state.smoke.x == 74 * 65536 + 123);
    in.player.movement_effects.smoke_blueprint = 0x50001;
    CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in));
    for (Uint16 i = 0; i < 3; ++i)
        CHECK(f->runtime.children[46U - i].kind == 2 &&
              f->runtime.children[46U - i].parameter == 0x500U + i &&
              f->runtime.children[46U - i].initialized &&
              f->runtime.children[46U - i].state.smoke.x == 86 * 65536 + 123);
    CHECK(SotnCharacter_Get(f->character, &after, NULL) && before.random == after.random &&
          before.mp == after.mp);
    const Sint32 position = f->runtime.children[46].state.smoke.x;
    const Uint16 pose = f->runtime.children[46].state.smoke.pose;
    in.entering = true;
    in.screen_shift_x = -16;
    in.room_shift_x = 16;
    CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in) &&
          f->runtime.children[46].state.smoke.x == position - 16 * 65536 &&
          f->runtime.children[46].state.smoke.pose == pose);
    f->runtime = (SotnBatRuntime){0};
    in.entering = false;
    in.screen_shift_x = in.room_shift_x = 0;
    Put16(f->dra + 0xd57c, 0);
    const SotnBatRuntime empty = f->runtime;
    CHECK(!SotnBatRuntime_Tick(&f->runtime, f->character, in) &&
          !SDL_memcmp(&empty, &f->runtime, sizeof(empty)));
    CHECK(SotnCharacter_Get(f->character, &after, NULL) && before.random == after.random &&
          before.mp == after.mp);
    Put16(f->dra + 0xd57c, 1);
    for (Uint16 i = 0; i < 8; ++i)
    {
        CHECK(SotnPlayerFactory_Begin(&f->runtime.factories[i].cursor, source, 0, 0));
        f->runtime.factories[i].cursor.initialized = true;
        f->runtime.factories[i].cursor.child = 2;
        f->runtime.factories[i].cursor.amount = f->runtime.factories[i].cursor.per_cycle = 1;
        f->runtime.factories[i].cursor.delay = 100;
    }
    CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in));
    for (Uint16 i = 0; i < 48; ++i)
        CHECK(!f->runtime.children[i].kind);
    Close(f);
    return true;
}

static bool NormalWaterRequests(Fixture *f)
{
    CHECK(Create(f));
    Uint8 *smoke = f->dra + 0xd1d4 + 6;
    smoke[0] = 2;
    smoke[1] = smoke[2] = 3;
    for (Uint16 i = 0; i < 24; ++i)
    {
        Put16(f->dra + 0xd57c + i * 4U, 1);
        Put16(f->dra + 0xd57e + i * 4U, (Uint16)(i + 1U));
    }
    Put16(f->dra + 0xd5dc, 65535);
    Uint8 *bp = f->dra + 0xd1d4 + 4 * 6U;
    bp[0] = 5;
    bp[1] = bp[2] = 1;
    Put32(f->dra + 0xdb98 + 13 * 4U, 0x800b0c00);
    Uint8 *profile = f->dra + 0x10c00;
    profile[0] = 1;
    profile[1] = profile[2] = profile[3] = 80;
    profile[4] = profile[5] = 2;
    Put16(profile + 6, 2);
    Put16(profile + 8, 8);
    Put16(profile + 10, 13);
    const SotnPlayerFactorySource source = {
        .x = 100 * 65536 + 123, .y = 120 * 65536 + 456, .priority = 148};
    SotnBatRuntimeInput in = {.dra = {f->dra, sizeof(f->dra)},
                              .executable = {f->exe, sizeof(f->exe)},
                              .player = f->state};
    in.player.water.feet = 60;
    in.player.movement_effects = (SotnPlayerMovementEffects){.water_bubble = true,
                                                             .bubble_source = source,
                                                             .smoke = true,
                                                             .smoke_source = source,
                                                             .smoke_blueprint = 0x50001};
    CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in));
    CHECK(f->runtime.children[47].kind == 5 && f->runtime.children[47].parameter == 0xd00 &&
          f->runtime.children[47].primitive_count == 2 &&
          f->runtime.children[47].source.x == source.x);
    for (Uint16 i = 0; i < 3; ++i)
        CHECK(f->runtime.children[46U - i].kind == 2 &&
              f->runtime.children[46U - i].parameter == 0x500U + i);
    f->runtime = (SotnBatRuntime){0};
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_WORDS; ++i)
        in.primitive_slots.occupied[i] = SDL_MAX_UINT64;
    SotnCharacterState before = {0}, after = {0};
    CHECK(SotnCharacter_Get(f->character, &before, NULL));
    CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in) && !f->runtime.children[47].kind &&
          f->runtime.children[46].kind == 2);
    CHECK(SotnCharacter_Get(f->character, &after, NULL) && after.random == before.random);
    Close(f);
    return true;
}

static bool WaterBlinkOwnership(Fixture *f)
{
    CHECK(Create(f));
    const SotnPlayerFactorySource source = {
        .x = 100 * 65536 + 123, .y = 120 * 65536 + 456, .priority = 148};
    SotnBatRuntimeInput in = {.dra = {f->dra, sizeof(f->dra)},
                              .executable = {f->exe, sizeof(f->exe)},
                              .image = {32, 48, -16, -24},
                              .player = f->state};
    in.player.water.waist = 1;
    in.player.animation.id = 0xc0;
    SotnCharacterState before = {0}, after = {0};
    CHECK(SotnCharacter_Get(f->character, &before, NULL));
    for (Uint16 kind = 0; kind < 2; ++kind)
    {
        const Uint16 parameter = kind ? 0x59 : 0x4e;
        Uint8 *profile = f->dra + 0xd670 + (parameter & 0x3fU) * 20U;
        for (Uint16 i = 0; i < 3; ++i)
            Put16(profile + 8U + i * 2U, 256);
        Put16(profile + 6, 0x123);
        Put16(profile + 14, kind ? 0x7005 : 0x7004);
        Put16(profile + 16, 0x31);
        Put16(profile + 18, 512);
        f->runtime = (SotnBatRuntime){0};
        in.holy_symbol_enabled = false;
        in.player.animation.id = 0xc0;
        for (Uint16 i = 0; i < SOTN_PRIMITIVE_WORDS; ++i)
            in.primitive_slots.occupied[i] = SDL_MAX_UINT64;
        CHECK(SotnPlayerFactory_Begin(&f->runtime.factories[0].cursor, source,
                                      (Uint32)parameter * 65536U + 44U, 0));
        CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in));
        SotnBatRuntimeChild *child = NULL;
        for (Uint16 i = 0; i < 48; ++i)
            if (f->runtime.children[i].kind == 11)
            {
                CHECK(!child);
                child = &f->runtime.children[i];
            }
        CHECK(child && !child->initialized && !child->primitive_count && !child->draw_count &&
              !child->state.blink.phase && !f->runtime.afterimage_reset);
        in.primitive_slots = (SotnPrimitiveSlots){0};
        CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in) && child->initialized &&
              child->primitive_count == 8 && child->draw_count == 8 &&
              child->state.blink.phase == 1 && f->runtime.afterimage_reset);
        const Uint32 generation = child->generation;
        for (Uint16 t = 0; t < 40; ++t)
        {
            CHECK(SotnPlayerFactory_Begin(&f->runtime.factories[0].cursor, source,
                                          (Uint32)parameter * 65536U + 44U, 0));
            CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in) &&
                  child->generation == generation && child->primitive_count == 8);
            Uint16 children = 0;
            for (Uint16 i = 0; i < 48; ++i)
                children += f->runtime.children[i].kind == 11 ? 1 : 0;
            CHECK(children == 1);
        }
        CHECK(child->state.blink.phase == 2 && child->state.blink.hold == 7);
        const Sint16 x = child->state.blink.quads[0].x[0];
        const Uint16 angle = child->state.blink.angle;
        in.entering = true;
        in.screen_shift_x = -16;
        CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in) &&
              child->state.blink.angle == angle && child->state.blink.quads[0].x[0] == x - 16);
        in.entering = false;
        in.screen_shift_x = 0;
        if (kind)
            in.player.animation.id = 0xc1;
        else
            in.holy_symbol_enabled = true;
        CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in) && child->state.blink.phase == 3);
        for (Uint16 t = 0; t < 30 && child->kind; ++t)
            CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in));
        CHECK(!child->kind && !child->primitive_count);
    }
    CHECK(SotnCharacter_Get(f->character, &after, NULL) && after.random == before.random &&
          after.hp == before.hp);
    Close(f);
    return true;
}

static bool DeathParticlesOwnership(Fixture *f)
{
    CHECK(Create(f));
    Uint8 *bp = f->dra + 0xd1d4 + 49U * 6U;
    bp[0] = 5;
    bp[1] = 3;
    bp[2] = 0xc1;
    bp[3] = 2;
    Put32(f->dra + 0xdb98 + 6U * 4U, 0x800a0100);
    Uint8 *profile = f->dra + 0x100;
    profile[0] = 2;
    profile[1] = profile[2] = profile[3] = 63;
    profile[4] = profile[5] = 3;
    Put16(profile + 6, 2);
    Put16(profile + 8, 49);
    Put16(profile + 10, 4);
    SotnCharacterState progress = {0};
    CHECK(SotnCharacter_Get(f->character, &progress, NULL));
    CHECK(SotnPlayerParticleVectors_Init(&f->runtime.particle_vectors,
                                         (SotnDataView){f->exe, sizeof(f->exe)}, &progress.random));
    CHECK(SotnCharacter_Restore(f->character, &progress));
    const SotnPlayerParticleVectors vectors = f->runtime.particle_vectors;
    const Uint32 random = progress.random;
    const SotnPlayerFactorySource source = {
        .x = 100 * 65536 + 123, .y = 120 * 65536 + 456, .priority = 148};
    SotnBatRuntimeInput in = {.dra = {f->dra, sizeof(f->dra)},
                              .executable = {f->exe, sizeof(f->exe)},
                              .player = f->state};
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_WORDS; ++i)
        in.primitive_slots.occupied[i] = SDL_MAX_UINT64;
    CHECK(SotnPlayerFactory_Begin(&f->runtime.factories[0].cursor, source, 0x60031, 0));
    for (Uint16 t = 0; t < 8; ++t)
        CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in));
    CHECK(SotnCharacter_Get(f->character, &progress, NULL) && progress.random == random);
    for (Uint16 i = 0; i < 48; ++i)
        CHECK(!f->runtime.children[i].kind);
    in.primitive_slots = (SotnPrimitiveSlots){0};
    in.primitive_slots.occupied[0] = 1;
    CHECK(SotnPlayerFactory_Begin(&f->runtime.factories[0].cursor, source, 0x60031, 0));
    Uint16 births = 0;
    for (Uint16 t = 0; t < 60; ++t)
    {
        CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in));
        for (Uint16 i = 0; i < 48; ++i)
        {
            const SotnBatRuntimeChild *c = &f->runtime.children[i];
            if (c->kind == 5)
            {
                CHECK(c->initialized && c->primitive_count == 3 && c->primitives[0] != 0 &&
                      c->parameter >= 0x600 && c->parameter <= 0x602);
                if (c->state.particles.lifetime == 47)
                    ++births;
            }
        }
    }
    CHECK(births == 3);
    CHECK(SotnCharacter_Get(f->character, &progress, NULL) && progress.random != random &&
          !SDL_memcmp(&vectors, &f->runtime.particle_vectors, sizeof(vectors)));
    for (Uint16 i = 0; i < 48; ++i)
        CHECK(!f->runtime.children[i].kind);
    Close(f);
    return true;
}

static bool OutlineOwnership(Fixture *f)
{
    CHECK(Create(f));
    Uint8 *bp = f->dra + 0xd1d4 + 40U * 6U;
    bp[0] = 31;
    bp[1] = bp[2] = 1;
    for (Uint16 i = 0; i < 3; ++i)
        Put16(f->dra + 0xd9b8 + 17U * 10U + i * 2U, (Uint16)(32U + i * 32U));
    Put16(f->dra + 0xd9b8 + 17U * 10U + 6U, 0x103);
    Put16(f->dra + 0xd9b8 + 17U * 10U + 8U, 49);
    const SotnPlayerFactorySource source = {
        .x = 120 * 65536 + 123, .y = 135 * 65536 + 456, .priority = 148};
    f->dra[0xcfb4 + 11U * 4U] = 18;
    f->dra[0xcfb4 + 11U * 4U + 1U] = 127;
    SotnBatRuntimeInput in = {.dra = {f->dra, sizeof(f->dra)},
                              .executable = {f->exe, sizeof(f->exe)},
                              .image = {32, 48, -16, -24},
                              .player = f->state};
    in.player.draw_pivot_y = 2;
    in.player.dark = (SotnPlayerDark){.source = source, .started = true, .ticks = 1048};
    CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in));
    CHECK(f->runtime.screen_flash && f->runtime.screen_flash_ticks == 17 &&
          f->runtime.screen_red == 127 && !f->runtime.screen_green && !f->runtime.screen_blue);
    in.player.dark.started = false;
    SotnBatRuntimeChild *c = &f->runtime.children[47];
    CHECK(c->kind == 31 && c->parameter == 0x1100 && c->initialized && c->primitive_count == 1 &&
          c->draw_count == 1 && c->state.outline.phase == 1 && c->state.outline.pivot_y == 2 &&
          c->source.x == source.x && f->runtime.afterimage_reset);
    const SotnPlayerEffectQuad *q = NULL;
    Uint16 count = 0;
    CHECK(SotnBatRuntime_Visual(&f->runtime, 47, &q, &count) && count == 1 &&
          q == &c->state.outline.quad && q->priority == 150);
    in.player.gravity.upside_down = true;
    CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in) &&
          c->state.outline.rotation == 0x800 && c->state.outline.draw_flags == 3);
    const Sint16 x = c->state.outline.quad.x[0];
    const Uint16 timer = c->state.outline.timer;
    in.entering = true;
    in.screen_shift_x = -16;
    in.room_shift_x = 16;
    CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in) &&
          c->state.outline.quad.x[0] == x - 16 && c->state.outline.timer == timer &&
          c->source.x == source.x + 16 * 65536);
    in.entering = false;
    in.screen_shift_x = in.room_shift_x = 0;
    Put16(f->dra + 0xd9b8 + 17U * 10U + 8U, 0);
    const SotnBatRuntime before = f->runtime;
    CHECK(!SotnBatRuntime_Tick(&f->runtime, f->character, in) &&
          !SDL_memcmp(&before, &f->runtime, sizeof(before)));
    Put16(f->dra + 0xd9b8 + 17U * 10U + 8U, 49);
    in.player.status = 1;
    CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in) && !c->kind &&
          !f->runtime.afterimage_reset);
    CHECK(SotnPlayerFactory_Begin(&f->runtime.factories[0].cursor, source, 0x110028, 0));
    for (Uint16 i = 0; i < SDL_arraysize(in.primitive_slots.occupied); ++i)
        in.primitive_slots.occupied[i] = SDL_MAX_UINT64;
    in.player.status = 0;
    CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in) && !c->kind &&
          !f->runtime.afterimage_reset);
    f->runtime.screen_flash_ticks = 18;
    in.entering = true;
    CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in) &&
          f->runtime.screen_flash_ticks == 18);
    in.entering = false;
    for (Uint16 t = 0; t < 18; ++t)
    {
        CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in) &&
              f->runtime.screen_flash_ticks == 17 - t &&
              f->runtime.screen_flash == ((t & 1U) == 0));
    }
    CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in) && !f->runtime.screen_flash);
    Close(f);
    return true;
}

static bool ContactAbsorptionOwnership(Fixture *f)
{
    CHECK(Create(f));
    for (Uint16 id = 4; id <= 8; id += 4)
    {
        Uint8 *profile = f->dra + 0xd670 + id * 20U;
        Put16(profile + 6, 0x123);
        Put16(profile + 8, 256);
        Put16(profile + 10, 256);
        Put16(profile + 12, 256);
        Put16(profile + 14, 16);
        Put16(profile + 16, 0x31);
        Put16(profile + 18, 512);
    }
    const SotnPlayerFactorySource source = {
        .x = 120 * 65536 + 123, .y = 135 * 65536 + 456, .priority = 148};
    SotnBatRuntimeInput in = {.dra = {f->dra, sizeof(f->dra)},
                              .executable = {f->exe, sizeof(f->exe)},
                              .player = f->state,
                              .image = {40, 64, -20, -48}};
    in.player.hp = in.player.hp_max = 70;
    in.player.healing = (SotnPlayerHealing){
        .source = source, .applied = true, .effects = true, .applied_kind = 2, .applied_amount = 1};
    in.player.absorption =
        (SotnPlayerAbsorption){.source = source, .applied = true, .amount = 39996};
    CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in) && !f->runtime.sound_count);
    CHECK(f->runtime.children[40].kind == 19 && f->runtime.children[41].kind == 19 &&
          f->runtime.children[40].state.healing_number.number == 1 &&
          f->runtime.children[41].state.healing_number.number == 39996 &&
          f->runtime.children[40].primitive_count == 20 &&
          f->runtime.children[41].primitive_count == 20);
    CHECK(f->runtime.children[47].kind == 11 && f->runtime.children[47].parameter == 0x4800 &&
          f->runtime.children[46].kind == 11 && f->runtime.children[46].parameter == 0x4400 &&
          f->runtime.children[47].primitive_count == 8 &&
          f->runtime.children[46].primitive_count == 8);
    const SotnBatRuntime before = f->runtime;
    in.freeze = 3;
    CHECK(!SotnBatRuntime_Tick(&f->runtime, f->character, in) &&
          !SDL_memcmp(&before, &f->runtime, sizeof(before)));
    in.freeze = 0;
    SDL_zero(f->runtime);
    in.player.healing.applied_kind = 1;
    in.player.healing.applied_amount = 8;
    CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in) && f->runtime.sound_count == 1 &&
          f->runtime.sounds[0] == 0x68e &&
          f->runtime.children[40].state.healing_number.number == 8 &&
          f->runtime.children[41].state.healing_number.number == 39996);
    SDL_zero(f->runtime);
    in.player.healing = (SotnPlayerHealing){0};
    in.other_occupied = 0xff00000000000000ULL;
    CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in) && !f->runtime.sound_count);
    for (Uint16 i = 0; i < 48; ++i)
        CHECK(f->runtime.children[i].kind != 19);
    SDL_zero(f->runtime);
    in.other_occupied = 0;
    SDL_memset(&in.primitive_slots, 255, sizeof(in.primitive_slots));
    CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, in) && !f->runtime.sound_count);
    Uint16 pending = 0;
    for (Uint16 i = 0; i < 48; ++i)
    {
        CHECK(f->runtime.children[i].kind != 19);
        if (f->runtime.children[i].kind == 11)
        {
            CHECK(!f->runtime.children[i].initialized && !f->runtime.children[i].primitive_count);
            ++pending;
        }
    }
    CHECK(pending == 2);
    Close(f);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok =
        Wing(f) && Fire(f) && Capacity(f) && Wall(f) && AtomicAndRebase(f) && Echo(f) &&
        PrimitivePressure(f) && WolfPressure(f) && WolfOrder(f) && WolfSpellOwnership(f) &&
        WolfEffectOwnership(f) && GravityImpactOwnership(f) && CommonPlayerLifetime(f) &&
        OrdinarySmokeOwnership(f) && NormalWaterRequests(f) && WaterBlinkOwnership(f) &&
        DeathParticlesOwnership(f) && OutlineOwnership(f) && ContactAbsorptionOwnership(f);
    SDL_free(f);
    return ok ? 0 : 1;
}
