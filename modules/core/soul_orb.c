// SPDX-License-Identifier: AGPL-3.0-or-later
// st/e_particles.h:EntitySoulStealOrb and st_common.h:LimitAngleChange,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Stage/SDK tables stay private.
#include <SDL3/SDL_error.h>
#include <sotn/soul_orb.h>

static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return bits < 32768U ? (Sint16)bits : (Sint16)((Sint32)bits - 65536);
}
static Sint32 Fixed(Uint32 bits)
{
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
static Sint16 Pixel(Sint32 value)
{
    return Word((Sint32)((Uint32)value >> 16));
}
static Uint16 Limit(Uint16 delta, Uint16 base, Uint16 target)
{
    const Uint16 diff = (Uint16)(target - base);
    const Uint16 magnitude = (diff & 0x800U) ? (Uint16)((0x800U - diff) & 0x7ffU) : diff;
    return magnitude > delta ? (Uint16)((diff & 0x800U) ? base - delta : base + delta) : target;
}
static bool Animate(SotnSoulOrb *s, const Uint8 *timeline)
{
    if (s->pose >= 7 || s->remaining < 0 || s->remaining > 254)
        return SDL_SetError("Soul orb animation lost its bounded private cursor");
    const Uint8 *record = timeline + s->pose * 2U;
    if (!s->remaining)
    {
        if (!record[0])
        {
            if (!timeline[0] || timeline[0] == 255 || timeline[1] < 1 || timeline[1] > 6)
                return SDL_SetError("Soul orb animation has an invalid loop record");
            s->remaining = timeline[0];
            s->frame = timeline[1];
            s->pose = 1;
            return true;
        }
        if (record[0] == 255 || record[1] < 1 || record[1] > 6)
            return SDL_SetError("Soul orb animation has an invalid private frame");
        s->remaining = record[0];
        s->frame = record[1];
        ++s->pose;
    }
    if (!s->pose || s->frame < 1 || s->frame > 6)
        return SDL_SetError("Soul orb animation has no current private frame");
    --s->remaining;
    return true;
}
bool SotnSoulOrb_Tick(SotnSoulOrb *orb, SotnSoulOrbInput in, Uint32 *random, SotnSoulOrbEvents *out)
{
    if (!orb || !random || !out || orb->removed || orb->phase > 1)
        return SDL_SetError("Soul orb requires an actual bounded actor/RNG/output snapshot");
    SotnSoulOrb s = *orb;
    SotnSoulOrbEvents events = {0};
    Uint32 rng = *random;
    if (!s.phase && !in.allocated)
    {
        s.removed = true;
        goto done;
    }
    if (!in.allocated || !in.tables.bytes || in.tables.size != 126)
        return SDL_SetError("Soul orb requires its real row and complete private stage tables");
    Sint16 target = 0;
    if (!s.phase)
    {
        s.x = in.x;
        s.y = in.y;
        if (!SotnMath_Atan2Vector(in.executable, in.player_y - Pixel(s.y), in.player_x - Pixel(s.x),
                                  &target))
            return false;
        rng = rng * 0x01010101U + 1U;
        const Uint16 angle = Read16(in.tables.bytes + ((rng >> 24) & 7U) * 2U);
        const bool subtract = (Pixel(s.y) > 112) != (Pixel(s.x) < in.player_x);
        s.angle = (Uint16)(subtract ? (Uint16)target - angle : (Uint16)target + angle);
        s.speed = 0x400;
        s.turn = 0;
        s.hitbox = false;
        s.phase = 1;
        goto done;
    }
    s.age = (Uint16)(s.age + 1U);
    if (s.age == 16)
        s.hitbox = true;
    if (in.hit)
    {
        events.contact = true;
        events.heal = in.heal_pending ? 0 : 8;
        s.removed = true;
        goto done;
    }
    if (Word(s.scale_x) < 256)
        s.scale_x = s.scale_y = (Uint16)(s.scale_y + 16U);
    if (s.turn < 0x200)
        s.turn = (Uint16)(s.turn + 2U);
    if (s.speed < 0x800)
        s.speed = (Uint16)(s.speed + 4U);
    if (!SotnMath_Atan2Vector(in.executable, in.player_y - Pixel(s.y), in.player_x - Pixel(s.x),
                              &target))
        return false;
    s.angle = Limit(s.turn, s.angle, (Uint16)target);
    SotnMathTrig trig = {0};
    if (!SotnMath_Trig(in.executable, s.angle, &trig))
        return false;
    s.velocity_x = trig.cosine * Word(s.speed) / 16;
    s.velocity_y = trig.sine * Word(s.speed) / 16;
    s.x = Fixed((Uint32)s.x + (Uint32)s.velocity_x);
    s.y = Fixed((Uint32)s.y + (Uint32)s.velocity_y);
    if (!Animate(&s, in.tables.bytes + 112))
        return false;
    const Uint8 *shape = in.tables.bytes + 16U + (s.frame - 1U) * 16U;
    const Sint16 x = Word(Pixel(s.x) + Word(Read16(shape))),
                 y = Word(Pixel(s.y) + Word(Read16(shape + 2))),
                 right = Word(x + Word(Read16(shape + 4))),
                 bottom = Word(y + Word(Read16(shape + 6)));
    for (Uint16 i = 0; i < 4; ++i)
    {
        events.x[i] = (i & 1U) ? right : x;
        events.y[i] = (i & 2U) ? bottom : y;
        events.u[i] = (Uint8)Read16(shape + ((i & 1U) ? 12 : 8));
        events.v[i] = (Uint8)Read16(shape + ((i & 2U) ? 14 : 10));
    }
    events.draw = true;
done:
    *orb = s;
    *random = rng;
    *out = events;
    return true;
}
bool SotnSoulOrb_Translate(SotnSoulOrb *s, Sint32 dx, Sint32 dy)
{
    if (!s || dx < -32768 || dx > 32767 || dy < -32768 || dy > 32767)
        return SDL_SetError("Soul orb rebase requires signed-word pixel shifts");
    s->x = Fixed((Uint32)s->x + (Uint32)dx * 65536U);
    s->y = Fixed((Uint32)s->y + (Uint32)dy * 65536U);
    return true;
}
