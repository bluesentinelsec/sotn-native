#include "character_fixture.h"
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
static bool Advance(SotnPlayer *p)
{
    return SotnPlayer_TimersTick(p) && SotnPlayer_AfterimageBegin(p);
}
static bool Words(void)
{
    const Uint16 values[] = {0, 1, 2, 255, 256, 32767, 32768, 65535};
    for (Uint16 index = 0; index < SOTN_PLAYER_TIMERS; ++index)
        for (Uint16 v = 0; v < SDL_arraysize(values); ++v)
        {
            SotnPlayer p = {0};
            p.timers[index] = values[v];
            CHECK(Advance(&p) && p.timers[index] == (values[v] ? values[v] - 1U : 0));
        }
    SotnPlayer p = {0};
    p.poison_ticks = 4095;
    p.subweapon_ticks = 4;
    p.dark_ticks = 1234;
    p.spell_use_ticks = 65535;
    p.invincibility_flash_ticks = 10;
    p.afterimage_timer15 = 300;
    CHECK(p.timers[0] == 4095 && p.timers[10] == 4 && p.timers[11] == 1234 && p.timers[13] == 10 &&
          !p.timers[14] && p.timers[15] == 300);
    const SotnPlayer before = p;
    SotnPlayerState copied = {0};
    CHECK(SotnPlayer_GetState(&p, &copied) && copied.dark.ticks == 1234 &&
          copied.spell_use_ticks == 65535 && copied.invulnerable && (copied.status & 0x100U) &&
          (copied.status & 0x800U) &&
          !SDL_memcmp(copied.player_timers, p.timers, sizeof(p.timers)) &&
          !SDL_memcmp(before.timers, p.timers, sizeof(p.timers)));
    CHECK(SotnPlayer_SetUseTimer(&p, 7) && p.subweapon_ticks == 7 && p.poison_ticks == 4095);
    CHECK(SotnPlayer_SetSpellUseTimer(&p, 4) && p.timers[12] == 4 && p.timers[10] == 7 &&
          p.poison_ticks == 4095 && p.invincibility_flash_ticks == 10 && !p.invincibility);
    const SotnPlayer independent = p;
    CHECK(!SotnPlayer_SetSpellUseTimer(&p, 33) && !SDL_memcmp(&p, &independent, sizeof(p)) &&
          !SotnPlayer_SetSpellUseTimer(NULL, 4));
    CHECK(SotnPlayer_GetState(&p, &copied) && copied.spell_use_ticks == 4 &&
          (copied.status & 0x1000U) && copied.player_timers[10] == 7 &&
          copied.player_timers[12] == 4);
    CHECK(SotnPlayer_SetSpellUseTimer(&p, 0) && SotnPlayer_GetState(&p, &copied) &&
          !(copied.status & 0x1000U) && (copied.status & 0x800U) && copied.player_timers[10] == 7);
    p.weapon_startup = 4;
    CHECK(SotnPlayer_GetState(&p, &copied) && (copied.status & 0x400U));
    p.invincibility_flash_ticks = 1;
    CHECK(Advance(&p) && !p.invincibility_flash_ticks && p.invincibility == 15);
    p.invincibility = 32767;
    SotnPlayer_Protect(&p, 12);
    CHECK(p.invincibility == 32767);
    p.invincibility = 32768;
    SotnPlayer_Protect(&p, 12);
    CHECK(p.invincibility == 12);
    p.invincibility = 65535;
    SotnPlayer_Protect(&p, 16);
    CHECK(p.invincibility == 16);
    return true;
}
static bool Expiry(void)
{
    for (Uint16 index = 0; index <= 1; ++index)
        for (Uint16 bits = 0; bits < 8; ++bits)
        {
            SotnPlayer p = {.timer_status = (bits & 1 ? 0x80U : 0U) | (bits & 2 ? 0x4000U : 0U) |
                                            (bits & 4 ? 0x8000U : 0U)};
            p.timers[index] = 1;
            CHECK(Advance(&p));
            const Uint16 born = bits & (1U | 4U) ? 0 : 11;
            CHECK(!p.timers[index] && p.color_ticks == born && p.invincibility == born &&
                  p.afterimage_timer15 == born);
        }
    SotnPlayer p = {.palette = 0x8100, .damage_palette = 0x8160, .gravity = {.timer = 0x8166}};
    p.hit_effect_ticks = p.palette_ticks = 2;
    CHECK(Advance(&p) && p.palette == 0x8166 && p.hit_effect_ticks == 1 && p.palette_ticks == 1 &&
          p.afterimage_timer15 == 11);
    CHECK(Advance(&p) && p.palette == 0x8100 && !p.hit_effect_ticks && !p.palette_ticks &&
          p.color_ticks == 11 && p.invincibility == 11 && p.afterimage_timer15 == 11);
    p = (SotnPlayer){.step = SOTN_PLAYER_FALL, .movement_flags = 0x10U};
    p.fall_ticks = 1;
    CHECK(Advance(&p) && p.animation.id == 28 && !(p.movement_flags & 0x10U));
    return true;
}
typedef struct Fixture
{
    Uint8 dra[0x11000], exe[0x24000], tiles[32 * 16 * 2], types[2];
    SotnPlayer *p;
} Fixture;
static bool Create(Fixture *f)
{
    SDL_memset(f, 0, sizeof(*f));
    TestCharacter_Fill(f->dra);
    for (Uint16 i = 0; i < 247; ++i)
        TestCharacter_Put32(f->dra + 0x101b8 + i * 4U, 0x800a0200);
    TestCharacter_Put32(f->dra + 0x200, 2U | (1U << 16));
    TestCharacter_Put32(f->dra + 0x204, 2U | (2U << 16));
    TestCharacter_Put32(f->dra + 0x208, 65535);
    f->types[1] = 3;
    for (Uint16 y = 10; y < 16; ++y)
        for (Uint16 x = 0; x < 32; ++x)
            f->tiles[(y * 32U + x) * 2U] = 1;
    CHECK(SotnPlayer_Create((SotnDataView){f->dra, sizeof(f->dra)},
                            (SotnCollisionMap){
                                {f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 16},
                            100, 135, &f->p) &&
          SotnPlayer_SetScriptContext(f->p, (SotnDataView){f->exe, sizeof(f->exe)}));
    return true;
}
static bool HoldsAndFailure(Fixture *f)
{
    CHECK(Create(f));
    SotnPlayer *p = f->p;
    for (Uint16 i = 0; i < SOTN_PLAYER_TIMERS; ++i)
        p->timers[i] = 300;
    Uint16 held[SOTN_PLAYER_TIMERS];
    SDL_memcpy(held, p->timers, sizeof(held));
    CHECK(SotnPlayer_TickSpellFreeze(p, SOTN_BUTTON_RIGHT, false) &&
          !SDL_memcmp(held, p->timers, sizeof(held)) && SotnPlayer_EndSpellFreeze(p) &&
          !SDL_memcmp(held, p->timers, sizeof(held)));
    CHECK(SotnPlayer_ChangeRoom(p, p->map, 1, -1) && !SDL_memcmp(held, p->timers, sizeof(held)));
    p->script_executable.size = 1;
    SotnPlayer before;
    SDL_memcpy(&before, p, sizeof(before));
    CHECK(!SotnPlayer_Tick(p, 0) && !SDL_memcmp(&before, p, sizeof(before)));
    p->script_executable.size = sizeof(f->exe);
    CHECK(SotnPlayer_Tick(p, 0));
    for (Uint16 i = 0; i < SOTN_PLAYER_TIMERS; ++i)
        CHECK(p->timers[i] == (i == SOTN_PLAYER_TIMER_AFTERIMAGE ? 11 : 299));
    p->step = SOTN_PLAYER_HIT;
    p->substep = 1;
    p->vy = -2 * 65536;
    p->invincibility = 300;
    CHECK(SotnPlayer_Tick(p, 0) && p->invincibility == 299);
    p->hp = 0;
    p->step = SOTN_PLAYER_KILL;
    p->death.phase = 128;
    p->healing.pending = 8;
    p->healing.pending_kind = 1;
    SDL_memcpy(held, p->timers, sizeof(held));
    CHECK(SotnPlayer_Tick(p, 0) && !p->hp && p->healing.pending == 8 &&
          !SDL_memcmp(held, p->timers, sizeof(held)));
    SotnPlayer_Close(p);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok = Words() && Expiry() && HoldsAndFailure(f);
    SDL_free(f);
    return ok ? 0 : 1;
}
