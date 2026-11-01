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
    Uint16 copies, uploads;
    bool fail;
} Owner;
static bool Capture(void *context, Uint32 generation)
{
    Owner *o = context;
    Uint8 pixels[SOTN_DEATH_IMAGE_BYTES];
    SDL_memset(pixels, 0xab, sizeof(pixels));
    if (o->fail || !SotnDeathImage_Begin(&o->image, (SotnDataView){pixels, sizeof(pixels)}))
        return SDL_SetError("Authored physical capture failure");
    o->generation = generation;
    ++o->copies;
    return true;
}
static bool Decay(void *context, Uint32 generation, bool *finished)
{
    Owner *o = context;
    if (o->fail || generation != o->generation || !SotnDeathImage_Tick(&o->image))
        return SDL_SetError("Authored physical upload failure");
    ++o->uploads;
    *finished = !o->image.remaining;
    return true;
}
static void P32(Uint8 *p, Uint32 value)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(value >> (i * 8U));
}
static bool Run(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000);
    CHECK(dra);
    Uint8 tiles[32 * 16 * 2] = {0}, types[1] = {0};
    for (Uint16 i = 0; i < 247; ++i)
        P32(dra + 0x101b8 + i * 4U, 0x800a0100);
    P32(dra + 0x100, 1U | 1U << 16);
    P32(dra + 0x101b8 + 0xc0U * 4U, 0x800a0200);
    for (Uint16 i = 0; i < 24; ++i)
        P32(dra + 0x200 + i * 4U, 1U | ((Uint32)i + 1U) << 16);
    P32(dra + 0x260, 65535);
    const SotnCollisionMap map = {{tiles, sizeof(tiles)}, {types, sizeof(types)}, 32, 16};
    const Uint16 effects[] = {0, 0x8000, 0x4000, 0x2000, 0xe000, 0x6000};
    const Uint16 variants[] = {0, 1, 2, 3, 1, 2};
    for (Uint16 c = 0; c < SDL_arraysize(effects); ++c)
    {
        SotnPlayer *p = NULL;
        Owner owner = {0};
        CHECK(SotnPlayer_Create((SotnDataView){dra, 0x11000}, map, 100, 135, &p));
        p->fixed_stage = true;
        p->draw_pivot_x = 12;
        p->draw_pivot_y = 20;
        p->contact_dispatch = true;
        CHECK(SotnPlayer_SetDeathCapture(p, Capture, Decay, &owner) &&
              SotnPlayer_ContactDamage(p, 70, 80, 0, 1, effects[c]));
        CHECK(!p->hp && p->step == SOTN_PLAYER_KILL && p->death.native && p->death.started &&
              p->death.generation == 1 && p->death.variant == variants[c] && p->death.color_blend &&
              p->death.red == 128 && p->death.blue == 128 &&
              p->death.blend == (variants[c] == 3 ? 0x30 : 0) && p->animation.id == 0xc0 &&
              !p->draw_pivot_x && !p->draw_pivot_y && !p->form_rotated && p->vx == 80896 &&
              p->vy == -212992 && !owner.copies);
        CHECK(SotnPlayer_QueueMailHealing(p));
        while (p->animation.remaining >= 0)
            CHECK(SotnPlayer_Tick(p, 0) && !p->hp && p->healing.pending && !p->death.started &&
                  !owner.copies);
        CHECK((p->animation.sprite & 0x8000U) && p->form_rotated);
        const SotnPlayer before = *p;
        owner.fail = true;
        CHECK(!SotnPlayer_Tick(p, 0) && !SDL_memcmp(&before, p, sizeof(before)) && !owner.copies);
        owner.fail = false;
        CHECK(SotnPlayer_Tick(p, 0) && p->death.captured && p->death.phase == 2 &&
              owner.copies == 1 && !owner.uploads);
        const SotnPlayerDeath frozen_death = p->death;
        const SotnDeathImage frozen_image = owner.image;
        CHECK(SotnPlayer_TickSpellFreeze(p, 0, false) &&
              !SDL_memcmp(&frozen_death, &p->death, sizeof(frozen_death)) &&
              !SDL_memcmp(&frozen_image, &owner.image, sizeof(frozen_image)) && owner.copies == 1 &&
              !owner.uploads && !p->hp && p->healing.pending);
        const SotnPlayer captured = *p;
        const SotnDeathImage image = owner.image;
        owner.fail = true;
        CHECK(!SotnPlayer_Tick(p, 0) && !SDL_memcmp(&captured, p, sizeof(captured)) &&
              !SDL_memcmp(&image, &owner.image, sizeof(image)));
        owner.fail = false;
        for (Uint16 i = 0; i < 64; ++i)
            CHECK(SotnPlayer_Tick(p, 0) && !p->hp && p->healing.pending && owner.uploads == i + 1U);
        CHECK(p->death.phase == 128 && p->substep == 128 && !p->vy && !p->death.color_blend &&
              !owner.image.remaining);
        for (Uint16 i = 0; i < SOTN_DEATH_IMAGE_BYTES; ++i)
            CHECK(!owner.image.pixels[i]);
        SotnPlayer_Close(p);
    }
    SotnPlayer *p = NULL;
    CHECK(SotnPlayer_Create((SotnDataView){dra, 0x11000}, map, 100, 135, &p));
    Owner owner = {0}, other = {0};
    CHECK(SotnPlayer_SetDeathCapture(p, Capture, Decay, &owner));
    p->death_serial = SDL_MAX_UINT32;
    const SotnPlayer before = *p;
    CHECK(!SotnPlayer_ContactDamage(p, 70, 80, 0, 1, 0) && !SDL_memcmp(&before, p, sizeof(before)));
    p->death_serial = 17;
    CHECK(SotnPlayer_ContactDamage(p, 70, 80, 0, 1, 0) && p->death.generation == 18 &&
          !SotnPlayer_SetDeathCapture(p, Capture, Decay, &other) &&
          SotnPlayer_SetDeathCapture(p, Capture, Decay, &owner));
    SotnPlayer_Close(p);
    SDL_free(dra);
    return true;
}
int main(void)
{
    return Run() ? 0 : 1;
}
