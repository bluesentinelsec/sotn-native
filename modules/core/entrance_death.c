// SPDX-License-Identifier: AGPL-3.0-or-later
// NO3 Death and shared NZ0/NO3 scripts from st/no3/e_death.c, cutscene.c, cutscene_events.h,
// and set_cutscene_script.h at sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
// All dialogue, portraits, event streams, equipment artwork and animations remain private.
#include "actors_internal.h"

bool ActorDialogue_Byte(SotnActors *s, size_t *cursor, Uint8 *out)
{
    SotnDataView value = {0};
    if (!Slice(s->nz0, *cursor, 1, &value))
        return false;
    *out = value.bytes[0];
    ++*cursor;
    return true;
}

bool ActorDialogue_Address(SotnActors *s, size_t *cursor, size_t *out)
{
    Uint32 address = 0;
    for (Uint32 i = 0; i < 4; ++i)
    {
        Uint8 value = 0;
        if (!ActorDialogue_Byte(s, cursor, &value))
            return false;
        address = (address << 4) | value;
    }
    if (address < 0x80000 || address - 0x80000 >= s->nz0.size)
        return SDL_SetError("Death script references a truncated private stage address");
    *out = address - 0x80000;
    return true;
}

static bool Clear(SotnActors *s, Uint32 slot)
{
    if (slot >= SOTN_STAGE_ACTOR_SLOTS)
        return SDL_SetError("Death event slot exceeds the original actor pool");
    ActorPrimitivesClear(s, &s->slots[slot]);
    s->slots[slot] = (Actor){0};
    return SotnRoomActors_Release(s->spawns, slot, false);
}

bool ActorDialogue_Events(SotnActors *s, const Actor *a)
{
    if (!s->death_event.events_active)
        return true;
    if (++s->death_event.event_timer == 65535)
    {
        s->death_event.events_active = false;
        return true;
    }
    for (Uint32 budget = 0; budget < 64; ++budget)
    {
        const size_t start = s->death_event.events;
        Uint8 high = 0, low = 0, opcode = 0, slot = 0, id = 0, flag = 0;
        if (!ActorDialogue_Byte(s, &s->death_event.events, &high) ||
            !ActorDialogue_Byte(s, &s->death_event.events, &low))
            return false;
        const Uint16 time = (Uint16)((Uint16)high << 8 | low);
        if (s->death_event.event_timer < time)
        {
            s->death_event.events = start;
            return true;
        }
        if (!ActorDialogue_Byte(s, &s->death_event.events, &opcode))
            return false;
        if (opcode == 0)
        {
            Uint8 x_high = 0, x_low = 0, y_high = 0, y_low = 0;
            Actor *child = NULL;
            if (!ActorDialogue_Byte(s, &s->death_event.events, &slot) ||
                !ActorDialogue_Byte(s, &s->death_event.events, &id) ||
                !ActorDialogue_Byte(s, &s->death_event.events, &x_high) ||
                !ActorDialogue_Byte(s, &s->death_event.events, &x_low) ||
                !ActorDialogue_Byte(s, &s->death_event.events, &y_high) ||
                !ActorDialogue_Byte(s, &s->death_event.events, &y_low) || !Clear(s, slot) ||
                !ActorSpawnAt(s, a, id, 0, slot, &child))
                return false;
            // DAI subtracts tilemap scroll in the source's screen-space event factory.
            // Native room coordinates already include that scroll, so no offset is added.
            child->state.x =
                ((Sint32)(((Uint16)x_high << 4) | x_low) + (s->stage_id == 6 ? 0 : s->camera_x)) *
                65536;
            child->state.y =
                ((Sint32)(((Uint16)y_high << 4) | y_low) + (s->stage_id == 6 ? 0 : s->camera_y)) *
                65536;
        }
        else if (opcode == 1)
        {
            if (!ActorDialogue_Byte(s, &s->death_event.events, &slot) || !Clear(s, slot))
                return false;
        }
        else if (opcode == 2 || opcode == 3)
        {
            if (!ActorDialogue_Byte(s, &s->death_event.events, &flag) || flag >= 32)
                return SDL_SetError("Death event flag exceeds its original bitfield");
            if (opcode == 3)
                s->death_event.flags |= 1U << flag;
            else if (!(s->death_event.flags & (1U << flag)))
            {
                s->death_event.events = start;
                if (s->stage_id == 25)
                    --s->death_event.event_timer;
                return true;
            }
            else
                s->death_event.flags &= ~(1U << flag);
        }
        else
            return SDL_SetError("Death event has unsupported opcode %u", opcode);
    }
    return SDL_SetError("Death event exceeds its per-tick instruction budget");
}

static void Lines(SotnDialogueState *d)
{
    SDL_memset(d->lines, 0, sizeof(d->lines));
    d->row = d->column = 0;
}

typedef struct DialogueProfile
{
    size_t script, palettes;
    Uint16 flag, page;
    bool repeat;
} DialogueProfile;
static bool Profile(Uint16 stage, DialogueProfile *out)
{
    switch (stage)
    {
    case 65:
    case 7:
        *out = (DialogueProfile){0x4ce0, 0x1a30, SOTN_ENTRANCE_DEATH, 0x94, false};
        return true;
    case 12:
        *out = (DialogueProfile){0x3b0c, 0x13d0, SOTN_LAB_MARIA, 0x94, false};
        return true;
    case 2:
        *out = (DialogueProfile){0x3f68, 0x19c4, SOTN_LIBRARY_MET, 0x91, true};
        return true;
    case 6:
        *out = (DialogueProfile){0x3a94, 0xee8, SOTN_CHAPEL_MARIA, 0x90, false};
        return true;
    default:
        return SDL_SetError("Stage %u has no integrated original dialogue profile", stage);
    }
}
bool ActorDialogue(SotnActors *s, Actor *a)
{
    DialogueProfile profile = {0};
    if (!Profile(s->stage_id, &profile))
        return false;
    SotnDialogueState *d = &s->death_event.display;
    if (a->state.step == 0)
    {
        if (!profile.repeat && s->world->flags[profile.flag])
        {
            d->background_enabled = s->stage_id == 65 || s->stage_id == 7;
            a->remove = true;
            return true;
        }
        s->death_event.active = true;
        s->death_event.script = profile.script;
        s->death_event.library_met = s->stage_id == 2 && s->world->flags[SOTN_LIBRARY_MET] != 0;
        d->portrait_page = profile.page;
        s->death_event.speed = 8;
        s->death_event.flags = 0;
        ++a->state.step;
        a->state.flags |= 0x2000;
        return true;
    }
    if (!ActorDialogue_Events(s, a))
        return false;
    if (s->death_event.wait)
    {
        --s->death_event.wait;
        return true;
    }
    if (d->transition)
    {
        --d->transition;
        return true;
    }
    for (Uint32 budget = 0; budget < 128; ++budget)
    {
        const size_t instruction = s->death_event.script;
        Uint8 opcode = 0, value = 0, side = 0;
        if (!ActorDialogue_Byte(s, &s->death_event.script, &opcode))
            return false;
        switch (opcode)
        {
        case 0:
            d->visible = false;
            s->death_event.active = false;
            if (!profile.repeat)
                s->world->flags[profile.flag] = 1;
            a->remove = true;
            return true;
        case 1:
            if (++d->row >= 5)
            {
                for (Uint32 i = 0; i < 4; ++i)
                    SDL_memcpy(d->lines[i], d->lines[i + 1], sizeof(d->lines[i]));
                SDL_memset(d->lines[4], 0, sizeof(d->lines[4]));
                d->row = 4;
                d->transition = d->transition_length = 6;
            }
            d->column = 0;
            return true;
        case 2:
            if (!ActorDialogue_Byte(s, &s->death_event.script, &value))
                return false;
            s->death_event.speed = value;
            break;
        case 3:
            if (!ActorDialogue_Byte(s, &s->death_event.script, &value))
                return false;
            s->death_event.wait = value;
            return true;
        case 4:
            d->visible = false;
            return true;
        case 5:
            if (!ActorDialogue_Byte(s, &s->death_event.script, &value) || value > 1 ||
                !ActorDialogue_Byte(s, &s->death_event.script, &side))
                return SDL_SetError("Death dialogue portrait selector is invalid");
            SotnDataView palette = {0};
            if (!Slice(s->nz0, profile.palettes + value * 2U, 2, &palette) ||
                Read16(palette.bytes) >= 0x300)
                return SDL_SetError("Dialogue portrait palette is outside its private CLUT range");
            d->palette = Read16(palette.bytes);
            d->speaker = value;
            d->portrait_slot = side & 1;
            d->mirrored = (side & 128) != 0;
            d->portrait_visible = d->visible = true;
            d->transition = d->transition_length = 6;
            Lines(d);
            return true;
        case 6:
            d->portrait_visible = false;
            d->transition = d->transition_length = 6;
            Lines(d);
            return true;
        case 7:
            if (!ActorDialogue_Byte(s, &s->death_event.script, &value) ||
                !ActorDialogue_Byte(s, &s->death_event.script, &side))
                return false;
            d->x = value;
            d->y = side;
            d->visible = true;
            d->transition = d->transition_length = 48;
            return true;
        case 8:
            d->visible = false;
            d->portrait_visible = false;
            d->transition = d->transition_length = 24;
            return true;
        case 9:
        case 20: {
            Uint8 high = 0, low = 0;
            if (!ActorDialogue_Byte(s, &s->death_event.script, &high) ||
                !ActorDialogue_Byte(s, &s->death_event.script, &low))
                return false;
            const Uint16 sound = (Uint16)((Uint16)high << 4 | low);
            if (sound == 0x92 || sound == 0x0a)
            {
                s->combat.music_change = true;
                s->combat.music = 0;
            }
            else if (sound == 0x10 || sound == 0x11)
            {
                s->combat.cd_command = sound;
                s->death_event.cd_busy = sound == 0x10;
            }
            else if ((sound >= 0x35f && sound <= 0x363) ||
                     (s->stage_id == 12 && sound >= 0x369 && sound <= 0x36d) ||
                     (s->stage_id == 6 && sound >= 0x375 && sound <= 0x379) ||
                     (s->stage_id == 2 && ((sound >= 0x3a3 && sound <= 0x3a8) || sound == 0x3b9)))
            {
                s->combat.voice = sound;
                s->combat.voice_change = true;
                s->death_event.voice_playing = true;
            }
            else if (!ActorSound(s, sound))
                return false;
            break;
        }
        case 10:
            if (!s->death_event.voice_playing)
            {
                s->death_event.script = instruction;
                return true;
            }
            break;
        case 11:
            if (s->death_event.voice_playing || s->death_event.cd_busy)
            {
                s->death_event.script = instruction;
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
        case 14: {
            size_t selector = 0;
            if (s->stage_id != 2 || !ActorDialogue_Address(s, &s->death_event.script, &selector) ||
                (selector != 0x3f60 && selector != 0x3f64) || s->death_event.library_reply > 1)
                return SDL_SetError(
                    "Library dialogue switch references an unsupported state selector");
            const Uint8 choice = (Uint8)(selector == 0x3f60 ? (Uint8)s->death_event.library_met
                                                            : s->death_event.library_reply);
            if (s->death_event.script > s->nz0.size ||
                (size_t)choice * 4 > s->nz0.size - s->death_event.script)
                return SDL_SetError("Library dialogue switch cases are truncated");
            s->death_event.script += (size_t)choice * 4;
            if (!ActorDialogue_Address(s, &s->death_event.script, &s->death_event.script))
                return false;
            break;
        }
        case 15:
            if (!ActorDialogue_Address(s, &s->death_event.script, &s->death_event.script))
                return false;
            break;
        case 16:
        case 17:
        case 22:
        case 24:
            if (!ActorDialogue_Byte(s, &s->death_event.script, &value) || value >= 32)
                return SDL_SetError("Death dialogue flag is outside its bitfield");
            if (opcode == 17)
                s->death_event.flags |= 1U << value;
            else if (opcode == 22)
                s->death_event.flags &= ~(1U << value);
            else if (!(s->death_event.flags & (1U << value)))
            {
                s->death_event.script = instruction;
                return true;
            }
            else if (opcode == 16)
                s->death_event.flags &= ~(1U << value);
            break;
        case 18:
            s->death_event.events_active = false;
            break;
        case 19: {
            size_t portrait = 0;
            if (!ActorDialogue_Address(s, &s->death_event.script, &portrait) ||
                !ActorDialogue_Byte(s, &s->death_event.script, &value) || value > 1 ||
                portrait > s->nz0.size || 48 * 72 > s->nz0.size - portrait)
                return SDL_SetError("Death dialogue portrait is truncated or invalid");
            d->portraits[value] = (Uint32)portrait;
            break;
        }
        case 21:
            s->death_event.flags = 0;
            break;
        case 23:
            return true;
        default:
            if (opcode < 32 || opcode > 126 || d->row >= 5 || d->column >= 24)
                return SDL_SetError(
                    "Death dialogue contains invalid text or an unsupported opcode");
            d->lines[d->row][d->column++] = (char)opcode;
            d->lines[d->row][d->column] = '\0';
            s->death_event.wait = s->death_event.speed;
            return true;
        }
    }
    return SDL_SetError("Death dialogue exceeds its per-tick instruction budget");
}

static bool Manager(SotnActors *s, Actor *a)
{
    if (a->death.timer == 0 && a->state.step < 4)
    {
        if (!ActorSound(s, 0x631))
            return false;
        a->death.timer = 512;
    }
    --a->death.timer;
    s->control = (SotnActorControl){.timer = 2};
    s->motion.protect = true;
    SotnDialogueState *d = &s->death_event.display;
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0xad0) ||
            !SotnCamera_Bounds(s->camera, s->camera->left, 252, s->camera->right,
                               s->camera->bottom))
            return false;
        s->control.buttons = SOTN_BUTTON_RIGHT;
        s->death_event.flags |= 0x100;
        break;
    case 1:
        if (Pixel(s->player.x) - s->camera_x < 52)
            s->control.buttons = SOTN_BUTTON_RIGHT;
        else
        {
            s->death_event.flags |= 0x80;
            ++a->state.step;
        }
        break;
    case 2:
        if (s->death_event.flags & 0x20)
        {
            if (s->character == NULL || !SotnCharacter_DeathTheft(s->character) ||
                !ActorSound(s, 0x6a2))
                return false;
            for (Uint16 i = 0; i < 6; ++i)
            {
                Actor *item = NULL;
                if (!ActorSpawn(s, a, 0x59, i, 160, 192, &item))
                    return false;
                if (item != NULL)
                {
                    item->state.x = s->player.x;
                    item->state.y = s->player.y;
                }
            }
            a->timer = 16;
            ++a->state.step;
        }
        break;
    case 3:
        if ((s->death_event.flags & 0x40) && a->timer && --a->timer == 0)
            ++a->state.step;
        break;
    case 4:
        d->fade = (Uint8)(d->fade + 8);
        if (d->fade >= 240)
        {
            ++a->state.step;
            if (!Clear(s, 144) || !ActorSound(s, 0xa1))
                return false;
            d->background_enabled = true;
            s->combat.music_change = true;
            s->combat.music = 0x30f;
        }
        break;
    case 5:
        if (d->fade < 8)
            return SDL_SetError("Death event background fade underflow");
        d->fade = (Uint8)(d->fade - 8);
        if (d->fade == 0)
        {
            if (!SotnCamera_Bounds(s->camera, s->camera->left, 0, s->camera->right,
                                   s->camera->bottom))
                return false;
            s->control = (SotnActorControl){0};
            a->remove = true;
        }
        break;
    default:
        return SDL_SetError("Death manager entered invalid state %u", a->state.step);
    }
    return true;
}

static bool Scythe(SotnActors *s, Actor *a)
{
    const Uint32 owner = a->enemy_state.owner_slot;
    if (owner >= SOTN_STAGE_ACTOR_SLOTS ||
        s->slots[owner].state.generation != a->enemy_state.owner_generation)
    {
        a->remove = true;
        return true;
    }
    const Actor *death = &s->slots[owner];
    a->state.x = death->state.x;
    a->state.y = death->state.y;
    if (a->state.step == 0)
    {
        if (!ActorInitialize(s, a, 0xb00))
            return false;
        a->state.animation_set = 0x8008;
        a->state.frame = 0;
        a->state.palette = 0x2d6;
        a->state.texture_offset = 0x44;
    }
    else
    {
        Uint8 animation = 0;
        if (death->death.scythe &&
            !ActorAnimate(s, a, death->death.scythe == 1 ? 0x1b40 : 0x1b4c, &animation))
            return false;
        if (!death->death.scythe)
            a->state.frame = 0;
    }
    return true;
}

static bool Shadow(SotnActors *s, Actor *a)
{
    if (a->state.step == 0)
    {
        const Uint16 frame = a->state.frame;
        if (!ActorInitialize(s, a, 0xb00))
            return false;
        a->state.animation_set = 0x8008;
        a->state.frame = frame;
        a->state.palette = 0x2d6;
        a->state.texture_offset = 0x44;
        a->state.rotated = a->state.parameters == 0;
        a->timer = a->state.parameters ? 64 : 32;
        a->state.opacity = 64;
        a->state.blend_mode = 0x30;
    }
    else
    {
        if (a->timer == 0)
            return SDL_SetError("Death afterimage timer underflow");
        if (--a->timer == 0)
            a->remove = true;
        else
            a->state.opacity = (Uint16)(a->state.opacity - (a->state.parameters ? 1 : 2));
    }
    return true;
}

static bool Afterimage(SotnActors *s, const Actor *a, Uint16 parameter)
{
    Actor *shadow = NULL;
    if (!ActorSpawn(s, a, 0x5e, parameter, 160, 192, &shadow))
        return false;
    if (shadow != NULL)
    {
        shadow->state.frame = a->state.frame;
        shadow->state.rotation = a->state.rotation;
    }
    return true;
}

static bool Death(SotnActors *s, Actor *a)
{
    Uint8 animation = 0;
    if (a->state.step >= 4 && a->state.step < 13)
    {
        if (a->death.timer)
            --a->death.timer;
        else
        {
            a->death.direction = !a->death.direction;
            a->death.timer = 127;
        }
        a->vy += a->death.direction ? 512 : -512;
        a->state.x += a->vx;
        a->state.y += a->vy;
    }
    if (a->state.step >= 13 && a->state.step < 18 && a->vy)
    {
        a->vy += a->vy < 0 ? 512 : -512;
        a->state.x += a->vx;
        a->state.y += a->vy;
    }
    if (a->state.step >= 19)
    {
        a->vx -= 3072;
        a->vy -= 10240;
        a->state.x += a->vx;
        a->state.y += a->vy;
    }
    switch (a->state.step)
    {
    case 0:
        if (s->death_event.flags & 0x80)
        {
            Actor *scythe = NULL;
            if (!ActorInitialize(s, a, 0xb00) || !Clear(s, (Uint32)a->slot + 1) ||
                !ActorSpawnAt(s, a, 0x5b, 0, (Uint32)a->slot + 1, &scythe))
                return false;
            a->state.animation_set = 0x8008;
            a->state.frame = 0;
            a->state.palette = 0x2d6;
            a->state.texture_offset = 0x44;
        }
        break;
    case 1:
        if (!ActorAnimate(s, a, 0x1bc8, &animation))
            return false;
        if (animation == 0)
        {
            SetStep(a, 2);
            a->state.rotated = true;
            a->state.rotation = 4096;
            a->state.y += 16 * 65536;
            a->state.frame = 58;
            a->death.x = Pixel(a->state.x);
            a->death.y = Pixel(a->state.y);
        }
        break;
    case 2:
        a->state.rotation -= 64;
        a->state.x = (a->death.x + (4096 - a->state.rotation) * 29 / 4096) * 65536;
        a->state.y = (a->death.y - (4096 - a->state.rotation) * 40 / 4096) * 65536;
        if ((a->state.rotation & 0x70) == 0 && !Afterimage(s, a, 0))
            return false;
        if (a->state.rotation == 0)
        {
            SetStep(a, 3);
            a->state.rotated = false;
        }
        break;
    case 3:
        if (!ActorAnimate(s, a, 0x1be0, &animation))
            return false;
        if (animation == 0)
        {
            SetStep(a, 4);
            if (!ActorSound(s, 0x7a1))
                return false;
            a->death.timer = 64;
            a->death.direction = false;
        }
        a->state.x = a->death.x * 65536;
        a->state.y = (a->death.y - 16) * 65536;
        break;
    case 4:
        a->death.phase += 4;
        if (a->death.phase == 96)
            s->death_event.flags |= 1;
        if (a->death.phase == 128)
        {
            a->state.frame = 16;
            a->death.phase = 32;
            SetStep(a, 5);
        }
        break;
    case 5:
        if (!ActorAnimate(s, a, 0x1b34, &animation))
            return false;
        if (a->death.phase == 0)
            return SDL_SetError("Death entrance pose timer underflow");
        if (--a->death.phase == 0)
            SetStep(a, 6);
        break;
    case 6:
    case 8:
    case 10:
    case 12:
        if (!ActorAnimate(s, a, 0x1b28, &animation))
            return false;
        if ((a->state.step == 6 && (s->death_event.flags & 2)) ||
            (a->state.step == 8 && (s->death_event.flags & 4)) ||
            (a->state.step == 10 && (s->death_event.flags & 8)) ||
            (a->state.step == 12 && (s->death_event.flags & 16)))
            SetStep(a, (Uint16)(a->state.step + 1));
        break;
    case 7:
    case 9:
    case 11: {
        const Uint16 step = a->state.step;
        if (!ActorAnimate(s, a, step == 7 ? 0x1b04 : step == 9 ? 0x1bb4 : 0x1b94, &animation))
            return false;
        a->death.scythe = step == 7 && a->state.frame >= 7 && a->state.frame < 11 ? 2 : 1;
        if (animation == 0)
            SetStep(a, (Uint16)(step + 1));
        if ((step == 7 && (s->death_event.flags & 4)) ||
            (step == 9 && (s->death_event.flags & 8)) ||
            (step == 11 && (s->death_event.flags & 16)))
            SetStep(a, (Uint16)(step + 2));
        break;
    }
    case 13:
        if (!ActorAnimate(s, a, 0x1b58, &animation))
            return false;
        a->death.scythe = 1;
        if (animation == 0)
            SetStep(a, 14);
        break;
    case 14:
        if (!ActorAnimate(s, a, 0x1b68, &animation))
            return false;
        if (animation == 0)
        {
            if (!ActorSound(s, 0x7a0))
                return false;
            SetStep(a, 15);
        }
        break;
    case 15:
        if (!ActorAnimate(s, a, 0x1b70, &animation))
            return false;
        if (animation == 0)
        {
            s->death_event.flags |= 0x20;
            SetStep(a, 16);
        }
        break;
    case 16:
        if (!ActorAnimate(s, a, 0x1b74, &animation))
            return false;
        if (a->state.frame != 30)
            a->death.scythe = 1;
        if (animation == 0)
            SetStep(a, 18);
        break;
    case 18:
        if (!ActorAnimate(s, a, 0x1b84, &animation))
            return false;
        a->death.scythe = a->state.frame == 1 ? 1 : 2;
        if (animation == 0)
        {
            SetStep(a, 19);
            if (!ActorSound(s, 0x7a1))
                return false;
            a->vx = 65536;
            a->vy = 5 * 65536;
            a->death.timer = 0;
        }
        break;
    case 19:
        if (!ActorAnimate(s, a, 0x1b8c, &animation))
            return false;
        a->death.scythe = a->state.frame == 1 ? 1 : 2;
        if ((a->death.timer++ & 3) == 0 && !Afterimage(s, a, 1))
            return false;
        if (Pixel(a->state.y) - s->camera_y < -32)
        {
            s->death_event.flags |= 0x40;
            a->remove = true;
            if (!Clear(s, (Uint32)a->slot + 1))
                return false;
        }
        break;
    default:
        return SDL_SetError("Opening Death actor entered invalid state %u", a->state.step);
    }
    return true;
}

static bool Item(SotnActors *s, Actor *a)
{
    const Uint16 parameter = a->state.parameters;
    if (parameter >= 6)
        return SDL_SetError("Death stolen-item index exceeds its private table");
    if (a->state.step == 0)
    {
        if (!ActorInitialize(s, a, 0xb00))
            return false;
    }
    else if (a->state.step == 1)
        ++a->state.step;
    else if (a->state.step == 2)
    {
        SotnDataView velocity = {0};
        if (!Slice(s->nz0, 0x1aec + parameter * 4U, 4, &velocity))
            return false;
        const Sint32 slope = Signed16(velocity.bytes), speed = Signed16(velocity.bytes + 2);
        a->vx = ActorSin(slope + 1024) * speed / 16;
        a->vy = ActorSin(slope) * speed / 16;
        a->timer = 16;
        ++a->state.step;
    }
    else if (a->state.step == 3)
    {
        if (a->timer == 0)
            return SDL_SetError("Death item expansion timer underflow");
        --a->timer;
        a->state.x += a->vx;
        a->state.y += a->vy;
        a->death.phase = (Uint16)((16 - a->timer) * 7 / 16);
        if (a->timer == 0)
        {
            SotnDataView timer = {0};
            if (!Slice(s->nz0, 0x1ae0 + parameter * 2U, 2, &timer))
                return false;
            a->timer = Read16(timer.bytes);
            ++a->state.step;
        }
    }
    else if (a->state.step == 4)
    {
        if (a->timer == 0)
            return SDL_SetError("Death item blink timer underflow");
        if (--a->timer == 0)
        {
            a->timer = 8;
            if (!ActorSound(s, 0x7a2))
                return false;
            ++a->state.step;
        }
    }
    else if (a->state.step == 5)
    {
        ShiftY(a, -32);
        if (a->death.phase && a->timer > 1)
            --a->death.phase;
        if (a->timer == 0)
            return SDL_SetError("Death item departure timer underflow");
        if (--a->timer == 0)
        {
            a->timer = 16;
            ++a->state.step;
        }
    }
    else if (a->state.step == 6)
    {
        if (a->timer == 0)
            return SDL_SetError("Death item collapse timer underflow");
        if (--a->timer == 0)
            a->remove = true;
    }
    else
        return SDL_SetError("Death item entered invalid state %u", a->state.step);
    /* Icon presentation is a native item renderer; no generated asset buffers live here. */
    a->state.scale_x = (Uint16)(a->death.phase * 32);
    a->state.scale_y = a->state.step == 6 ? (Uint16)(a->timer * 16) : a->state.scale_x;
    return true;
}

bool ActorDeath(SotnActors *s, Actor *a)
{
    switch (a->state.id)
    {
    case 0x57:
        return ActorDialogue(s, a);
    case 0x58:
        return Manager(s, a);
    case 0x59:
        return Item(s, a);
    case 0x5a:
        return Death(s, a);
    case 0x5b:
        return Scythe(s, a);
    case 0x5e:
        return Shadow(s, a);
    default:
        return SDL_SetError("Death dispatcher received unrelated actor %u", a->state.id);
    }
}

bool SotnActors_Dialogue(SotnActors *s, bool voice_playing, SotnDialogueState *out)
{
    if (s == NULL)
        return SDL_InvalidParamError("actors");
    s->death_event.voice_playing = voice_playing;
    if (out != NULL)
        *out = s->death_event.display;
    return true;
}

bool SotnActors_SetDialogueBusy(SotnActors *s, bool busy)
{
    if (s == NULL)
        return SDL_InvalidParamError("actors");
    s->death_event.cd_busy = busy;
    return true;
}
