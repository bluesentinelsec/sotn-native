#include <SDL3/SDL.h>
#include <sotn/player.h>

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
    Uint8 dra[0x11000], exe[0x24000], tiles[32 * 16 * 2], types[3];
    SotnPlayer *player;
    SotnPlayerState state;
    SotnCharacterState character;
    SotnWorldState world;
    SotnPlayerBlinkShared shared;
    struct
    {
        SotnPlayerBlink effect;
        Uint32 generation;
    } visuals[2];
    Uint32 tick;
    bool water;
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
    CHECK(SotnPlayer_GetState(f->player, &f->state));
    const SotnPlayerBlink *effect = NULL;
    for (Uint16 i = 0; i < 2; ++i)
        if (f->state.bat.generation && f->visuals[i].generation == f->state.bat.generation)
            effect = &f->visuals[i].effect;
    const SotnCharacterStats stats = {.total = {6, 6, 6, 6}, .attack = {6, 6}, .unarmed_attack = 6};
    CHECK(SotnPlayer_SetCharacter(f->player, &f->character, stats));
    CHECK(SotnPlayer_SetRelics(f->player, &f->world));
    CHECK(SotnPlayer_SetMistContext(f->player, NULL, 0, f->tick, f->water));
    CHECK(SotnPlayer_SetBatContext(f->player, effect, effect ? f->state.bat.generation : 0,
                                   !f->visuals[0].generation || !f->visuals[1].generation));
    CHECK(SotnPlayer_Tick(f->player, buttons) && SotnPlayer_GetState(f->player, &f->state));
    CHECK(f->character.mp >= f->state.mp_spent);
    f->character.mp = (Uint16)(f->character.mp - f->state.mp_spent);
    f->character.hp = f->state.hp;
    if (f->state.bat.started || f->state.bat.return_started)
    {
        Uint16 i = 0;
        while (i < 2 && f->visuals[i].generation)
            ++i;
        CHECK(i < 2);
        f->visuals[i].generation = f->state.bat.generation;
        f->visuals[i].effect = (SotnPlayerBlink){.parameter = f->state.bat.started ? 0x20 : 0x21};
    }
    const bool bat = f->state.animation_bank == 13;
    for (Sint32 i = 1; i >= 0; --i)
    {
        if (!f->visuals[i].generation)
            continue;
        const SotnPlayerBlinkInput in = {
            .dra = {f->dra, sizeof(f->dra)},
            .executable = {f->exe, sizeof(f->exe)},
            .image = bat ? (SotnPlayerImage){32, 48, -16, (Sint16)(-24 + f->state.bat.settle_y)}
                         : (SotnPlayerImage){40, 64, -20, -48},
            .x = (Sint16)(f->state.x / 65536),
            .y = (Sint16)(f->state.y / 65536),
            .rotation = f->state.bat.rotation,
            .priority = 144,
            .signal = f->state.bat.signal,
            .status = f->state.status,
            .facing_left = f->state.facing_left,
            .rotated = f->state.rotated,
            .image_present = f->state.animation.sprite != 0};
        SotnPlayerBlinkEvents events = {0};
        CHECK(SotnPlayerBlink_Tick(&f->visuals[i].effect, &f->shared, in, &events));
        if (f->visuals[i].effect.removed)
            f->visuals[i].generation = 0;
    }
    ++f->tick;
    return true;
}
static bool Create(Fixture *f)
{
    SDL_memset(f, 0, sizeof(*f));
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
    const Uint16 ids[3] = {0x20, 0x21, 0x1b};
    for (Uint16 i = 0; i < 3; ++i)
    {
        Uint8 *p = f->dra + 0xd670 + ids[i] * 20U;
        Put16(p + 6, 0x123);
        Put16(p + 8, 256);
        Put16(p + 10, 256);
        Put16(p + 12, 256);
        Put16(p + 14, i == 2 ? 0x7008 : 16);
        Put16(p + 16, 0x31);
        Put16(p + 18, 512);
    }
    f->types[1] = 3;
    f->types[2] = 0xf8;
    for (Uint16 y = 10; y < 16; ++y)
        for (Uint16 x = 0; x < 32; ++x)
            f->tiles[(y * 32U + x) * 2U] = 1;
    f->character = (SotnCharacterState){.hp = 70, .hp_max = 70, .mp = 20, .mp_max = 20};
    const SotnCollisionMap map = {
        {f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 16};
    CHECK(SotnPlayer_Create((SotnDataView){f->dra, sizeof(f->dra)}, map, 135, 135, &f->player));
    for (Uint16 i = 0; i < 20; ++i)
        CHECK(Tick(f, 0));
    return true;
}
static bool Flight(Fixture *f)
{
    CHECK(Create(f));
    CHECK(Tick(f, SOTN_BUTTON_BAT) && !f->state.bat.active);
    CHECK(Tick(f, 0));
    f->world.relics[SOTN_RELIC_BAT] = 1;
    CHECK(Tick(f, SOTN_BUTTON_BAT) && !f->state.bat.active);
    CHECK(Tick(f, 0));
    f->world.relics[SOTN_RELIC_BAT] = 3;
    f->character.mp = 1;
    CHECK(Tick(f, SOTN_BUTTON_BAT) && !f->state.bat.active);
    CHECK(Tick(f, 0));
    f->character.mp = 20;
    CHECK(Tick(f, SOTN_BUTTON_BAT) && f->state.bat.started && f->state.bat.hidden &&
          f->state.animation_bank == 1 && f->state.animation.sprite != 0);
    CHECK(!SotnPlayer_SetBatContext(f->player, NULL, 0, true));
    Uint16 waited = 0;
    while (f->state.substep == 0 && waited < 50)
    {
        CHECK(Tick(f, 0));
        ++waited;
    }
    CHECK(waited >= 34 && waited <= 37 && f->state.animation_bank == 13 && !f->state.bat.hidden &&
          f->state.bat.signal == 3 && f->character.mp == 20);
    const Sint32 x = f->state.x;
    for (Uint16 i = 0; i < 40; ++i)
        CHECK(Tick(f, SOTN_BUTTON_RIGHT));
    CHECK(f->state.x == x + 50 * 65536 && f->state.velocity_x == 81920 &&
          f->state.bat.rotation == 384 && f->state.solid_height == 7);
    CHECK(Tick(f, SOTN_BUTTON_JUMP | SOTN_BUTTON_RIGHT));
    CHECK(Tick(f, SOTN_BUTTON_JUMP | SOTN_BUTTON_RIGHT));
    CHECK(f->state.substep == 1 && f->state.velocity_x < 81920);
    CHECK(Tick(f, SOTN_BUTTON_LEFT | SOTN_BUTTON_UP));
    CHECK(f->state.velocity_x == -57344 && f->state.velocity_y == -57344 && f->state.facing_left);
    CHECK(Tick(f, SOTN_BUTTON_BAT) && f->state.step == SOTN_PLAYER_UNMORPH_BAT &&
          f->state.bat.return_started && f->state.bat.hidden && !f->state.rotated);
    waited = 0;
    bool restored = false;
    while (f->state.bat.active && waited < 100)
    {
        CHECK(Tick(f, 0));
        restored |= f->state.animation_bank == 1 && f->state.bat.signal == 1;
        ++waited;
    }
    CHECK(restored && !f->state.bat.active && f->state.animation_bank == 1 &&
          !f->state.bat.hidden && waited >= 35 && waited < 90);
    const Uint32 generation = f->state.bat.generation;
    CHECK(Tick(f, SOTN_BUTTON_BAT) && !f->state.bat.active);
    for (Uint16 i = 0; i < 30; ++i)
        CHECK(Tick(f, 0));
    CHECK(Tick(f, SOTN_BUTTON_BAT) && f->state.bat.active && f->state.bat.generation > generation);
    SotnPlayer_Close(f->player);
    return true;
}
static bool WallAndWater(Fixture *f)
{
    CHECK(Create(f));
    f->world.relics[SOTN_RELIC_BAT] = 3;
    for (Uint16 y = 0; y < 10; ++y)
        f->tiles[(y * 32U + 10U) * 2U] = 2;
    CHECK(Tick(f, SOTN_BUTTON_BAT));
    for (Uint16 i = 0; i < 80; ++i)
        CHECK(Tick(f, SOTN_BUTTON_RIGHT));
    CHECK(f->state.x < 160 * 65536 && (f->state.contacts & 4));
    f->water = true;
    CHECK(Tick(f, 0) && f->state.bat.return_started && !f->state.mp_spent);
    SotnPlayer_Close(f->player);
    return true;
}
static bool Cramped(Fixture *f)
{
    CHECK(Create(f));
    f->world.relics[SOTN_RELIC_BAT] = 3;
    CHECK(Tick(f, SOTN_BUTTON_BAT));
    for (Uint16 i = 0; i < 40; ++i)
        CHECK(Tick(f, 0));
    for (Uint16 x = 0; x < 32; ++x)
        f->tiles[(7U * 32U + x) * 2U] = 1;
    CHECK(Tick(f, 0) && Tick(f, SOTN_BUTTON_BAT));
    Uint16 wait = 0;
    while (f->state.bat.active && wait < 100)
    {
        CHECK(Tick(f, 0));
        ++wait;
    }
    CHECK(!f->state.bat.active && f->state.bat.cramped && f->state.step == SOTN_PLAYER_STUCK &&
          f->state.sound == 0x6f2);
    SotnPlayer_Close(f->player);
    return true;
}
static bool DamageAndExhaustion(Fixture *f)
{
    CHECK(Create(f));
    f->world.relics[SOTN_RELIC_BAT] = 3;
    CHECK(Tick(f, SOTN_BUTTON_BAT));
    for (Uint16 i = 0; i < 40; ++i)
        CHECK(Tick(f, 0));
    CHECK(SotnPlayer_Damage(f->player, 1, 10, 0));
    CHECK(SotnPlayer_GetState(f->player, &f->state));
    CHECK(f->state.hp == 69 && f->state.step == SOTN_PLAYER_HIT &&
          f->state.velocity_y == -2 * 65536 && f->state.animation_bank == 1 &&
          !f->state.bat.active && !f->state.rotated);
    SotnPlayer_Close(f->player);
    CHECK(Create(f));
    f->world.relics[SOTN_RELIC_BAT] = 3;
    f->character.mp = 2;
    CHECK(Tick(f, SOTN_BUTTON_BAT));
    Uint16 cost = 0;
    for (Uint16 i = 0; i < 100 && !f->state.bat.return_started; ++i)
    {
        CHECK(Tick(f, 0));
        cost = (Uint16)(cost + f->state.mp_spent);
    }
    CHECK(f->state.bat.return_started && cost == 1 && f->character.mp == 1);
    SotnPlayer_Close(f->player);
    return true;
}

int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok = Flight(f) && WallAndWater(f) && Cramped(f) && DamageAndExhaustion(f);
    SDL_free(f);
    return ok ? 0 : 1;
}
