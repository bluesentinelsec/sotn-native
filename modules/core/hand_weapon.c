// SPDX-License-Identifier: AGPL-3.0-or-later
// Ordinary weapon/w_000.c, w_002.c, w_009.c, w_011.c, w_026.c, w_031.c,
// w_037.c, w_043.c, w_052.c and
// dra/alu_anim.c, sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
// All original animation, hitbox, palette and artwork tables remain private inputs.
#include <sotn/combat.h>
#include <sotn/hand_weapon.h>
static Sint16 Word(Sint32 n)
{
    Uint16 v = (Uint16)(Uint32)n;
    return v < 32768 ? (Sint16)v : (Sint16)((Sint32)v - 65536);
}
static Sint32 Fixed(Uint32 v)
{
    return v <= SDL_MAX_SINT32 ? (Sint32)v : (Sint32)((Sint64)v - 4294967296LL);
}
static Sint32 Add(Sint32 a, Sint32 b)
{
    return Fixed((Uint32)a + (Uint32)b);
}
static Sint16 Pixel(Sint32 n)
{
    return Word((Sint32)((Uint32)n >> 16));
}
static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
static Uint32 Read32(const Uint8 *p)
{
    return Read16(p) | (Uint32)Read16(p + 2) << 16;
}
static bool View(SotnDataView o, Uint64 at, size_t count, const Uint8 **out)
{
    if (!o.bytes || at > o.size || count > o.size - at)
        return SDL_SetError("Ordinary weapon private table bounds");
    *out = o.bytes + (size_t)at;
    return true;
}
static bool Address(SotnDataView o, Uint32 base, Uint32 at, size_t count, const Uint8 **out)
{
    if (at < base || (at & 1U))
        return SDL_SetError("Ordinary weapon private pointer");
    return View(o, at - base, count, out);
}
static void Transform(SotnHandWeapon *w, SotnHandWeaponInput in)
{
    w->draw_flags = in.draw_flags;
    w->scale_y = in.scale_y;
    w->pivot_y = in.pivot_y;
}
static bool Properties(SotnHandWeapon *w, SotnHandWeaponInput in, Uint32 *random,
                       SotnHandWeaponEvents *e)
{
    if (!in.properties || !in.properties(in.context, random, &w->properties))
        return SDL_SetError("Ordinary weapon requires actual equipment property ownership");
    w->hitbox_state = w->properties.hit_type;
    if (w->hitbox_state == 4 && in.family != 255)
        w->properties.attack = 255;
    e->properties = true;
    return true;
}
static bool Hitbox(SotnHandWeapon *w, SotnDataView o, Uint32 base, Uint32 props, Uint16 packed)
{
    const Uint8 *p = NULL;
    if (!Address(o, base, props + ((packed >> 9) & 127U) * 4U, 4, &p))
        return false;
    w->hitbox_x = (Sint16)(p[0] < 128 ? p[0] : (Sint32)p[0] - 256);
    w->hitbox_y = (Sint16)(p[1] < 128 ? p[1] : (Sint32)p[1] - 256);
    w->hitbox_width = p[2];
    w->hitbox_height = p[3];
    w->frame = packed & 511U;
    return true;
}
bool SotnHandWeapon_IsShieldFamily(Uint16 family)
{
    return family == 9 || family == 11 || family == 26 || family == 52;
}
static Uint16 ShieldAnimationTable(Uint16 family)
{
    return family == 9 ? 0xa40 : family == 11 ? 0xb94 : family == 26 ? 0xa60 : 0xb84;
}
static bool ShieldFrame(SotnHandWeapon *w, SotnDataView o, Uint16 table, Uint16 *time,
                        Uint16 *packed)
{
    const Uint8 *p = NULL;
    if (w->animation >= 14 || w->pose >= 64 || !View(o, table + w->animation * 4U, 4, &p) ||
        !Address(o, 0x8017a000U, Read32(p) + w->pose * 4U, 4, &p))
        return SDL_SetError("Ordinary shield private animation cursor");
    *time = Read16(p);
    *packed = Read16(p + 2);
    return true;
}
static bool ShieldAnimation(SotnHandWeapon *w, SotnDataView o, Uint16 family)
{
    Uint16 time = 0, packed = 0;
    const Uint16 table = ShieldAnimationTable(family);
    if (!ShieldFrame(w, o, table, &time, &packed))
        return false;
    if (w->remaining != -1)
    {
        if (!w->remaining)
            w->remaining = Word(time);
        else if (!--w->remaining)
        {
            ++w->pose;
            if (!ShieldFrame(w, o, table, &time, &packed))
                return false;
            if (!time)
                w->pose = packed;
            else if (time == 65535U)
            {
                --w->pose;
                w->remaining = -1;
            }
            else if (time == 65534U)
            {
                w->animation = packed & 255U;
                w->pose = packed >> 8;
            }
            else
                w->remaining = Word(time);
            if (!ShieldFrame(w, o, table, &time, &packed))
                return false;
            if (!w->remaining)
                w->remaining = Word(time);
        }
    }
    return Hitbox(w, o, 0x8017a000U, 0x8017a000U + table - 0x48U, packed);
}
static Uint16 ShieldMovement(Uint16 animation, Uint16 crouch, bool *follow)
{
    *follow = true;
    switch (animation)
    {
    case 9:
    case 10:
    case 11:
        return 9;
    case 7:
        return 8;
    case 8:
        return 7;
    case 12:
        return 6;
    case 13:
        return 5;
    case 24:
    case 25:
        return 4;
    case 14:
    case 15:
        return 3;
    case 26:
        return 2;
    default:
        *follow = false;
        return crouch;
    }
}
static bool Shield(SotnHandWeapon *w, SotnHandWeaponProgram *program, SotnHandWeaponInput in,
                   Uint32 *random, SotnHandWeaponEvents *e)
{
    if (!in.held && w->phase < 3)
    {
        w->remaining = 0;
        w->pose = 0;
        w->phase = 3;
    }
    if (w->phase != 4)
    {
        w->x = in.x;
        w->y = in.y;
        w->facing_left = in.facing_left;
    }
    if ((in.status & 0x10000U) && w->phase != 4)
    {
        w->priority = (Uint16)(in.priority + 2U);
        w->phase = 4;
        w->velocity_x = in.contacts & 1U ? in.velocity_x : Fixed((Uint32)in.velocity_x * 2U);
        w->velocity_y = -229376;
        w->timer = 128;
        w->flags = 0x8000000U;
        w->frame = 62;
    }
    const Uint16 crouch = in.step == 2 && in.substep != in.step;
    switch (w->phase)
    {
    case 0:
        w->bank = (Uint16)(0x8010U + in.hand * 2U);
        w->palette = (Uint16)(0x110U + in.hand * 24U);
        w->texture = (Uint16)(100U + in.hand * 2U);
        w->flags = 0x60000;
        w->priority = (Uint16)(in.priority - 2U);
        e->shield = 1;
        if (!Properties(w, in, random, e))
            return false;
        if (in.family == 52)
        {
            program->shield_attack = w->properties.attack;
            w->properties.attack = 1;
        }
        ++w->phase;
        SDL_FALLTHROUGH;
    case 1:
        w->animation = (Uint16)(10U + crouch);
        if (w->remaining < 0)
            ++w->phase;
        break;
    case 2: {
        bool follow = false;
        w->animation = ShieldMovement(in.animation, crouch, &follow);
        w->pose = follow ? in.pose : 0;
        w->remaining = 2;
        break;
    }
    case 3:
        e->shield = 2;
        w->animation = (Uint16)(12U + crouch);
        if (w->remaining < 0)
        {
            w->removed = true;
            return true;
        }
        break;
    case 4:
        w->hitbox_state = 0;
        e->shield = 2;
        w->draw_flags |= 4U;
        w->y = Add(w->y, w->velocity_y);
        w->x = Add(w->x, w->velocity_x);
        w->velocity_y = Add(w->velocity_y, 10240);
        w->rotation = Word(w->rotation + 128);
        w->timer = Word(w->timer - 1);
        if (w->timer < 16)
            w->draw_flags |= 128U;
        w->timer = Word(w->timer - 1);
        if (!w->timer)
        {
            w->removed = true;
            return true;
        }
        break;
    }
    if (w->phase != 4 && !ShieldAnimation(w, in.overlay, in.family))
        return false;
    // The original assignment also overwrites the earlier phase-four rotate/blink flags.
    Transform(w, in);
    return true;
}
static void Tiles(SotnHandWeapon *w, SotnHandWeaponInput in, Uint32 *random)
{
    Sint16 modifier = 0;
    if (in.draw_flags & 2U)
        modifier = in.animation >= 0x43 && in.animation < 0x45 ? -1 : -3;
    for (Uint16 i = 0; i < SOTN_HAND_WEAPON_TILES; ++i)
    {
        SotnPlayerEffectQuad *q = &w->tiles[i];
        if (!w->tile_started[i])
        {
            if (in.remaining == 1 && in.pose == 3)
            {
                const Sint16 x = w->facing_left ? Word(-w->hitbox_x) : w->hitbox_x;
                q->x[0] = Word(Pixel(w->x) + x + (Sint32)(SotnCombat_Random(random) % 48U) - 24);
                q->y[0] = Word(Pixel(w->y) + w->hitbox_y + modifier +
                               (Sint32)(SotnCombat_Random(random) & 7U));
                w->tile_started[i] = true;
                w->tile_age[i] = 9;
                w->tile_delay[i] = (Uint8)(SotnCombat_Random(random) & 3U);
            }
        }
        else if (w->tile_delay[i])
            --w->tile_delay[i];
        else if (w->tile_age[i])
        {
            if (--w->tile_age[i] < 3)
                q->u[0] = q->v[0] = 1;
            q->draw_mode &= 0xfff7U;
            if (w->tile_age[i] & 1U)
                q->y[0] = Word(q->y[0] - 1);
            q->x[0] = Word(q->x[0] + (w->facing_left ? 2 : -2));
        }
        else
            q->draw_mode |= 8U;
    }
}
bool SotnHandWeapon_Tick(SotnHandWeapon *out, SotnHandWeaponProgram *program,
                         SotnHandWeaponInput in, Uint32 *random, SotnHandWeaponEvents *events)
{
    if (!out || !program || !random || !events || out->removed || in.hand > 1 ||
        out->phase > (SotnHandWeapon_IsShieldFamily(in.family) ? 4 : 1) ||
        (in.family != 0 && in.family != 2 && in.family != 31 && in.family != 37 &&
         in.family != 43 && !SotnHandWeapon_IsShieldFamily(in.family) && in.family != 255))
        return SDL_SetError("Ordinary weapon requires a supported live hand/program context");
    SotnHandWeapon w = *out;
    SotnHandWeaponProgram shared = *program;
    SotnHandWeaponEvents e = {0};
    Uint32 rng = *random;
    if (SotnHandWeapon_IsShieldFamily(in.family))
    {
        if (!Shield(&w, &shared, in, &rng, &e))
            return false;
    }
    else
    {
        const Uint16 variant = (w.parameter & 0x7fffU) >> 8;
        const Uint32 base = in.family == 255 ? 0x800a0000U : 0x8017a000U;
        const Uint16 count = in.family == 0    ? 5
                             : in.family == 43 ? 3
                             : in.family == 37 ? 4
                             : in.family == 31 ? 2
                                               : 1;
        const Uint32 descriptor = in.family == 255  ? 0xd53cU
                                  : in.family == 0  ? 0x106cU
                                  : in.family == 2  ? 0xbc4U
                                  : in.family == 31 ? 0x8ecU
                                  : in.family == 37 ? 0x6a4U
                                                    : 0x1014U;
        const Uint8 *d = NULL, *p = NULL;
        if (variant >= count || !View(in.overlay, descriptor + variant * 16U, 16, &d))
            return SDL_SetError("Ordinary weapon descriptor bounds");
        w.x = in.x;
        w.y = in.y;
        w.facing_left = in.facing_left;
        if (in.animation < d[12] || in.animation >= d[12] + 7 || !in.attack_mode)
            w.removed = true;
        else
        {
            if (!w.phase)
            {
                w.flags = 0x60000;
                if (in.family != 255)
                {
                    w.bank = (Uint16)(0x8010U + in.hand * 2U);
                    w.palette = (Uint16)(0x110U + in.hand * 24U + Read16(d + 8));
                    w.texture = (Uint16)(100U + in.hand * 2U);
                    w.priority = (Uint16)(in.priority + (in.family == 31 && !variant ? 2 : -2));
                }
                if (in.family == 0 || in.family == 43)
                    w.blend = 0x30;
                if (in.family == 43 && in.allocated)
                {
                    w.allocated = true;
                    w.flags |= 0x800000U;
                    for (Uint16 i = 0; i < SOTN_HAND_WEAPON_TILES; ++i)
                    {
                        SotnPlayerEffectQuad *q = &w.tiles[i];
                        q->u[0] = q->v[0] = 2;
                        q->red[0] = variant == 2 ? 0 : 32;
                        q->green[0] = variant == 2 ? 127 : 32;
                        q->blue[0] = variant == 2 ? 32 : 255;
                        q->priority = (Uint16)(in.priority + 2U);
                        q->draw_mode = 0x239;
                    }
                }
                if (!Properties(&w, in, &rng, &e))
                    return false;
                ++w.phase;
            }
            w.animation = (Uint16)(in.animation - d[12]);
            if (in.remaining == 1 && in.pose == d[13])
                e.sound = Read16(d + 10);
            if (in.pose >= 64 || !Address(in.overlay, base, Read32(d) + w.animation * 4U, 4, &p) ||
                !Address(in.overlay, base, Read32(p) + in.pose * 2U, 2, &p))
                return SDL_SetError("Ordinary weapon private player-frame bounds");
            const Uint16 packed = Read16(p);
            if (packed == 65535U)
                w.removed = true;
            else
            {
                if (!Hitbox(&w, in.overlay, base, Read32(d + 4), packed))
                    return false;
                if (in.remaining < 0)
                    w.removed = true;
                else
                {
                    if (in.family == 31 && w.animation < 2 && in.pose == 6)
                        w.hitbox_state = 4;
                    if (in.family == 37)
                    {
                        if (!shared.short_pose && in.pose == 1 && variant >= 1 && variant <= 2)
                            e.factory =
                                58U | ((in.hand + 1U) << 12) | ((Uint32)(variant - 1U) << 16);
                        shared.short_pose = in.pose;
                    }
                    if (in.family != 255)
                        Transform(&w, in);
                    if (in.family == 43 && w.allocated)
                        Tiles(&w, in, &rng);
                }
            }
        }
    }
    *out = w;
    *program = shared;
    *random = rng;
    *events = e;
    return true;
}
