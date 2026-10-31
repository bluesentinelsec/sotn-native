#include "player_internal.h"
#include <sotn/hud_resources.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s: %s", __LINE__, #v, SDL_GetError());                               \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static bool Resources(void)
{
    SotnHudResources s = {0};
    SotnHudResourcesEvents e = {0};
    for (Uint32 clock = 0; clock < 128; ++clock)
    {
        CHECK(SotnHudResources_Tick(
                  &s, (SotnHudResourcesInput){.game_timer = clock, .mp = 1, .mp_max = 20}, &e) &&
              e.mp == (clock % 64U == 0 ? 2 : 1) && !e.heal);
        CHECK(SotnHudResources_Tick(
                  &s,
                  (SotnHudResourcesInput){
                      .game_timer = clock, .mp = 1, .mp_max = 20, .mystic_pendant = true},
                  &e) &&
              e.mp == (clock % 64U == 0 || clock % 64U == 31 ? 2 : 1));
        CHECK(SotnHudResources_Tick(&s,
                                    (SotnHudResourcesInput){.game_timer = clock,
                                                            .player_status = 0x20000000,
                                                            .mp = 1,
                                                            .mp_max = 20,
                                                            .mystic_pendant = true},
                                    &e) &&
              e.mp == 1);
    }
    CHECK(SotnHudResources_Tick(
              &s,
              (SotnHudResourcesInput){
                  .game_timer = SDL_MAX_UINT32, .mp = 19, .mp_max = 20, .mystic_pendant = true},
              &e) &&
          e.mp == 19);
    CHECK(
        SotnHudResources_Tick(&s,
                              (SotnHudResourcesInput){
                                  .game_timer = 0, .mp = 20, .mp_max = 20, .mystic_pendant = true},
                              &e) &&
        e.mp == 20);
    CHECK(SotnHudResources_Tick(
              &s,
              (SotnHudResourcesInput){
                  .game_timer = 0, .player_status = 0x18150000, .mp = 0, .mp_max = 20},
              &e) &&
          e.mp == 1);
    CHECK(SotnHudResources_Tick(
              &s, (SotnHudResourcesInput){.game_timer = 0, .mp = e.mp, .mp_max = 20}, &e) &&
          e.mp == 2);
    for (Uint16 i = 0; i < 128; ++i)
    {
        CHECK(SotnHudResources_Tick(&s,
                                    (SotnHudResourcesInput){.player_status = 0x14000000,
                                                            .mp_max = 20,
                                                            .healing_mail = true},
                                    &e) &&
              e.heal == (i == 127));
    }
    CHECK(!s.healing_mail_timer);
    s.healing_mail_timer = 127;
    CHECK(SotnHudResources_Tick(&s,
                                (SotnHudResourcesInput){.player_status = 0x10000000,
                                                        .mp_max = 20,
                                                        .healing_mail = true},
                                &e) &&
          !s.healing_mail_timer && !e.heal);
    for (Uint16 form = 0; form < 3; ++form)
    {
        s.healing_mail_timer = 127;
        CHECK(SotnHudResources_Tick(
                  &s,
                  (SotnHudResourcesInput){.player_status = 0x04000000U | (1U << form),
                                          .mp_max = 20,
                                          .healing_mail = true},
                  &e) &&
              !s.healing_mail_timer && !e.heal);
    }
    s.healing_mail_timer = 127;
    CHECK(SotnHudResources_Tick(
              &s, (SotnHudResourcesInput){.player_status = 0x14000000, .mp_max = 20}, &e) &&
          !s.healing_mail_timer && !e.heal);
    s.healing_mail_timer = 128;
    const SotnHudResources before = s;
    const SotnHudResourcesEvents prior = e;
    CHECK(!SotnHudResources_Tick(&s, (SotnHudResourcesInput){.mp_max = 20}, &e) &&
          !SDL_memcmp(&s, &before, sizeof(s)) && !SDL_memcmp(&e, &prior, sizeof(e)));
    s.healing_mail_timer = 0;
    CHECK(!SotnHudResources_Tick(&s, (SotnHudResourcesInput){.mp = 21, .mp_max = 20}, &e) &&
          !SDL_memcmp(&e, &prior, sizeof(e)));
    return true;
}
static void P32(Uint8 *p, Uint32 v)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (i * 8U));
}
static bool Healing(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000);
    CHECK(dra);
    Uint8 tiles[32 * 16 * 2] = {0}, types[2] = {0};
    for (Uint16 id = 0; id < 247; ++id)
        P32(dra + 0x101b8 + id * 4U, 0x800a0100);
    for (Uint16 i = 0; i < 32; ++i)
        P32(dra + 0x100 + i * 4U, 3U | ((Uint32)i + 1U) << 16);
    P32(dra + 0x180, 65535);
    types[1] = 3;
    for (Uint16 y = 10; y < 16; ++y)
        for (Uint16 x = 0; x < 32; ++x)
            tiles[(y * 32 + x) * 2] = 1;
    SotnPlayer *p = NULL;
    CHECK(SotnPlayer_Create(
        (SotnDataView){dra, 0x11000},
        (SotnCollisionMap){{tiles, sizeof(tiles)}, {types, sizeof(types)}, 32, 16}, 100, 135, &p));
    p->fixed_stage = true;
    p->hp = 40;
    CHECK(SotnPlayer_QueueBloodHealing(p, 8, 1) && p->healing.pending_kind == 1 &&
          SotnPlayer_QueueMailHealing(p) && p->healing.pending == 1 &&
          p->healing.pending_kind == 2 && SotnPlayer_QueueBloodHealing(p, 16, 1) &&
          p->healing.pending == 1 && p->healing.pending_kind == 2);
    CHECK(SotnPlayer_TickSpellFreeze(p, 0, false) && p->hp == 40 && p->healing.pending == 1 &&
          p->healing.pending_kind == 2 && !p->healing.applied_kind);
    CHECK(SotnPlayer_Tick(p, 0) && p->hp == 41 && !p->healing.pending && !p->healing.pending_kind &&
          p->healing.applied && p->healing.applied_kind == 2 && p->healing.effects &&
          p->healing.source.x == 100 * 65536);
    CHECK(SotnPlayer_Tick(p, 0) && !p->healing.applied && !p->healing.applied_kind &&
          !p->healing.effects);
    p->hp = p->hp_max;
    CHECK(SotnPlayer_QueueMailHealing(p) && SotnPlayer_Tick(p, 0) && p->hp == p->hp_max &&
          p->healing.applied_kind == 2 && p->healing.effects);
    CHECK(SotnPlayer_QueueMailHealing(p));
    p->animation.id = 247;
    const SotnPlayer before = *p;
    CHECK(!SotnPlayer_Tick(p, 0) && !SDL_memcmp(p, &before, sizeof(*p)));
    p->animation.id = 1;
    CHECK(SotnPlayer_Tick(p, 0));
    CHECK(SotnPlayer_Petrify(p, 0, 10, 0) && SotnPlayer_QueueMailHealing(p) &&
          SotnPlayer_Tick(p, 0) && p->healing.applied_kind == 2 && !p->healing.effects);
    p->invincibility = 0;
    CHECK(SotnPlayer_Damage(p, 9999, 10, 0) && !p->hp && SotnPlayer_QueueMailHealing(p) &&
          SotnPlayer_Tick(p, 0) && !p->hp && p->healing.pending == 1 &&
          p->healing.pending_kind == 2 && !p->healing.applied);
    SotnPlayer_Close(p);
    SDL_free(dra);
    return true;
}
int main(void)
{
    return Resources() && Healing() ? 0 : 1;
}
