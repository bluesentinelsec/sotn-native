// SPDX-License-Identifier: AGPL-3.0-or-later
// weapon/w_022.c and dra/alu_anim.c:UpdateAnim, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Timelines/artwork remain private inputs.
#include <sotn/magic_missile.h>
static Sint16 MagicWord(Sint32 v)
{
    const Uint16 b = (Uint16)(Uint32)v;
    return b < 32768 ? (Sint16)b : (Sint16)((Sint32)b - 65536);
}
static Sint32 MagicFixed(Uint32 b)
{
    return b <= SDL_MAX_SINT32 ? (Sint32)b : (Sint32)((Sint64)b - 4294967296LL);
}
static Sint32 MagicShift(Sint32 v, Sint16 pixels)
{
    const Uint16 high = (Uint16)((Uint32)v >> 16);
    return MagicFixed((Uint32)(Uint16)MagicWord(high + pixels) << 16 | ((Uint32)v & 65535U));
}
bool SotnMagicMissile_Tick(SotnMagicMissile *state, SotnMagicMissileInput in,
                           SotnMagicMissileEvents *out)
{
    if (!state || !out || state->removed || state->phase > 2 || in.hand > 1 || in.priority > 509 ||
        in.remaining < -1 || in.hit_flags > 3 ||
        (!in.arrow && state->phase == 2 && (state->lifetime < 1 || state->lifetime > 16)))
        return SDL_SetError("Magic Missile requires its actual role, callback and player priority");
    SotnMagicMissile s = *state;
    SotnMagicMissileEvents e = {0};
    if (!in.arrow)
    {
        switch (s.phase)
        {
        case 0:
            s.bank = (Uint16)(0x8010U + in.hand * 2U);
            s.palette = (Uint16)(0x110 + in.hand * 24U);
            s.texture = (Uint16)(0x64 + in.hand * 2U);
            s.priority = (Uint16)(in.priority + 2U);
            s.facing_left = in.facing_left;
            s.flags = 0x00140000;
            s.x = MagicShift(s.x, s.facing_left ? 32 : -32);
            s.y = MagicShift(s.y, -6);
            e.animation = 1;
            e.sound = 0x669;
            e.clear_shield = true;
            e.use_ticks = 4;
            s.phase = 1;
            break;
        case 1:
            if (in.remaining < 0)
            {
                s.lifetime = 16;
                s.draw_flags |= 0x80;
                s.phase = 2;
            }
            e.factory = in.pose == 20 && in.remaining == 2;
            break;
        case 2:
            s.removed = --s.lifetime == 0;
            break;
        }
    }
    else
    {
        switch (s.phase)
        {
        case 0:
            if (!in.parent.exists)
            {
                s.removed = true;
                break;
            }
            s.facing_left = in.parent.facing_left;
            s.bank = in.parent.bank;
            s.texture = in.parent.texture;
            s.palette = in.parent.palette;
            s.item = in.parent.item;
            s.priority = (Uint16)(in.priority + 2U);
            s.flags = 0x08100000;
            s.velocity_x = s.facing_left ? -5 * 65536 : 5 * 65536;
            e.animation = 2;
            e.properties = true;
            s.phase = 1;
            break;
        case 1:
            if (in.hit_flags == 1)
            {
                e.animation = 3;
                s.phase = 2;
            }
            else
                s.x = MagicFixed((Uint32)s.x + (Uint32)s.velocity_x);
            break;
        case 2:
            s.removed = in.remaining < 0;
            break;
        }
    }
    *state = s;
    *out = e;
    return true;
}
static Uint16 MagicRead16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
bool SotnMagicMissile_Animate(SotnMagicMissileAnimation *animation, SotnDataView overlay,
                              Uint8 timeline)
{
    if (timeline < 1 || timeline > 3)
        return SDL_SetError("Magic Missile requires its actual bow, flight or impact timeline");
    const Uint32 offset = timeline == 1 ? 0x77c : timeline == 2 ? 0x800 : 0x760;
    const Uint16 count = timeline == 1 ? 33 : timeline == 2 ? 2 : 7;
    if (!animation || !overlay.bytes || overlay.size < offset + count * 4U ||
        animation->pose >= count || animation->remaining < -1)
        return SDL_SetError("Magic Missile animation requires its bounded private timeline");
    SotnMagicMissileAnimation next = *animation;
    if (next.remaining != -1)
    {
        if (next.remaining == 0)
        {
            const Uint16 duration = MagicRead16(overlay.bytes + offset + next.pose * 4U);
            if (!duration || duration > 32767)
                return SDL_SetError("Magic Missile animation has an invalid birth duration");
            next.remaining = (Sint16)duration;
        }
        else if (--next.remaining == 0)
        {
            if (++next.pose >= count)
                return SDL_SetError("Magic Missile animation escaped its private timeline");
            Uint16 duration = MagicRead16(overlay.bytes + offset + next.pose * 4U);
            if (!duration)
            {
                next.pose = MagicRead16(overlay.bytes + offset + next.pose * 4U + 2U);
                if (next.pose >= count)
                    return SDL_SetError("Magic Missile animation has an invalid loop target");
                duration = MagicRead16(overlay.bytes + offset + next.pose * 4U);
            }
            else if (duration == 0xffffU)
            {
                --next.pose;
                next.remaining = -1;
            }
            if (next.remaining != -1)
            {
                if (!duration || duration > 32767)
                    return SDL_SetError("Magic Missile animation has an unsupported control");
                next.remaining = (Sint16)duration;
            }
        }
    }
    next.frame = MagicRead16(overlay.bytes + offset + next.pose * 4U + 2U) & 511U;
    *animation = next;
    return true;
}
