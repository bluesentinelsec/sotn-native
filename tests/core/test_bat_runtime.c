#include "character_fixture.h"
#include <SDL3/SDL.h>
#include <sotn/bat_runtime.h>

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
    Uint32 tick, spent;
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
            .other_occupied = f->occupied}));
    if (f->runtime.use_ticks)
        CHECK(SotnPlayer_SetUseTimer(f->player, f->runtime.use_ticks));
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
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok = Wing(f) && Fire(f) && Capacity(f) && Wall(f) && AtomicAndRebase(f);
    SDL_free(f);
    return ok ? 0 : 1;
}
