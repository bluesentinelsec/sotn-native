// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/wolfForm.c:func_8012F178, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Captured pixels remain privately owned.
#include <SDL3/SDL_error.h>
#include <sotn/wolf_root_effect.h>
static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
static void Texture(SotnPlayerEffectQuad *q)
{
    q->u[0] = q->u[2] = 0;
    q->u[1] = q->u[3] = 63;
    q->v[0] = q->v[1] = 192;
    q->v[2] = q->v[3] = 255;
    q->page = 0x118;
}
static void Position(SotnPlayerEffectQuad *q, Sint16 x, Sint16 y, bool facing)
{
    const Sint32 left = x + (facing ? -20 : -44), top = y - 24;
    q->x[0] = q->x[2] = Word(left);
    q->x[1] = q->x[3] = Word(left + 63);
    q->y[0] = q->y[1] = Word(top);
    q->y[2] = q->y[3] = Word(top + 63);
}
static Uint8 Fade(Uint8 value, Uint8 decrement)
{
    return value > 12 ? (Uint8)(value - decrement) : 0;
}
bool SotnWolfRootEffect_Tick(SotnWolfRootEffect *effect, SotnWolfRootEvents request, Sint16 x,
                             Sint16 y, Sint32 velocity_x, Uint16 priority, bool facing_left)
{
    if (!effect || request.image_index > 4 || velocity_x < -16 * 65536 || velocity_x > 16 * 65536 ||
        priority < 6 || priority > 511)
        return SDL_SetError(
            "Wolf root effects require six actual bounded image/primitive snapshots");
    for (Uint16 i = 0; i < 5; ++i)
        if (effect->images[i].state < 0 || effect->images[i].state > 3 ||
            (effect->images[i].state == 3 &&
             (effect->images[i].timer < 1 || effect->images[i].timer > 32)))
            return SDL_SetError("Wolf retained image has an invalid original phase or timer");
    SotnWolfRootEffect next = *effect;
    for (Uint16 i = 0; i < 5; ++i)
    {
        SotnPlayerEffectQuad *q = &next.quads[i];
        SotnWolfRootImage *image = &next.images[i];
        if ((Sint32)i == request.image_index && image->state)
            image->state = 2;
        switch (image->state)
        {
        case 0:
            Texture(q);
            q->draw_mode = 0x13f;
            image->state = 1;
            /* fall through */
        case 1:
            q->priority = (Uint16)(priority - 6U);
            q->draw_mode |= 8;
            break;
        case 2:
            image->facing_left = facing_left;
            Position(q, x, y, facing_left);
            q->draw_mode = (Uint16)(q->draw_mode & 0xfff7U);
            for (Uint16 vertex = 0; vertex < 4; ++vertex)
                q->red[vertex] = q->green[vertex] = q->blue[vertex] = 128;
            q->priority = (Uint16)(priority - 6U);
            image->timer = 32;
            image->state = 3;
            break;
        case 3: {
            const Uint8 left = Fade(q->green[2], image->facing_left ? 6 : 12),
                        right = Fade(q->green[3], image->facing_left ? 12 : 6);
            q->red[0] = q->blue[0] = q->green[0] = q->red[2] = q->blue[2] = q->green[2] = left;
            q->red[1] = q->blue[1] = q->green[1] = q->red[3] = q->blue[3] = q->green[3] = right;
            q->priority = (Uint16)(priority - 6U);
            if (!--image->timer)
                image->state = 1;
            q->draw_mode = (Uint16)(q->draw_mode & 0xfff7U);
            break;
        }
        }
    }
    SotnPlayerEffectQuad *q = &next.quads[5];
    if (!request.rush_image)
        q->draw_mode |= 8;
    else
    {
        Texture(q);
        q->priority = priority;
        q->draw_mode = 0x335;
        next.images[5].facing_left = facing_left;
        Position(q, x, y, facing_left);
        const Sint32 difference = SDL_abs(velocity_x) - 3 * 65536;
        const Sint32 shifted =
            difference >= 0 ? difference / 256 : (Sint32)(-((-(Sint64)difference + 255) / 256));
        const Uint8 amount = (Uint8)(Uint32)SDL_min(shifted, 127);
        const Uint8 upper = (Uint8)(amount + 128U), lower = (Uint8)(128U - amount);
        const Uint8 left = facing_left ? upper : lower, right = facing_left ? lower : upper;
        q->blue[0] = facing_left ? upper : 255;
        q->blue[1] = facing_left ? 255 : lower;
        q->blue[2] = q->blue[3] = 255;
        q->red[0] = q->green[0] = q->red[2] = q->green[2] = left;
        q->red[1] = q->green[1] = q->red[3] = q->green[3] = right;
    }
    *effect = next;
    return true;
}
