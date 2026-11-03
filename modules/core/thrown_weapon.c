// SPDX-License-Identifier: AGPL-3.0-or-later
// weapon/w_016.c and dra/alu_anim.c:UpdateAnim, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Artwork/SDK tables remain private.
#include <sotn/combat.h>
#include <sotn/thrown_weapon.h>
static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return bits < 32768 ? (Sint16)bits : (Sint16)((Sint32)bits - 65536);
}
static Sint32 Fixed(Uint32 bits)
{
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
static Sint16 Pixel(Sint32 value)
{
    return Word((Sint32)((Uint32)value >> 16));
}
static Sint32 High(Sint32 old, Sint32 high)
{
    return Fixed(((Uint32)old & 65535U) | (Uint32)(Uint16)high << 16);
}
static Sint32 Add(Sint32 a, Sint32 b)
{
    return Fixed((Uint32)a + (Uint32)b);
}
static void Factory(SotnThrownEvents *e, const SotnThrownWeapon *s, Uint32 packed, bool dispatch)
{
    e->factories[e->factory_count++] =
        (SotnThrownFactory){.source = {.x = s->x,
                                       .y = s->y,
                                       .flags = s->flags,
                                       .priority = s->priority,
                                       .facing_left = s->facing_left},
                            .packed = packed,
                            .weapon_dispatch = dispatch};
}
static void Impact(SotnThrownEvents *e, Uint8 shake)
{
    e->sounds[e->sound_count++] = 0x655;
    e->shakes[e->shake_count++] = shake;
}
static bool Query(SotnThrownInput in, Sint32 x, Sint32 y, SotnCollider *out)
{
    return SotnCollision_Query(&in.collision, Word(x), Word(y), out);
}
static bool Iron(SotnThrownWeapon *s, SotnThrownInput in, Uint32 *random, SotnThrownEvents *e)
{
    SotnCollider c = {0};
    Sint16 x, y, shift;
    Uint16 effects;
    switch (s->phase)
    {
    case 0:
        s->bank = (Uint16)(0x8010U + in.hand * 2U);
        s->texture = (Uint16)(100U + in.hand * 2U);
        s->palette = (Uint16)(0x11dU + in.hand * 24U);
        s->frame = 17;
        s->facing_left = in.facing_left;
        s->priority = (Uint16)(in.priority - 2U);
        s->flags = 0x08000000;
        s->velocity_y = -147456 - (Sint32)(SotnCombat_Random(random) & 32767U);
        s->velocity_x = (Sint32)(SotnCombat_Random(random) & 32767U) + 81920;
        if (s->facing_left)
            s->velocity_x = -s->velocity_x;
        s->y = High(s->y, Pixel(in.player_y) + in.offset_y);
        e->properties = e->clear_shield = true;
        s->width = s->height = 8;
        e->sounds[e->sound_count++] = 0x6ee;
        s->radius = 64;
        e->use_ticks = 4;
        ++s->phase;
        return true;
    case 1:
        s->x = Add(s->x, s->velocity_x);
        s->y = Add(s->y, s->velocity_y);
        if (s->velocity_y < 458752)
            s->velocity_y = Add(s->velocity_y, 8192);
        shift = s->velocity_x < 0 ? -5 : 5;
        if (!Query(in, Pixel(s->x) + shift, Pixel(s->y), &c))
            return false;
        if (c.effects & SOTN_EFFECT_SIDE)
        {
            Impact(e, 4);
            s->x = High(s->x, Pixel(s->x) + (shift < 0 ? c.tile_right : c.tile_left));
            s->velocity_x /= -2;
            s->facing_left = !s->facing_left;
        }
        if (!Query(in, Pixel(s->x), Pixel(s->y) - 6, &c))
            return false;
        if (c.effects & SOTN_EFFECT_SOLID)
        {
            Impact(e, 4);
            s->y = High(s->y, Pixel(s->y) + c.bottom + 1);
            s->velocity_x /= 2;
            s->velocity_y = 65536;
        }
        if (!Query(in, Pixel(s->x), Pixel(s->y) + 7, &c))
            return false;
        if (c.effects & SOTN_EFFECT_SOLID)
        {
            Impact(e, 4);
            s->y = High(s->y, Pixel(s->y) + c.top);
            if (s->lifetime)
                ++s->phase;
            s->velocity_y = -73728;
            s->velocity_x /= 2;
            s->lifetime = Word(s->lifetime + 1);
            Factory(e, s, 0x9001f, false);
        }
        return true;
    case 2:
        if (!s->velocity_x && !(s->radius = Word(s->radius - 1)))
        {
            s->phase = 3;
            s->radius = 32;
            s->draw_flags = 0x80;
        }
        x = Pixel(s->x);
        y = Word(Pixel(s->y) + 11);
        if (!Query(in, x, y, &c))
            return false;
        effects = (Uint16)c.effects;
        if (effects & SOTN_EFFECT_SOLID)
        {
            shift = Word(c.top + 4);
            y = Word(y + shift - 5);
            if (!Query(in, x, y, &c))
                return false;
            if (!(c.effects & SOTN_EFFECT_SOLID))
                s->y = High(s->y, Pixel(s->y) + shift);
            else if (!(c.effects & SOTN_EFFECT_SLOPE_FLOOR))
            {
                s->phase = 1;
                s->velocity_y = 0;
                s->width = 8;
                return true;
            }
            else
            {
                s->y = High(s->y, Pixel(s->y) + 3 + shift + c.top);
                effects = (Uint16)c.effects;
            }
            effects &= 0xf000U;
            if (effects == 0xc000 || effects == 0xd000 || effects == 0xe000)
            {
                s->facing_left = false;
                s->velocity_x = Add(s->velocity_x, effects == 0xc000   ? 1024
                                                   : effects == 0xd000 ? 768
                                                                       : 512);
            }
            if (effects == 0x8000 || effects == 0x9000 || effects == 0xa000)
            {
                s->facing_left = true;
                s->velocity_x = Add(s->velocity_x, effects == 0x8000   ? -1024
                                                   : effects == 0x9000 ? -768
                                                                       : -512);
            }
            s->velocity_x = SDL_clamp(s->velocity_x, -65536, 65536);
            if (!(effects & SOTN_EFFECT_SLOPE_FLOOR))
            {
                shift = s->velocity_x < 0 ? -5 : 5;
                if (!Query(in, Pixel(s->x) + shift, Pixel(s->y), &c))
                    return false;
                if (c.effects & SOTN_EFFECT_SIDE)
                {
                    if (s->velocity_x)
                        Impact(e, 1);
                    s->x = High(s->x, Pixel(s->x) + (shift < 0 ? c.tile_right : c.tile_left));
                }
                s->velocity_x = s->velocity_x < 0 ? SDL_min(0, s->velocity_x + 512)
                                                  : SDL_max(0, s->velocity_x - 512);
            }
        }
        else
        {
            s->phase = 1;
            s->velocity_y = 0;
            s->width = 4;
        }
        s->x = Add(s->x, s->velocity_x);
        const Sint32 magnitude =
            s->velocity_x < 0 ? Fixed(0U - (Uint32)s->velocity_x) : s->velocity_x;
        s->width = magnitude >= 32768 ? 8 : 0;
        return true;
    case 3:
        s->radius = Word(s->radius - 1);
        if (!s->radius)
            s->removed = true;
        break;
    case 4:
        break;
    }
    return true;
}
bool SotnThrownWeapon_Tick(SotnThrownWeapon *state, SotnThrownInput in, Uint32 *random,
                           Uint32 *fire_counter, SotnThrownEvents *out)
{
    if (!state || !random || !out || state->removed || in.role > SOTN_THROWN_IRON ||
        in.role < SOTN_THROWN_KNIFE || in.hand > 1 || in.priority < 4 || in.priority > 511 ||
        state->phase > (in.role == SOTN_THROWN_IRON ? 4 : 1) ||
        (in.role == SOTN_THROWN_FIRE && !fire_counter))
        return SDL_SetError("Weapon16 requires its actual callback, player, BSS and RNG owners");
    SotnThrownWeapon s = *state;
    SotnThrownEvents e = {0};
    Uint32 rng = *random, counter = fire_counter ? *fire_counter : 0;
    const bool birth = !s.phase;
    switch (in.role)
    {
    case SOTN_THROWN_KNIFE:
        if (birth)
        {
            s.bank = (Uint16)(0x8010U + in.hand * 2U);
            s.texture = (Uint16)(100U + in.hand * 2U);
            s.palette = (Uint16)(0x11aU + in.hand * 24U);
            s.frame = 14;
            s.facing_left = in.facing_left;
            s.priority = (Uint16)(in.priority - 2U);
            s.draw_flags = 4;
            if (!in.primitive_allocated)
            {
                s.removed = true;
                break;
            }
            s.flags = 0x08800000;
            s.primitive_x = s.x;
            s.primitive_y = s.y;
            s.primitive_anchor_x = Pixel(s.x);
            s.primitive_anchor_y = Pixel(s.y);
            s.primitive_velocity_x = s.facing_left ? -229376 : 229376;
            s.turn = s.facing_left ? 256 : -256;
            s.angle = s.facing_left ? 4096 : 2048;
            e.properties = e.clear_shield = true;
            s.width = s.height = 4;
            e.sounds[e.sound_count++] = 0x69e;
            e.use_ticks = 4;
            ++s.phase;
        }
        else
        {
            if (s.radius < 448)
                s.radius = Word(s.radius + 8);
            s.rotation = Word(s.rotation + SDL_abs((Sint32)s.turn));
            s.angle = Word(s.angle + s.turn);
            s.primitive_x = Add(High(s.primitive_x, s.primitive_anchor_x), s.primitive_velocity_x);
            s.primitive_y = High(s.primitive_y, s.primitive_anchor_y);
            s.primitive_anchor_x = Pixel(s.primitive_x);
            s.primitive_anchor_y = Pixel(s.primitive_y);
        }
        {
            SotnMathTrig trig = {0};
            if (!SotnMath_Trig(in.executable, s.angle, &trig))
                return false;
            s.x = Add(s.primitive_x, (Sint32)trig.cosine * s.radius);
            s.y = Add(s.primitive_y, -(Sint32)trig.sine * s.radius);
        }
        if ((Uint16)s.lifetime & 1U)
            Factory(&e, &s, 58U | ((Uint32)in.hand + 1U) << 12, false);
        s.lifetime = Word(s.lifetime + 1);
        break;
    case SOTN_THROWN_TRAIL:
        if (birth)
        {
            s.bank = in.parent.bank;
            s.frame = in.parent.frame;
            s.facing_left = in.parent.facing_left;
            s.priority = (Uint16)(in.priority - 4U);
            s.flags = 0x08000000;
            s.palette = in.parent.palette;
            s.texture = in.parent.texture;
            s.lifetime = 22;
            s.draw_flags = 12;
            s.blend = 0x10;
            s.rotation = in.parent.rotation;
            s.opacity = 128;
            ++s.phase;
        }
        s.lifetime = Word(s.lifetime - 1);
        if (!s.lifetime)
            s.removed = true;
        else if (s.opacity >= 48)
            s.opacity -= 8;
        break;
    case SOTN_THROWN_BOOMERANG:
        if (birth)
        {
            s.bank = (Uint16)(0x8010U + in.hand * 2U);
            s.texture = (Uint16)(100U + in.hand * 2U);
            s.palette = (Uint16)(0x11aU + in.hand * 24U);
            s.frame = 15;
            s.facing_left = in.facing_left;
            s.priority = (Uint16)(in.priority - 2U);
            s.flags = 0x0c000000;
            s.draw_flags = 4;
            s.y = High(s.y, Pixel(in.player_y) + in.offset_y - (in.player_step == 2 ? 0 : 8));
            s.x = High(s.x,
                       Pixel(in.player_x) + (s.facing_left ? -in.offset_x - 20 : in.offset_x + 20));
            s.angle = -32;
            s.speed = 72;
            s.acceleration = 10;
            s.timer = 0;
            s.rotation = 1024;
            e.properties = e.clear_shield = true;
            s.width = s.height = 12;
            e.sounds[e.sound_count++] = 0x69e;
            if (s.parameter & 0x7f00U)
                e.sounds[e.sound_count++] = 0x65b;
            e.use_ticks = 4;
            ++s.phase;
        }
        else
        {
            s.angle = Word(s.angle + s.acceleration);
            s.rotation = Word(s.rotation - 256);
            SotnMathTrig trig = {0};
            if (!SotnMath_Trig(in.executable, s.angle, &trig))
                return false;
            s.velocity_x = Fixed((Uint32)(Sint32)trig.cosine * (Uint32)s.speed);
            s.velocity_y = Fixed(0U - (Uint32)(Sint32)trig.sine * (Uint32)s.speed);
            s.x = Fixed((Uint32)s.x +
                        (s.facing_left ? 0U - (Uint32)s.velocity_x : (Uint32)s.velocity_x));
            s.y = Add(s.y, s.velocity_y);
            if (s.timer < 35)
                s.acceleration = Word(s.acceleration + 2);
            if (s.timer > 35)
            {
                s.speed = Add(s.speed, 3);
                if (s.acceleration > 6)
                    s.acceleration = Word(s.acceleration - 6);
                if (s.timer == 64)
                    s.flags &= 0xfbffffffU;
            }
            if (s.timer <= 0x7000)
                s.timer = Word(s.timer + 1);
            if ((Uint16)s.timer & 1U)
                Factory(&e, &s, 58U | ((Uint32)in.hand + 1U) << 12, false);
            if (s.parameter & 0x7f00U)
                Factory(&e, &s, 110U | ((Uint32)in.hand + 1U) << 12, false);
        }
        break;
    case SOTN_THROWN_JAVELIN:
        if (birth)
        {
            s.bank = (Uint16)(0x8010U + in.hand * 2U);
            s.texture = (Uint16)(100U + in.hand * 2U);
            s.palette = (Uint16)(0x11bU + in.hand * 24U);
            s.frame = 16;
            s.facing_left = in.facing_left;
            s.priority = (Uint16)(in.priority - 2U);
            s.flags = 0x08000000;
            s.draw_flags = 4;
            s.y = High(s.y, Pixel(in.player_y) + in.offset_y - (in.player_step == 2 ? 6 : 14));
            s.x = High(s.x,
                       Pixel(in.player_x) + (s.facing_left ? -in.offset_x - 10 : in.offset_x + 10));
            s.velocity_x = s.facing_left ? -393216 : 393216;
            s.velocity_y = -73728;
            e.properties = e.clear_shield = true;
            s.width = s.height = 12;
            e.sounds[e.sound_count++] = 0x60b;
            e.use_ticks = 4;
            ++s.phase;
        }
        else
        {
            if (s.velocity_y < 458752)
                s.velocity_y = Add(s.velocity_y, 3072);
            s.x = Add(s.x, s.velocity_x);
            s.y = Add(s.y, s.velocity_y);
        }
        if (s.velocity_x == SDL_MIN_SINT32 ||
            !SotnMath_Atan2Vector(in.executable, s.velocity_y, SDL_abs(s.velocity_x), &s.rotation))
            return false;
        break;
    case SOTN_THROWN_FIRE:
        if (birth)
        {
            s.bank = 2;
            e.fire_animation = true;
            s.priority = (Uint16)(in.priority - 4U);
            s.flags = 0x08100000;
            s.velocity_y = -65536;
            if (!(counter & 1U))
                Factory(&e, &s, 0xb0004, true);
            s.y = High(s.y, Pixel(s.y) - 6 + (Sint32)(SotnCombat_Random(&rng) % 12U));
            s.x = High(s.x, Pixel(s.x) - 3 + (Sint32)(SotnCombat_Random(&rng) & 7U));
            ++counter;
            if (!(SotnCombat_Random(&rng) & 1U))
                s.blend |= 0x70;
            ++s.phase;
        }
        s.y = Add(s.y, s.velocity_y);
        if (in.remaining < 0)
            s.removed = true;
        break;
    case SOTN_THROWN_IRON:
        if (!Iron(&s, in, &rng, &e))
            return false;
        break;
    }
    *state = s;
    *random = rng;
    if (fire_counter)
        *fire_counter = counter;
    *out = e;
    return true;
}
static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
bool SotnThrownWeapon_AnimateFire(SotnThrownAnimation *animation, SotnDataView overlay)
{
    if (!animation || !overlay.bytes || overlay.size < 0x5ec || animation->pose >= 15 ||
        animation->remaining < -1)
        return SDL_SetError("Weapon16 fire animation requires its actual private bounded timeline");
    SotnThrownAnimation a = *animation;
    if (a.remaining != -1)
    {
        if (!a.remaining)
        {
            const Uint16 duration = Read16(overlay.bytes + 0x5b0 + a.pose * 4U);
            if (!duration || duration > 32767)
                return SDL_SetError("Weapon16 fire has invalid private birth duration");
            a.remaining = (Sint16)duration;
        }
        else if (!--a.remaining)
        {
            if (++a.pose >= 15)
                return SDL_SetError("Weapon16 fire escaped its actual private timeline");
            const Uint16 duration = Read16(overlay.bytes + 0x5b0 + a.pose * 4U);
            if (duration == 0xffffU)
            {
                --a.pose;
                a.remaining = -1;
            }
            else
            {
                if (!duration || duration > 32767)
                    return SDL_SetError("Weapon16 fire has invalid private duration/end control");
                a.remaining = (Sint16)duration;
            }
        }
    }
    a.frame = Read16(overlay.bytes + 0x5b2 + a.pose * 4U) & 511U;
    *animation = a;
    return true;
}
