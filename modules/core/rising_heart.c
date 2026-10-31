// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/7879C.c:EntitySmallRisingHeart, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. SDK tables and sprite artwork stay private.
#include <SDL3/SDL_error.h>
#include <sotn/game_math.h>
#include <sotn/rising_heart.h>
static Sint32 Fixed(Uint32 bits)
{
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
bool SotnRisingHeart_Tick(SotnRisingHeart *heart, SotnDataView executable,
                          SotnPlayerFactorySource source, Uint16 priority, Uint32 clock)
{
    if (!heart || heart->removed || priority < 2 || priority > 511 ||
        (heart->initialized && (!heart->timer || heart->timer > 128 || heart->priority > 509)))
        return SDL_SetError("Rising heart requires its actual live child and original priority");
    SotnRisingHeart next = *heart;
    if (!next.initialized)
    {
        next = (SotnRisingHeart){.x = source.x,
                                 .y = Fixed((Uint32)source.y - 16U * 65536U),
                                 .speed = 64,
                                 .timer = 128,
                                 .priority = (Uint16)(priority - 2),
                                 .initialized = true};
    }
    else
    {
        SotnMathTrig trig = {0};
        if (!SotnMath_Trig(executable, next.angle, &trig))
            return false;
        if (next.timer < 32)
            next.blink = true;
        next.y = Fixed((Uint32)next.y - 32768U);
        next.x = Fixed((Uint32)next.x + (Uint32)((Sint32)trig.cosine * 8));
        next.angle = Word((Sint32)next.angle + next.speed);
        if (!(clock & 3U))
            next.speed = Word((Sint32)next.speed - 1);
        next.removed = --next.timer == 0;
    }
    *heart = next;
    return true;
}
bool SotnRisingHeart_Rebase(SotnRisingHeart *heart, Sint16 x, Sint16 y)
{
    if (!heart || heart->removed)
        return SDL_SetError("Rising heart translation requires its actual live child");
    if (heart->initialized)
    {
        heart->x = Fixed((Uint32)heart->x + (Uint32)(Sint32)x * 65536U);
        heart->y = Fixed((Uint32)heart->y + (Uint32)(Sint32)y * 65536U);
    }
    return true;
}
