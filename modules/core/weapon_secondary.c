// SPDX-License-Identifier: AGPL-3.0-or-later
// Weapon func_ptr_80170004 secondaries: weapon/w_038.c, w_039.c, w_055.c and weapon/shared.h, plus
// dra/7879C.c
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
    Uint32 frame_start; /**< w_055 D_8017B0B8: the descriptor table's frameStart byte. */
    Uint32 timeline;    /**< w_055 private AnimationFrame list. */
    Uint16 timeline_count, rows;
    Uint16 bank, palette, texture;
} Layout;
static bool Program(Uint16 family, Layout *out)
{
    if (family == 55)
        *out = (Layout){.frame_start = 0x10b8,
                        .timeline = 0x1174,
                        .timeline_count = 5,
                        .rows = 64,
                        .bank = 0x8011,
                        .palette = 0x117,
                        .texture = 0x65};
    else if (family == 38 || family == 39)
        *out = (Layout){.rows = family == 38 ? 24 : 32};
    else if (family == 53)
        *out = (Layout){.rows = 1};
    else if (family == 48)
        *out = (Layout){.rows = 10};
    else
        return SDL_SetError("Weapon secondary requires a selected actual resident program");
    return true;
}

bool SotnWeaponSecondary_Supported(Uint16 family)
{
    return family == 55 || family == 38 || family == 39 || family == 53 || family == 48;
}
bool SotnWeaponSecondary_SupportedRole(Uint16 family, Uint16 role)
{
    return SotnWeaponSecondary_Supported(family) && (role == 1 || (role == 2 && family == 48));
}
Uint8 SotnWeaponSecondary_RoleRows(Uint16 family, Uint16 role)
{
    return !SotnWeaponSecondary_SupportedRole(family, role) ? 0
           : role == 2                                      ? 16
                                                            : SotnWeaponSecondary_Rows(family);
}
Uint8 SotnWeaponSecondary_Rows(Uint16 family)
{
    Layout layout = {0};
    return SotnWeaponSecondary_Supported(family) && Program(family, &layout) ? (Uint8)layout.rows
                                                                             : 0;
}

// Shared SetWeaponProperties with the raw parent's equip id; hit type4 keeps attack255.
static bool Properties(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, Uint32 *rng,
                       SotnWeaponSecondaryEvents *e)
{
    if (!in.properties || !in.properties(in.context, rng, &s->properties))
        return SDL_SetError("Weapon secondary requires actual equipment property ownership");
    s->hitbox_state = s->properties.hit_type;
    if (s->hitbox_state == 4)
        s->properties.attack = 255;
    e->properties = true;
    return true;
}
static Uint16 Rand(Uint32 *rng)
{
    return (Uint16)(SotnCombat_Random(rng) & 0x7fffU);
}

// w_038/w_039: the lunge sparks follow the player's screen position while step_s stays 0x5A.
static bool Lunge(SotnWeaponSecondary *s, SotnHandWeaponProgram *program,
                  SotnWeaponSecondaryInput in, Uint32 *rng, SotnWeaponSecondaryEvents *e)
{
    const Uint16 rows = SotnWeaponSecondary_Rows(in.family);
    if (!in.attack_mode || in.substep != 0x5a)
    {
        s->removed = true;
        return true;
    }
    if (!s->phase)
    {
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->allocated = true;
        for (Uint16 i = 0; i < rows; ++i)
        {
            SotnWeaponSecondaryRow *r = &s->rows[i];
            *r = (SotnWeaponSecondaryRow){.hidden = true};
            if (in.family == 38)
            {
                r->red[1] = (Uint8)((Rand(rng) & 0xfU) + 63U);
                r->green[1] = 255;
                r->blue[1] = 63;
                r->red[0] = 15;
                r->green[0] = 63;
                r->blue[0] = 15;
                r->timer = (Uint8)(i + 1U);
            }
            else
            {
                r->red[0] = r->red[1] = 0xbf;
                r->green[0] = r->green[1] = 0x3f;
                r->blue[0] = r->blue[1] = 0xff;
                r->timer = (Uint8)(i * 2U + 1U);
            }
        }
        s->row_priority = (Uint16)(in.priority + 4U);
        s->draw_mode = 0x331;
        s->flags = 0x860000U;
        if (!Properties(s, in, rng, e))
            return false;
        program->secondary_counter = 40;
        s->hitbox_width = 24;
        s->hitbox_height = 10;
        s->hitbox_x = 38;
        s->hitbox_y = -12;
        s->phase = 1;
    }
    else
    {
        Uint16 sound = 0;
        if ((program->secondary_counter & 7) == 7)
            e->sounds[sound++] = 0x640;
        program->secondary_counter = (Sint32)((Uint32)program->secondary_counter - 1U);
        if (!program->secondary_counter)
        {
            e->sounds[sound] = 0x6f0;
            s->removed = true;
            return true;
        }
    }
    const Sint16 x = Word(Word(Pixel(in.x) - in.camera_x) + (in.facing_left ? -34 : 34)),
                 y = Word(Word(Pixel(in.y) - in.camera_y) - 14 - ((in.draw_flags & 2U) ? 3 : 0));
    for (Uint16 i = 0; i < rows; ++i)
    {
        SotnWeaponSecondaryRow *r = &s->rows[i];
        if (r->phase == 0)
        {
            if (--r->timer)
                continue;
            const Sint16 px = Word(x + (Sint32)(Rand(rng) % 24U) - 12),
                         py = Word(y + (Sint32)(Rand(rng) % 18U) - 9);
            if (in.family == 38)
            {
                r->x[0] = Word(px + (in.facing_left ? 16 : -12));
                r->x[1] = Word(px + (in.facing_left ? -12 : 16));
                r->y[0] = r->y[1] = py;
                r->timer = 3;
            }
            else
            {
                // Row0 starts the chain; later rows continue from the previous retained end.
                const bool outward = i == 0 || (i & 1U) == 0;
                if (i == 0)
                {
                    r->x[0] = Word(px + (in.facing_left ? 16 : -16));
                    r->y[0] = py;
                }
                else
                {
                    r->x[0] = program->secondary_x[i - 1U];
                    r->y[0] = program->secondary_y[i - 1U];
                }
                r->x[1] = Word(px + ((in.facing_left != 0) == outward ? -16 : 16));
                r->y[1] = py;
                program->secondary_x[i] = r->x[1];
                program->secondary_y[i] = r->y[1];
                r->timer = 9;
            }
            r->hidden = false;
            ++r->phase;
        }
        else if (r->phase == 1 && !--r->timer)
        {
            if (in.family == 38)
            {
                r->timer = 2;
                r->phase = 0;
            }
            else
                ++r->phase;
            r->hidden = true;
        }
    }
    return true;
}

static bool View(SotnDataView o, Uint32 at, Uint32 size, const Uint8 **out)
{
    if (!o.bytes || at > o.size || o.size - at < size)
        return SDL_SetError("Weapon secondary private table is outside its program");
    *out = o.bytes + at;
    return true;
}
// w_053 (shared with w_056): one textured GT4 slash per child. Roots0/10/20/30/40 request a
// chained child every callback; nine callbacks complete it.
static bool Slash(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, Uint32 *rng,
                  SotnWeaponSecondaryEvents *e)
{
    const Uint16 params = (s->parameter >> 8) & 0x7fU;
    const Uint8 *config = NULL, *cluts = NULL;
    if (params >= 48 || !View(in.overlay, 0xfc4U + params * 14U, 14, &config) ||
        !View(in.overlay, 0xf9cU, 40, &cluts))
        return SDL_SetError("Weapon53 slash parameter exceeds its private table");
    const bool root = params == 0 || params == 10 || params == 20 || params == 30;
    if (!s->phase)
    {
        s->facing_left = in.facing_left;
        s->x = (Sint32)((Uint32)s->x + (Uint32)(in.facing_left ? -20 : 20) * 65536U);
        s->y = (Sint32)((Uint32)s->y + (Uint32)(in.crouch ? 11 : -5) * 65536U);
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->allocated = true;
        const Uint8 v = in.hand ? 0x80 : 0;
        SotnPlayerEffectQuad q = {
            .page = 0x19, .priority = (Uint16)(in.priority + 2U), .draw_mode = 0x137};
        q.u[0] = q.u[2] = Read16(config) ? 0xc0 : 0x80;
        q.u[1] = q.u[3] = Read16(config) ? 0x80 : 0xc0;
        q.v[0] = q.v[1] = Read16(config + 2) ? (Uint8)(v | 0x40U) : v;
        q.v[2] = q.v[3] = Read16(config + 2) ? v : (Uint8)(v | 0x40U);
        for (Uint16 i = 0; i < 4; ++i)
        {
            q.red[i] = config[10];
            q.green[i] = config[11];
            q.blue[i] = config[12];
        }
        s->quad = q;
        s->flags = 0x08820000U;
        if (root || (params > 0x27 && params <= 0x31))
        {
            if (!Properties(s, in, rng, e))
                return false;
            s->hitbox_width = 30;
            s->hitbox_height = 32;
        }
        if (!(params & 3U))
            e->sounds[0] = 0x60c;
        s->rotate = (Sint16)Read16(config + 8);
        s->lifetime = (Sint16)Read16(config + 6);
        s->age = 0;
        s->phase = 1;
    }
    if (root || params == 0x28)
        e->chain = 0x38U | ((in.hand + 1U) << 12) | ((Uint32)(s->age + params + 1U) << 16);
    s->quad.palette = (Uint16)((in.hand ? 0x18 : 0) +
                               Read16(cluts + (Read16(config + 4) ? 20U : 0U) + s->age * 2U));
    if (++s->age >= 9)
    {
        s->removed = true;
        return true;
    }
    if (params >= 0x1e && params < 0x32)
        s->hitbox_x = 0x12;
    else
    {
        // posX.i.hi only: the child keeps its own fraction.
        s->x = (Sint32)((Uint32)(Pixel(in.x) + (s->facing_left ? -0x26 : 0x26)) * 65536U |
                        ((Uint32)s->x & 0xffffU));
        s->y = (Sint32)((Uint32)(Pixel(in.y) + (in.crouch ? 0xb : -5)) * 65536U |
                        ((Uint32)s->y & 0xffffU));
    }
    // Corners at rotate+0x600, -0x400, -0x800, +0x400, scaled by the private radius.
    const Sint16 steps[4] = {0x600, -0x400, -0x800, 0x400};
    Sint16 angle = s->rotate;
    for (Uint16 i = 0; i < 4; ++i)
    {
        angle = Word(angle + steps[i]);
        SotnMathTrig t = {0};
        if (!SotnMath_Trig(in.executable, angle, &t))
            return false;
        const Sint32 dx = Asr(Asr(t.cosine, 4) * s->lifetime, 8),
                     dy = Asr(Asr(t.sine, 4) * s->lifetime, 8);
        s->quad.x[i] = Word(s->facing_left ? -dx : dx);
        s->quad.y[i] = Word(-dy);
    }
    return true;
}

// w_048 func_ptr_8017B810: a per-animation anchor/extent row (whole or halved extent).
static bool ThunderAnchor(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, bool half,
                          Sint16 *px, Sint16 *py)
{
    const Uint8 *row = NULL;
    if (in.animation < 0x41 || in.animation >= 0x48 ||
        !View(in.overlay, 0x1000U + (in.animation - 0x41U) * 8U, 8, &row))
        return SDL_SetError("Weapon48 anchor requires its private animation row");
    const Sint16 ax = (Sint16)Read16(row), ay = (Sint16)Read16(row + 2),
                 dx = (Sint16)Read16(row + 4), dy = (Sint16)Read16(row + 6);
    const Sint16 x = Word(Pixel(in.x) - in.camera_x), y = Word(Pixel(in.y) - in.camera_y);
    const Sint16 hx = half ? (Sint16)(dx / 2) : dx;
    s->x = (Sint32)((Uint32)(Word(x + (in.facing_left ? -ax : ax)) + in.camera_x) * 65536U |
                    ((Uint32)s->x & 0xffffU));
    s->y = (Sint32)((Uint32)(Word(y + ay) + in.camera_y) * 65536U | ((Uint32)s->y & 0xffffU));
    *px = in.facing_left ? (Sint16)-hx : hx;
    *py = half ? (Sint16)(dy / 2) : dy;
    return true;
}
// w_048 func_ptr_80170004: ten LINE_G2 lightning segments from the anchor toward its target.
static bool Lightning(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, Uint32 *rng,
                      SotnWeaponSecondaryEvents *e)
{
    if (in.animation < 65 || in.animation >= 72 || !in.attack_mode)
    {
        s->removed = true;
        return true;
    }
    const Uint16 params = (s->parameter >> 8) & 0x7fU;
    Sint16 x = 0, y = 0, tx = 0, ty = 0;
    if (params < 16)
    {
        x = Word(Word(Pixel(in.x) - in.camera_x) + (in.facing_left ? -12 : 12));
        y = Word(Word(Pixel(in.y) - in.camera_y) - 26);
        tx = x;
        ty = Word(y - 48);
    }
    else
    {
        Sint16 dx = 0, dy = 0;
        if (!ThunderAnchor(s, in, params < 32, &dx, &dy))
            return false;
        x = Word(Pixel(s->x) - in.camera_x);
        y = Word(Pixel(s->y) - in.camera_y);
        tx = Word(x + dx);
        ty = Word(y + dy);
    }
    s->facing_left = in.facing_left;
    if (in.draw_flags & 2U)
    {
        const Sint16 lift = (in.animation - 65 == 2 || in.animation - 65 == 3) ? 1 : 3;
        const Sint16 extra = params < 16 ? 3 : 0;
        y = Word(y - extra - lift);
        ty = Word(ty - extra - lift);
    }
    s->x = (Sint32)((Uint32)(x + in.camera_x) * 65536U | ((Uint32)s->x & 0xffffU));
    s->y = (Sint32)((Uint32)(y + in.camera_y) * 65536U | ((Uint32)s->y & 0xffffU));
    if (!s->phase)
    {
        const Uint8 *color = NULL;
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        if (!View(in.overlay, 0xfe0U + (params & 0xfU) * 8U, 6, &color))
            return false;
        s->allocated = true;
        s->flags = 0x860000U;
        const Uint32 red = Read16(color), blue = Read16(color + 2), green = Read16(color + 4);
        for (Uint16 i = 0; i < 10; ++i)
        {
            SotnWeaponSecondaryRow *r = &s->rows[i];
            *r = (SotnWeaponSecondaryRow){0};
            r->red[0] = (Uint8)(red * (10U - i) / 10U);
            r->blue[0] = (Uint8)(blue * (10U - i) / 10U);
            r->green[0] = (Uint8)(green * (10U - i) / 10U);
            r->red[1] = (Uint8)(red * (9U - i) / 10U);
            r->blue[1] = (Uint8)(blue * (9U - i) / 10U);
            r->green[1] = (Uint8)(green * (9U - i) / 10U);
            r->angle = (Sint16)SotnCombat_Random(rng);
        }
        s->row_priority = (Uint16)(in.priority + 4U);
        s->draw_mode = 0x335;
        if (!Properties(s, in, rng, e))
            return false;
        if (params < 16)
        {
            s->hitbox_x = 0;
            s->hitbox_y = -20;
            s->hitbox_width = 4;
            s->hitbox_height = 20;
            s->remaining = 40;
        }
        else
        {
            if (params >= 32)
            {
                if (!(params & 0xfU))
                {
                    s->hitbox_x = 32;
                    s->hitbox_y = 0;
                    s->hitbox_width = 32;
                    s->hitbox_height = 6;
                }
                else
                    s->hitbox_state = 0;
            }
            s->remaining = 6;
        }
        s->phase = 1;
    }
    else if (!(s->remaining = Word(s->remaining - 1)))
    {
        s->removed = true;
        return true;
    }
    const Sint32 dx = tx - x, dy = ty - y;
    Sint16 cx = x, cy = y;
    for (Uint16 i = 0; i < 10; ++i)
    {
        SotnWeaponSecondaryRow *r = &s->rows[i];
        r->x[0] = cx;
        r->y[0] = cy;
        const Sint16 angle = r->angle;
        r->angle = Word(r->angle + 768);
        SotnMathTrig t = {0};
        if (!SotnMath_Trig(in.executable, angle, &t))
            return false;
        r->x[1] = Word(Word(x + dx * (i + 1) / 10) + Asr(Asr(t.cosine, 4) * 3, 8));
        r->y[1] = Word(Word(y + dy * (i + 1) / 10) - Asr(Asr(t.sine, 4) * 3, 8));
        cx = r->x[1];
        cy = r->y[1];
    }
    return true;
}
// w_048 func_ptr_80170008: a sixteen-wedge GT4 ring that grows and fades its blue center.
static bool Ring(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, SotnWeaponSecondaryEvents *e)
{
    if (!(in.animation > 0x40 && in.animation < 0x48) || !in.attack_mode)
    {
        s->removed = true;
        return true;
    }
    const Sint16 x = Word(Word(Pixel(in.x) - in.camera_x) + (in.facing_left ? -12 : 12)),
                 y = Word(Word(Pixel(in.y) - in.camera_y) - 26 - ((in.draw_flags & 2U) ? 5 : 0));
    s->x = (Sint32)((Uint32)(x + in.camera_x) * 65536U | ((Uint32)s->x & 0xffffU));
    s->y = (Sint32)((Uint32)(y + in.camera_y) * 65536U | ((Uint32)s->y & 0xffffU));
    if (!s->phase)
    {
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->allocated = true;
        s->flags = 0x860000U;
        for (Uint16 i = 0; i < 16; ++i)
        {
            SotnPlayerEffectQuad q = {.page = 0x1a,
                                      .palette = 0x15f,
                                      .priority = (Uint16)(in.priority + 2U),
                                      .draw_mode = 0x335};
            for (Uint16 k = 0; k < 2; ++k)
            {
                SotnMathTrig t = {0};
                if (!SotnMath_Trig(in.executable, (Sint32)(i + k) * 256, &t))
                    return false;
                q.u[k] = (Uint8)(Asr(t.cosine * 2, 8) + 32);
                q.v[k] = (Uint8)(-32 - Asr(t.sine * 2, 8));
                if (q.v[k] < 4)
                    q.v[k] = 0xff;
            }
            q.u[2] = q.u[3] = 0x20;
            q.v[2] = q.v[3] = 0xe0;
            q.blue[2] = q.blue[3] = 0x80;
            s->quads[i] = q;
        }
        s->remaining = 4;
        s->age = 0x28;
        e->sounds[0] = 0x665;
        s->phase = 1;
    }
    else
    {
        if (s->remaining < 24)
            s->remaining = (Sint16)(s->remaining + 4);
        if (!--s->age)
        {
            s->removed = true;
            return true;
        }
    }
    const Uint8 blue = s->age < 16 ? (Uint8)(s->age * 8U) : 0x80;
    for (Uint16 i = 0; i < 16; ++i)
    {
        SotnPlayerEffectQuad *q = &s->quads[i];
        q->x[2] = q->x[3] = x;
        q->y[2] = q->y[3] = y;
        q->blue[2] = q->blue[3] = blue;
        for (Uint16 k = 0; k < 2; ++k)
        {
            SotnMathTrig t = {0};
            if (!SotnMath_Trig(in.executable, (Sint32)(i + k) * 256, &t))
                return false;
            q->x[k] = Word(x + Asr(Asr(t.cosine, 4) * s->remaining, 8));
            q->y[k] = Word(y - Asr(Asr(t.sine, 4) * s->remaining, 8));
        }
    }
    return true;
}

bool SotnWeaponSecondary_Tick(SotnWeaponSecondary *state, SotnHandWeaponProgram *program,
                              SotnWeaponSecondaryInput in, Uint32 *random,
                              SotnWeaponSecondaryEvents *events)
{
    Layout layout = {0};
    if (!state || !program || !random || !events || in.hand > 1 || in.role > 2 ||
        !Program(in.family, &layout) ||
        !SotnWeaponSecondary_SupportedRole(in.family, in.role ? in.role : 1) || state->removed ||
        state->phase > 1 || state->remaining < -1)
        return SDL_SetError("Weapon secondary requires its actual live bounded child");
    const size_t at = layout.frame_start + ((state->parameter >> 4) & 0x7f0U);
    if (in.family == 55 && (!in.overlay.bytes || at >= in.overlay.size))
        return SDL_SetError("Weapon secondary descriptor frameStart is outside its program");
    SotnWeaponSecondary s = *state;
    SotnHandWeaponProgram shared = *program;
    SotnWeaponSecondaryEvents e = {0};
    Uint32 rng = *random;
    if (in.family == 53)
    {
        if (!Slash(&s, in, &rng, &e))
            return false;
        goto commit;
    }
    if (in.family == 48)
    {
        if (!(in.role == 2 ? Ring(&s, in, &e) : Lightning(&s, in, &rng, &e)))
            return false;
        goto commit;
    }
    s.x = in.x;
    s.y = in.y;
    s.facing_left = in.facing_left;
    if (in.family != 55)
    {
        if (!Lunge(&s, &shared, in, &rng, &e))
            return false;
        goto commit;
    }
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
            *r = (SotnWeaponSecondaryRow){.red = {0, 127}, .green = {0, 127}, .blue = {0, 255}};
            r->angle = (Sint16)(SotnCombat_Random(&rng) & 0xfffU);
            r->length = (Sint16)((SotnCombat_Random(&rng) & 0xffU) + 512U);
            r->growth = (Sint16)(r->length / 2);
        }
        s.row_priority = (Uint16)(in.priority + 2U);
        s.draw_mode = 0x331;
        s.bank = (Uint16)(layout.bank + in.hand * 2U);
        s.palette = (Uint16)(layout.palette + in.hand * 24U);
        s.texture = (Uint16)(layout.texture + in.hand * 2U);
        s.flags = 0x960000U;
        s.pose = 0;
        s.remaining = 0;
        s.frame = 0;
        s.priority = (Uint16)(in.priority + 4U);
        s.blend = 0x30;
        if (!Properties(&s, in, &rng, &e))
            return false;
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
        if (r->red[1] >= 17)
            r->red[1] = (Uint8)(r->red[1] - 12U);
        if (r->green[1] >= 17)
            r->green[1] = (Uint8)(r->green[1] - 12U);
        if (r->blue[1] >= 17)
            r->blue[1] = (Uint8)(r->blue[1] - 24U);
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
    *program = shared;
    *random = rng;
    *events = e;
    return true;
}

bool SotnWeaponSecondary_Animate(SotnWeaponSecondary *state, SotnDataView overlay, Uint16 family)
{
    Layout layout = {0};
    if (!state || !Program(family, &layout) || family != 55 || state->removed || !state->phase ||
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
