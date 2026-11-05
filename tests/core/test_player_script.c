#include "player_internal.h"

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
    Uint8 dra[0x11000], exe[0x24000], tiles[64 * 16 * 2], types[2];
    SotnPlayer *p;
    SotnCollisionMap map;
    Uint32 clock;
} Fixture;

static void Put32(Uint8 *p, Uint32 value)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(value >> (8U * i));
}
static bool Create(Fixture *f)
{
    SotnPlayer_Close(f->p);
    SDL_memset(f, 0, sizeof(*f));
    for (Uint16 i = 0; i < 247; ++i)
        Put32(f->dra + 0x101b8 + i * 4U, 0x800a0100);
    Put32(f->dra + 0x100, 0x00010002);
    Put32(f->dra + 0x101b8 + 0x33 * 4U, 0x800a0200);
    for (Uint16 i = 0; i < 3; ++i)
        Put32(f->dra + 0x200 + i * 4U, 3U | ((Uint32)i + 1U) << 16);
    Put32(f->dra + 0x20c, 65535);
    for (Uint16 i = 0; i < 1025; ++i)
    {
        const Uint16 value = (Uint16)(i * 4U);
        f->exe[0x1dbcc + i * 2U] = (Uint8)value;
        f->exe[0x1dbcd + i * 2U] = (Uint8)(value >> 8);
    }
    f->types[1] = 3;
    for (Uint16 y = 10; y < 16; ++y)
        for (Uint16 x = 0; x < 64; ++x)
            f->tiles[(y * 64U + x) * 2U] = 1;
    f->map = (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, 2}, 64, 16};
    CHECK(SotnPlayer_Create((SotnDataView){f->dra, sizeof(f->dra)}, f->map, 256, 135, &f->p));
    CHECK(SotnPlayer_SetScriptContext(f->p, (SotnDataView){f->exe, sizeof(f->exe)}));
    return true;
}
static bool Tick(Fixture *f, Uint16 buttons)
{
    f->p->damage_clock = ++f->clock;
    CHECK(SotnPlayer_Tick(f->p, buttons));
    return true;
}
static bool Input(Fixture *f)
{
    CHECK(Create(f));
    CHECK(SotnPlayer_SetSimulatedPad(f->p, (SotnPlayerSimulatedPad){SOTN_BUTTON_RIGHT, 0, 2}));
    CHECK(Tick(f, SOTN_BUTTON_LEFT) && f->p->buttons == SOTN_BUTTON_RIGHT &&
          f->p->simulated_pad.remaining == 1 && f->p->vx == 98304);
    CHECK(Tick(f, SOTN_BUTTON_LEFT) && !f->p->simulated_pad.remaining);
    CHECK(Tick(f, SOTN_BUTTON_LEFT) && f->p->buttons == SOTN_BUTTON_LEFT && !f->p->tapped &&
          f->p->held == SOTN_BUTTON_LEFT && f->p->vx == -98304);
    CHECK(Create(f));
    CHECK(SotnPlayer_SetSimulatedPad(f->p, (SotnPlayerSimulatedPad){.mode = 2, .remaining = 1}));
    CHECK(Tick(f, SOTN_BUTTON_RIGHT) && f->p->step == SOTN_PLAYER_SCRIPT);
    // The raw mode-two word retains the hold even after physical input resumes.
    CHECK(Tick(f, SOTN_BUTTON_RIGHT) && f->p->step == SOTN_PLAYER_SCRIPT &&
          f->p->buttons == SOTN_BUTTON_RIGHT && !f->p->vx && !f->p->tapped);
    CHECK(SotnPlayer_SetSimulatedPad(f->p, (SotnPlayerSimulatedPad){0}));
    CHECK(Tick(f, 0) && f->p->step == SOTN_PLAYER_STAND && f->p->timers[4] == 12 &&
          f->p->invincibility == 12);
    return true;
}
static bool Reaction(Fixture *f)
{
    CHECK(Create(f));
    f->p->facing_left = true;
    f->p->gravity.upside_down = true;
    f->p->step = SOTN_PLAYER_HIGH_JUMP;
    f->p->vx = 12345;
    f->p->vy = -65536;
    f->p->x.parts.low = 123;
    const Sint32 x = f->p->x.value;
    CHECK(SotnPlayer_SetSimulatedPad(f->p, (SotnPlayerSimulatedPad){.mode = 1, .remaining = 1}));
    CHECK(Tick(f, SOTN_BUTTON_RIGHT) && f->p->step == SOTN_PLAYER_SCRIPT_REACTION &&
          f->p->substep == 1 && f->p->animation.id == 0x33 && f->p->facing_left &&
          !f->p->gravity.upside_down && !f->p->vx && !f->p->vy && f->p->timers[2] == 6 &&
          f->p->palette == 0x8100 && f->p->script_reaction.started &&
          f->p->script_reaction.source.x == x && f->p->movement_effects.smoke &&
          !f->p->movement_effects.smoke_blueprint && f->p->hp == 70 && f->p->mp == 20 &&
          f->p->damage_random == 0);
    SotnPlayerState state = {0};
    CHECK(SotnPlayer_GetState(f->p, &state) && !state.can_pause &&
          (state.status & 0x38110000U) == 0x38110000U && state.palette == 0x8100);
    CHECK(SotnPlayer_ChangeRoom(f->p, f->map, 16, 0) &&
          f->p->script_reaction.source.x == x + 16 * 65536 && f->p->timers[2] == 6);
    CHECK(SotnPlayer_ChangeRoom(f->p, f->map, -16, 0));
    for (Uint16 i = 1; i <= 6; ++i)
    {
        CHECK(Tick(f, SOTN_BUTTON_JUMP | SOTN_BUTTON_LEFT_HAND));
        CHECK(f->p->palette == (i < 6 ? 0x8166 : 0x8100) && f->p->timers[2] == 6 - i &&
              !f->p->script_reaction.started && !f->p->movement_effects.smoke);
    }
    for (Uint16 i = 0; f->p->step == SOTN_PLAYER_SCRIPT_REACTION && i < 30; ++i)
        CHECK(Tick(f, 0));
    CHECK(f->p->step == SOTN_PLAYER_STAND && f->p->sound == 0x6f2 && f->p->timers[4] == 12 &&
          !f->p->body_color_blend && f->p->palette == 0x8100 && f->p->invincibility == 12);
    f->clock = 15;
    CHECK(Tick(f, 0) && f->p->body_color_blend && f->p->body_grey[0] == 160 &&
          f->p->body_grey[1] == 192 && f->p->body_grey[2] == 192 && f->p->body_grey[3] == 224);
    for (Uint16 i = 0; i < 11; ++i)
        CHECK(Tick(f, 0));
    CHECK(!f->p->body_color_blend && !f->p->timers[4] && !f->p->invincibility &&
          f->p->damage_random == 0 && f->p->hp == 70);
    return true;
}
static bool AtomicAndHold(Fixture *f)
{
    CHECK(Create(f));
    CHECK(SotnPlayer_SetSimulatedPad(f->p, (SotnPlayerSimulatedPad){.mode = 1, .remaining = 2}));
    Put32(f->dra + 0x101b8 + 0x33 * 4U, 0x800b1000);
    const SotnPlayer before = *f->p;
    CHECK(!SotnPlayer_Tick(f->p, 0) && !SDL_memcmp(&before, f->p, sizeof(before)));
    Put32(f->dra + 0x101b8 + 0x33 * 4U, 0x800a0200);
    CHECK(Tick(f, 0));
    const SotnPlayer holding = *f->p;
    CHECK(SotnPlayer_TickSpellFreeze(f->p, SOTN_BUTTON_JUMP, false) &&
          f->p->timers[2] == holding.timers[2] &&
          f->p->simulated_pad.remaining == holding.simulated_pad.remaining &&
          f->p->animation.remaining == holding.animation.remaining &&
          !f->p->script_reaction.started);
    f->p->animation.remaining = -1;
    CHECK(SotnPlayer_SetSimulatedPad(f->p, (SotnPlayerSimulatedPad){0}) && Tick(f, 0));
    f->exe[0x1dbcd] = 0x20;
    const SotnPlayer pulse = *f->p;
    f->p->damage_clock = 0;
    const SotnPlayer ready = *f->p;
    CHECK(!SotnPlayer_Tick(f->p, 0) && !SDL_memcmp(&ready, f->p, sizeof(ready)) &&
          pulse.timers[4] == f->p->timers[4]);
    return true;
}
static bool Invalid(Fixture *f)
{
    CHECK(Create(f));
    const SotnPlayer before = *f->p;
    CHECK(!SotnPlayer_SetSimulatedPad(f->p, (SotnPlayerSimulatedPad){.mode = 3}));
    CHECK(!SotnPlayer_SetSimulatedPad(f->p, (SotnPlayerSimulatedPad){.buttons = 1}));
    CHECK(!SotnPlayer_SetSimulatedPad(f->p, (SotnPlayerSimulatedPad){.remaining = SDL_MAX_UINT32}));
    CHECK(!SotnPlayer_SetScriptContext(f->p, (SotnDataView){0}));
    CHECK(!SDL_memcmp(&before, f->p, sizeof(before)));
    f->p->hp = 0;
    f->p->step = SOTN_PLAYER_KILL;
    CHECK(!SotnPlayer_SetSimulatedPad(f->p, (SotnPlayerSimulatedPad){.mode = 1, .remaining = 1}));
    return true;
}
static bool GrabPalette(Fixture *f)
{
    CHECK(Create(f));
    CHECK(SotnPlayer_RequestGrab(f->p, (SotnPlayerGrab){.state = SOTN_GRAB_START}));
    CHECK(Tick(f, 0) && f->p->step == SOTN_PLAYER_BOSS_GRAB && f->p->timers[2] == 8 &&
          f->p->damage_palette == 0x8166 && f->p->palette == 0x8100);
    CHECK(Tick(f, 0) && f->p->palette == 0x8166);
    CHECK(SotnPlayer_RequestGrab(f->p, (SotnPlayerGrab){.state = SOTN_GRAB_DAMAGE}));
    CHECK(Tick(f, 0) && f->p->timers[2] == 4 && f->p->damage_palette == 0x8166 && f->p->hp == 69);
    for (Uint16 i = 0; i < 4; ++i)
        CHECK(Tick(f, 0));
    CHECK(f->p->palette == 0x8100 && !f->p->timers[2]);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok = Input(f) && Reaction(f) && AtomicAndHold(f) && Invalid(f) && GrabPalette(f);
    SotnPlayer_Close(f->p);
    SDL_free(f);
    return ok ? 0 : 1;
}
