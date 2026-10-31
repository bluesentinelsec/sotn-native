// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/7879C.c:EntityNumberMovesToHpMeter, selected type1 healing path, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. SDK tables and digit artwork remain private.
#include <SDL3/SDL_error.h>
#include <sotn/healing_number.h>
static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
static Sint16 Pixel(Sint32 value)
{
    return Word(value >= 0 ? value / 65536 : (Sint32)(-((-(Sint64)value + 65535) / 65536)));
}
static Sint32 Position(Sint32 previous, Sint32 pixel)
{
    const Uint32 bits = ((Uint32)(Uint16)(Uint32)pixel << 16) | ((Uint32)previous & 65535U);
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
static Sint32 Shift(Sint32 value, Uint16 bits)
{
    return value >= 0 ? value >> bits : (Sint32)(-((-(Sint64)value + ((1U << bits) - 1U)) >> bits));
}
bool SotnHealingNumber_Tick(SotnHealingNumber *number, SotnDataView exe,
                            SotnPlayerFactorySource source, bool allocated)
{
    if (!number || number->removed || !number->number || number->number > 9999 ||
        number->phase > 11 || number->extra_digits > 3 ||
        (number->phase && (!allocated || number->half_width < 0 || number->half_width > 8 ||
                           number->half_height < 1 || number->half_height > 10 ||
                           number->timer < -11 || number->timer > 24)))
        return SDL_SetError("Healing number requires its actual live child and twenty-row lease");
    SotnHealingNumber n = *number;
    Sint16 trail = -1;
    switch (n.phase)
    {
    case 0: {
        if (!allocated)
        {
            n.removed = true;
            *number = n;
            return true;
        }
        n.x = source.x;
        n.y = source.y;
        n.half_width = 0;
        n.half_height = 2;
        n.timer = 8;
        Uint16 remaining = n.number;
        n.extra_digits = remaining >= 1000 ? 3 : remaining >= 100 ? 2 : remaining >= 10 ? 1 : 0;
        for (Uint16 i = 0; i < 4; ++i)
        {
            const Uint16 digit = remaining % 10U;
            remaining /= 10;
            const Uint8 u = (Uint8)(32U + (digit ? digit - 1U : 9U) * 8U);
            n.digits[i] = (SotnPlayerEffectQuad){.u = {u, (Uint8)(u + 8), u, (Uint8)(u + 8)},
                                                 .v = {64, 64, 73, 73},
                                                 .page = 0x1a,
                                                 .palette = 0x1b8,
                                                 .priority = 0x1b8,
                                                 .draw_mode = n.extra_digits < i ? 8 : 0};
        }
        for (Uint16 i = 0; i < 16; ++i)
            n.tiles[i].hidden = true;
        ++n.phase;
        break;
    }
    case 1:
    case 3:
        ++n.half_width;
        ++n.half_height;
        n.y = Position(n.y, Pixel(n.y) - 1);
        if (--n.timer == 0)
        {
            n.timer = n.phase == 1 ? 8 : 3;
            ++n.phase;
        }
        break;
    case 2:
    case 4:
        --n.half_width;
        --n.half_height;
        if (n.half_width == 4)
        {
            n.timer = n.phase == 2 ? 4 : 24;
            ++n.phase;
        }
        break;
    case 5:
        if (--n.timer == 0)
        {
            const Sint16 dx = Word(Pixel(n.x) - 13), dy = Word(Pixel(n.y) - 70);
            Sint16 angle = 0;
            Uint32 root = 0;
            const Uint32 squared = (Uint32)((Sint64)dx * dx + (Sint64)dy * dy);
            if (!SotnMath_Atan2Vector(exe, -(Sint32)dy, dx, &angle) ||
                !SotnMath_Sqrt0(exe, squared, &root))
                return false;
            n.angle = (Sint16)((Uint16)angle & 4095U);
            n.distance = Word((Sint32)root);
            n.timer = 16;
            n.swirl = 0;
            ++n.phase;
        }
        break;
    case 6: {
        if (n.half_width != 2)
        {
            --n.half_width;
            --n.half_height;
        }
        trail = --n.timer;
        const Sint16 radius = Word(n.distance * n.timer / 16);
        SotnMathTrig direction = {0}, swirl = {0};
        n.swirl = Word(n.swirl + 256);
        if (!SotnMath_Trig(exe, n.angle, &direction) || !SotnMath_Trig(exe, n.swirl, &swirl))
            return false;
        n.x = Position(n.x,
                       13 + Shift(Shift(direction.cosine, 4) * radius, 8) + Shift(swirl.cosine, 9));
        n.y =
            Position(n.y, 70 - Shift(Shift(direction.sine, 4) * radius, 8) - Shift(swirl.sine, 9));
        if (!n.timer)
            ++n.phase;
        break;
    }
    case 7:
    case 9:
        ++n.half_width;
        ++n.half_height;
        if (n.half_width == 7)
        {
            ++n.phase;
            n.timer = 24;
        }
        break;
    case 8:
        if (n.half_width != 4)
        {
            --n.half_width;
            --n.half_height;
        }
        else
            ++n.phase;
        break;
    case 10:
        if (n.half_width != 4)
        {
            --n.half_width;
            --n.half_height;
        }
        if (--n.timer == 0)
            ++n.phase;
        break;
    case 11:
        n.lower_x = Word(n.lower_x + 2);
        n.x = Position(n.x, Pixel(n.x) - 2);
        n.timer = Word(n.timer - 1);
        if (!((Uint16)n.timer & 2U))
            --n.half_height;
        if (!n.half_height)
        {
            n.removed = true;
            *number = n;
            return true;
        }
        break;
    }
    if (n.phase != 11)
        n.lower_x = Pixel(n.x);
    Sint16 upper = Word(Pixel(n.x) + n.half_width * n.extra_digits);
    Sint16 lower = Word(n.lower_x + n.half_width * n.extra_digits);
    const Sint16 y = Word(Pixel(n.y) - 16);
    for (Uint16 i = 0; i < 4; ++i)
    {
        SotnPlayerEffectQuad *q = &n.digits[i];
        q->x[0] = Word(upper - n.half_width);
        q->x[1] = Word(upper + n.half_width);
        q->x[2] = Word(lower - n.half_width);
        q->x[3] = Word(lower + n.half_width);
        q->y[0] = q->y[1] = Word(y - n.half_height);
        q->y[2] = q->y[3] = Word(y + n.half_height);
        upper = Word(upper - (n.half_width * 2 - 3));
        lower = Word(lower - (n.half_width * 2 - 3));
    }
    for (Uint16 i = 0; i < 16; ++i)
        if (n.tiles[i].started)
        {
            if (--n.tiles[i].timer == 0)
                n.tiles[i].hidden = true;
            n.tiles[i].y = Word(n.tiles[i].y + 1);
        }
        else if (trail == (Sint16)i)
        {
            n.tiles[i].hidden = false;
            n.tiles[i].x = Pixel(n.x);
            n.tiles[i].y = Pixel(n.y);
            n.tiles[i].started = true;
            n.tiles[i].timer = 12;
        }
    *number = n;
    return true;
}
bool SotnHealingNumber_Rebase(SotnHealingNumber *n, Sint16 x, Sint16 y)
{
    if (!n || n->removed || n->phase > 11)
        return SDL_SetError("Healing number rebase requires its actual live child");
    if (!n->phase)
        return true;
    n->x = Position(n->x, Pixel(n->x) + x);
    n->y = Position(n->y, Pixel(n->y) + y);
    n->lower_x = Word(n->lower_x + x);
    for (Uint16 i = 0; i < 4; ++i)
        for (Uint16 v = 0; v < 4; ++v)
        {
            n->digits[i].x[v] = Word(n->digits[i].x[v] + x);
            n->digits[i].y[v] = Word(n->digits[i].y[v] + y);
        }
    for (Uint16 i = 0; i < 16; ++i)
        if (n->tiles[i].started)
        {
            n->tiles[i].x = Word(n->tiles[i].x + x);
            n->tiles[i].y = Word(n->tiles[i].y + y);
        }
    return true;
}
