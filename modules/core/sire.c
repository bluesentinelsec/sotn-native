// SPDX-License-Identifier: AGPL-3.0-or-later
// weapon/w_018.c and dra/alu_anim.c:UpdateAnim, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. No private artwork or timeline arrays.
#include <sotn/combat.h>
#include <sotn/sire.h>
static Sint16 SireWord(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return bits < 32768 ? (Sint16)bits : (Sint16)((Sint32)bits - 65536);
}
static Sint32 SireFixed(Uint32 bits)
{
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
static Sint32 SireHigh(Sint32 old, Sint32 source, Sint16 delta)
{
    const Uint16 high = (Uint16)(((Uint32)source >> 16) + (Uint32)(Sint32)delta);
    return SireFixed((Uint32)high << 16 | ((Uint32)old & 65535U));
}
bool SotnSireParent_Tick(SotnSireParent *parent, SotnSireParentInput in, SotnSireParentEvents *out)
{
    if (!parent || !out || parent->removed || parent->phase > 3 || in.hand > 1 || in.priority < 4 ||
        in.priority > 511)
        return SDL_SetError("Sire parent callback requires its actual controller context");
    SotnSireParent s = *parent;
    SotnSireParentEvents e = {.properties = -1, .clear_attack = true};
    switch (s.phase)
    {
    case 0:
        s.x = SireHigh(s.x, in.player_x, 0);
        s.y = SireHigh(s.y, in.player_y, -56);
        s.bank = (Uint16)(16U + in.hand * 2U);
        s.palette = (Uint16)(0x110U + in.hand * 24U);
        s.texture = (Uint16)(0x64U + in.hand * 2U);
        s.facing_left = false;
        s.flags = 0x04100000;
        s.priority = (Uint16)(in.priority - 4U);
        s.animation = 1;
        e.properties = 1;
        e.sound = 0x66a;
        e.use_ticks = 4;
        ++s.phase;
        break;
    case 1:
        if (in.remaining < 0)
        {
            s.blend = 0x30;
            s.animation = 2;
            e.reset_pose = true;
            s.lifetime = 71;
            e.flash = 5;
            e.shake = 3;
            e.sound = 0x635;
            ++s.phase;
        }
        break;
    case 2:
        if (s.lifetime % 8 == 0 && s.lifetime >= 9)
        {
            e.burst_factory = true;
            if (in.factory_allocated)
            {
                e.flash = 6;
                e.properties = 0;
            }
        }
        s.lifetime = SireWord(s.lifetime - 1);
        if (!s.lifetime)
        {
            s.draw_flags = 3;
            s.scale_x = s.scale_y = 256;
            s.lifetime = 14;
            ++s.phase;
        }
        break;
    case 3:
        s.scale_x = SireWord(s.scale_x - 32);
        if (s.scale_y < 1024)
            s.scale_y = SireWord(s.scale_y + 192);
        if (s.scale_x == 64)
        {
            s.pivot_x = 1;
            e.sound = 0x636;
            e.particle_factory = true;
        }
        if (s.scale_x == 32)
            s.palette = 0x815f;
        if (s.scale_x <= 0)
            s.scale_x = 6;
        s.lifetime = SireWord(s.lifetime - 1);
        if (!s.lifetime)
            s.removed = true;
        break;
    }
    *parent = s;
    *out = e;
    return true;
}
bool SotnSireChild_Tick(SotnSireChild *child, SotnSireChildInput in, Uint32 *random,
                        SotnSireChildEvents *out)
{
    if (!child || !out || !random || child->removed || child->phase > 2 || in.hand > 1 ||
        in.priority < 2 || in.priority > 511)
        return SDL_SetError("Sire child callback requires its actual retained parent context");
    SotnSireChild s = *child;
    SotnSireChildEvents e = {0};
    Uint32 rng = *random;
    if (in.parent_item != 72)
    {
        s.removed = true;
        goto done;
    }
    switch (s.phase)
    {
    case 0: {
        s.bank = (Uint16)(16U + in.hand * 2U);
        s.palette = in.parent_palette >= 0x128U ? 0x128 : 0x110;
        s.texture = in.parent_texture;
        s.facing_left = false;
        s.flags = 0x08100000;
        s.priority = (Uint16)(in.priority - 2U);
        s.blend = 0x30;
        s.draw_flags = 3;
        s.animation = 3;
        s.scale_x = s.scale_y = 256;
        SotnMathTrig trig = {0};
        const Uint16 angle = SotnCombat_Random(&rng);
        if (!SotnMath_Trig(in.executable, angle, &trig))
            return false;
        s.velocity_x = (Sint32)trig.cosine * 128;
        s.velocity_y = -(Sint32)trig.sine * 128;
        const Sint32 scale = SotnCombat_Random(&rng) & 7U;
        s.x = SireFixed((Uint32)s.x + (Uint32)(scale * s.velocity_x));
        s.y = SireFixed((Uint32)s.y + (Uint32)((scale + 2) * s.velocity_y));
        e.sound = 0x69d;
        s.lifetime = 6;
        ++s.phase;
        break;
    }
    case 1:
        s.lifetime = SireWord(s.lifetime - 1);
        if (!s.lifetime)
            ++s.phase;
        break;
    case 2:
        if (s.scale_x < 512)
            s.scale_x = SireWord(s.scale_x + 16);
        s.scale_y = s.scale_x;
        s.x = SireFixed((Uint32)s.x + (Uint32)s.velocity_x);
        s.y = SireFixed((Uint32)s.y + (Uint32)s.velocity_y);
        break;
    }
done:
    *child = s;
    *random = rng;
    *out = e;
    return true;
}

static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
bool SotnSire_Animate(SotnSireAnimation *animation, SotnDataView overlay, Uint8 timeline)
{
    if (timeline < 1 || timeline > 3)
        return SDL_SetError("Sire animation requires its actual bat/portrait/burst timeline");
    const Uint32 offset = timeline == 1 ? 0x6a8 : timeline == 2 ? 0x6e4 : 0x724;
    const Uint16 count = timeline == 1 ? 15 : timeline == 2 ? 16 : 3;
    if (!animation || !overlay.bytes || overlay.size < offset + count * 4U ||
        animation->pose >= count || animation->remaining < -1)
        return SDL_SetError("Sire animation requires its actual bounded private timeline");
    SotnSireAnimation next = *animation;
    if (next.remaining != -1)
    {
        if (next.remaining == 0)
        {
            const Uint16 duration = Read16(overlay.bytes + offset + next.pose * 4U);
            if (!duration || duration > 32767)
                return SDL_SetError("Sire animation has an invalid birth duration");
            next.remaining = (Sint16)duration;
        }
        else if (--next.remaining == 0)
        {
            if (++next.pose >= count)
                return SDL_SetError("Sire animation escaped its private timeline");
            Uint16 duration = Read16(overlay.bytes + offset + next.pose * 4U);
            if (!duration)
            {
                next.pose = Read16(overlay.bytes + offset + next.pose * 4U + 2U);
                if (next.pose >= count)
                    return SDL_SetError("Sire animation has an invalid loop target");
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
                    return SDL_SetError("Sire animation has an unsupported control");
                next.remaining = (Sint16)duration;
            }
        }
    }
    const Uint16 frame = Read16(overlay.bytes + offset + next.pose * 4U + 2U);
    next.frame = frame & 511U;
    *animation = next;
    return true;
}
