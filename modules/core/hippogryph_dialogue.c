// SPDX-License-Identifier: AGPL-3.0-or-later
// Reconstructed US BO5 EntityCutscene and shared cutscene helpers, sotn-decomp
// 7bc54d8157b6. Scripts, encoded names, portraits, palettes and line speeds remain private.
#include "actors_internal.h"

enum
{
    BASE = 7,
    PORTRAIT = 5,
    BACKGROUND = 6,
    TAIL_LINES = 1,
    TAIL_NAMES = 2
};
static bool Tail(SotnActors *s, Actor *a, Uint16 count, Uint8 kind)
{
    if (a->state.primitive_count != BASE || a->dialogue.tail)
        return SDL_SetError("BO5 dialogue already owns a temporary primitive reservation");
    Actor extra = {.state = {.generation = a->state.generation}};
    if (!ActorPrimitives(s, &extra, count))
    {
        a->remove = true;
        return true;
    }
    SDL_memcpy(a->primitive + BASE, extra.primitive, count * sizeof(a->primitive[0]));
    a->state.primitive_count = (Uint16)(BASE + count);
    a->dialogue.tail = kind;
    return true;
}
static bool ReleaseTail(SotnActors *s, Actor *a)
{
    for (Uint16 i = BASE; i < a->state.primitive_count; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (!p)
            return false;
        *p = (ActorPrimitive){0};
    }
    a->state.primitive_count = BASE;
    a->dialogue.tail = 0;
    return true;
}
static void Quad(SotnActorPrimitive *p, Sint16 left, Sint16 top, Sint16 right, Sint16 bottom)
{
    p->offset_x[0] = p->offset_x[2] = left;
    p->offset_x[1] = p->offset_x[3] = right;
    p->offset_y[0] = p->offset_y[1] = top;
    p->offset_y[2] = p->offset_y[3] = bottom;
}
static bool TextLine(SotnActors *s, Actor *a)
{
    SotnDialogueState *d = &s->death_event.display;
    if (d->row >= 5)
        return SDL_SetError("BO5 dialogue row exceeds its original five-line cache");
    ActorPrimitive *p = ActorPrimitiveGet(s, a, d->row);
    SotnDataView palette = {0};
    if (!p || !Slice(s->nz0, 0xb88U + d->speaker * 2U, 2, &palette) ||
        Read16(palette.bytes) >= 0x300)
        return SDL_SetError("BO5 dialogue text palette or reservation is invalid");
    SDL_memset(d->lines[d->row], 0, sizeof(d->lines[0]));
    d->column = 0;
    p->state = (SotnActorPrimitive){.active = true,
                                    .textured = true,
                                    .screen_space = true,
                                    .quad = true,
                                    .page = 16,
                                    .palette = Read16(palette.bytes),
                                    .priority_set = true,
                                    .priority = 511};
    Quad(&p->state, (Sint16)(d->x + 4), a->dialogue.next_line_y, (Sint16)(d->x + 196),
         (Sint16)(a->dialogue.next_line_y + 12));
    const Uint8 v = (Uint8)(128 + d->row * 12U);
    p->state.u[1] = p->state.u[3] = 192;
    p->state.v[0] = p->state.v[1] = v;
    p->state.v[2] = p->state.v[3] = (Uint8)(v + 12);
    return true;
}
static void ResetText(SotnActors *s, Actor *a)
{
    SotnDialogueState *d = &s->death_event.display;
    d->row = d->column = 0;
    a->dialogue.rolling = false;
    a->dialogue.next_line_y = (Sint16)(d->y + 20);
    s->death_event.wait = 0;
    s->death_event.speed = 8;
}
static bool HideText(SotnActors *s, const Actor *a)
{
    for (Uint16 i = 0; i < 5; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (!p)
            return false;
        p->state.active = false;
    }
    return true;
}
static bool Names(SotnActors *s, Actor *a, Uint8 speaker)
{
    SotnDataView pointer = {0};
    if (!Slice(s->nz0, 0xc1cU + speaker * 4U, 4, &pointer) || Read32(pointer.bytes) < 0x80180000U)
        return SDL_SetError("BO5 dialogue name pointer is invalid");
    size_t cursor = Read32(pointer.bytes) - 0x80180000U;
    Uint8 glyphs[24] = {0};
    Sint16 positions[24] = {0};
    Uint16 count = 0;
    Sint16 x = 56;
    bool ended = false;
    for (Uint16 scanned = 0; scanned < 64; ++scanned)
    {
        Uint8 ch = 0;
        if (!ActorDialogue_Byte(s, &cursor, &ch))
            return false;
        if (ch == 255)
        {
            if (!ActorDialogue_Byte(s, &cursor, &ch))
                return false;
            if (!ch)
            {
                ended = true;
                break;
            }
        }
        if (!ch)
        {
            x += 4;
            continue;
        }
        if (count == SDL_arraysize(glyphs))
            return SDL_SetError("BO5 dialogue name exceeds its bounded glyph count");
        glyphs[count] = ch;
        positions[count++] = x;
        x += 8;
    }
    if (!ended || !count)
        return SDL_SetError("BO5 dialogue name has no bounded terminator or glyphs");
    if (!Tail(s, a, count, TAIL_NAMES) || a->remove)
        return a->remove;
    for (Uint16 i = 0; i < count; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, (Uint16)(BASE + i));
        if (!p)
            return false;
        p->state = (SotnActorPrimitive){.textured = true,
                                        .screen_space = true,
                                        .quad = true,
                                        .page = 30,
                                        .palette = 0x196,
                                        .priority_set = true,
                                        .priority = 511};
        Quad(&p->state, positions[i], (Sint16)(s->death_event.display.y + 6),
             (Sint16)(positions[i] + 8), (Sint16)(s->death_event.display.y + 14));
        const Uint8 u = (Uint8)((glyphs[i] & 15U) * 8), v = (Uint8)((glyphs[i] & 240U) / 2);
        p->state.u[0] = p->state.u[2] = u;
        p->state.u[1] = p->state.u[3] = (Uint8)(u + 8);
        p->state.v[0] = p->state.v[1] = v;
        p->state.v[2] = p->state.v[3] = (Uint8)(v + 8);
    }
    return true;
}
static bool Sound(SotnActors *s, Uint16 id)
{
    if (id >= 0x36e && id <= 0x374)
    {
        s->combat.voice = id;
        s->combat.voice_change = true;
        s->death_event.voice_playing = true;
        return true;
    }
    if (id == 10)
    {
        s->combat.music_change = true;
        s->combat.music = 0;
        s->combat.voice_stop = true;
        s->death_event.voice_playing = false;
        return true;
    }
    return ActorSound(s, id);
}
static bool Run(SotnActors *s, Actor *a)
{
    SotnDialogueState *d = &s->death_event.display;
    for (Uint16 budget = 0; budget < 1024; ++budget)
    {
        if (s->death_event.wait && !s->hippogryph_skip)
        {
            --s->death_event.wait;
            return true;
        }
        const size_t start = s->death_event.script;
        Uint8 op = 0, value = 0, side = 0;
        if (!ActorDialogue_Byte(s, &s->death_event.script, &op))
            return false;
        switch (op)
        {
        case 0:
            a->state.step = 7;
            return true;
        case 1:
            if (s->hippogryph_skip)
                break;
            if (!a->dialogue.rolling)
                a->dialogue.next_line_y += 12;
            d->row = (Uint8)((d->row + 1U) % 5U);
            if (!TextLine(s, a))
                return false;
            if (a->dialogue.rolling || d->row == 4)
            {
                a->dialogue.rolling = true;
                a->timer = a->substep = 0;
                a->state.step = 2;
                return true;
            }
            break;
        case 2:
            if (!ActorDialogue_Byte(s, &s->death_event.script, &value))
                return false;
            s->death_event.speed = value;
            break;
        case 3:
            if (!ActorDialogue_Byte(s, &s->death_event.script, &value))
                return false;
            s->death_event.wait = value;
            if (!s->hippogryph_skip)
                return true;
            break;
        case 4:
            if (!s->hippogryph_skip)
                return HideText(s, a);
            break;
        case 5: {
            if (!ActorDialogue_Byte(s, &s->death_event.script, &value) || value > 1 ||
                !ActorDialogue_Byte(s, &s->death_event.script, &side))
                return SDL_SetError("BO5 dialogue portrait selector is invalid");
            if (s->hippogryph_skip)
                break;
            SotnDataView uv = {0}, palette = {0};
            ActorPrimitive *p = ActorPrimitiveGet(s, a, PORTRAIT);
            if (!p || !Slice(s->nz0, 0xb78, 8, &uv) ||
                !Slice(s->nz0, 0xb80U + value * 2U, 2, &palette) ||
                Read16(palette.bytes) >= 0x300 || uv.bytes[side & 1U] > 208 ||
                uv.bytes[4 + (side & 1U)] > 183)
                return SDL_SetError("BO5 dialogue portrait UV or palette is invalid");
            const Uint8 u = uv.bytes[side & 1U], v = uv.bytes[4 + (side & 1U)];
            p->state = (SotnActorPrimitive){.active = true,
                                            .textured = true,
                                            .screen_space = true,
                                            .quad = true,
                                            .page = 0x90,
                                            .palette = Read16(palette.bytes),
                                            .priority_set = true,
                                            .priority = 510};
            Quad(&p->state, (Sint16)(d->x - 30), (Sint16)(d->y + 36), (Sint16)(d->x - 30),
                 (Sint16)(d->y + 36));
            p->state.u[0] = p->state.u[2] = (Uint8)(u + (side & 128U ? 47 : 0));
            p->state.u[1] = p->state.u[3] = (Uint8)(u + (side & 128U ? 0 : 47));
            p->state.v[0] = p->state.v[1] = v;
            p->state.v[2] = p->state.v[3] = (Uint8)(v + 72);
            d->speaker = value;
            ResetText(s, a);
            if (!TextLine(s, a) || !Names(s, a, value) || a->remove)
                return a->remove;
            a->timer = 6;
            a->state.step = 3;
            return true;
        }
        case 6:
            if (s->hippogryph_skip)
                break;
            if (!HideText(s, a) || !ReleaseTail(s, a))
                return false;
            a->timer = 6;
            a->state.step = 4;
            return true;
        case 7:
            if (!ActorDialogue_Byte(s, &s->death_event.script, &value) ||
                !ActorDialogue_Byte(s, &s->death_event.script, &side))
                return false;
            if (!s->hippogryph_skip)
            {
                d->x = value;
                d->y = side;
                ActorPrimitive *p = ActorPrimitiveGet(s, a, BACKGROUND);
                if (!p)
                    return false;
                Quad(&p->state, 4, side, 248, (Sint16)(side + 72));
                a->dialogue.border_width = 246;
                a->timer = 24;
                a->state.step = 5;
                a->substep = 0;
                return true;
            }
            break;
        case 8:
            if (!s->hippogryph_skip)
            {
                a->timer = 24;
                a->state.step = 6;
                return true;
            }
            break;
        case 9:
        case 20: {
            Uint8 high = 0, low = 0;
            if (!ActorDialogue_Byte(s, &s->death_event.script, &high) ||
                !ActorDialogue_Byte(s, &s->death_event.script, &low))
                return false;
            if ((!s->hippogryph_skip || op == 20) && !Sound(s, (Uint16)((Uint16)high << 4 | low)))
                return false;
            break;
        }
        case 10:
        case 11:
            if (!s->hippogryph_skip &&
                (op == 10 ? !s->death_event.voice_playing
                          : s->death_event.voice_playing || s->death_event.cd_busy))
            {
                s->death_event.script = start;
                return true;
            }
            break;
        case 12:
            if (!ActorDialogue_Address(s, &s->death_event.script, &s->death_event.events))
                return false;
            s->death_event.event_timer = 0;
            s->death_event.events_active = true;
            break;
        case 13:
            break;
        case 16:
        case 17:
        case 22:
        case 24:
            if (!ActorDialogue_Byte(s, &s->death_event.script, &value) || value >= 32)
                return SDL_SetError("BO5 dialogue flag exceeds its original bitfield");
            if (op == 17)
                s->death_event.flags |= 1U << value;
            else if (op == 22)
                s->death_event.flags &= ~(1U << value);
            else if (!(s->death_event.flags & (1U << value)))
            {
                s->death_event.script = start;
                return true;
            }
            else if (op == 16)
                s->death_event.flags &= ~(1U << value);
            break;
        case 18:
            s->death_event.events_active = false;
            break;
        case 19: {
            size_t portrait = 0;
            if (!ActorDialogue_Address(s, &s->death_event.script, &portrait) ||
                !ActorDialogue_Byte(s, &s->death_event.script, &value) || value > 1 ||
                portrait > s->nz0.size || 48U * 72U > s->nz0.size - portrait)
                return SDL_SetError("BO5 private portrait pixels are truncated or invalid");
            if (!s->hippogryph_skip)
                d->portraits[value] = (Uint32)portrait;
            break;
        }
        case 21:
            s->death_event.flags = 0;
            s->hippogryph_skip = a->dialogue.started = false;
            break;
        case 23:
            return true;
        default:
            if (op < 32 || op > 126)
                return SDL_SetError("BO5 dialogue has an unsupported opcode or US glyph");
            if (s->hippogryph_skip)
                break;
            if (d->row >= 5 || d->column >= 24)
                return SDL_SetError("BO5 dialogue text exceeds its original visible cache width");
            d->lines[d->row][d->column++] = (char)op;
            d->lines[d->row][d->column] = '\0';
            s->death_event.wait = s->death_event.speed;
            return true;
        }
    }
    return SDL_SetError("BO5 dialogue exceeds its bounded per-tick instruction budget");
}
static bool Window(SotnActors *s, Actor *a)
{
    SotnDialogueState *d = &s->death_event.display;
    if (a->substep == 0)
    {
        a->dialogue.started = true;
        SotnDataView speeds = {0};
        if (!Slice(s->nz0, 0xb8c, 144, &speeds))
            return false;
        for (Uint16 i = 0; i < 72; ++i)
            if (Signed16(speeds.bytes + i * 2U) < 0 || Signed16(speeds.bytes + i * 2U) > 1024)
                return SDL_SetError("BO5 dialogue line speed exceeds its bounded private profile");
        if (!Tail(s, a, 72, TAIL_LINES) || a->remove)
            return a->remove;
        for (Uint16 i = 0; i < 72; ++i)
        {
            ActorPrimitive *p = ActorPrimitiveGet(s, a, (Uint16)(BASE + i));
            if (!p)
                return false;
            p->vx = 3952;
            p->vy = Signed16(speeds.bytes + i * 2U);
            p->state = (SotnActorPrimitive){.active = true,
                                            .screen_space = true,
                                            .line = true,
                                            .red = 127,
                                            .line_red = 127,
                                            .priority_set = true,
                                            .priority = 510};
            p->state.offset_x[0] = p->state.offset_x[1] = 247;
            p->state.offset_y[0] = p->state.offset_y[1] = (Sint16)(d->y + i);
        }
        ++a->substep;
        return true;
    }
    ActorPrimitive *background = ActorPrimitiveGet(s, a, BACKGROUND);
    if (!background)
        return false;
    if (a->substep == 1)
    {
        if (a->dialogue.tail != TAIL_LINES || a->state.primitive_count != BASE + 72)
            return SDL_SetError("BO5 dialogue lost its opening line reservation");
        bool waiting = false;
        for (Uint16 i = 0; i < 72; ++i)
        {
            ActorPrimitive *p = ActorPrimitiveGet(s, a, (Uint16)(BASE + i));
            if (!p)
                return false;
            p->vx = Wrap16(p->vx - p->vy);
            p->vy = Wrap16(p->vy + 2);
            const Sint32 left = p->vx / 16;
            p->state.offset_x[0] = (Sint16)SDL_max(4, left);
            waiting = waiting || left >= 5;
        }
        if (!waiting)
        {
            if (!ReleaseTail(s, a))
                return false;
            background->state.active = true;
            ++a->substep;
        }
        return true;
    }
    if (a->substep != 2)
        return SDL_SetError("BO5 dialogue entered an invalid window phase");
    SotnActorPrimitive *p = &background->state;
    const Uint8 red = p->vertex_red[0], blue = p->vertex_blue[0], grey = a->dialogue.border_grey;
    for (Uint16 i = 0; i < 4; ++i)
        p->vertex_red[i] = red < 16 ? 0 : (Uint8)(red - 16);
    p->vertex_blue[0] = p->vertex_blue[1] = blue >= 127 ? 127 : (Uint8)(blue + 8);
    a->dialogue.border_grey = grey < 16 ? 16 : (Uint8)(grey - 15);
    if (red < 16 && blue >= 127 && grey < 16)
        a->state.step = 1;
    return true;
}
bool ActorHippogryphDialogue(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 25 || a->state.id != 0x2a || !a->state.generation)
        return SDL_SetError("BO5 dialogue requires its original Alucard stage and active owner");
    SotnDialogueState *d = &s->death_event.display;
    if (!a->state.step)
    {
        if (s->world->flags[SOTN_MARIA_HIPPOGRYPH])
        {
            s->hippogryph_encounter = 0;
            a->remove = true;
            return true;
        }
        if (a->state.primitive_count)
            return SDL_SetError("BO5 dialogue cannot initialize over a live reservation");
        if (!ActorPrimitives(s, a, BASE))
            return true;
        d->primitive_text = true;
        d->portrait_page = 0x90;
        s->death_event.active = true;
        s->death_event.events_active = false;
        s->death_event.script = 0x1d5c;
        s->death_event.flags = 0;
        s->hippogryph_skip = false;
        a->dialogue.started = false;
        a->dialogue.border_grey = 255;
        ResetText(s, a);
        a->state.flags |= 0x802000U;
        ActorPrimitive *p = ActorPrimitiveGet(s, a, BACKGROUND);
        if (!p)
            return false;
        p->state = (SotnActorPrimitive){.screen_space = true,
                                        .quad = true,
                                        .blend = true,
                                        .gouraud = true,
                                        .priority_set = true,
                                        .priority = 509};
        for (Uint16 i = 0; i < 4; ++i)
            p->state.vertex_red[i] = 255;
        Quad(&p->state, 4, 0, 248, 0);
        ++a->state.step;
        return true;
    }
    if (a->dialogue.started && !s->hippogryph_skip &&
        ((s->world->cutscenes_seen & 0x200U) || s->world->game_clear) &&
        s->tapped_buttons == SOTN_BUTTON_START)
    {
        ActorPrimitivesClear(s, a);
        a->state.primitive_count = 0;
        a->dialogue.tail = 0;
        a->state.flags ^= 0x800000U;
        s->hippogryph_skip = true;
        a->state.step = 1;
        a->substep = 0;
        if (!Sound(s, 10))
            return false;
    }
    if (!ActorDialogue_Events(s, a))
        return false;
    if (a->state.step == 1)
        return Run(s, a);
    if (a->state.step == 7)
    {
        s->world->flags[SOTN_MARIA_HIPPOGRYPH] = 1;
        s->world->flags[SOTN_LAB_MARIA] = 1;
        s->world->cutscenes_seen |= 0x300U;
        s->death_event.active = false;
        a->remove = true;
        return true;
    }
    if (a->state.primitive_count < BASE)
        return SDL_SetError("BO5 dialogue lost its original primitive reservation");
    if (a->state.step == 2)
    {
        const Uint16 hidden = (Uint16)((d->row + 1U) % 5U);
        for (Uint16 i = 0; i < 5; ++i)
        {
            ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
            if (!p)
                return false;
            if (i == hidden)
            {
                if (!a->substep)
                {
                    p->state.v[0] = p->state.v[1] = (Uint8)(p->state.v[0] + 2);
                    p->state.offset_y[2] -= 2;
                    p->state.offset_y[3] -= 2;
                    if (p->state.offset_y[2] == p->state.offset_y[0])
                    {
                        ++a->substep;
                        p->state.active = false;
                    }
                }
            }
            else
                for (Uint16 vertex = 0; vertex < 4; ++vertex)
                    p->state.offset_y[vertex] -= 2;
        }
        if (++a->timer >= 6)
            a->state.step = 1;
        return true;
    }
    if (a->state.step == 5)
        return Window(s, a);
    if (a->state.step == 3 || a->state.step == 4)
    {
        if (!a->timer || a->timer > 6)
            return SDL_SetError("BO5 dialogue portrait timer is invalid");
        ActorPrimitive *p = ActorPrimitiveGet(s, a, PORTRAIT);
        if (!p)
            return false;
        const Sint16 direction = a->state.step == 3 ? 1 : -1;
        for (Uint16 i = 0; i < 4; ++i)
        {
            p->state.offset_x[i] += (Sint16)(direction * (i & 1U ? 4 : -4));
            p->state.offset_y[i] += (Sint16)(direction * (i & 2U ? 6 : -6));
        }
        if (a->state.step == 4 && p->state.offset_x[1] >= (Sint32)d->x - 2)
            p->state.offset_x[1] = p->state.offset_x[3] = (Sint16)(d->x - 3);
        if (!--a->timer)
        {
            a->state.step = 1;
            if (direction > 0)
                for (Uint16 i = BASE; i < a->state.primitive_count; ++i)
                {
                    ActorPrimitive *name = ActorPrimitiveGet(s, a, i);
                    if (!name)
                        return false;
                    name->state.active = true;
                }
        }
        return true;
    }
    if (a->state.step == 6)
    {
        if (!a->timer || a->timer > 24)
            return SDL_SetError("BO5 dialogue closing timer is invalid");
        ActorPrimitive *p = ActorPrimitiveGet(s, a, BACKGROUND);
        if (!p)
            return false;
        --a->timer;
        if (a->timer >= 12)
            a->dialogue.border_width = (Uint8)(a->dialogue.border_width - 20 - (a->timer & 1U));
        else if (!a->timer)
        {
            a->state.step = 1;
            p->state.active = false;
        }
        else
        {
            p->state.offset_y[2] -= 6;
            p->state.offset_y[3] -= 6;
        }
        return true;
    }
    return SDL_SetError("BO5 dialogue entered invalid original state %u", a->state.step);
}
