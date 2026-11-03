// SPDX-License-Identifier: AGPL-3.0-or-later
// weapon/w_015.c and dra/alu_anim.c:UpdateAnim, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. All private artwork/tables stay private.
#include <sotn/combat.h>
#include <sotn/star_weapon.h>
static Sint16 Word(Sint32 v)
{
    Uint16 b = (Uint16)(Uint32)v;
    return b < 32768 ? (Sint16)b : (Sint16)((Sint32)b - 65536);
}
static Sint32 Fixed(Uint32 v)
{
    return v <= SDL_MAX_SINT32 ? (Sint32)v : (Sint32)((Sint64)v - 4294967296LL);
}
static Sint32 Add(Sint32 a, Sint32 b)
{
    return Fixed((Uint32)a + (Uint32)b);
}
static Sint16 Pixel(Sint32 v)
{
    return Word((Sint32)((Uint32)v >> 16));
}
static Sint32 High(Sint32 v, Sint32 hi)
{
    return Fixed(((Uint32)v & 65535U) | (Uint32)(Uint16)hi << 16);
}
static bool Table(SotnDataView o, Uint16 offset, Uint16 index, Uint16 count, Uint16 *out)
{
    size_t at = (size_t)offset + index * 2U;
    if (!out || index >= count || !o.bytes || at > o.size || o.size - at < 2)
        return SDL_SetError("Weapon15 requires its actual bounded private table");
    *out = (Uint16)(o.bytes[at] | (Uint16)o.bytes[at + 1] << 8);
    return true;
}
static void Factory(SotnStarEvents *e, const SotnThrownWeapon *s, Uint16 hand, Uint32 packed)
{
    e->effects.factories[e->effects.factory_count++] =
        (SotnThrownFactory){.source = {.x = s->x,
                                       .y = s->y,
                                       .flags = s->flags,
                                       .priority = s->priority,
                                       .facing_left = s->facing_left},
                            .packed = packed | ((Uint32)hand + 1U) << 12};
}
static bool Query(SotnStarInput in, Sint32 x, Sint32 y, SotnCollider *out)
{
    return SotnCollision_Query(&in.collision, Word(x), Word(y), out);
}
static bool Tnt(SotnStarWeapon *n, SotnStarInput in, Uint32 *rng, SotnStarEvents *e)
{
    SotnThrownWeapon *s = &n->motion;
    SotnCollider c = {0};
    switch (s->phase)
    {
    case 0:
        s->bank = (Uint16)(0x8010U + in.hand * 2U);
        s->texture = (Uint16)(100U + in.hand * 2U);
        s->palette = (Uint16)(0x110U + in.hand * 24U);
        s->facing_left = in.facing_left;
        s->priority = (Uint16)(in.priority - 2U);
        s->flags = in.primitive_allocated ? 0x08800000 : 0x08000000;
        if (in.primitive_allocated)
            for (Uint16 i = 0; i < 32; ++i)
            {
                n->sparks[i] = (SotnStarSpark){.direction = (Sint16)(SotnCombat_Random(rng) & 15U),
                                               .draw_mode = 10};
                n->sparks[i].delay = (Sint16)(SotnCombat_Random(rng) & 15U);
            }
        s->lifetime = 10;
        s->velocity_y = -163840;
        s->velocity_x = s->facing_left ? -98304 : 98304;
        s->y = High(s->y, Pixel(in.player_y) + in.offset_y - 8 - (in.player_step == 2 ? 0 : 6));
        e->fixed_tnt_properties = true;
        e->effects.clear_shield = true;
        s->width = s->height = 4;
        e->effects.sounds[e->effects.sound_count++] = 0x60c;
        e->effects.use_ticks = 4;
        ++s->phase;
        break;
    case 1:
        if (!(in.game_timer & 7U))
            e->effects.sounds[e->effects.sound_count++] = 0x6a3;
        s->frame = 13;
        s->x = Add(s->x, s->velocity_x);
        s->y = Add(s->y, s->velocity_y);
        if (s->velocity_y <= 458751)
            s->velocity_y = Add(s->velocity_y, 8192);
        if (n->hit_flags)
        {
            s->velocity_x = Fixed((Uint32)s->velocity_x * 2U) / 3;
            n->hit_flags = 0;
            return true;
        }
        const Sint16 shift = s->velocity_x < 0 ? -4 : 4;
        if (!Query(in, Pixel(s->x) + shift, Pixel(s->y), &c))
            return false;
        if (c.effects & SOTN_EFFECT_SIDE)
        {
            s->x = High(s->x, Pixel(s->x) + (shift < 0 ? c.tile_right : c.tile_left));
            s->velocity_x = -(s->velocity_x / 2);
        }
        if (!Query(in, Pixel(s->x), Pixel(s->y) - 4, &c))
            return false;
        if (c.effects & SOTN_EFFECT_SOLID)
        {
            s->y = High(s->y, Pixel(s->y) + 1 + c.bottom);
            s->velocity_y = 65536;
            s->velocity_x /= 2;
        }
        if (!Query(in, Pixel(s->x), Pixel(s->y) + 6, &c))
            return false;
        if (c.effects & SOTN_EFFECT_SOLID)
        {
            Factory(e, s, in.hand, 76);
            Factory(e, s, in.hand, 0x10004e);
            e->effects.shakes[e->effects.shake_count++] = 3;
            ++s->phase;
            s->velocity_y = -393216;
            s->lifetime = 16;
            if (s->flags & 0x800000U)
                for (Uint16 i = 0; i < 32; ++i)
                    n->sparks[i].draw_mode |= 8;
        }
        if (s->flags & 0x800000U)
            for (Uint16 i = 0; i < 32; ++i)
            {
                SotnStarSpark *p = &n->sparks[i];
                if (!p->delay)
                {
                    Uint16 x = 0, y = 0;
                    if (!Table(in.overlay, 0x6fc, (Uint16)p->direction, 16, &x) ||
                        !Table(in.overlay, 0x71c, (Uint16)p->direction, 16, &y))
                        return false;
                    const Sint32 dx = Word(x) * p->age / 15, dy = Word(y) * p->age / 15;
                    p->x = Word(Pixel(s->x) + (s->facing_left ? -8 - dx : 8 + dx));
                    p->y = Word(Pixel(s->y) + dy);
                    p->age = Word(p->age + 1);
                    if (p->age > 15)
                    {
                        p->age = 0;
                        p->direction = (Sint16)(SotnCombat_Random(rng) & 15U);
                    }
                    p->draw_mode &= 0xfff7U;
                }
                else
                    p->delay = Word(p->delay - 1);
            }
        break;
    case 2:
        n->attack_cleared = true;
        if (!(s->radius & 7))
            e->effects.sounds[e->effects.sound_count++] = 0x65b;
        s->frame = 0;
        s->y = Add(s->y, s->velocity_y);
        s->lifetime = Word(s->lifetime - 1);
        if (!s->lifetime)
        {
            s->removed = true;
            return true;
        }
        if (s->lifetime & 1)
        {
            Factory(e, s, in.hand, 78U + ((Uint32)(Uint16)(s->radius + 16) << 16));
            s->radius = Word(s->radius + 1);
        }
        break;
    case 3:
        n->attack_cleared = true;
        if (!(s->lifetime & 7))
        {
            e->effects.sounds[e->effects.sound_count++] = 0x65b;
            Factory(e, s, in.hand, 104);
        }
        s->frame = 0;
        s->lifetime = Word(s->lifetime - 1);
        if (!s->lifetime)
            s->removed = true;
        break;
    }
    return true;
}
bool SotnStarWeapon_Tick(SotnStarWeapon *state, SotnStarInput in, Uint32 *random,
                         Uint32 *fire_counter, SotnStarEvents *out)
{
    if (!state || !random || !out || state->motion.removed || in.role < SOTN_STAR_PROJECTILE ||
        in.role > SOTN_STAR_SCATTER || in.hand > 1 || in.priority < 4 || in.priority > 511 ||
        state->motion.phase > (in.role == SOTN_STAR_TNT          ? 3
                               : in.role == SOTN_STAR_PROJECTILE ? 2
                                                                 : 1) ||
        (in.role == SOTN_STAR_FLAME && !fire_counter))
        return SDL_SetError("Weapon15 requires actual callback/player/BSS and RNG owners");
    SotnStarWeapon n = *state;
    SotnThrownWeapon *s = &n.motion;
    SotnStarEvents e = {0};
    Uint32 rng = *random, counter = fire_counter ? *fire_counter : 0;
    const Uint16 variant = (s->parameter >> 8) & 127U;
    switch (in.role)
    {
    case SOTN_STAR_PROJECTILE:
        if (variant > 3)
            return SDL_SetError("Weapon15 projectile requires original variant0..3");
        if (!s->phase)
        {
            s->bank = (Uint16)(0x8010U + in.hand * 2U);
            s->palette = (Uint16)(0x110U + in.hand * 24U);
            s->texture = (Uint16)(100U + in.hand * 2U);
            if (!Table(in.overlay, 0x6b8, variant, 4, &s->frame))
                return false;
            s->facing_left = in.facing_left;
            s->priority = (Uint16)(in.priority - 2U);
            s->flags = 0x08000000;
            s->velocity_x = variant == 3 ? 349525 : 524288;
            if (s->facing_left)
                s->velocity_x = -s->velocity_x;
            s->y = High(s->y, Pixel(in.player_y) + in.offset_y - 3 - (in.player_step == 2 ? 0 : 6));
            s->x = High(s->x, Pixel(in.player_x) + (s->facing_left ? -in.offset_x : in.offset_x));
            e.effects.properties = e.effects.clear_shield = true;
            s->width = s->height = 4;
            if (variant == 1 || variant == 3)
                s->draw_flags |= 4;
            if (variant == 3)
                s->palette = 0x8160;
            e.effects.sounds[e.effects.sound_count++] = variant ? 0x625 : 0x60a;
            if (variant == 3)
                e.effects.sounds[e.effects.sound_count++] = 0x661;
            e.effects.use_ticks = 4;
            ++s->phase;
        }
        else if (s->phase == 1)
        {
            s->rotation = Word(s->rotation + 256);
            s->x = Add(s->x, s->velocity_x);
            if (variant == 2 && !(s->lifetime & 1))
                Factory(&e, s, in.hand, 58);
            if (variant == 3)
                Factory(&e, s, in.hand, 80);
            s->lifetime = Word(s->lifetime + 1);
            if (n.hit_flags)
            {
                s->lifetime = 24;
                n.attack_cleared = true;
                s->draw_flags |= 0x80;
                ++s->phase;
            }
        }
        else
        {
            s->lifetime = Word(s->lifetime - 1);
            if (!s->lifetime)
                s->removed = true;
        }
        break;
    case SOTN_STAR_TRAIL:
        if (!s->phase)
        {
            s->bank = in.parent.image.bank;
            s->frame = in.parent.image.frame;
            s->facing_left = in.parent.image.facing_left;
            s->priority = (Uint16)(in.parent.priority - 2U);
            s->flags = 0x08000000;
            s->palette = in.parent.image.palette;
            s->texture = in.parent.image.texture;
            s->lifetime = 10;
            s->blend = 0x10;
            s->draw_flags = 8;
            s->opacity = 128;
            ++s->phase;
        }
        if (s->opacity >= 48)
            s->opacity = (Uint8)(s->opacity + 248U);
        s->lifetime = Word(s->lifetime - 1);
        if (!s->lifetime)
            s->removed = true;
        break;
    case SOTN_STAR_FLAME:
        if (!SotnThrownWeapon_Tick(s,
                                   (SotnThrownInput){.role = SOTN_THROWN_FIRE,
                                                     .priority = in.priority,
                                                     .remaining = in.remaining},
                                   &rng, &counter, &e.effects))
            return false;
        if (e.effects.fire_animation)
            e.animation_offset = 0x6c0;
        break;
    case SOTN_STAR_TNT:
        if (!Tnt(&n, in, &rng, &e))
            return false;
        break;
    case SOTN_STAR_EXPLOSION:
    case SOTN_STAR_SCATTER:
        if (!s->phase)
        {
            if (!in.parent.exists)
            {
                s->removed = true;
                break;
            }
            s->bank = (Uint16)(0x8011U + in.hand * 2U);
            s->palette = (Uint16)(0x111U + in.hand * 24U);
            s->texture = (Uint16)(101U + in.hand * 2U);
            e.animation_offset = 0x770;
            s->flags = 0x08100000;
            s->priority = (Uint16)(in.priority - 2U);
            s->blend = 0x30;
            if (in.role == SOTN_STAR_SCATTER)
            {
                Sint16 angle = 0;
                SotnMathTrig trig = {0};
                if (!SotnMath_Atan2Vector(in.executable, Fixed(0U - (Uint32)in.parent.velocity_y),
                                          in.parent.velocity_x, &angle))
                    return false;
                angle = Word(angle + 0x500 + (Sint32)(SotnCombat_Random(&rng) % 0x600U));
                if (!SotnMath_Trig(in.executable, angle, &trig))
                    return false;
                s->velocity_x = trig.cosine * 64;
                s->velocity_y = trig.sine * -64;
                n.acceleration_x = -s->velocity_x / 13;
                n.acceleration_y = -s->velocity_y / 13;
            }
            else if ((s->parameter >> 8) >= 16)
            {
                Uint16 scale = 0;
                if (!Table(in.overlay, 0x78c, s->parameter >> 8, 24, &scale))
                    return false;
                n.scale_x = n.scale_y = scale;
            }
            else
            {
                s->priority = (Uint16)(in.priority + 2U);
                if (in.buttons & 0x4000U)
                {
                    const Sint16 angle = (Sint16)(SotnCombat_Random(&rng) % 1535U + 256U);
                    SotnMathTrig trig = {0};
                    if (!SotnMath_Trig(in.executable, angle, &trig))
                        return false;
                    s->velocity_x = trig.cosine * 64;
                    s->velocity_y = trig.sine * -64;
                    s->x = Add(s->x, s->velocity_x);
                    s->y = Add(s->y, s->velocity_y);
                    n.acceleration_x = -s->velocity_x / 16;
                    n.acceleration_y = -s->velocity_y / 16;
                }
                else
                {
                    s->velocity_y = -65536;
                    s->y = High(s->y, Pixel(s->y) - 12 + (Sint32)(SotnCombat_Random(&rng) % 24U));
                    s->x = High(s->x, Pixel(s->x) - 20 + (Sint32)(SotnCombat_Random(&rng) % 40U));
                }
            }
            n.item = in.parent.item;
            e.effects.properties = true;
            e.inherit_group = true;
            s->width = s->height = 12;
            ++s->phase;
        }
        else
        {
            if (in.remaining < 0)
            {
                s->removed = true;
                break;
            }
            s->x = Add(s->x, s->velocity_x);
            s->y = Add(s->y, s->velocity_y);
            if (in.role == SOTN_STAR_SCATTER || !(s->parameter >> 8))
            {
                s->velocity_x = Add(s->velocity_x, n.acceleration_x);
                s->velocity_y = Add(s->velocity_y, n.acceleration_y);
            }
        }
        break;
    }
    *state = n;
    *random = rng;
    if (fire_counter)
        *fire_counter = counter;
    *out = e;
    return true;
}
bool SotnStarWeapon_Animate(SotnThrownAnimation *animation, SotnDataView overlay, Uint16 offset)
{
    if (!animation || (offset != 0x6c0 && offset != 0x770) || !overlay.bytes ||
        overlay.size < (size_t)offset + 60 || animation->pose >= 15 || animation->remaining < -1)
        return SDL_SetError("Weapon15 animation requires its actual private timeline");
    SotnThrownAnimation n = *animation;
    Uint16 duration = 0, frame = 0;
    if (n.remaining != -1)
    {
        if (!n.remaining)
        {
            if (!Table(overlay, offset, (Uint16)(n.pose * 2U), 30, &duration))
                return false;
            if (!duration || duration > 32767)
                return SDL_SetError("Weapon15 private birth duration is invalid");
            n.remaining = (Sint16)duration;
        }
        else if (!--n.remaining)
        {
            if (++n.pose >= 15 || !Table(overlay, offset, (Uint16)(n.pose * 2U), 30, &duration))
                return SDL_SetError("Weapon15 escaped its actual private timeline");
            if (duration == 0xffffU)
            {
                --n.pose;
                n.remaining = -1;
            }
            else
            {
                if (!duration || duration > 32767)
                    return SDL_SetError("Weapon15 private duration/end control is invalid");
                n.remaining = (Sint16)duration;
            }
        }
    }
    if (!Table(overlay, offset, (Uint16)(n.pose * 2U + 1U), 30, &frame))
        return false;
    n.frame = frame & 511U;
    *animation = n;
    return true;
}
