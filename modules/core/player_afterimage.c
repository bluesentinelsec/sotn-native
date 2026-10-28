// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/alu_anim.c afterimage capture/fade and dra/6DF70.c reset/enable/force,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Tables remain private.
#include <SDL3/SDL.h>
#include <sotn/player_afterimage.h>
static Sint16 Word(Sint32 n)
{
    const Uint16 bits = (Uint16)(Uint32)n;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
static Sint16 High(Sint32 n)
{
    return Word((Sint32)((Uint32)n >> 16));
}
static Sint32 SetHigh(Sint32 n, Sint16 high)
{
    const Uint32 bits = (Uint32)(Uint16)high << 16 | ((Uint32)n & 65535U);
    return (Sint32)(bits <= SDL_MAX_SINT32 ? (Sint64)bits : (Sint64)bits - 4294967296LL);
}
bool SotnPlayerAfterimage_Init(SotnPlayerAfterimage *s)
{
    if (!s)
        return SDL_InvalidParamError("afterimage");
    *s = (SotnPlayerAfterimage){0};
    for (Uint16 i = 0; i < 3; ++i)
        s->displays[i] =
            (SotnAfterimageDisplay){.bank = 1, .palette = 0x8100, .texture_slot = (Uint16)(i + 1U)};
    return true;
}
bool SotnPlayerAfterimage_Reset(SotnPlayerAfterimage *s, bool clear, Sint32 duration,
                                Uint16 *timer15)
{
    if (!s || !timer15)
        return SDL_SetError("Afterimage reset requires its cursor and actual player timer");
    if (clear)
    {
        s->reset = true;
        for (Uint16 i = 0; i < 3; ++i)
            s->displays[i].sprite = 0;
        for (Uint16 i = 0; i < 6; ++i)
            s->samples[i].sprite = 0;
    }
    s->disabled = true;
    s->index = 10;
    if (duration)
        *timer15 = duration < 4 ? 4 : (Uint16)(Uint32)duration;
    return true;
}
bool SotnPlayerAfterimage_Enable(SotnPlayerAfterimage *s)
{
    if (!s)
        return SDL_InvalidParamError("afterimage");
    s->disabled = s->reset = false;
    return true;
}
bool SotnPlayerAfterimage_Force(SotnPlayerAfterimage *s)
{
    if (!s)
        return SDL_InvalidParamError("afterimage");
    s->index = 0;
    return true;
}
bool SotnPlayerAfterimage_Capture(SotnPlayerAfterimage *s, SotnDataView dra, SotnAfterimageInput in)
{
    if (!s)
        return SDL_InvalidParamError("afterimage");
    if (s->disabled)
        return true;
    if (s->index > 10 || s->write_index >= 6)
        return SDL_SetError("Afterimage capture lost its bounded activity/write cursor");
    SotnPlayerAfterimage next = *s;
    if (in.animation == 0x5e || in.animation == 0x5d || in.animation == 0x60 ||
        in.animation == 0x61 || in.animation == 0x62)
    {
        next.index = 10;
        goto done;
    }
    if ((in.tapped & 0xf6ffU) || ((in.held ^ in.pressed) & in.held & 0xf6ffU) ||
        in.velocity_y > 32768)
        next.index = next.timer = 0;
    else
    {
        if (next.index >= 10)
            return true;
        if (!dra.bytes || dra.size < 0xcf18 + 20U)
            return SDL_SetError("Afterimage capture timing table exceeds its private image");
        if (!next.timer)
            next.timer = dra.bytes[0xcf18 + next.index];
        next.timer = (Uint8)(next.timer - 1U);
        if (!next.timer)
        {
            ++next.index;
            next.timer = dra.bytes[0xcf18 + next.index];
        }
    }
    if (next.pose)
        --next.pose;
    else
    {
        next.samples[next.write_index] = (SotnAfterimageSample){.x = High(in.x),
                                                                .y = High(in.y),
                                                                .sprite = in.sprite,
                                                                .facing = in.facing_left,
                                                                .palette = in.palette,
                                                                .red = 128,
                                                                .green = 128,
                                                                .blue = 128};
        next.pose = 2;
        next.write_index = (Uint16)((next.write_index + 1U) % 6U);
    }
done:
    *s = next;
    return true;
}
bool SotnPlayerAfterimage_Present(SotnPlayerAfterimage *s, SotnDataView dra, Uint32 timer,
                                  Uint16 priority)
{
    if (!s || s->index > 10 || !dra.bytes || dra.size < 0xcf4c || priority < 2 || priority > 511)
        return SDL_SetError("Afterimage fade requires its private tables and player priority");
    SotnPlayerAfterimage next = *s;
    const Uint8 reduction = dra.bytes[0xcf2c + next.index],
                threshold = dra.bytes[0xcf3c + next.index];
    for (Uint16 i = 0; i < 6; ++i)
    {
        SotnAfterimageSample *p = &next.samples[i];
        if (p->red > threshold)
            p->red = (Uint8)(p->red - reduction);
        if (p->red < 112 && p->blue < 240)
            p->blue = (Uint8)(p->blue + 6U);
        p->blend = p->red < 88 ? 16 : 0;
        if (p->red <= threshold)
            p->sprite = 0;
        if (!((i ^ timer) & 1U))
            continue;
        SotnAfterimageDisplay *display = &next.displays[i / 2U];
        display->x = SetHigh(display->x, p->x);
        display->y = SetHigh(display->y, p->y);
        display->sprite = p->sprite;
        display->blend = (Uint8)p->blend;
        display->facing_left = p->facing != 0;
        display->palette = p->palette;
        display->priority = (Uint16)(priority - 2U);
        if (next.reset)
            display->sprite = p->sprite = 0;
        display->red = display->green = p->red;
        display->blue = p->blue;
        display->color_enabled = true;
    }
    *s = next;
    return true;
}
bool SotnPlayerAfterimage_Rebase(SotnPlayerAfterimage *s, Sint32 x, Sint32 y)
{
    if (!s || x < SDL_MIN_SINT16 || x > SDL_MAX_SINT16 || y < SDL_MIN_SINT16 || y > SDL_MAX_SINT16)
        return SDL_SetError("Afterimage rebase requires signed-word pixel shifts");
    for (Uint16 i = 0; i < 6; ++i)
    {
        s->samples[i].x = Word(s->samples[i].x + x);
        s->samples[i].y = Word(s->samples[i].y + y);
    }
    for (Uint16 i = 0; i < 3; ++i)
    {
        s->displays[i].x = SetHigh(s->displays[i].x, Word(High(s->displays[i].x) + x));
        s->displays[i].y = SetHigh(s->displays[i].y, Word(High(s->displays[i].y) + y));
    }
    return true;
}
