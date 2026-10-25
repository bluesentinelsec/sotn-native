// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted from sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5,
// src/boss/bo4/unk_49A6C.c:EntityDopplegangerBlinkWhite and dra/4B758.c:func_800EB758.
// Original frame, effect-profile and SDK tables remain private.
#include "actors_internal.h"

typedef struct BlinkImage
{
    Sint16 pivot_x, pivot_y;
    Uint16 width, height;
} BlinkImage;
static bool Resolve(SotnDataView image, Uint32 address, size_t length, Uint32 alignment,
                    SotnDataView *out)
{
    if (address < 0x80180000 || address % alignment)
        return SDL_SetError("BO4 blink references an invalid private address");
    return Slice(image, address - 0x80180000, length, out);
}
static bool Image(SotnActors *s, const SotnDoppleBody *body, BlinkImage *out)
{
    SotnDataView header = {0}, banks = {0}, frames = {0}, first = {0}, definition = {0},
                 sheets = {0}, sheet = {0};
    const Uint16 bank = body->animation_set & 0x7fffU, frame = body->animation.sprite & 0x7fffU;
    if ((bank != 1 && bank != 2) || !(body->animation_set & 0x8000U) ||
        !Slice(s->nz0, 0, 52, &header) ||
        !Resolve(s->nz0, Read32(header.bytes + 20), 24 * 4, 4, &banks))
        return SDL_SetError("BO4 blink requires its packed body or bat frame bank");
    const Uint32 frame_table = Read32(banks.bytes + bank * 4U);
    if (!Resolve(s->nz0, frame_table, 8, 4, &first))
        return false;
    const Uint32 frame_first = Read32(first.bytes + 4);
    if (frame_first <= frame_table || (frame_first - frame_table) % 4 ||
        (frame_first - frame_table) / 4 > 1024 || frame >= (frame_first - frame_table) / 4 ||
        !Resolve(s->nz0, frame_table, frame_first - frame_table, 4, &frames) ||
        !Resolve(s->nz0, Read32(frames.bytes + frame * 4U), 6, 2, &definition) ||
        !(Read16(definition.bytes) & 0x8000U))
        return SDL_SetError("BO4 blink frame is outside its packed definitions");
    const Uint32 table = Read32(header.bytes + (bank == 1 ? 44 : 48));
    if (!Resolve(s->nz0, table, 4, 4, &first))
        return false;
    const Uint32 sheet_first = Read32(first.bytes), index = Read16(definition.bytes) & 0x7fffU;
    if (sheet_first <= table || (sheet_first - table) % 4 || (sheet_first - table) / 4 > 1024 ||
        index >= (sheet_first - table) / 4 ||
        !Resolve(s->nz0, table, sheet_first - table, 4, &sheets) ||
        !Resolve(s->nz0, Read32(sheets.bytes + index * 4U), 4, 4, &sheet))
        return SDL_SetError("BO4 blink sheet is missing its original backing image");
    const Uint16 width = sheet.bytes[0], height = sheet.bytes[1];
    if (!width || width % 4 || !height ||
        !Resolve(s->nz0, Read32(sheets.bytes + index * 4U), 4 + (size_t)width * height / 2, 4,
                 &sheet))
        return SDL_SetError("BO4 blink sheet pixels are truncated");
    Sint32 x = Signed16(definition.bytes + 2) + sheet.bytes[2];
    const Sint32 y = Signed16(definition.bytes + 4) + sheet.bytes[3];
    if (bank == 2)
    {
        SotnDataView pivot = {0};
        if (frame >= 18 || !Slice(s->nz0, 0x11e8 + frame * 2U, 2, &pivot))
            return SDL_SetError("BO4 blink bat pose exceeds its original pivot table");
        x = Signed16(pivot.bytes) + body->bat_offset + sheet.bytes[2];
    }
    if (x < -1024 || x > 1024 || y < -1024 || y > 1024)
        return SDL_SetError("BO4 blink pivot exceeds its supported sprite geometry");
    *out = (BlinkImage){(Sint16)x, (Sint16)y, width, height};
    return true;
}
static bool Rotate(SotnActors *s, const SotnDoppleBody *body, Sint32 *x, Sint32 *y)
{
    if (!body->rotated)
        return true;
    const Sint64 square = (Sint64)*x * *x + (Sint64)*y * *y;
    Uint32 distance = 0;
    Sint16 direction = 0;
    SotnMathTrig trig = {0};
    if (square > 0x7ffff)
        return SDL_SetError("BO4 blink vertex exceeds its original signed SDK radius");
    if (!SotnMath_Sqrt12(s->executable, (Uint32)square * 4096U, &distance) ||
        !SotnMath_Atan2(s->executable, *y, *x, &direction) ||
        !SotnMath_Trig(s->executable,
                       direction + (body->facing_left ? -body->rotation : body->rotation), &trig))
        return false;
    *x = ActorShift((Sint64)ActorShift(trig.cosine, 4) * distance + 524288, 20);
    *y = ActorShift((Sint64)ActorShift(trig.sine, 4) * distance + 524288, 20);
    return true;
}
static bool Sin(SotnActors *s, Uint32 angle, Sint32 *out)
{
    SotnMathTrig trig = {0};
    if (!SotnMath_Trig(s->executable, (Sint32)(angle & 4095U), &trig))
        return false;
    *out = trig.sine;
    return true;
}
static bool Channel(SotnActors *s, Uint32 angle, Sint16 brightness, Sint16 denominator, Uint8 *out)
{
    Sint32 sine = 0;
    if (denominator <= 0 || !Sin(s, angle, &sine))
        return SDL_SetError("BO4 blink color has invalid private phase or divisor");
    *out = (Uint8)(((sine + 4096) / 64) * brightness / denominator);
    return true;
}
static bool Geometry(SotnActors *s, Actor *a, BlinkImage image, const Sint16 *profile,
                     Uint16 parameter)
{
    const SotnDoppleBody *body = &s->dopple.body;
    const Sint32 left = body->facing_left ? -image.pivot_x : image.pivot_x;
    const Sint32 top = image.pivot_y, width = image.width, height = image.height;
    a->blink.angle = (Uint16)(a->blink.angle + a->blink.speed);
    Uint16 wave = a->blink.wave;
    for (Uint16 i = 0; i < 8; ++i)
    {
        ActorPrimitive *primitive = ActorPrimitiveGet(s, a, i);
        if (primitive == NULL)
            return false;
        SotnActorPrimitive *p = &primitive->state;
        Sint32 x[4] = {0}, y[4] = {0};
        const bool fan = (parameter & 0x40U) != 0;
        if (fan)
        {
            const Sint32 px[9] = {0, width / 2, width, width, width, width / 2, 0, 0, 0};
            const Sint32 py[9] = {0, 0, 0, height / 2, height, height, height, height / 2, 0};
            for (Uint16 vertex = 0; vertex < 2; ++vertex)
            {
                const Uint16 edge = (Uint16)(i + vertex);
                x[vertex] = left + (body->facing_left ? -px[edge] : px[edge]);
                y[vertex] = top + py[edge];
                p->u[vertex] = (Uint8)(4 + px[edge]);
                p->v[vertex] = (Uint8)(1 + py[edge]);
            }
            SotnMathTrig center = {0};
            if (!SotnMath_Trig(s->executable, a->blink.angle, &center))
                return false;
            const Sint32 sway_x =
                profile[7] == 0x700a ? 0 : ActorShift((Sint64)ActorShift(center.cosine, 4) * 3, 12);
            const Sint32 sway_y =
                profile[7] == 0x700a ? 0 : ActorShift((Sint64)ActorShift(center.sine, 4) * 6, 8);
            x[2] = x[3] = left + (body->facing_left ? -width / 2 : width / 2) + sway_x;
            y[2] = y[3] = top + height / 2 - sway_y;
            p->u[2] = p->u[3] = (Uint8)(4 + width / 2);
            p->v[2] = p->v[3] = (Uint8)(1 + height / 2);
        }
        else
        {
            x[0] = x[2] = body->facing_left ? left - width + 1 : left;
            x[1] = x[3] = body->facing_left ? left + 1 : left + width;
            if (parameter == 32 || parameter == 33)
            {
                Sint32 sine = 0;
                if (!Sin(s, wave, &sine))
                    return false;
                const Sint32 upper = ActorShift(sine, 7) * a->blink.sway / 256;
                x[0] += upper;
                x[1] += upper;
                wave = (Uint16)(wave + 0x600);
                if (!Sin(s, wave, &sine))
                    return false;
                const Sint32 lower = ActorShift(sine, 7) * a->blink.sway / 256;
                x[2] += lower;
                x[3] += lower;
            }
            y[0] = y[1] = top + height * i / 8;
            y[2] = y[3] = top + height * (i + 1) / 8;
            p->u[0] = p->u[2] = (Uint8)(body->facing_left ? 4 + width - 1 : 4);
            p->u[1] = p->u[3] = (Uint8)(body->facing_left ? 3 : 4 + width);
            p->v[0] = p->v[1] = (Uint8)(1 + height * i / 8);
            p->v[2] = p->v[3] = (Uint8)(1 + height * (i + 1) / 8);
        }
        for (Uint16 vertex = 0; vertex < 4; ++vertex)
        {
            if (!Rotate(s, body, &x[vertex], &y[vertex]) || x[vertex] < -2048 || x[vertex] > 2048 ||
                y[vertex] < -2048 || y[vertex] > 2048)
                return SDL_SetError("BO4 blink transformed vertex exceeds its primitive bound");
            p->offset_x[vertex] = (Sint16)x[vertex];
            p->offset_y[vertex] = (Sint16)y[vertex];
        }
        p->x = a->state.x;
        p->y = a->state.y;
        p->active = p->textured = p->quad = p->gouraud = true;
        if (parameter != 32 && parameter != 33)
        {
            const Uint16 offsets[3] = {(Uint16)profile[0], (Uint16)profile[2], (Uint16)profile[1]};
            const Sint16 divisors[3] = {profile[4], profile[6], profile[5]};
            Uint8 *colors[3] = {p->vertex_red, p->vertex_green, p->vertex_blue};
            for (Uint16 channel = 0; channel < 3; ++channel)
            {
                Uint8 first = 0, next = 0;
                if (!Channel(s, s->dopple.blink_angles[(i + offsets[channel]) % 8],
                             a->blink.brightness, divisors[channel], &first) ||
                    !Channel(s, s->dopple.blink_angles[(i + offsets[channel] + 1) % 8],
                             a->blink.brightness, divisors[channel], &next))
                    return false;
                colors[channel][0] = first;
                colors[channel][1] = fan ? next : first;
                colors[channel][2] = fan ? 0 : next;
                colors[channel][3] = fan ? 0 : next;
            }
            s->dopple.blink_angles[i] += a->blink.speed;
        }
    }
    return true;
}

bool ActorDoppleBlink(SotnActors *s, Actor *a)
{
    if (s == NULL || a == NULL || s->stage_id != 26 || a->state.id != 0x10b)
        return SDL_SetError("BO4 blink requires its original child namespace");
    SotnDoppleBody *body = &s->dopple.body;
    const Uint16 parameter = (a->state.parameters & 0x7f00U) >> 8, index = parameter & 63U;
    if (a->enemy_state.owner_slot >= SOTN_STAGE_ACTOR_SLOTS ||
        a->enemy_state.owner_generation == 0 ||
        s->slots[a->enemy_state.owner_slot].state.generation != a->enemy_state.owner_generation)
    {
        a->remove = true;
        s->dopple.blink_lock = false;
        return true;
    }
    if (index != 0x1d && (body->motion.status & 2U))
    {
        s->dopple.blink_lock = false;
        a->remove = true;
        return true;
    }
    if (s->dopple.blink_lock && parameter != 32 && parameter != 33 && index != 0x1d)
    {
        a->remove = true;
        return true;
    }
    if (!(body->animation.sprite & 0x7fffU))
    {
        a->remove = true;
        return true;
    }
    BlinkImage image = {0};
    SotnDataView bytes = {0};
    Sint16 profile[10] = {0};
    if (index >= 36)
        return SDL_SetError("BO4 blink profile exceeds its private table");
    if (!Image(s, body, &image) || !Slice(s->nz0, 0x18a0 + index * 20U, 20, &bytes))
        return false;
    for (size_t i = 0; i < 10; ++i)
        profile[i] = (Sint16)Signed16(bytes.bytes + i * 2);
    if (profile[3] < 0 || profile[3] >= 768 || profile[0] < 0 || profile[1] < 0 || profile[2] < 0 ||
        profile[4] <= 0 || profile[5] <= 0 || profile[6] <= 0 || image.width > 248 ||
        image.height > 254)
        return SDL_SetError("BO4 blink color profile or UV extent is invalid");
    a->state.x = body->motion.x;
    a->state.y = body->motion.y;
    a->state.facing_left = body->facing_left;
    switch (a->state.step)
    {
    case 0:
        if (s->slots[0].state.priority > 509)
            return SDL_SetError("BO4 blink priority exceeds its original ordering table");
        for (Uint16 i = 16; i < 80; ++i)
            if (i != a->slot && s->slots[i].state.generation &&
                s->slots[i].state.id == a->state.id &&
                s->slots[i].state.parameters == a->state.parameters)
            {
                a->remove = true;
                return true;
            }
        if (!ActorPrimitives(s, a, 8))
        {
            a->remove = true;
            return true;
        }
        a->state.flags = 0x10800000U;
        a->state.priority = (Uint16)(s->slots[0].state.priority + 2);
        for (Uint16 i = 0; i < 8; ++i)
        {
            s->dopple.blink_angles[i] = i * 512U;
            ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
            if (p == NULL)
                return false;
            p->state.page = (Uint16)(0x10U | ((Uint16)profile[8] & 0x60U));
            p->state.palette = (Uint16)profile[3];
            p->state.blend = (profile[8] & 1) != 0;
            for (Uint16 v = 0; v < 4; ++v)
                p->state.vertex_red[v] = p->state.vertex_green[v] = p->state.vertex_blue[v] = 128;
        }
        a->blink.speed = (Uint16)profile[9];
        a->blink.brightness = a->blink.sway = 0;
        a->state.step = parameter == 32 ? 8 : parameter == 33 ? 10 : 1;
        break;
    case 1:
        a->blink.brightness = (Sint16)(a->blink.brightness + (profile[7] == 0x7008 ? 80 : 10));
        if (a->blink.brightness > 256)
        {
            a->blink.brightness = 256;
            a->blink.hold = (Uint16)profile[7];
            ++a->state.step;
        }
        break;
    case 2:
        if (profile[7] >= 0x7000)
        {
            a->blink.hold = 8;
            switch ((Uint16)profile[7])
            {
            case 0x7000:
                if (body->timers[1] == 0)
                    ++a->state.step;
                break;
            case 0x7001:
            case 0x7007:
                if (body->step != 11)
                    ++a->state.step;
                break;
            case 0x7005:
            case 0x7006:
                if (body->animation.id != 0xc0)
                    ++a->state.step;
                break;
            case 0x7008:
                if (!(body->motion.status & 0x400000U))
                    ++a->state.step;
                break;
            case 0x7009:
                if (body->substep != 3 || body->step != 6)
                    ++a->state.step;
                break;
            }
        }
        if (!a->blink.hold)
            return SDL_SetError("BO4 blink hold timer underflow");
        if (--a->blink.hold == 0)
            ++a->state.step;
        break;
    case 3:
        a->blink.brightness = (Sint16)(a->blink.brightness - 10);
        if (a->blink.brightness < 0)
        {
            a->remove = true;
            return true;
        }
        break;
    case 8:
        s->dopple.blink_lock = true;
        a->blink.sway = (Sint16)(a->blink.sway + 8);
        a->blink.wave = (Uint16)(a->blink.wave + 256);
        if (a->blink.sway > 128)
        {
            a->blink.sway = 128;
            body->form_phase = 2;
            ++a->state.step;
        }
        break;
    case 9:
    case 11:
        a->blink.sway = (Sint16)(a->blink.sway - 8);
        a->blink.wave = (Uint16)(a->blink.wave + 256);
        if (a->blink.sway < 0)
        {
            body->form_phase = 3;
            a->state.parameters = 0x1b00;
            a->state.step = 1;
            for (Uint16 i = 0; i < 8; ++i)
            {
                ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
                if (!p)
                    return false;
                p->state.palette = 0x15f;
            }
            s->dopple.blink_lock = false;
            return true;
        }
        break;
    case 10:
        s->dopple.blink_lock = true;
        a->blink.sway = (Sint16)SDL_min(128, a->blink.sway + 8);
        a->blink.wave = (Uint16)(a->blink.wave + 256);
        if (body->form_phase == 1)
            ++a->state.step;
        break;
    default:
        return SDL_SetError("BO4 blink entered unsupported step %u", a->state.step);
    }
    if (!Geometry(s, a, image, profile, parameter))
        return false;
    body->afterimage_disabled = body->afterimage_reset = true;
    body->timers[15] = 4;
    if (index == 0 || index == 7)
        body->timers[14] = SDL_max(body->timers[14], 10);
    return true;
}
