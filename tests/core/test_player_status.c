#include "player_internal.h"
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s: %s", __LINE__, #v, SDL_GetError());                               \
            return false;                                                                          \
        }                                                                                          \
    } while (0)

static bool Snapshots(void)
{
    const struct
    {
        SotnPlayerStep step;
        Uint16 substep;
        Uint32 status;
    } cases[] = {
        {SOTN_PLAYER_STAND, 0, 0x10000000},        {SOTN_PLAYER_STAND, 3, 0x10000000},
        {SOTN_PLAYER_STAND, 4, 0x18100000},        {SOTN_PLAYER_WALK, 0, 0x14000000},
        {SOTN_PLAYER_CROUCH, 0, 0x10000020},       {SOTN_PLAYER_CROUCH, 2, 0x10000000},
        {SOTN_PLAYER_FALL, 0, 0x10002000},         {SOTN_PLAYER_JUMP, 0, 0x10002000},
        {SOTN_PLAYER_HIT, 1, 0x38110000},          {SOTN_PLAYER_KILL, 1, 0x18150000},
        {SOTN_PLAYER_KILL, 0x80, 0x181d0000},      {SOTN_PLAYER_STONE, 0, 0x38110080},
        {SOTN_PLAYER_BOSS_GRAB, 1, 0x38110040},    {SOTN_PLAYER_SCRIPT, 0, 0x38110000},
        {SOTN_PLAYER_HIGH_JUMP, 0, 0x38000000},    {SOTN_PLAYER_BAT, 0, 0x28100001},
        {SOTN_PLAYER_UNMORPH_BAT, 0, 0x28500001},  {SOTN_PLAYER_MIST, 0, 0x28100002},
        {SOTN_PLAYER_UNMORPH_MIST, 0, 0x28900002}, {SOTN_PLAYER_WOLF, 0, 0x6c100204},
        {SOTN_PLAYER_WOLF, 1, 0x2c100204},         {SOTN_PLAYER_WOLF, 3, 0x2c100224},
        {SOTN_PLAYER_WOLF, 8, 0x6c100204},         {SOTN_PLAYER_UNMORPH_WOLF, 0, 0x68100000},
        {SOTN_PLAYER_DARK, 0, 0x38000200},         {SOTN_PLAYER_STUCK, 0, 0x18100030}};
    SotnPlayer *p = SDL_calloc(1, sizeof(*p));
    CHECK(p);
    for (Uint16 i = 0; i < SDL_arraysize(cases); ++i)
        for (Uint16 timers = 0; timers < 32; ++timers)
        {
            *p = (SotnPlayer){.step = cases[i].step,
                              .substep = cases[i].substep,
                              .status = SDL_MAX_UINT32,
                              .invincibility = (timers & 1) ? 1 : 0,
                              .timers = {(timers & 2) ? 1 : 0},
                              .poison_ticks = (timers & 4) ? 1 : 0,
                              .curse_ticks = (timers & 8) ? 1 : 0,
                              .dark = {.ticks = (timers & 16) ? 1 : 0}};
            const SotnPlayer before = *p;
            Uint32 expected = cases[i].status | ((timers & 1) ? 0x100U : 0U) |
                              ((timers & 2) ? 0x800U : 0U) | ((timers & 4) ? 0x28104000U : 0U) |
                              ((timers & 8) ? 0x28108000U : 0U) | ((timers & 16) ? 0x2000000U : 0U);
            SotnPlayerState s = {0};
            CHECK(SotnPlayer_GetState(p, &s) && s.status == expected &&
                  !SDL_memcmp(p, &before, sizeof(before)));
            p->bell_inside = true;
            CHECK(SotnPlayer_GetState(p, &s) && s.status == (expected | 0x100008U));
        }
    SDL_free(p);
    return true;
}

int main(void)
{
    return Snapshots() ? 0 : 1;
}
