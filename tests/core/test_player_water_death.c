#include "player_internal.h"
#include <sotn/death_image.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s: %s", __LINE__, #v, SDL_GetError());                               \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
typedef struct Owner
{
    SotnDeathImage image;
    Uint32 generation;
    Uint16 captures, decays;
    bool fail;
} Owner;
// Authored copied-image adapters; real native texture commits have separate integration checks.
static bool Capture(void *context, Uint32 generation)
{
    Owner *o = context;
    Uint8 pixels[SOTN_DEATH_IMAGE_BYTES] = {0};
    SDL_memset(pixels, 0xab, sizeof(pixels));
    if (o->fail)
        return SDL_SetError("Authored capture failure");
    if (!SotnDeathImage_Begin(&o->image, (SotnDataView){pixels, sizeof(pixels)}))
        return false;
    o->generation = generation;
    ++o->captures;
    return true;
}
static bool Decay(void *context, Uint32 generation, bool *finished)
{
    Owner *o = context;
    if (o->fail || generation != o->generation)
        return SDL_SetError("Authored decay/ownership failure");
    if (!SotnDeathImage_Tick(&o->image))
        return false;
    ++o->decays;
    *finished = o->image.remaining == 0;
    return true;
}
static void P32(Uint8 *p, Uint32 v)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (i * 8U));
}
static bool Run(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000);
    CHECK(dra);
    Uint8 tiles[32 * 16 * 2] = {0}, types[1] = {0};
    for (Uint16 i = 0; i < 247; ++i)
        P32(dra + 0x101b8 + i * 4U, 0x800a0100);
    P32(dra + 0x100, 1U | 1U << 16);
    P32(dra + 0x101b8 + 0xc1U * 4U, 0x800a0200);
    for (Uint16 i = 0; i < 20; ++i)
        P32(dra + 0x200 + i * 4U, 1U | ((Uint32)i + 1U) << 16);
    P32(dra + 0x250, 65535);
    const SotnCollisionMap map = {{tiles, sizeof(tiles)}, {types, sizeof(types)}, 32, 16};
    SotnPlayer *p = NULL;
    CHECK(SotnPlayer_Create((SotnDataView){dra, 0x11000}, map, 100, 135, &p));
    p->hp = 1;
    p->fixed_stage = true;
    p->draw_pivot_y = 2;
    const SotnWaterDamageEvents lethal = {.hp = 0, .damage = 1, .response = 6};
    const SotnPlayer unbound = *p;
    CHECK(!SotnPlayer_StartWaterDeath(p, lethal) && !SDL_memcmp(p, &unbound, sizeof(*p)));
    Owner owner = {0};
    CHECK(SotnPlayer_SetDeathCapture(p, Capture, Decay, &owner));
    p->invincibility = 1;
    const SotnPlayer protected = *p;
    CHECK(!SotnPlayer_StartWaterDeath(p, lethal) && !SDL_memcmp(p, &protected, sizeof(*p)));
    p->invincibility = 0;
    CHECK(SotnPlayer_StartWaterDeath(p, lethal) && !p->hp && p->step == SOTN_PLAYER_KILL_WATER);
    CHECK(SotnPlayer_Tick(p, SOTN_BUTTON_RIGHT | SOTN_BUTTON_LEFT_HAND) && p->water_death.started &&
          p->water_death.motion.rotation == 512 && !p->water_death.motion.draw_flags &&
          p->animation.id == 0xc1 && p->sound == 0x6f6 && p->draw_pivot_y == 2 && !p->attack_mode &&
          !p->vx);
    const Sint32 x = p->x.value;
    const Sint32 held_animation = p->animation.remaining;
    Owner unrelated = {0};
    CHECK(!SotnPlayer_SetDeathCapture(p, Capture, Decay, &unrelated));
    CHECK(SotnPlayer_TickSpellFreeze(p, 0, true) && p->animation.remaining == held_animation &&
          !p->water_death.started && !owner.captures && !owner.decays);
    while (p->animation.remaining >= 0)
        CHECK(SotnPlayer_Tick(p, SOTN_BUTTON_RIGHT | SOTN_BUTTON_LEFT_HAND) && p->x.value == x &&
              !p->attack.active && !p->water_death.started);
    owner.fail = true;
    const SotnPlayer before_capture = *p;
    CHECK(!SotnPlayer_Tick(p, 0) && !SDL_memcmp(p, &before_capture, sizeof(*p)) &&
          !owner.captures && !owner.image.captured);
    owner.fail = false;
    CHECK(SotnPlayer_Tick(p, 0) && p->water_death.captured);
    CHECK(owner.captures == 1 && p->step == SOTN_PLAYER_KILL && p->substep == 2 &&
          owner.image.remaining == 64 && p->water_death.motion.draw_flags == 4);
    owner.fail = true;
    const SotnPlayer before = *p;
    CHECK(!SotnPlayer_Tick(p, 0) && !SDL_memcmp(p, &before, sizeof(*p)) &&
          owner.image.remaining == 64 && !owner.decays);
    owner.fail = false;
    for (Uint16 i = 0; i < 64; ++i)
        CHECK(SotnPlayer_Tick(p, SOTN_BUTTON_LEFT_HAND) && !p->hp && p->invincibility == 12);
    CHECK(owner.decays == 64 && owner.captures == 1 && !owner.image.remaining &&
          p->substep == 128 && p->death.phase == 128 && !p->vy &&
          !p->water_death.motion.color_blend);
    SotnPlayerState state = {0};
    CHECK(SotnPlayer_GetState(p, &state) && !state.can_pause && state.invulnerable &&
          (state.status & 0x40000U) && state.water_death.captured);
    CHECK(SotnPlayer_QueueBloodHealing(p, 8, 1) && p->healing.pending == 8 && !p->hp &&
          SotnPlayer_Tick(p, 0) && p->healing.pending == 8 && !p->hp && !p->healing.applied &&
          owner.decays == 64);
    CHECK(!SotnPlayer_Restore(p, 1, 0));
    SotnPlayer_Close(p);
    SDL_free(dra);
    return true;
}
int main(void)
{
    return Run() ? 0 : 1;
}
