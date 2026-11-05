// SPDX-License-Identifier: AGPL-3.0-or-later
// Ordinary weapon/w_000.c, w_001.c, w_002.c, w_003.c, w_004.c, w_005.c, w_009.c, w_011.c, w_026.c,
// w_030.c, w_031.c, w_033.c, w_034.c, w_035.c, w_036.c, w_037.c, w_043.c, w_052.c, w_057.c and
// dra/alu_anim.c, sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. All original animation,
// hitbox, palette and artwork tables remain private inputs.
#include <sotn/combat.h>
#include <sotn/game_math.h>
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
// w_051's color words live at D_8017AD68.. in declaration order (_r, _b, _g, color_step, _a);
// reads of its private tables that land there see the live program values.
static Sint16 *GlowWord(SotnHandWeaponProgram *program, Uint32 at)
{
    Sint16 *words[5] = {&program->glow_red, &program->glow_blue, &program->glow_green,
                        &program->glow_step, &program->glow_alpha};
    return at >= 0xd68 && at < 0xd72 && !(at & 1U) ? words[(at - 0xd68) / 2U] : NULL;
}
static bool GlowRead(SotnDataView o, SotnHandWeaponProgram *program, Uint32 at, Sint16 *out)
{
    const Sint16 *live = GlowWord(program, at);
    const Uint8 *p = NULL;
    if (live)
        *out = *live;
    else if (View(o, at, 2, &p))
        *out = (Sint16)Read16(p);
    else
        return false;
    return true;
}
static bool GlowTable(SotnDataView o, Uint32 index, Sint32 *out)
{
    const Uint8 *p = NULL;
    if (!View(o, 0xd74U + index * 4U, 4, &p))
        return false;
    *out = (Sint32)Read32(p);
    return true;
}
// w_051 func_8017B1DC: pulse one colour of the descriptor's palette, then upload the CLUT rows.
static bool Glow(SotnDataView o, SotnHandWeaponProgram *program, Uint16 hand, Uint16 row,
                 SotnHandWeaponEvents *e)
{
    Sint16 *channel[3] = {&program->glow_red, &program->glow_green, &program->glow_blue};
    if (program->glow_step == 2)
        return true;
    if (program->glow_step != 0 && program->glow_step != 1)
        return SDL_SetError("Weapon51 glow left its original two-phase schedule");
    const bool rising = program->glow_step == 0;
    Uint16 changed = 0;
    for (Uint16 k = 0; k < 3; ++k)
        *channel[k] = (Sint16)(*channel[k] + (rising ? 2 : -2));
    for (Uint16 k = 0; k < 3; ++k)
    {
        Sint32 limit = 0;
        if (!GlowTable(o, row * 8U + (rising ? 4U : 0U) + k, &limit))
            return false;
        if (rising ? *channel[k] >= limit : *channel[k] <= limit)
        {
            ++changed;
            *channel[k] = (Sint16)limit;
        }
    }
    program->glow_alpha = program->glow_red || program->glow_green || program->glow_blue;
    if (changed == 3)
        ++program->glow_step;
    const Uint32 index = hand * 24U * 16U + 0x2bU + row * 0x20U;
    e->clut = true;
    e->clut_palette = (Uint16)(0x110U + index / 16U);
    e->clut_color = (Uint16)(index % 16U);
    e->clut_value = (Uint16)((Uint32)program->glow_red | (Uint32)program->glow_green << 5 |
                             (Uint32)program->glow_blue << 10 | (Uint32)program->glow_alpha << 15);
    return true;
}
// Original PlaySfx/CreateEntFactoryFromEntity calls keep their order within one callback.
static bool AppendSound(SotnHandWeaponEvents *e, Uint16 sound)
{
    if (!e->sound)
        e->sound = sound;
    else
    {
        Uint16 n = 0;
        while (n < SDL_arraysize(e->extra_sounds) && e->extra_sounds[n])
            ++n;
        if (n == SDL_arraysize(e->extra_sounds))
            return SDL_SetError("Ordinary weapon exceeded its per-callback sound requests");
        e->extra_sounds[n] = sound;
    }
    return true;
}
static bool AppendFactory(SotnHandWeaponEvents *e, Uint32 request)
{
    if (!e->factory)
        e->factory = request;
    else
    {
        Uint16 n = 0;
        while (n < SDL_arraysize(e->more_factories) && e->more_factories[n])
            ++n;
        if (n == SDL_arraysize(e->more_factories))
            return SDL_SetError("Ordinary weapon exceeded its per-callback factory requests");
        e->more_factories[n] = request;
    }
    return true;
}
bool SotnHandWeapon_IsShieldFamily(Uint16 family)
{
    return family == 8 || family == 9 || family == 10 || family == 11 || family == 23 ||
           family == 25 || family == 26 || family == 52 || family == 58;
}
// w_008/w_009 share one layout, as do w_025/w_026 and w_052/w_058.
static Uint16 ShieldAnimationTable(Uint16 family)
{
    return family == 8 || family == 9     ? 0xa40
           : family == 10                 ? 0xb68
           : family == 11                 ? 0xb94
           : family == 23                 ? 0xb20
           : family == 25 || family == 26 ? 0xa60
                                          : 0xb84;
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
    // w_052: while its spell's armed flag stands, a raised hit (actual collision hitFlags)
    // requests func_8010E168(0,0x60), kind1 healing and a heart.
    const bool armed = in.family == 52 && program->alucard_active && !(w->parameter & 0x7f00U);
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
    if (armed && (w->phase == 1 || w->phase == 2) && w->hit_flags)
    {
        e->shield_hit = true;
        w->hit_flags = 0;
    }
    const Uint16 crouch = in.step == 2 && in.substep != in.step;
    // w_010 (Iron shield): a raised hit requests WFACTORY(56,0)'s thunder, then
    // func_8010E168(0,0x60) (BlinkWhite0x15 from the shield and protection0x60).
    if (in.family == 10 && (w->phase == 1 || w->phase == 2) && w->hit_flags)
    {
        if (!AppendFactory(e, ((in.hand + 1U) << 12) | 56U))
            return false;
        e->guard_blink = true;
        w->hit_flags = 0;
    }
    // w_023 (Herald shield): a raised block of a projectile (hitFlags3) adds a heart, then waits
    // out unk84's 0x20 cooldown, which counts down on every update; other hitFlags stay set.
    if (in.family == 23)
    {
        if ((w->phase == 1 || w->phase == 2) && !w->guard_cooldown && w->hit_flags == 3)
        {
            e->hearts = true;
            w->hit_flags = 0;
            w->guard_cooldown = 0x20;
        }
        if (w->guard_cooldown)
            --w->guard_cooldown;
    }
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
        // w_052 and w_058 cache the property attack and override it to one.
        if (in.family == 52 || in.family == 58)
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
    // The armed shield holds MP regeneration, drains one MP every sixteenth game update and
    // restores its cached attack; raised, it also attacks (hitbox |2) until its timer or MP ends,
    // then rolls its properties again and releases the 0x3A letters.
    if (armed)
    {
        e->mp_hold = true;
        e->mp_drain = !(in.game_timer % 16U);
        w->properties.attack = (Sint16)program->shield_attack;
        if (w->phase != 1 && w->phase != 2)
            return true;
        w->hitbox_state |= 2;
        program->alucard_timer = (Sint32)((Uint32)program->alucard_timer - 1U);
        if (!program->alucard_timer || (Sint32)in.mp - (e->mp_drain ? 1 : 0) <= 0)
        {
            program->alucard_active = 0;
            if (!Properties(w, in, random, e))
                return false;
            w->properties.attack = 1;
            if (!AppendFactory(e, ((in.hand + 1U) << 14) | 0x3aU))
                return false;
        }
    }
    return true;
}
static Sint16 Signed16(const Uint8 *p)
{
    return Word(Read16(p));
}
// w_001.c EntityWeaponAttack birth: four mandatory GT4 trails before SetWeaponProperties.
static bool FistBirth(SotnHandWeapon *w, SotnHandWeaponInput in, const Uint8 *descriptor)
{
    const Uint8 *colors = NULL;
    if (!View(in.overlay, 0xfb0, 8, &colors))
        return false;
    for (Uint16 i = 0; i < SOTN_HAND_WEAPON_FIST_ROWS; ++i)
    {
        SotnPlayerEffectQuad *q = &w->tiles[i];
        *q = (SotnPlayerEffectQuad){.page = 0x1a,
                                    .palette = Read16(descriptor + 8),
                                    .priority = (Uint16)(in.priority + 4U),
                                    .draw_mode = 0x335};
        q->u[0] = q->u[2] = 0x80;
        q->u[1] = q->u[3] = 0xa0;
        q->v[0] = q->v[1] = (Uint8)(0x80U + i * 4U);
        q->v[2] = q->v[3] = (Uint8)(0x80U + (i + 1U) * 4U);
        q->red[0] = q->green[0] = q->blue[0] = colors[i * 2U];
        q->red[2] = q->green[2] = q->blue[2] = colors[i * 2U + 1U];
    }
    w->allocated = true;
    w->flags = 0x860000U;
    return true;
}
static Uint16 FistPhase(Uint16 animation, Uint16 pose, Uint16 variant)
{
    // w_001's phase switches name descriptors0..3 only; Tulkas's descriptor4 never shows trails.
    if (variant > 3)
        return 0;
    const bool first = variant == 0;
    if (animation <= 3)
        return first ? pose == 2 || pose == 3 : pose >= 2 && pose <= 4;
    if (animation == 4 || animation == 5)
    {
        const bool active = first ? pose == 1 || pose == 2 : pose >= 1 && pose <= 3;
        return active ? (animation == 4 ? 1 : 2) : 0;
    }
    return 0;
}
// w_001.c trail placement: one libc call per row, then the SDK follow-through rotation.
static bool FistTrails(SotnHandWeapon *w, SotnHandWeaponInput in, Uint16 variant, Uint32 *random)
{
    const Uint16 phase = FistPhase(w->animation, in.pose, variant);
    if (!phase)
    {
        for (Uint16 i = 0; i < SOTN_HAND_WEAPON_FIST_ROWS; ++i)
            w->tiles[i].draw_mode |= 8U;
        return true;
    }
    const Uint8 *offsets = NULL;
    if (w->animation > 6 || !View(in.overlay, 0xf94 + w->animation * 4U, 4, &offsets))
        return false;
    Sint16 x_offset = Signed16(offsets), y_offset = Signed16(offsets + 2);
    if (in.facing_left)
        x_offset = Word(-x_offset);
    const Sint16 x = Word(Pixel(in.x) - in.camera_x + x_offset);
    Sint16 y = Word(Pixel(in.y) - in.camera_y + y_offset);
    if (in.draw_flags & 2U)
        y = Word(y - 3);
    const Sint16 base_angle = in.facing_left ? 512 : -512;
    for (Uint16 i = 0; i < SOTN_HAND_WEAPON_FIST_ROWS; ++i)
    {
        SotnPlayerEffectQuad *q = &w->tiles[i];
        const Sint32 spread = (Sint32)(SotnCombat_Random(random) & 15U) + 16;
        q->x[0] = q->x[2] = Word(x + (in.facing_left ? -16 : 16));
        q->x[1] = q->x[3] = Word(in.facing_left ? x + spread : x - spread);
        q->y[0] = q->y[1] = Word(-8 + y + i * 4);
        q->y[2] = q->y[3] = Word(q->y[0] + 4);
        q->draw_mode &= 0xfff7U;
        if (phase != 2)
            continue;
        for (Uint16 v = 0; v < 4; ++v)
        {
            const Sint16 width = Word(q->x[v] - x), height = Word(q->y[v] - y);
            Uint32 root = 0;
            Sint16 direction = 0;
            SotnMathTrig trig = {0};
            const Sint32 squared = (Sint32)width * width + (Sint32)height * height;
            if (!SotnMath_Sqrt12(in.executable, (Uint32)squared << 12, &root) ||
                !SotnMath_Atan2(in.executable, -height, width, &direction) ||
                !SotnMath_Trig(in.executable, Word(base_angle + direction), &trig))
                return false;
            const Sint32 size = (Sint32)(root >> 12);
            q->x[v] = Word(x + (((trig.cosine >> 4) * size) >> 8));
            q->y[v] = Word(y - (((trig.sine >> 4) * size) >> 8));
        }
    }
    return true;
}
// w_054.c birth: twelve optional LINE_G2 rows, each consuming its original rand() calls.
static void GurthangBirth(SotnHandWeapon *w, SotnHandWeaponInput in, Uint32 *random)
{
    w->allocated = true;
    w->flags |= 0x800000U;
    for (Uint16 i = 0; i < SOTN_HAND_WEAPON_GURTHANG_ROWS; ++i)
    {
        SotnPlayerEffectQuad *q = &w->tiles[i];
        *q = (SotnPlayerEffectQuad){0};
        if (i <= 3)
        {
            q->red[2] = (Uint8)(SotnCombat_Random(random) & 3U);
            q->blue[2] = 1;
        }
        if (i < 7)
        {
            q->red[2] = (Uint8)(SotnCombat_Random(random) & 7U);
            q->blue[2] = 1;
        }
        else if (i & 1U)
        {
            q->red[2] = (Uint8)(SotnCombat_Random(random) & 15U);
            q->blue[2] = 2;
        }
        else
        {
            q->red[2] = (Uint8)(SotnCombat_Random(random) & 7U);
            q->blue[2] = 1;
        }
        q->red[3] = (Uint8)(i / 6U);
        if (i == 11)
        {
            q->red[2] = 0x18;
            q->red[3] = 2;
            q->blue[2] = 3;
        }
        q->red[0] = q->red[1] = 255;
        q->blue[0] = q->blue[1] = q->green[0] = q->green[1] =
            (Uint8)(SotnCombat_Random(random) & 0x1fU);
        q->priority = in.priority;
        q->draw_mode = 0x308;
    }
}
// w_054.c (and w_056.c, with its own tables) post-hitbox rows on frames0xD/0x22/0x2C/0x3E;
// other frames leave them unchanged.
static bool GurthangTrails(SotnHandWeapon *w, SotnHandWeaponInput in)
{
    const Uint16 frame = w->frame;
    const Uint16 index = frame == 0xd    ? 0
                         : frame == 0x22 ? 1
                         : frame == 0x2c ? 2
                         : frame == 0x3e ? 3
                                         : 4;
    if (index == 4 || !w->allocated)
        return true;
    const Uint8 *xs = NULL, *ys = NULL;
    const Uint32 table = in.family == 56 ? 0xf94U : 0xef4U;
    if (!View(in.overlay, table + index * 2U, 2, &xs) ||
        !View(in.overlay, table + 8U + index * 2U, 2, &ys))
        return false;
    const Sint16 px = Word(Pixel(w->x) - in.camera_x), py = Word(Pixel(w->y) - in.camera_y);
    for (Uint16 i = 0; i < SOTN_HAND_WEAPON_GURTHANG_ROWS; ++i)
    {
        SotnPlayerEffectQuad *q = &w->tiles[i];
        Sint16 x = Word(Read16(xs) - i), step = -1;
        if (w->facing_left)
        {
            x = Word(-x);
            step = 1;
        }
        q->x[2] = Word(step + q->x[2]);
        const Sint32 y = Word(Read16(ys)) + i / 6;
        q->x[0] = Word(x + px);
        q->x[1] = Word(q->x[0] + q->x[2]);
        q->y[0] = Word(y + py);
        q->y[1] = Word(y + py + q->red[3]);
        if (q->red[2] > q->red[3])
            q->red[3] = (Uint8)(q->red[3] + q->blue[2]);
        q->draw_mode &= 0xfff7U;
    }
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
// Reconstructed ordinary callbacks; shields and unarmed255 are included.
static bool OrdinaryFamily(Uint16 family)
{
    switch (family)
    {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 31:
    case 38:
    case 39:
    case 56:
    case 48:
    case 49:
    case 50:
    case 53:
    case 55:
    case 32:
    case 33:
    case 30:
    case 34:
    case 35:
    case 36:
    case 37:
    case 43:
    case 51:
    case 54:
    case 57:
    case 7:
    case 255:
        return true;
    default:
        return SotnHandWeapon_IsShieldFamily(family);
    }
}
bool SotnHandWeapon_Tick(SotnHandWeapon *out, SotnHandWeaponProgram *program,
                         SotnHandWeaponInput in, Uint32 *random, SotnHandWeaponEvents *events)
{
    if (!out || !program || !random || !events || out->removed || in.hand > 1 ||
        out->phase > (SotnHandWeapon_IsShieldFamily(in.family) ? 4
                      : in.family == 32                        ? 2
                                                               : 1) ||
        !OrdinaryFamily(in.family))
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
        // w_054 and w_056 swap to descriptor2 while the player's Dark Metamorphosis timer runs.
        const Uint16 variant = (in.family == 54 || in.family == 56) && in.dark_ticks
                                   ? 2
                                   : (Uint16)((w.parameter & 0x7fffU) >> 8);
        const Uint32 base = in.family == 255 ? 0x800a0000U : 0x8017a000U;
        // Weapon1 descriptor4 is Fist of Tulkas's QCF special188.
        const Uint16 count = in.family == 0    ? 5
                             : in.family == 1  ? 5
                             : in.family == 57 ? 4
                             : in.family == 54 ? 3
                             : in.family == 56 ? 3
                             : in.family == 55 ? 2
                             : in.family == 38 ? 2
                             : in.family == 53 ? 5
                             : in.family == 48 ? 2
                             : in.family == 49 ? 4
                             : in.family == 50 ? 4
                             : in.family == 39 ? 2
                             : in.family == 32 ? 2
                             : in.family == 51 ? 4
                             : in.family == 4  ? 2
                             : in.family == 5  ? 2
                             : in.family == 7  ? 2
                             : in.family == 43 ? 3
                             : in.family == 37 ? 4
                             : in.family == 31 ? 2
                                               : 1;
        const Uint32 descriptor = in.family == 255  ? 0xd53cU
                                  : in.family == 0  ? 0x106cU
                                  : in.family == 1  ? 0xf44U
                                  : in.family == 3  ? 0xc28U
                                  : in.family == 54 ? 0xec4U
                                  : in.family == 56 ? 0xf64U
                                  : in.family == 55 ? 0x10acU
                                  : in.family == 38 ? 0xdc0U
                                  : in.family == 53 ? 0xf4cU
                                  : in.family == 48 ? 0xfc0U
                                  : in.family == 49 ? 0xffcU
                                  : in.family == 50 ? 0xfc0U
                                  : in.family == 39 ? 0xdc0U
                                  : in.family == 32 ? 0x8f8U
                                  : in.family == 51 ? (in.special ? 0xd18U : 0xcd8U)
                                  : in.family == 4  ? 0x12f4U
                                  : in.family == 5  ? 0x12f4U
                                  : in.family == 7  ? 0xc28U
                                  : in.family == 33 ? 0xc3cU
                                  : in.family == 34 ? 0x934U
                                  : in.family == 30 ? 0x5e4U
                                  : in.family == 35 ? 0x530U
                                  : in.family == 36 ? 0x540U
                                  : in.family == 57 ? 0x106cU
                                  : in.family == 2  ? 0xbc4U
                                  : in.family == 6  ? 0xbc4U
                                  : in.family == 31 ? 0x8ecU
                                  : in.family == 37 ? 0x6a4U
                                                    : 0x1014U;
        const Uint8 *d = NULL, *p = NULL;
        // Estoc's variant8 reads w_051's D_8017AD58 words past the four-entry table as its
        // descriptor (frameStart0xD6).
        const bool estoc = in.family == 51 && !in.special && variant == 8;
        if ((variant >= count && !estoc) || (in.special && in.family != 51) ||
            !View(in.overlay, descriptor + variant * 16U, 16, &d))
            return SDL_SetError("Ordinary weapon descriptor bounds");
        // func_ptr_80170004 retires outside animations0xD6..0xD7 instead of the seven frames.
        const Uint16 window = in.special ? 2 : 7;
        w.x = in.x;
        w.y = in.y;
        w.facing_left = in.facing_left;
        // w_032 Combat knife lingers four updates in slot16 instead of destroying itself.
        const bool linger = in.family == 32;
        if (linger && (in.animation < d[12] || in.animation >= d[12] + window || !in.attack_mode) &&
            w.phase != 2)
        {
            w.phase = 2;
            w.timer = 4;
            w.frame = 0;
            w.hitbox_state = 0;
        }
        if (linger && w.phase == 2)
        {
            w.timer = Word(w.timer - 1);
            if (!w.timer)
                w.removed = true;
        }
        else if (!linger && (in.animation < d[12] || in.animation >= d[12] + window ||
                             (!in.special && !in.attack_mode)))
            w.removed = true;
        else
        {
            if (!w.phase)
            {
                // Weapon1 assigns flags only after its mandatory rows exist.
                if (in.family != 1)
                    w.flags = 0x60000;
                // Weapon1 sets no sprite bank, palette, priority or blend of its own.
                if (in.family != 255 && in.family != 1)
                {
                    // w_051 draws from DRA sprite bank10 with no per-hand bank offset.
                    w.bank =
                        in.family == 51 || in.family == 30 ? 10 : (Uint16)(0x8010U + in.hand * 2U);
                    // w_051's special adds subType*2 instead of its descriptors' zero palette.
                    w.palette = (Uint16)(0x110U + in.hand * 24U +
                                         (in.special ? variant * 2U : Read16(d + 8)));
                    w.texture = (Uint16)(100U + in.hand * 2U);
                    w.priority =
                        (Uint16)(in.priority +
                                 ((in.family == 31 || in.family == 32) && !variant ? 2 : -2));
                }
                // w_003/w_036/w_057 share w_002's callback but add BLEND_TRANSP|BLEND_ADD.
                if (in.family == 0 || in.family == 3 || in.family == 4 || in.family == 5 ||
                    in.family == 7 || in.family == 36 || in.family == 43 || in.family == 54 ||
                    in.family == 55 || in.family == 57 || in.family == 53 || in.family == 48 ||
                    in.family == 49 || in.family == 50 || in.family == 6 || in.family == 56)
                    w.blend = 0x30;
                // w_053 descriptors1/3 and w_030 use BLEND_TRANSP|BLEND_QUARTER.
                if ((in.family == 53 && (variant == 1 || variant == 3)) || in.family == 30)
                    w.blend = 0x70;
                else if (in.family == 35 || in.family == 34)
                    w.blend = 0x10; // w_035 Red Rust and w_034 Shotel: BLEND_TRANSP only.
                // w_051 descriptors1/2 add, descriptor3 is transparent; the glow restarts from
                // words read at D_8017AD58[subType*16-2/0/+2].
                if (in.family == 51 && variant)
                {
                    w.blend = variant == 3 ? 0x10 : variant <= 2 ? 0x30 : 0;
                    const Uint32 at = 0xd58U + (Uint32)(variant * 16 - 2) * 2U;
                    Sint16 red = 0, green = 0, blue = 0;
                    if (!GlowRead(in.overlay, &shared, at, &red) ||
                        !GlowRead(in.overlay, &shared, at + 4U, &green) ||
                        !GlowRead(in.overlay, &shared, at + 8U, &blue))
                        return false;
                    shared.glow_red = red;
                    shared.glow_green = green;
                    shared.glow_blue = blue;
                    shared.glow_step = 0;
                    shared.glow_alpha = 0;
                }
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
                if (in.family == 1 && !in.allocated)
                    w.removed = true;
                else if (in.family == 1 && !FistBirth(&w, in, d))
                    return false;
                // w_001 descriptor4 (Tulkas's QCF) plays SFX_VO_ALU_ATTACK_A after its rows.
                if (in.family == 1 && variant == 4 && !w.removed)
                    e.sound = 0x6ee;
                if ((in.family == 54 || in.family == 56) && in.allocated)
                    GurthangBirth(&w, in, &rng);
                // w_055 descriptor1 plays SFX_MAGIC_SWITCH and requests WFACTORY(0x38,0)
                // before SetWeaponProperties.
                if (in.family == 55 && variant == 1)
                {
                    e.sound = 0x69d;
                    e.factory = 0x38U | ((in.hand + 1U) << 12);
                }
                // w_048 descriptor1 requests the bolt root, three branches and the ring.
                if (in.family == 48 && variant)
                {
                    const Uint32 hand = (in.hand + 1U) << 12;
                    e.factory = 0x38U | hand;
                    e.more_factories[0] = 0x3aU | hand | 1U << 16;
                    e.more_factories[1] = 0x3aU | hand | 2U << 16;
                    e.more_factories[2] = 0x3aU | hand | 3U << 16;
                    e.more_factories[3] = 0x40U | hand;
                }
                // w_053 descriptors1/3/4 request WFACTORY(56,0/10/0x14) before properties.
                if (in.family == 53 && (variant == 1 || variant == 3 || variant == 4))
                    e.factory = 0x38U | ((in.hand + 1U) << 12) |
                                ((Uint32)(variant == 1   ? 0
                                          : variant == 3 ? 10
                                                         : 0x14)
                                 << 16);
                // w_049 descriptor1 requests the WFACTORY(0x5B,0) sparks; descriptor3 the
                // WFACTORY(0x5F,0) swirls and the WFACTORY(0x40,1) ring.
                if (in.family == 49 && (variant == 1 || variant == 3))
                {
                    const Uint32 hand = (in.hand + 1U) << 12;
                    e.factory = (variant == 1 ? 0x5bU : 0x5fU) | hand;
                    if (variant == 3)
                        e.more_factories[0] = 0x40U | hand | 1U << 16;
                }
                // w_050 descriptor1 plays SFX_MAGIC_WEAPON_APPEAR_A and requests the
                // WFACTORY(0x3A,0) sparkles and WFACTORY(0x40,0) ring before its properties.
                if (in.family == 50 && variant == 1)
                {
                    const Uint32 hand = (in.hand + 1U) << 12;
                    if (!AppendSound(&e, 0x669) || !AppendFactory(&e, 0x3aU | hand) ||
                        !AppendFactory(&e, 0x40U | hand))
                        return false;
                }
                if (!w.removed)
                {
                    if (!Properties(&w, in, &rng, &e))
                        return false;
                    if (in.family == 48 || in.family == 49 || in.family == 50)
                        shared.previous_pose = 0;
                    // w_054 descriptors1/2 double under Dark Metamorphosis and keep it at
                    // least0x38; descriptor1 also requests the profile0x11 outline factory.
                    // w_056 requests its WFACTORY(56,0) slash chain once the rolled attack
                    // reaches0x1E (Muramasa's blood counter raises it).
                    if (in.family == 56 && w.properties.attack >= 0x1e)
                        e.factory = 0x38U | ((in.hand + 1U) << 12);
                    if (in.family == 54 && variant)
                    {
                        if (in.dark_ticks)
                            w.properties.attack = Word(w.properties.attack * 2);
                        if (in.dark_ticks < 0x38)
                        {
                            e.dark_minimum = 0x38;
                            if (variant == 1)
                                e.outline = 0x11003dU;
                        }
                    }
                    ++w.phase;
                }
            }
            if (w.removed)
                goto commit;
            w.animation = (Uint16)(in.animation - d[12]);
            if (!in.special && in.remaining == 1 && in.pose == d[13])
            {
                // A same-update birth sound keeps its earlier place in the queue.
                if (e.sound)
                    e.extra_sounds[0] = Read16(d + 10);
                else
                    e.sound = Read16(d + 10);
                // w_004 Holy rod and w_005 Star flail add SFX_ANIME_SWORD_B.
                if (in.family == 4 || in.family == 5)
                    e.extra_sounds[0] = 0x640;
                // w_032 adds SFX_VO_ALU_ATTACK_B after a SWISH_B descriptor sound.
                if (in.family == 32 && e.sound == 0x60b)
                    e.extra_sounds[0] = 0x6ef;
            }
            // w_033 Nunchaku repeats its swing and adds SFX_WEAPON_SWISH_B at pose12.
            if (in.family == 33 && in.remaining == 1 && in.pose == 12)
            {
                e.extra_sounds[0] = Read16(d + 10);
                e.extra_sounds[1] = 0x60b;
            }
            if (in.pose >= 64 || !Address(in.overlay, base, Read32(d) + w.animation * 4U, 4, &p) ||
                !Address(in.overlay, base, Read32(p) + in.pose * 2U, 2, &p))
                return SDL_SetError("Ordinary weapon private player-frame bounds");
            const Uint16 packed = Read16(p);
            // UpdateUnarmedAnim < 0: FFFF before the hitbox write, negative timer after it.
            bool ended = packed == 65535U;
            if (!ended)
            {
                if (!Hitbox(&w, in.overlay, base, Read32(d + 4), packed))
                    return false;
                ended = in.remaining < 0;
            }
            if (ended && !linger)
                w.removed = true;
            else if (linger)
            {
                if (ended)
                {
                    w.phase = 2;
                    w.timer = 4;
                    w.frame = 0;
                    w.hitbox_state = 0;
                }
                if (w.animation < 4 && in.pose == 6)
                    w.hitbox_state = 4;
                Transform(&w, in);
            }
            else
            {
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
                    // w_051's special requests the WFACTORY(0x3E,0) fan root on pose1's last
                    // update.
                    if (in.special && in.pose == 1 && in.remaining == 1)
                        e.factory = 0x3eU | ((in.hand + 1U) << 12);
                    if (in.family != 255 && in.family != 1)
                        Transform(&w, in);
                    // The special keeps its glow still on pose4 once the colour started.
                    if (in.family == 51 && variant &&
                        (!in.special || in.pose != 4 || shared.glow_step == 0) &&
                        !Glow(in.overlay, &shared, in.hand, (Uint16)(variant - 1U), &e))
                        return false;
                    if (in.family == 43 && w.allocated)
                        Tiles(&w, in, &rng);
                    if (in.family == 1 && !FistTrails(&w, in, variant, &rng))
                        return false;
                    // Descriptor4 requests the WFACTORY(0x3E,0) fireball on pose2's last update.
                    if (in.family == 1 && variant == 4 && in.pose == 2 && in.remaining == 1)
                        e.factory = 0x3eU | ((in.hand + 1U) << 12);
                    if ((in.family == 54 || in.family == 56) && !GurthangTrails(&w, in))
                        return false;
                    // w_050's first pose1 after pose0 requests (descriptor1 only) WFACTORY(0x44,0)
                    // with SFX_FM_EXPLODE_B and SFX_MAGIC_SWITCH, then FACTORY(0x61, descriptor)
                    // with every descriptor.
                    if (in.family == 50)
                    {
                        if (!shared.previous_pose && in.pose == 1)
                        {
                            const Uint32 hand = (in.hand + 1U) << 12;
                            if (variant == 1 &&
                                (!AppendFactory(&e, 0x44U | hand) || !AppendSound(&e, 0x65b) ||
                                 !AppendSound(&e, 0x69d)))
                                return false;
                            if (!AppendFactory(&e, 0x61U | hand | (Uint32)variant << 16))
                                return false;
                        }
                        shared.previous_pose = in.pose;
                    }
                    // w_049 requests the WFACTORY(0x5D) puffs and SFX_FM_EXPLODE_B the first
                    // update pose1 follows pose0 (descriptors1/3 param0, descriptor2 param1).
                    if (in.family == 49)
                    {
                        if (!shared.previous_pose && in.pose == 1 && variant)
                        {
                            Uint16 n = 0;
                            while (n < 8 && (n ? e.more_factories[n - 1U] : e.factory))
                                ++n;
                            const Uint32 puffs =
                                0x5dU | ((in.hand + 1U) << 12) | (variant == 2 ? 1U << 16 : 0U);
                            if (!n)
                                e.factory = puffs;
                            else
                                e.more_factories[n - 1U] = puffs;
                            if (!e.sound)
                                e.sound = 0x65b;
                            else if (!e.extra_sounds[0])
                                e.extra_sounds[0] = 0x65b;
                            else
                                e.extra_sounds[1] = 0x65b;
                        }
                        shared.previous_pose = in.pose;
                    }
                    // w_048 requests four more bolts the first update pose1 follows pose0.
                    if (in.family == 48)
                    {
                        if (!shared.previous_pose && in.pose == 1)
                        {
                            Uint16 n = 0;
                            while (n < 8 && (n ? e.more_factories[n - 1U] : e.factory))
                                ++n;
                            const Uint32 hand = (in.hand + 1U) << 12,
                                         bolt = variant ? 0x20U : 0x10U;
                            const Uint32 bolts[4] = {
                                0x38U | hand | bolt << 16, 0x3aU | hand | (bolt + 1U) << 16,
                                0x3aU | hand | (bolt + 2U) << 16, 0x3aU | hand | (bolt + 3U) << 16};
                            for (Uint16 k = 0; k < 4; ++k)
                            {
                                if (!n)
                                    e.factory = bolts[k];
                                else
                                    e.more_factories[n - 1U] = bolts[k];
                                ++n;
                            }
                        }
                        shared.previous_pose = in.pose;
                    }
                    // w_007 descriptor1 (Shield rod's special173) requests its 0x4A orb with the
                    // weapon's animation index on pose4 while the player's poseTimer is7.
                    if (in.family == 7 && variant && in.pose == 4 && in.remaining == 7)
                        e.factory = 0x4aU | ((in.hand + 1U) << 12) | (Uint32)w.animation << 16;
                    // w_038/w_039 descriptor1 requests WFACTORY(0x38,0) on pose4's last update.
                    if ((in.family == 38 || in.family == 39) && variant && in.pose == 4 &&
                        in.remaining == 1)
                        e.factory = 0x38U | ((in.hand + 1U) << 12);
                    // w_005 Star flail widens its reach on poses6 and7 after the hitbox write.
                    if (in.family == 5 && in.pose >= 6 && in.pose < 8)
                    {
                        w.hitbox_x = Word(w.hitbox_x + 7);
                        w.hitbox_width = (Uint8)(w.hitbox_width + 7U);
                    }
                }
            }
        }
    }
commit:
    *out = w;
    *program = shared;
    *random = rng;
    *events = e;
    return true;
}
