#include "player_internal.h"
#include <sotn/player_glow.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s: %s", __LINE__, #v, SDL_GetError());                               \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static void Put32(Uint8 *p, Uint32 v)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (i * 8U));
}
typedef struct Fixture
{
    Uint8 dra[0x11000], tiles[32 * 16 * 2], types[2];
    SotnPlayer *p;
    SotnCollisionMap map;
} Fixture;
static bool Create(Fixture *f, bool enabled)
{
    SotnPlayer_Close(f->p);
    SDL_memset(f, 0, sizeof(*f));
    for (Uint16 i = 0; i < 247; ++i)
        Put32(f->dra + 0x101b8 + i * 4U, 0x800a0100);
    for (Uint16 i = 0; i < 3; ++i)
    {
        Put32(f->dra + 0x100 + i * 4U, 2U | ((Uint32)i + 1U) << 16);
        Put32(f->dra + 0x200 + i * 4U, 2U | ((Uint32)i + 1U) << 16);
    }
    Put32(f->dra + 0x10c, 0);
    Put32(f->dra + 0x20c, 65535);
    Put32(f->dra + 0x101b8 + 0xba * 4U, 0x800a0200);
    SDL_memset(f->dra + 0xcf18, 3, 20);
    SDL_memset(f->dra + 0xcf2c, 5, 16);
    SDL_memset(f->dra + 0xcf3c, 28, 16);
    f->dra[0x8410 + 12] = 10;
    /* Authored ordinary unarmed fallback for an unaffordable/disabled command. */
    Put32(f->dra + 0xc0, 0x800a0800);
    f->dra[0x812] = 0x24;
    f->dra[0x815] = 5;
    f->dra[0x81a] = 8;
    f->dra[0x828] = 2;
    Put32(f->dra + 0xd53c, 0x800a0900);
    Put32(f->dra + 0xd540, 0x800a0a00);
    f->dra[0xd549] = 1;
    for (Uint16 i = 0; i < 7; ++i)
        Put32(f->dra + 0x900 + i * 4U, 0x800a0b00);
    f->dra[0xa08] = 18;
    f->dra[0xa09] = 240;
    f->dra[0xa0a] = 10;
    f->dra[0xa0b] = 7;
    f->types[1] = 3;
    for (Uint16 y = 10; y < 16; ++y)
        for (Uint16 x = 0; x < 32; ++x)
            f->tiles[(y * 32U + x) * 2U] = 1;
    f->map = (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, 2}, 32, 16};
    CHECK(SotnPlayer_Create((SotnDataView){f->dra, sizeof(f->dra)}, f->map, 128, 135, &f->p));
    CHECK(SotnPlayer_SetDarkContext(f->p, enabled));
    return true;
}
static bool Command(Fixture *f, bool face, bool final)
{
    const Uint16 back = face ? SOTN_BUTTON_RIGHT : SOTN_BUTTON_LEFT,
                 forward = face ? SOTN_BUTTON_LEFT : SOTN_BUTTON_RIGHT;
    f->p->facing_left = face;
    const Uint16 sequence[] = {back, (Uint16)(back | SOTN_BUTTON_UP), SOTN_BUTTON_UP,
                               (Uint16)(forward | SOTN_BUTTON_UP), forward};
    for (Uint16 i = 0; i < 5; ++i)
        CHECK(SotnPlayer_Tick(f->p, sequence[i]));
    if (final)
        CHECK(SotnPlayer_Tick(f->p, (Uint16)(forward | SOTN_BUTTON_LEFT_HAND)));
    return true;
}
static bool Casting(Fixture *f)
{
    for (Uint16 face = 0; face < 2; ++face)
    {
        CHECK(Create(f, true));
        f->p->mp = 10;
        CHECK(Command(f, face != 0, true));
        SotnPlayerState s = {0};
        CHECK(SotnPlayer_GetState(f->p, &s));
        CHECK(s.step == SOTN_PLAYER_DARK && s.animation.id == 0xba && !s.mp && s.mp_spent == 10 &&
              s.dark.started && s.dark.ticks == 1048 && !s.velocity_x && !s.velocity_y &&
              s.invulnerable && s.status == 0x3a000300U && s.sound == 0x6f3 &&
              s.extra_sound == 0x67d && !s.attack.active);
        const SotnPlayerDark before = s.dark;
        const Sint32 x = s.x;
        CHECK(SotnPlayer_ChangeRoom(f->p, f->map, 16, 0) &&
              f->p->dark.source.x == before.source.x + 16 * 65536 &&
              f->p->dark.ticks == before.ticks);
        CHECK(SotnPlayer_ChangeRoom(f->p, f->map, -16, 0) &&
              !SDL_memcmp(&before, &f->p->dark, sizeof(before)));
        Uint16 t = 0;
        while (f->p->step == SOTN_PLAYER_DARK && t < 16)
        {
            CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_RIGHT) && !f->p->dark.started &&
                  !f->p->mp_spent && f->p->x.value == x);
            ++t;
        }
        CHECK(t < 16 && f->p->step == SOTN_PLAYER_STAND && f->p->dark.ticks == 1048 - t &&
              (SotnPlayer_Status(f->p) & 0x2000000U));
        f->p->dark.ticks = 1;
        CHECK(SotnPlayer_Tick(f->p, 0) && !f->p->dark.ticks &&
              !(SotnPlayer_Status(f->p) & 0x2000000U));
    }
    CHECK(Create(f, true));
    f->p->mp = 9;
    CHECK(Command(f, false, true) && f->p->step != SOTN_PLAYER_DARK && f->p->mp == 9 &&
          !f->p->dark.ticks);
    CHECK(Create(f, false) && Command(f, false, true) && f->p->step != SOTN_PLAYER_DARK &&
          f->p->mp == 20);
    CHECK(Create(f, true));
    f->p->attack_mode = 1;
    f->p->shield_cursor.phase = 1;
    CHECK(Command(f, false, true) && f->p->step == SOTN_PLAYER_DARK && f->p->attack_mode == 1 &&
          !f->p->shield_cursor.phase);
    for (Uint16 t = 0; t < 16; ++t)
        CHECK(SotnPlayer_Tick(f->p, 0));
    CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_RIGHT) && f->p->step == SOTN_PLAYER_WALK);
    CHECK(Create(f, true));
    f->p->dark.ticks = 99;
    CHECK(SotnPlayer_Damage(f->p, 1, 120, 0) && f->p->dark.ticks == 99 && f->p->hp == 69);
    CHECK(Create(f, true) && Command(f, false, false));
    const SotnPlayer before = *f->p;
    Put32(f->dra + 0x101b8 + 0xba * 4U, 0x800b1000);
    CHECK(!SotnPlayer_Tick(f->p, SOTN_BUTTON_RIGHT | SOTN_BUTTON_LEFT_HAND) &&
          !SDL_memcmp(&before, f->p, sizeof(before)));
    return true;
}
static bool FormCancellation(void)
{
    typedef void (*Input)(SotnPlayer *);
    const Input handlers[] = {SotnPlayer_BatInput, SotnPlayer_MistInput, SotnPlayer_WolfInput};
    const Uint16 relics[] = {SOTN_RELIC_BAT, SOTN_RELIC_MIST, SOTN_RELIC_WOLF};
    const Uint16 buttons[] = {SOTN_BUTTON_BAT, SOTN_BUTTON_MIST, SOTN_BUTTON_WOLF};
    const SotnPlayerStep steps[] = {SOTN_PLAYER_BAT, SOTN_PLAYER_MIST, SOTN_PLAYER_WOLF};
    for (Uint16 i = 0; i < 3; ++i)
        for (Uint16 enabled = 0; enabled < 2; ++enabled)
        {
            SotnPlayer *p = SDL_calloc(1, sizeof(*p));
            CHECK(p);
            p->mp = 20;
            p->dark.ticks = 99;
            p->bat_context = p->mist_context = p->wolf_context = true;
            p->buttons = p->tapped = buttons[i];
            p->relics[relics[i]] = enabled ? 3 : 1;
            handlers[i](p);
            CHECK(!p->failed && p->dark.ticks == (enabled ? 0 : 99) &&
                  p->step == (enabled ? steps[i] : SOTN_PLAYER_STAND));
            SDL_free(p);
        }
    return true;
}
static bool Glow(void)
{
    Uint8 *dra = SDL_calloc(1, 0xcf00);
    CHECK(dra);
    Put32(dra + 0x3bb8 + 23U * 4U, 0x800a3000);
    Put32(dra + 0x3000, 0x20004);
    Put32(dra + 0x3004, 0x100a);
    Put32(dra + 0x3008, 1);
    Put32(dra + 0x300c, 0x800a37d8);
    Put32(dra + 0x3010, 0x800a37da);
    Put32(dra + 0x3014, SDL_MAX_UINT32);
    Put32(dra + 0xce50, 0x803f);
    Put32(dra + 0xce54, 0x83e0);
    SotnDataView view = {dra, 0xcf00};
    SotnPlayerGlow g = {0};
    CHECK(SotnPlayerGlow_Init(&g, view) && g.initialized && !g.absorbing && !g.color && !g.phase &&
          g.interval == 2);
    CHECK(SotnPlayerGlow_Tick(&g) && g.phase == 1 && g.delay == 1);
    CHECK(SotnPlayerGlow_Set(&g, view, true) && g.color == 0x803f && g.phase == 1 && g.delay == 1);
    CHECK(SotnPlayerGlow_Tick(&g) && g.color == 0x803f && !g.delay);
    CHECK(SotnPlayerGlow_Tick(&g) && g.color == 0x805e && g.delay == 1 && g.phase == 1);
    const SotnPlayerGlow before = g;
    CHECK(SotnPlayerGlow_Set(&g, view, true) && !SDL_memcmp(&g, &before, sizeof(g)));
    CHECK(SotnPlayerGlow_Set(&g, view, false) && !g.color && g.phase == before.phase &&
          g.delay == before.delay);
    const SotnPlayerGlow normal = g;
    Put32(dra + 0xce50, 0x10000);
    CHECK(!SotnPlayerGlow_Set(&g, view, true) && !SDL_memcmp(&g, &normal, sizeof(g)));
    CHECK(!SotnPlayerGlow_Init(&g, (SotnDataView){0}) && !SDL_memcmp(&g, &normal, sizeof(g)));
    g.phase = 2;
    CHECK(!SotnPlayerGlow_Tick(&g));
    SDL_free(dra);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    bool ok = Casting(f) && Glow() && FormCancellation();
    SotnPlayer_Close(f->p);
    SDL_free(f);
    return ok ? 0 : 1;
}
