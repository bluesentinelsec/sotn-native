// SPDX-License-Identifier: AGPL-3.0-or-later
// weapon/w_019.c:EntityWeaponAttack and dra/alu_anim.c:UpdateAnim,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
// Original images/palettes/animation tables remain private runtime inputs.
#include <SDL3/SDL_error.h>
#include <sotn/neutron.h>
static Sint16 NeutronWord(Sint32 v)
{
    const Uint16 n = (Uint16)(Uint32)v;
    return n < 32768 ? (Sint16)n : (Sint16)((Sint32)n - 65536);
}
static Sint16 NeutronPixel(Sint32 v)
{
    return NeutronWord((Sint32)((Uint32)v >> 16));
}
static Sint32 NeutronHigh(Sint32 previous, Sint32 source)
{
    const Uint32 n = ((Uint32)previous & 65535U) | ((Uint32)source & 0xffff0000U);
    return n <= SDL_MAX_SINT32 ? (Sint32)n : (Sint32)((Sint64)n - 4294967296LL);
}
bool SotnNeutron_Tick(SotnNeutron *state, SotnNeutronInput in, SotnNeutronEvents *out)
{
    if (!state || !out || state->removed || state->phase > 3 || in.priority > 507)
        return SDL_SetError("Neutron callback requires its actual callback cursor/priority");
    SotnNeutron s = *state;
    SotnNeutronEvents e = {0};
    switch (s.phase)
    {
    case 0:
        s.x = NeutronHigh(s.x, in.player_x);
        s.y = NeutronHigh(s.y, in.player_y);
        if (!in.allocated)
        {
            s.removed = true;
            goto done;
        }
        s.halo = (SotnPlayerEffectQuad){.u = {0x40, 0x7f, 0x40, 0x7f},
                                        .v = {0xc0, 0xc0, 0xff, 0xff},
                                        .page = 0x1a,
                                        .palette = 0x1b0,
                                        .priority = (Uint16)(in.priority + 4),
                                        .draw_mode = 0x35};
        s.extent = 127;
        s.facing_left = false;
        s.flags = 0x04840000;
        s.priority = (Uint16)(in.priority + 4);
        s.draw_flags = 7;
        s.scale_x = s.scale_y = 256;
        e.sounds[0] = 0x62f;
        e.sounds[1] = 0x654;
        e.use_ticks = 4;
        s.phase = 1;
        break;
    case 1:
        s.x = NeutronHigh(s.x, in.player_x);
        s.y = NeutronHigh(s.y, in.player_y);
        if (s.extent > 8)
            s.extent = NeutronWord(s.extent - 16);
        else
        {
            s.halo.draw_mode = 8;
            s.extent = 0;
            s.flags |= 0x100000;
            e.begin_animation = true;
            e.properties = true;
            s.phase = 2;
        }
        break;
    case 2:
        e.disable_attack = true;
        s.x = NeutronHigh(s.x, in.player_x);
        s.y = NeutronHigh(s.y, in.player_y);
        if (in.remaining == 1 && in.pose == 3)
            s.blend = 0x30;
        if (in.pose == 5)
        {
            s.opacity = 128;
            s.flags &= ~0x40000U;
            s.draw_flags |= 8;
            s.phase = 3;
        }
        break;
    case 3:
        s.scale_x = NeutronWord(s.scale_x + 16);
        s.scale_y = s.scale_x;
        s.rotation = NeutronWord(s.rotation + 64);
        if (s.opacity >= 5)
            s.opacity = (Uint8)(s.opacity + 254U);
        if (in.remaining < 0)
        {
            s.removed = true;
            goto done;
        }
        break;
    }
    const Uint16 extent = (Uint16)s.extent;
    const Sint16 x = NeutronPixel(s.x), y = NeutronPixel(s.y);
    s.halo.x[0] = s.halo.x[2] = NeutronWord(x - extent);
    s.halo.x[1] = s.halo.x[3] = NeutronWord(x + extent);
    s.halo.y[0] = s.halo.y[1] = NeutronWord(y - extent);
    s.halo.y[2] = s.halo.y[3] = NeutronWord(y + extent);
    const Uint8 gray = SDL_min(128, (Uint8)(s.halo.blue[3] + 16U));
    for (Uint16 i = 0; i < 4; ++i)
        s.halo.red[i] = s.halo.green[i] = s.halo.blue[i] = gray;
done:
    *state = s;
    *out = e;
    return true;
}

static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
bool SotnNeutron_Animate(SotnNeutronAnimation *animation, SotnDataView overlay)
{
    const Uint32 offset = 0x52c;
    const Uint16 count = 7;
    if (!animation || !overlay.bytes || overlay.size < offset + count * 4U ||
        animation->pose >= count || animation->remaining < -1)
        return SDL_SetError("Neutron animation requires its actual bounded private timeline");
    SotnNeutronAnimation next = *animation;
    if (next.remaining != -1)
    {
        if (next.remaining == 0)
        {
            const Uint16 duration = Read16(overlay.bytes + offset + next.pose * 4U);
            if (!duration || duration > 32767)
                return SDL_SetError("Neutron animation has an invalid birth duration");
            next.remaining = (Sint16)duration;
        }
        else if (--next.remaining == 0)
        {
            if (++next.pose >= count)
                return SDL_SetError("Neutron animation escaped its private timeline");
            Uint16 duration = Read16(overlay.bytes + offset + next.pose * 4U);
            if (!duration)
            {
                next.pose = Read16(overlay.bytes + offset + next.pose * 4U + 2U);
                if (next.pose >= count)
                    return SDL_SetError("Neutron animation has an invalid loop target");
                duration = Read16(overlay.bytes + offset + next.pose * 4U);
            }
            else if (duration == 0xffffU)
            {
                --next.pose;
                next.remaining = -1;
            }
            if (next.remaining != -1)
            {
                if (!duration || duration > 32767)
                    return SDL_SetError("Neutron animation has an unsupported control");
                next.remaining = (Sint16)duration;
            }
        }
    }
    const Uint16 frame = Read16(overlay.bytes + offset + next.pose * 4U + 2U);
    next.frame = frame & 511U;
    *animation = next;
    return true;
}
