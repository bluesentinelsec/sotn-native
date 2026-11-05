// SPDX-License-Identifier: AGPL-3.0-or-later
// weapon/w_020.c and dra/alu_anim.c:UpdateAnim, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. All artwork/tables remain private inputs.
#include <sotn/combat.h>
#include <sotn/karma_coin.h>
static Sint16 Word(Sint32 v)
{
    Uint16 n = (Uint16)(Uint32)v;
    return n < 32768 ? (Sint16)n : (Sint16)((Sint32)n - 65536);
}
static Sint32 Fixed(Uint32 v)
{
    return v <= SDL_MAX_SINT32 ? (Sint32)v : (Sint32)((Sint64)v - 4294967296LL);
}
static Sint32 Add(Sint32 a, Sint32 b)
{
    return Fixed((Uint32)a + (Uint32)b);
}
static Sint16 Pixel(Sint32 a)
{
    return Word((Sint32)((Uint32)a >> 16));
}
static Sint32 SetHigh(Sint32 a, Sint32 b)
{
    return Fixed(((Uint32)a & 65535U) | ((Uint32)(Uint16)b << 16));
}
static Sint32 Shift(Sint32 a, Uint16 n)
{
    Sint64 d = (Sint64)1 << n;
    return (Sint32)(a >= 0 ? a / d : -((-(Sint64)a + d - 1) / d));
}
static bool ReadWord(SotnDataView o, Uint32 at, Uint16 *out)
{
    if (!o.bytes || at > o.size || o.size - at < 2)
        return SDL_SetError("SotnKarmaCoin private word bounds");
    *out = (Uint16)(o.bytes[at] | (Uint16)o.bytes[at + 1] << 8);
    return true;
}
static bool ReadDouble(SotnDataView o, Uint32 at, Uint32 *out)
{
    Uint16 a = 0, b = 0;
    if (!ReadWord(o, at, &a) || !ReadWord(o, at + 2, &b))
        return false;
    *out = (Uint32)a | (Uint32)b << 16;
    return true;
}
bool SotnKarmaCoin_LoadProgram(SotnKarmaShared *shared, SotnDataView o)
{
    if (!shared)
        return SDL_InvalidParamError("shared");
    SotnKarmaShared n = {0};
    for (Uint16 i = 0; i < 24; ++i)
    {
        Uint32 v = 0;
        if (!ReadDouble(o, 0xadc + i * 4U, &v))
            return false;
        n.angles[i] = Fixed(v);
    }
    n.loaded = true;
    *shared = n;
    return true;
}
static void SetAnimation(SotnKarmaCoin *k, Uint16 offset)
{
    k->animation = offset;
    k->pose = 0;
    k->remaining = 0;
}
static Uint16 AnimationCount(Uint16 offset)
{
    switch (offset)
    {
    case 0x850:
        return 5;
    case 0x864:
        return 25;
    case 0x8c8:
    case 0x8d8:
        return 4;
    case 0x8e8:
        return 13;
    case 0x954:
        return 33;
    case 0x9d8:
        return 2;
    default:
        return 0;
    }
}
static bool Animate(SotnKarmaCoin *k, SotnDataView o, bool props)
{
    Uint16 count = AnimationCount(k->animation), duration = 0, frame = 0;
    if (!count || k->pose >= count || k->remaining < -1)
        return SDL_SetError("SotnKarmaCoin private animation cursor");
    if (k->remaining != -1)
    {
        if (!k->remaining)
        {
            if (!ReadWord(o, k->animation + k->pose * 4U, &duration))
                return false;
            k->remaining = Word(duration);
        }
        else if (!--k->remaining)
        {
            if (++k->pose >= count)
                return SDL_SetError("SotnKarmaCoin private animation end");
            if (!ReadWord(o, k->animation + k->pose * 4U, &duration))
                return false;
            if (!duration)
            {
                if (!ReadWord(o, k->animation + k->pose * 4U + 2, &k->pose) || k->pose >= count ||
                    !ReadWord(o, k->animation + k->pose * 4U, &duration))
                    return false;
                k->remaining = Word(duration);
            }
            else if (duration == 65535U)
            {
                --k->pose;
                k->remaining = -1;
            }
            else if (duration >= 32768U)
                return SDL_SetError("SotnKarmaCoin unsupported animation jump");
            else
                k->remaining = (Sint16)duration;
        }
    }
    if (!ReadWord(o, k->animation + k->pose * 4U + 2, &frame))
        return false;
    if (props)
    {
        size_t at = 0x9e0 + ((frame >> 9) & 127U) * 4U;
        if (!o.bytes || at > o.size || o.size - at < 4)
            return SDL_SetError("SotnKarmaCoin private hitbox bounds");
        k->hitbox_x = (Sint8)(o.bytes[at] < 128 ? o.bytes[at] : (Sint16)o.bytes[at] - 256);
        k->hitbox_y =
            (Sint8)(o.bytes[at + 1] < 128 ? o.bytes[at + 1] : (Sint16)o.bytes[at + 1] - 256);
        k->hitbox_width = o.bytes[at + 2];
        k->hitbox_height = o.bytes[at + 3];
    }
    k->frame = frame & 511U;
    return true;
}
static bool Query(SotnKarmaInput in, Sint32 x, Sint32 y, SotnCollider *out)
{
    return SotnCollision_Query(&in.collision, Word(x), Word(y), out);
}
static bool Head(SotnKarmaCoin *k, SotnKarmaInput in, Uint16 palette)
{
    if (!in.overlay.bytes || in.overlay.size < 0xadc)
        return SDL_SetError("SotnKarmaCoin private model bounds");
    SotnPlayerEffectQuad *q = &k->quads[0];
    for (Uint16 v = 0; v < 4; ++v)
        q->red[v] = q->green[v] = q->blue[v] = (Uint8)(v < 2 ? 95 : 31);
    q->priority = 444;
    k->quads[1] =
        (SotnPlayerEffectQuad){.x = {84, 172, 84, 172},
                               .y = {48, 48, 152, 152},
                               .u = {128, 215, 128, 215},
                               .v = {(Uint8)(in.hand * 128U), (Uint8)(in.hand * 128U),
                                     (Uint8)(in.hand * 128U + 103), (Uint8)(in.hand * 128U + 103)},
                               .priority = 436,
                               .page = 25,
                               .palette = palette,
                               .draw_mode = 8};
    for (Uint16 i = 0; i < 24; ++i)
    {
        q = &k->quads[i + 2];
        *q = (SotnPlayerEffectQuad){
            .page = 25, .palette = palette, .priority = 438, .draw_mode = 61};
        const Uint8 *p = in.overlay.bytes + 0xa1c + i * 8U;
        for (Uint16 j = 0; j < 4; ++j)
        {
            Uint16 v = (Uint16[]){0, 2, 1, 3}[j];
            q->x[v] = (Sint16)(p[j * 2] + 84);
            q->y[v] = (Sint16)(p[j * 2 + 1] + 48);
            q->u[v] = (Uint8)(p[j * 2] + 128);
            q->v[v] = (Uint8)(p[j * 2 + 1] + in.hand * 128U);
            q->red[v] = q->green[v] = q->blue[v] = 128;
        }
    }
    return true;
}
static bool Geometry(SotnKarmaCoin *k, SotnKarmaShared *s, SotnKarmaInput in, Uint16 palette)
{
    if (!in.overlay.bytes || in.overlay.size < 0xadc)
        return SDL_SetError("SotnKarmaCoin private model bounds");
    SotnPlayerEffectQuad *q = &k->quads[0];
    q->x[0] = 116;
    q->x[1] = 140;
    q->x[2] = Word(128 - k->width);
    q->x[3] = Word(128 + k->width);
    q->y[0] = q->y[1] = 0;
    q->y[2] = q->y[3] = 240;
    q->draw_mode = 0x435;
    k->quads[1].palette = palette;
    Sint16 last_x1 = 0, last_y1 = 0, last_x3 = 0, last_y3 = 0;
    for (Uint16 i = 0; i < 24; ++i)
    {
        q = &k->quads[i + 2];
        q->draw_mode &= 0xfff7U;
        const Uint8 *p = in.overlay.bytes + 0xa1c + i * 8U;
        SotnMathTrig a = {0}, b = {0};
        if (!SotnMath_Trig(in.executable, Word(s->angles[i]), &a) ||
            !SotnMath_Trig(in.executable, Word(s->angles[(i + 1U) % 24U]), &b))
            return false;
        if (!i)
        {
            Sint32 dy = -Shift(a.sine, 5) * k->scale_y / 256,
                   dx = Shift(a.cosine, 5) * k->scale_x / 256;
            q->x[0] = Word(p[0] + dx + 84);
            q->y[0] = Word(p[1] + dy + 48);
            q->x[2] = Word(p[2] + dx + 84);
            q->y[2] = Word(p[3] + dy + 48);
        }
        else
        {
            q->x[0] = last_x1;
            q->y[0] = last_y1;
            q->x[2] = last_x3;
            q->y[2] = last_y3;
        }
        Sint16 dy = Word(-Shift(b.sine, 5) * k->scale_y / 256),
               dx = Word(Shift(b.cosine, 5) * k->scale_x / 256);
        last_x1 = q->x[1] = Word(p[4] + dx + 84);
        last_y1 = q->y[1] = Word(p[5] + dy + 48);
        last_x3 = q->x[3] = Word(p[6] + dx + 84);
        last_y3 = q->y[3] = Word(p[7] + dy + 48);
        Uint8 color0 = (Uint8)((Shift(a.sine + 4096, 6) * k->brightness) / 256),
              color1 = (Uint8)((Shift(b.sine + 4096, 6) * k->brightness) / 256);
        for (Uint16 v = 0; v < 4; ++v)
            q->red[v] = q->green[v] = q->blue[v] = (v & 1U) ? color1 : color0;
        s->angles[i] = Add(s->angles[i], k->angular_speed);
    }
    return true;
}
bool SotnKarmaCoin_Tick(SotnKarmaCoin *state, SotnKarmaShared *shared, SotnKarmaInput in,
                        Uint32 *random, SotnKarmaEvents *out)
{
    if (!state || !shared || !random || !out || !shared->loaded || state->removed ||
        state->phase > (in.child ? 1 : 9) || in.hand > 1 || in.priority < 4 || in.priority > 511)
        return SDL_SetError("SotnKarmaCoin callback owner bounds");
    SotnKarmaCoin k = *state;
    SotnKarmaShared s = *shared;
    Uint32 rng = *random;
    SotnKarmaEvents e = {0};
    Uint16 pal = 0;
    if (!ReadWord(in.overlay, 0xa10 + (in.game_timer % 3U) * 2U, &pal))
        return false;
    if (in.child)
    {
        if (!in.parent_exists)
        {
            k.removed = true;
            goto done;
        }
        if (!k.phase)
        {
            k.bank = in.bank;
            k.texture = in.texture;
            k.flags = 0x08100000;
            k.priority = in.priority;
            if (Fixed(s.child_counter) % 4 == 1)
            {
                k.velocity_x = 4096;
                k.velocity_y = 17408;
                SetAnimation(&k, 0x9d8);
                k.x = SetHigh(k.x, 140);
                k.y = SetHigh(k.y, 100);
                k.priority = (Uint16)(in.priority - 4U);
            }
            else
            {
                k.priority = in.priority;
                SetAnimation(&k, 0x954);
                k.pose = (Uint16)(SotnCombat_Random(&rng) & 31U);
                k.x = SetHigh(k.x, (Fixed(s.child_counter) % 3) * 10 + 148);
                k.y = SetHigh(k.y, (Fixed(s.child_counter) % 3) * 10 + 76);
                k.velocity_x = 4864;
                k.velocity_y = (Sint32)(SotnCombat_Random(&rng) & 8191U) + 16384;
            }
            ++s.child_counter;
            ++k.phase;
        }
        k.palette = (Uint16)(pal + in.hand * 24U);
        k.x = Add(k.x, k.velocity_x);
        k.y = Add(k.y, k.velocity_y);
        goto done;
    }
    pal = (Uint16)((pal & 32767U) + in.hand * 24U);
    SotnCollider c = {0};
    switch (k.phase)
    {
    case 0:
        if (!in.allocated)
        {
            k.removed = true;
            break;
        }
        k.quads[0].draw_mode = 8;
        k.bank = (Uint16)(0x8010U + in.hand * 2U);
        k.palette = (Uint16)(0x110U + in.hand * 24U);
        k.texture = (Uint16)(100U + in.hand * 2U);
        k.priority = (Uint16)(in.priority - 2U);
        k.facing_left = in.facing_left;
        k.flags = 0x08900000;
        SetAnimation(&k, 0x850);
        k.velocity_x = (Sint32)(SotnCombat_Random(&rng) & 16383U) + 57344;
        if (k.facing_left)
            k.velocity_x = -k.velocity_x;
        k.velocity_y = -262144;
        k.facing_left = false;
        e.clear_shield = true;
        ++k.phase;
        break;
    case 1:
        k.x = Add(k.x, k.velocity_x);
        k.y = Add(k.y, k.velocity_y);
        k.velocity_y = Add(k.velocity_y, 10240);
        if (!Query(in, Pixel(k.x), Pixel(k.y) + 1, &c))
            return false;
        if (c.effects & 1)
        {
            k.y = SetHigh(k.y, Pixel(k.y) + c.top);
            SetAnimation(&k, 0x864);
            e.sound = 0x6a9;
            ++k.phase;
            break;
        }
        Sint16 offset = k.velocity_x < 0 ? -4 : 4;
        if (!Query(in, Pixel(k.x) + offset, Pixel(k.y), &c))
            return false;
        if (c.effects & 2)
        {
            k.x = SetHigh(k.x, Pixel(k.x) + (offset < 0 ? c.tile_right : c.tile_left));
            k.velocity_x = -(k.velocity_x / 2);
        }
        if (!Query(in, Pixel(k.x), Pixel(k.y) - 8, &c))
            return false;
        if (c.effects & 1)
        {
            k.y = SetHigh(k.y, Pixel(k.y) + c.bottom + 1);
            k.velocity_y = 65536;
            k.velocity_x /= 2;
        }
        break;
    case 2:
        if (k.pose % 8U == 5 && k.remaining == 1)
        {
            Uint32 volume = 0;
            if (k.volume_index < 0 || k.volume_index > 2 ||
                !ReadDouble(in.overlay, 0xb44 + (Uint32)k.volume_index * 4U, &volume))
                return SDL_SetError("SotnKarmaCoin bounce volume index");
            e.sound = 0x6a9;
            e.volume_sound = true;
            e.volume = (Uint16)volume;
            k.volume_index = Word(k.volume_index + 1);
        }
        if (k.remaining < 0)
        {
            SetAnimation(&k, (SotnCombat_Random(&rng) & 1U) ? 0x8c8 : 0x8d8);
            ++k.phase;
        }
        break;
    case 3:
        if (k.pose == 1 && k.remaining == 56)
            e.sound = 0x682;
        if (k.remaining < 0)
        {
            if (k.animation == 0x8d8)
            {
                e.use_ticks = 4;
                SetAnimation(&k, 0x8e8);
                k.priority = 438;
                k.flags &= ~0x100000U;
                e.flash = 7;
                e.sound = 0x665;
                e.properties = true;
                k.phase = 4;
            }
            else
            {
                e.protect_ticks = 4;
                k.draw_flags = 128;
                k.blend = 48;
                k.x = SetHigh(k.x, 128);
                k.y = SetHigh(k.y, 0);
                k.priority = 442;
                k.frame = 29;
                k.flags &= ~0x08100000U;
                k.timer = 224;
                k.opacity = 128;
                e.flash = 8;
                e.sound = 0x6b1;
                if (!Head(&k, in, pal))
                    return false;
                k.angular_speed = 64;
                k.brightness = 0;
                k.scale_x = k.scale_y = 256;
                k.width = 32;
                e.fixed_properties = true;
                k.phase = 5;
            }
        }
        break;
    case 4:
        if (k.remaining < 0)
        {
            k.removed = true;
            break;
        }
        if (!Animate(&k, in.overlay, true))
            return false;
        break;
    case 5:
        if (!(in.game_timer & 3U))
            k.width = Word(k.width + 1);
        if (k.width >= 97)
            k.width = 96;
        k.brightness = Word(k.brightness + 8);
        if (k.brightness > 288)
            k.brightness = 288;
        k.scale_x = Word(k.scale_x - 4);
        if (k.scale_x < 40)
        {
            k.scale_x = 40;
            e.factory = 84U | ((in.hand + 1U) << 12);
            ++k.phase;
        }
        k.scale_y = k.scale_x;
        if (!Geometry(&k, &s, in, pal))
            return false;
        break;
    case 6:
        if (!(in.game_timer & 3U))
            k.width = Word(k.width + 1);
        if (k.width >= 97)
            k.width = 96;
        k.scale_x = Word(k.scale_x - 1);
        if (k.scale_x < 0)
        {
            k.quads[1].draw_mode &= 0xfff7U;
            k.scale_x = 0;
            ++k.phase;
        }
        k.scale_y = k.scale_x;
        if (!Geometry(&k, &s, in, pal))
            return false;
        break;
    case 7:
        k.brightness = Word(k.brightness - 16);
        if (k.brightness < 0)
            k.brightness = 0;
        k.timer = Word(k.timer - 1);
        if (k.timer < 0)
            ++k.phase;
        if (!(in.game_timer & 3U))
            k.width = Word(k.width + 1);
        if (k.width >= 97)
            k.width = 96;
        if (!(k.timer & 1) && k.quads[0].blue[3] > 1)
            --k.quads[0].blue[3];
        k.quads[0].red[2] = k.quads[0].green[2] = k.quads[0].blue[2] = k.quads[0].red[3] =
            k.quads[0].green[3] = k.quads[0].blue[3];
        if (!Geometry(&k, &s, in, pal))
            return false;
        break;
    case 8:
        k.draw_flags |= 8;
        --k.opacity;
        if (k.opacity < 4)
            k.opacity = 4;
        if (k.quads[0].blue[1] > 8)
            k.quads[0].blue[1] -= 2;
        for (Uint16 v = 0; v < 2; ++v)
            k.quads[0].red[v] = k.quads[0].green[v] = k.quads[0].blue[v] = k.quads[0].blue[1];
        k.brightness = Word(k.brightness + 16);
        if (k.brightness > 288)
        {
            k.quads[1].draw_mode |= 8;
            e.sound = 0x636;
            ++k.phase;
        }
        if (!Geometry(&k, &s, in, pal))
            return false;
        break;
    case 9:
        k.draw_flags |= 8;
        k.opacity = (Uint8)(k.opacity - 2);
        if (k.opacity < 4)
            k.opacity = 4;
        if (k.quads[0].blue[1] >= 9)
            k.quads[0].blue[1] -= 2;
        for (Uint16 v = 0; v < 2; ++v)
            k.quads[0].red[v] = k.quads[0].green[v] = k.quads[0].blue[v] = k.quads[0].blue[1];
        k.brightness = Word(k.brightness - 8);
        if (k.brightness < 0)
            k.brightness = 0;
        k.scale_x = Word(k.scale_x + 12);
        if (k.scale_x > 512)
        {
            k.removed = true;
            break;
        }
        k.scale_y = k.scale_x;
        if (!Geometry(&k, &s, in, pal))
            return false;
        break;
    }
done:
    *state = k;
    *shared = s;
    *random = rng;
    *out = e;
    return true;
}

bool SotnKarmaCoin_Animate(SotnKarmaCoin *state, SotnDataView overlay)
{
    if (!state)
        return SDL_InvalidParamError("state");
    SotnKarmaCoin next = *state;
    if (!Animate(&next, overlay, false))
        return false;
    *state = next;
    return true;
}
