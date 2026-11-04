// SPDX-License-Identifier: AGPL-3.0-or-later
// Weapon func_ptr_80170004 secondaries: weapon/w_055.c and weapon/shared.h, plus dra/7879C.c
// UpdateAnim, sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Descriptor, timeline and
// artwork tables remain private inputs.
#include <sotn/combat.h>
#include <sotn/game_math.h>
#include <sotn/weapon_secondary.h>

static Sint16 Word(Sint32 n)
{
    const Uint16 v = (Uint16)(Uint32)n;
    return v < 32768 ? (Sint16)v : (Sint16)((Sint32)v - 65536);
}
static Sint16 Pixel(Sint32 n)
{
    return Word((Sint32)((Uint32)n >> 16));
}
static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
// Portable arithmetic right shift matching the MIPS sra the original compiles to.
static Sint32 Asr(Sint32 v, unsigned int n)
{
    return v >= 0 ? v >> n : -(Sint32)(((Uint32)(-(v + 1)) >> n) + 1U);
}

typedef struct Layout
{
    Uint32 frame_start; /**< D_8017B0B8: the descriptor table's frameStart byte. */
    Uint32 timeline;    /**< Private AnimationFrame list. */
    Uint16 timeline_count;
    Uint16 bank, palette, texture;
} Layout;
static bool Program(Uint16 family, Layout *out)
{
    if (family != 55)
        return SDL_SetError("Weapon secondary requires a selected actual resident program");
    *out = (Layout){.frame_start = 0x10b8,
                    .timeline = 0x1174,
                    .timeline_count = 5,
                    .bank = 0x8011,
                    .palette = 0x117,
                    .texture = 0x65};
    return true;
}

bool SotnWeaponSecondary_Supported(Uint16 family)
{
    return family == 55;
}
Uint8 SotnWeaponSecondary_Rows(Uint16 family)
{
    return family == 55 ? SOTN_WEAPON_SECONDARY_ROWS : 0;
}

bool SotnWeaponSecondary_Tick(SotnWeaponSecondary *state, SotnWeaponSecondaryInput in,
                              Uint32 *random, SotnWeaponSecondaryEvents *events)
{
    Layout layout = {0};
    if (!state || !random || !events || in.hand > 1 || !Program(in.family, &layout) ||
        state->removed || state->phase > 1 || state->remaining < -1)
        return SDL_SetError("Weapon secondary requires its actual live bounded child");
    const size_t at = layout.frame_start + ((state->parameter >> 4) & 0x7f0U);
    if (!in.overlay.bytes || at >= in.overlay.size)
        return SDL_SetError("Weapon secondary descriptor frameStart is outside its program");
    SotnWeaponSecondary s = *state;
    SotnWeaponSecondaryEvents e = {0};
    Uint32 rng = *random;
    s.x = in.x;
    s.y = in.y;
    s.facing_left = in.facing_left;
    s.draw_flags = in.draw_flags;
    s.scale_y = in.scale_y;
    s.pivot_y = in.pivot_y;
    const Uint8 start = in.overlay.bytes[at];
    if (in.animation < start || in.animation >= start + 7U || !in.attack_mode)
    {
        s.removed = true;
        goto commit;
    }
    const Sint32 x = Word(Pixel(s.x) - in.camera_x) + (in.facing_left ? -12 : 12),
                 y = Word(Pixel(s.y) - in.camera_y) - 28;
    if (!s.phase)
    {
        if (!in.allocated)
        {
            s.removed = true;
            goto commit;
        }
        s.allocated = true;
        for (Uint16 i = 0; i < SOTN_WEAPON_SECONDARY_ROWS; ++i)
        {
            SotnWeaponSecondaryRow *r = &s.rows[i];
            *r = (SotnWeaponSecondaryRow){.red = 127, .green = 127, .blue = 255};
            r->angle = (Sint16)(SotnCombat_Random(&rng) & 0xfffU);
            r->length = (Sint16)((SotnCombat_Random(&rng) & 0xffU) + 512U);
            r->growth = (Sint16)(r->length / 2);
        }
        s.row_priority = (Uint16)(in.priority + 2U);
        s.bank = (Uint16)(layout.bank + in.hand * 2U);
        s.palette = (Uint16)(layout.palette + in.hand * 24U);
        s.texture = (Uint16)(layout.texture + in.hand * 2U);
        s.flags = 0x960000U;
        s.pose = 0;
        s.remaining = 0;
        s.frame = 0;
        s.priority = (Uint16)(in.priority + 4U);
        s.blend = 0x30;
        if (!in.properties || !in.properties(in.context, &rng, &s.properties))
            return SDL_SetError("Weapon secondary requires actual equipment property ownership");
        s.hitbox_state = s.properties.hit_type;
        if (s.hitbox_state == 4)
            s.properties.attack = 255;
        e.properties = true;
        s.hitbox_width = s.hitbox_height = 32;
        s.hitbox_x = 12;
        s.hitbox_y = -28;
        s.phase = 1;
    }
    else if (s.frame == 1 && s.remaining == 1)
    {
        s.hitbox_width = s.hitbox_height = 12;
        s.hitbox_x = 13;
        s.hitbox_y = -28;
    }
    if (s.remaining < 0)
    {
        s.removed = true;
        goto commit;
    }
    for (Uint16 i = 0; i < SOTN_WEAPON_SECONDARY_ROWS; ++i)
    {
        SotnWeaponSecondaryRow *r = &s.rows[i];
        if (r->red >= 17)
            r->red = (Uint8)(r->red - 12U);
        if (r->green >= 17)
            r->green = (Uint8)(r->green - 12U);
        if (r->blue >= 17)
            r->blue = (Uint8)(r->blue - 24U);
        SotnMathTrig trig = {0};
        if (!SotnMath_Trig(in.executable, r->angle, &trig))
            return false;
        const Sint32 scale = r->length;
        r->length = Word(r->length + r->growth);
        r->x[0] = Word(x);
        r->y[0] = Word(y);
        r->x[1] = Word(x + Asr(Asr(trig.cosine, 8) * scale, 8));
        r->y[1] = Word(y - Asr(Asr(trig.sine, 8) * scale, 8));
    }
commit:
    *state = s;
    *random = rng;
    *events = e;
    return true;
}

bool SotnWeaponSecondary_Animate(SotnWeaponSecondary *state, SotnDataView overlay, Uint16 family)
{
    Layout layout = {0};
    if (!state || !Program(family, &layout) || state->removed || !state->phase ||
        state->pose >= layout.timeline_count || state->remaining < -1 || !overlay.bytes ||
        overlay.size < layout.timeline + layout.timeline_count * 4U)
        return SDL_SetError("Weapon secondary animation requires its actual private timeline");
    const Uint8 *t = overlay.bytes + layout.timeline;
    SotnWeaponSecondary s = *state;
    if (s.remaining == 0)
    {
        const Uint16 duration = Read16(t + s.pose * 4U);
        if (!duration || duration > 32767)
            return SDL_SetError("Weapon secondary animation has an invalid birth duration");
        s.remaining = (Sint16)duration;
    }
    else if (s.remaining != -1 && --s.remaining == 0)
    {
        if (++s.pose >= layout.timeline_count)
            return SDL_SetError("Weapon secondary animation escaped its private timeline");
        Uint16 duration = Read16(t + s.pose * 4U);
        if (duration == 0xffffU)
        {
            --s.pose;
            s.remaining = -1;
        }
        else
        {
            if (!duration)
            {
                s.pose = Read16(t + s.pose * 4U + 2U);
                if (s.pose >= layout.timeline_count)
                    return SDL_SetError("Weapon secondary animation has an invalid loop target");
                duration = Read16(t + s.pose * 4U);
            }
            if (!duration || duration > 32767)
                return SDL_SetError("Weapon secondary animation has an unsupported control");
            s.remaining = (Sint16)duration;
        }
    }
    s.frame = Read16(t + s.pose * 4U + 2U) & 511U;
    *state = s;
    return true;
}
