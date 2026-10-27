// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected BO0 e_laser_attack/e_olrox_drool algorithms at
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Runtime inputs remain private.
#include "boss_projection.h"
#include "olrox_monster_internal.h"
static Sint16 Word(Sint32 n)
{
    return (Sint16)Wrap16(n);
}
static void Whole(Sint32 *p, Sint32 n)
{
    *p = (Sint32)((Sint64)Word(n) * 65536 + ((Uint32)*p & 65535U));
}
static bool Move(Actor *a)
{
    const Sint64 x = (Sint64)a->state.x + a->vx, y = (Sint64)a->state.y + a->vy;
    if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32)
        return SDL_SetError("Olrox attack exceeds its fixed-point motion bounds");
    a->state.x = (Sint32)x;
    a->state.y = (Sint32)y;
    return true;
}
static Actor *Head(SotnActors *s, const Actor *a)
{
    if (!a->enemy_state.owner_generation || a->enemy_state.owner_slot >= SOTN_STAGE_ACTOR_SLOTS)
        return NULL;
    Actor *head = &s->slots[a->enemy_state.owner_slot];
    return head->state.generation == a->enemy_state.owner_generation && head->state.id == 0x35
               ? head
               : NULL;
}
static bool Trig(SotnActors *s, Sint32 angle, SotnMathTrig *out)
{
    return SotnMath_Trig(s->executable, Word(angle), out);
}
static ActorPrimitive *Part(SotnActors *s, Actor *a, Uint16 i)
{
    return ActorPrimitiveGet(s, a, i);
}
static void Quad(SotnActorPrimitive *p, Sint32 x0, Sint32 x1, Sint32 y0, Sint32 y1)
{
    p->quad = p->screen_space = true;
    p->x = p->y = 0;
    p->offset_x[0] = p->offset_x[2] = Word(x0);
    p->offset_x[1] = p->offset_x[3] = Word(x1);
    p->offset_y[0] = p->offset_y[1] = Word(y0);
    p->offset_y[2] = p->offset_y[3] = Word(y1);
}
static bool Pair(SotnActors *s, Actor *a, Uint16 index, ActorPrimitive **out)
{
    ActorPrimitive *p = Part(s, a, index), *meta = Part(s, a, (Uint16)(index + 1U));
    if (!p || !meta)
        return false;
    SDL_zero(p->olrox_effect);
    p->olrox_effect.tag = 8;
    meta->olrox_effect.tag = 1;
    meta->state.active = false;
    *out = p;
    return true;
}
static bool FreePair(SotnActors *s, Actor *a, Uint16 index)
{
    ActorPrimitive *p = Part(s, a, index), *meta = Part(s, a, (Uint16)(index + 1U));
    if (!p || !meta)
        return false;
    p->olrox_effect.tag = meta->olrox_effect.tag = 0;
    p->state.active = meta->state.active = false;
    return true;
}
static bool Find(SotnActors *s, Actor *a, Uint16 start, Uint16 end, Uint16 amount, Uint16 *out)
{
    *out = SDL_MAX_UINT16;
    if (start > end || end > a->state.primitive_count || !amount || amount > 2)
        return SDL_SetError("Olrox effect search requires a bounded owned packet range");
    for (Uint16 i = start; (Uint32)i + amount <= end; ++i)
    {
        ActorPrimitive *p = Part(s, a, i);
        if (!p)
            return false;
        if (p->olrox_effect.tag)
            continue;
        bool free = true;
        for (Uint16 j = 1; j < amount; ++j)
        {
            ActorPrimitive *q = Part(s, a, (Uint16)(i + j));
            if (!q)
                return false;
            free = free && !q->olrox_effect.tag;
        }
        if (free)
        {
            *out = i;
            break;
        }
    }
    return true;
}
static bool ProjectPair(SotnActors *s, ActorPrimitive *p)
{
    const Sint64 x = (Sint64)p->olrox_effect.x + p->olrox_effect.vx,
                 y = (Sint64)p->olrox_effect.y + p->olrox_effect.vy;
    if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32)
        return SDL_SetError("Olrox paired effect exceeds its fixed-point anchor bounds");
    p->olrox_effect.x = (Sint32)x;
    p->olrox_effect.y = (Sint32)y;
    const Uint8 tag = p->olrox_effect.tag, grey = p->olrox_effect.grey;
    SotnPrimitiveQuadPose pose = {.x_angle = tag & 32U ? p->olrox_effect.angle_x : 0,
                                  .y_angle = tag & 32U ? p->olrox_effect.angle_y : 0,
                                  .z_angle = p->olrox_effect.angle_z,
                                  .width = p->olrox_effect.width,
                                  .height = p->olrox_effect.height,
                                  .scale_x = tag & 16U ? p->olrox_effect.scale_x : 4096,
                                  .scale_y = tag & 16U ? p->olrox_effect.scale_y : 4096,
                                  .anchor_x = Pixel(p->olrox_effect.x),
                                  .anchor_y = Pixel(p->olrox_effect.y)};
    SotnProjectedPoint out[4] = {0};
    if (!SotnPrimitive_ProjectQuad(s->executable, pose, out))
        return false;
    p->state.x = p->state.y = 0;
    p->state.quad = p->state.screen_space = p->state.priority_set = p->state.active =
        p->state.gouraud = true;
    for (Uint16 i = 0; i < 4; ++i)
    {
        p->state.offset_x[i] = (Sint16)out[i].x;
        p->state.offset_y[i] = (Sint16)out[i].y;
        p->state.vertex_red[i] = p->state.vertex_green[i] = p->state.vertex_blue[i] = grey;
    }
    return true;
}
static bool Spawn(SotnActors *s, const Actor *a, Uint16 id, Uint16 parameters, Actor **out)
{
    if (!ActorSpawn(s, a, id, parameters, 160, 192, out))
        return false;
    if (*out)
    {
        (*out)->state.x = Pixel(a->state.x) * 65536;
        (*out)->state.y = Pixel(a->state.y) * 65536;
        (*out)->state.facing_left = false;
    }
    return true;
}
static bool Fireball(SotnActors *s, Actor *a)
{
    if (a->state.flags & 0x100U)
    {
        Actor *explosion = NULL;
        if (!Spawn(s, a, 2, 0, &explosion))
            return false;
        a->remove = true;
        return true;
    }
    if (!a->state.step)
    {
        Actor *head = Head(s, a);
        if (!head)
        {
            a->remove = true;
            return true;
        }
        if (a->olrox.fire_rank > 3 || !ActorInitialize(s, a, 0x72c))
            return SDL_SetError("Olrox fireball requires its original rank and private definition");
        a->state.animation_set = 9;
        a->state.frame = 1;
        a->state.hitbox_width = a->state.hitbox_height = 5;
        a->state.transform_flags_set = true;
        a->state.transform_flags |= SOTN_ACTOR_ROTATE | SOTN_ACTOR_SCALE_X;
        a->state.scale_x = 320;
        a->state.rotation = Word(head->state.rotation - 256);
        const Sint16 angle =
            Word(a->state.facing_left ? a->state.rotation : 2048 - a->state.rotation);
        SotnMathTrig t = {0};
        SotnDataView scale = {0};
        if (!Trig(s, angle, &t) || !Slice(s->nz0, 0x12f8 + a->olrox.fire_rank * 2U, 2, &scale))
            return false;
        Whole(&a->state.x, Pixel(a->state.x) + ActorShift((Sint64)t.cosine * 16, 12));
        Whole(&a->state.y, Pixel(a->state.y) + ActorShift(-(Sint64)t.sine * 16, 12));
        const Sint32 speed = Signed16(scale.bytes);
        if (speed < 0 || speed > 4096)
            return SDL_SetError("Olrox fireball has an invalid private speed");
        a->vx = speed * t.cosine;
        a->vy = -speed * t.sine;
        if (!ActorPrimitives(s, a, 2))
        {
            SDL_ClearError();
            a->remove = true;
            return true;
        }
        a->state.flags |= 0x800000U;
        ActorPrimitive *p = NULL;
        if (!Pair(s, a, 0, &p))
            return false;
        p->state = (SotnActorPrimitive){.active = true,
                                        .quad = true,
                                        .textured = true,
                                        .screen_space = true,
                                        .blend = true,
                                        .gouraud = true,
                                        .modulate = true,
                                        .page = 0x3a,
                                        .palette = 0x16d,
                                        .priority_set = true,
                                        .priority = (Uint16)(a->state.priority + 1U),
                                        .u = {224, 224, 192, 192},
                                        .v = {128, 144, 128, 144}};
        p->olrox_effect.width = 52;
        p->olrox_effect.height = 22;
        p->olrox_effect.angle_z = Word(-angle);
        p->olrox_effect.grey = 96;
        p->olrox_effect.x = (Pixel(a->state.x) - s->camera_x) * 65536;
        p->olrox_effect.y = (Pixel(a->state.y) - s->camera_y) * 65536;
        if (!ActorSound(s, 0x652))
            return false;
        return true;
    }
    if (a->state.step != 1)
        return SDL_SetError("Olrox fireball entered unsupported original state %u", a->state.step);
    Uint8 animated = 0;
    if (!Move(a) || !ActorAnimate(s, a, 0x12e4, &animated))
        return false;
    ActorPrimitive *p = Part(s, a, 0);
    if (!p)
        return false;
    const Sint16 angle = Word(a->state.facing_left ? a->state.rotation : 2048 - a->state.rotation);
    SotnMathTrig t = {0};
    if (!Trig(s, angle, &t))
        return false;
    p->olrox_effect.x =
        (Pixel(a->state.x) - s->camera_x - ActorShift((Sint64)t.cosine * 24, 12)) * 65536;
    p->olrox_effect.y =
        (Pixel(a->state.y) - s->camera_y - ActorShift(-(Sint64)t.sine * 24, 12)) * 65536;
    if (!ProjectPair(s, p))
        return false;
    p->state.v[0] = p->state.v[2] = (Uint8)(p->state.v[2] + 1U);
    p->state.v[1] = p->state.v[3] = (Uint8)(p->state.v[3] + 1U);
    if (p->state.v[1] == 176)
    {
        p->state.v[0] = p->state.v[2] = 128;
        p->state.v[1] = p->state.v[3] = 144;
    }
    for (Uint16 i = 0; i < 4; i += 2)
        p->state.vertex_red[i] = p->state.vertex_green[i] = p->state.vertex_blue[i] = 0;
    const Sint32 x = Pixel(a->state.x), y = Pixel(a->state.y);
    if (x < 48 || x > 464 || y < 80 || y > 464)
        a->state.flags |= 0x100U;
    return true;
}
static bool DripSound(SotnActors *s, const Actor *a)
{
    const Sint32 x = Pixel(a->state.x) - s->camera_x - 128,
                 y = Pixel(a->state.y) - s->camera_y - 128;
    Sint32 pan = SDL_clamp(ActorShift(SDL_abs(x) - 32, 5), 0, 8), volume = SDL_abs(x) - 96;
    if (x < 0)
        pan = -pan;
    const Sint32 dy = SDL_abs(y) - 112;
    if (dy > 0)
        volume += dy;
    volume = 88 - ActorShift(SDL_max(0, volume), 1);
    return volume <= 0 || ActorSoundVolumePan(s, 0x66b, (Uint16)volume, (Sint8)pan);
}
static bool Drool(SotnActors *s, Actor *a)
{
    Actor *head = Head(s, a);
    if (!head)
    {
        a->remove = true;
        return true;
    }
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0x60c))
            return false;
        if (!ActorPrimitives(s, a, 1))
        {
            SDL_ClearError();
            a->remove = true;
            return true;
        }
        a->state.flags |= 0x800000U;
        a->state.hitbox_state = 0;
        ActorPrimitive *p = Part(s, a, 0);
        if (!p)
            return false;
        p->state = (SotnActorPrimitive){.active = true,
                                        .line = true,
                                        .screen_space = true,
                                        .priority_set = true,
                                        .priority = 160,
                                        .blend = true,
                                        .line_blend_mode = 1,
                                        .red = 96,
                                        .green = 128,
                                        .blue = 96,
                                        .line_red = 0,
                                        .line_green = 32,
                                        .line_blue = 0};
        p->state.offset_x[0] = p->state.offset_x[1] = Word(Pixel(a->state.x) - s->camera_x);
        p->state.offset_y[0] = p->state.offset_y[1] = Word(Pixel(a->state.y) - s->camera_y);
        return true;
    }
    if (a->state.step != 1 || a->state.parameters < 16 || a->state.parameters > 23)
        return SDL_SetError("Olrox drool requires its original active state and mouth distance");
    ActorPrimitive *p = Part(s, a, 0);
    if (!p)
        return false;
    SotnActorPrimitive *q = &p->state;
    if (Word(Pixel(a->state.y)) > 464)
    {
        q->offset_y[1] = Word(q->offset_y[1] + 2);
        if (!a->substep)
        {
            if (!DripSound(s, a) || !ActorOuterDust(s, a, 1, 2, 0, 0, 3, 0))
                return false;
            a->substep = 1;
        }
    }
    else
    {
        if (a->vy > SDL_MAX_SINT32 - 4096)
            return SDL_SetError("Olrox drool gravity overflows its fixed-point velocity");
        a->vy += 4096;
        const Sint64 y = (Sint64)a->state.y + a->vy;
        if (y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32)
            return SDL_SetError("Olrox drool overflows its fixed-point ordinate");
        a->state.y = (Sint32)y;
        if (q->offset_y[0] - q->offset_y[1] > 8)
        {
            q->offset_y[0] = Word(Pixel(a->state.y) - s->camera_y);
            q->offset_y[1] = Word(q->offset_y[0] - 9);
            const Sint64 screen_x = (Sint64)head->state.x - (Sint64)s->camera_x * 65536,
                         delta = screen_x - a->olrox_laser.previous_x,
                         x = (Sint64)a->state.x + delta / 4;
            if (screen_x < SDL_MIN_SINT32 || screen_x > SDL_MAX_SINT32 || x < SDL_MIN_SINT32 ||
                x > SDL_MAX_SINT32)
                return SDL_SetError("Olrox drool exceeds its original horizontal following bounds");
            a->state.x = (Sint32)x;
        }
        else
        {
            Whole(&a->state.x, Pixel(head->state.x));
            Whole(&a->state.y, Pixel(head->state.y));
            const Sint16 angle = Word(head->state.rotation - 384);
            SotnMathTrig t = {0};
            if (!Trig(s, angle, &t))
                return false;
            const Sint32 distance = a->state.parameters;
            Whole(&a->state.x, Pixel(a->state.x) + (head->state.facing_left ? 1 : -1) *
                                                       ActorShift((Sint64)distance * t.cosine, 12));
            Whole(&a->state.y, Pixel(a->state.y) - ActorShift((Sint64)distance * t.sine, 12));
            const Sint16 height = Word(q->offset_y[0] - q->offset_y[1] + 1);
            q->offset_y[1] = Word(Pixel(a->state.y) - s->camera_y);
            q->offset_y[0] = Word(q->offset_y[1] + height);
            if (height > 8)
                Whole(&a->state.y, Pixel(a->state.y) + 8);
        }
    }
    q->offset_x[0] = q->offset_x[1] = Word(Pixel(a->state.x) - s->camera_x);
    const Sint64 previous_x = (Sint64)head->state.x - (Sint64)s->camera_x * 65536;
    if (previous_x < SDL_MIN_SINT32 || previous_x > SDL_MAX_SINT32)
        return SDL_SetError("Olrox drool parent exceeds its fixed-point screen bounds");
    a->olrox_laser.previous_x = (Sint32)previous_x;
    if (q->offset_y[0] < q->offset_y[1])
        a->remove = true;
    return true;
}
static bool Pulse(SotnActors *s, ActorPrimitive *p)
{
    if (!p->olrox_effect.phase)
    {
        const Sint16 x = (Sint16)(ActorRandom(s) & 63U),
                     y = (Sint16)((ActorRandom(s) & 31U) + 288U);
        p->olrox_effect.height = (Sint16)((ActorRandom(s) & 15U) + 16U);
        p->state = (SotnActorPrimitive){.active = true,
                                        .line = true,
                                        .screen_space = true,
                                        .priority_set = true,
                                        .priority = 200,
                                        .blend = true,
                                        .line_blend_mode = 2,
                                        .green = 255,
                                        .blue = 255,
                                        .line_green = 255,
                                        .line_blue = 255};
        p->state.offset_x[0] = p->state.offset_x[1] = x;
        p->state.offset_y[0] = 256;
        p->state.offset_y[1] = y;
        ++p->olrox_effect.phase;
    }
    p->state.offset_y[0] = Word(p->state.offset_y[0] - p->olrox_effect.height);
    p->state.offset_y[1] = Word(p->state.offset_y[1] - p->olrox_effect.height);
    if (p->state.offset_y[1] < 64)
    {
        p->olrox_effect.tag = 0;
        p->state.active = false;
    }
    return true;
}
static void Stretch(ActorPrimitive *p)
{
    if (!p->olrox_effect.phase)
    {
        p->state = (SotnActorPrimitive){.active = true,
                                        .textured = true,
                                        .page = 0x7a,
                                        .palette = 0x194,
                                        .priority_set = true,
                                        .priority = 200,
                                        .blend = true,
                                        .u = {0, 32, 0, 32},
                                        .v = {96, 96, 128, 128}};
        Quad(&p->state, 0, 64, 256, 256);
        ++p->olrox_effect.phase;
    }
    p->state.offset_y[0] = p->state.offset_y[1] = Word(p->state.offset_y[1] - 16);
    if (p->state.offset_y[0] == 64)
    {
        p->olrox_effect.tag = 0;
        p->state.active = false;
    }
}
static bool Spark(SotnActors *s, Actor *a, Uint16 index)
{
    ActorPrimitive *p = Part(s, a, index);
    if (!p)
        return false;
    if (!p->olrox_effect.phase)
    {
        p->state.textured = false;
        p->state.priority_set = true;
        p->state.priority = 206;
        p->state.blend = false;
        p->olrox_effect.tag |= 48U;
        p->olrox_effect.vy = -(Sint32)(ActorRandom(s) & 15U) * 8192 - 8192;
        p->olrox_effect.grey = 0;
        p->olrox_effect.width = (Sint16)((ActorRandom(s) & 7U) + 4U);
        p->olrox_effect.height = (Sint16)((ActorRandom(s) & 7U) + 4U);
        p->olrox_effect.scale_x = p->olrox_effect.scale_y = 4096;
        p->olrox_effect.angle_x = p->olrox_effect.angle_y = p->olrox_effect.angle_z = 0;
        ++p->olrox_effect.phase;
    }
    else
    {
        p->olrox_effect.angle_y =
            Word(p->olrox_effect.angle_y + (Sint32)((ActorRandom(s) & 3U) + 2U) * 64);
        p->olrox_effect.angle_x = Word(p->olrox_effect.angle_x + 64);
        p->olrox_effect.angle_z = Word(p->olrox_effect.angle_z + 64);
        p->olrox_effect.scale_x = Word(p->olrox_effect.scale_x - 32);
        p->olrox_effect.scale_y = p->olrox_effect.scale_x;
        if (p->olrox_effect.scale_x < 3072)
            p->olrox_effect.grey = (Uint8)ActorShift(4095 - p->olrox_effect.scale_x, 4);
        if (p->olrox_effect.scale_x < 256)
            return FreePair(s, a, index);
    }
    if (!ProjectPair(s, p))
        return false;
    for (Uint16 i = 0; i < 4; ++i)
        p->state.vertex_blue[i] = 0;
    return true;
}
static bool Muzzle(SotnActors *s, const Actor *a, const Actor *head, Sint16 *x, Sint16 *y,
                   Sint16 *angle)
{
    *angle = Word(2048 - (head->state.rotation - 256));
    if (a->state.facing_left)
        *angle = Word(2048 - *angle);
    SotnMathTrig t = {0};
    if (!Trig(s, *angle, &t))
        return false;
    *x = Word(Pixel(head->state.x) - s->camera_x + ActorShift((Sint64)t.cosine * 16, 12));
    *y = Word(Pixel(head->state.y) - s->camera_y + ActorShift(-(Sint64)t.sine * 16, 12));
    return true;
}
static void Triangle(SotnActorPrimitive *p)
{
    p->offset_x[3] = p->offset_x[2];
    p->offset_y[3] = p->offset_y[2];
    p->u[3] = p->u[2];
    p->v[3] = p->v[2];
    p->vertex_red[3] = p->vertex_red[2];
    p->vertex_green[3] = p->vertex_green[2];
    p->vertex_blue[3] = p->vertex_blue[2];
}
static bool LaserExplosion(SotnActors *s, Actor *a, Uint16 parameter)
{
    Actor *effect = NULL;
    if (!Spawn(s, a, 0x47, parameter, &effect))
        return false;
    if (effect)
    {
        Whole(&effect->state.x, Pixel(effect->state.x) + a->olrox_laser.width / 2);
        Whole(&effect->state.y, Pixel(effect->state.y) - 16);
    }
    return true;
}
static bool LaserTarget(SotnActors *s, Actor *a, Actor *head)
{
    ActorPrimitive *first = Part(s, a, 0), *second = Part(s, a, 1), *restore = Part(s, a, 2);
    if (!first || !second || !restore)
        return false;
    const Sint16 x1 = second->state.offset_x[0], x0 = second->state.offset_x[1],
                 y0 = second->state.offset_y[0];
    Whole(&a->state.x, x1 + s->camera_x);
    a->olrox_laser.width = Word(x0 - x1);
    const Uint16 palette = first->state.palette;
    first->state = (SotnActorPrimitive){.active = true,
                                        .palette = palette,
                                        .textured = true,
                                        .blend = true,
                                        .page = 0x130,
                                        .priority_set = true,
                                        .priority = 204,
                                        .u = {(Uint8)(a->state.parameters & 1U ? 64 : 0),
                                              (Uint8)(a->state.parameters & 1U ? 127 : 63),
                                              (Uint8)(a->state.parameters & 1U ? 64 : 0),
                                              (Uint8)(a->state.parameters & 1U ? 127 : 63)},
                                        .v = {64, 64, 255, 255}};
    Quad(&first->state, x1, x0, y0 - 192, y0);
    const Sint16 left = a->state.parameters & 1U ? 64 : 0;
    second->state = (SotnActorPrimitive){.active = true,
                                         .priority_set = true,
                                         .priority = 199,
                                         .target = 3,
                                         .clip_left = left,
                                         .clip_right = (Sint16)(left + 64),
                                         .clip_top = 256,
                                         .clip_bottom = 512,
                                         .target_offset_x = left,
                                         .target_offset_y = 256,
                                         .target_clear = true};
    restore->state =
        (SotnActorPrimitive){.active = true, .priority_set = true, .priority = 201, .target = 4};
    for (Uint16 i = 3; i < 27; ++i)
    {
        ActorPrimitive *p = Part(s, a, i);
        if (!p)
            return false;
        p->state.active = false;
        p->olrox_effect.tag = 0;
    }
    a->olrox_laser.timer = a->olrox_laser.delay = 1;
    a->olrox_laser.lifetime = 64;
    a->state.hitbox_state = 1;
    const Sint16 half = (Sint16)(a->olrox_laser.width / 2);
    a->state.hitbox_x = a->state.facing_left ? Word(-half) : half;
    const Sint32 width = SDL_abs(half);
    if (width > 8)
        a->state.hitbox_width = (Uint16)(width - 8);
    head->olrox.laser_busy = 0;
    if (!LaserExplosion(s, a, 0))
        return false;
    ++a->state.step;
    return true;
}
static bool LaserEffects(SotnActors *s, Actor *a)
{
    Uint16 index = SDL_MAX_UINT16;
    if (!Find(s, a, 3, 19, 1, &index))
        return false;
    if (index != SDL_MAX_UINT16)
    {
        ActorPrimitive *p = Part(s, a, index);
        if (!p)
            return false;
        p->olrox_effect.tag = 4;
        p->olrox_effect.phase = 0;
    }
    for (Uint16 i = 3; i < 19; ++i)
    {
        ActorPrimitive *p = Part(s, a, i);
        if (!p)
            return false;
        if ((p->olrox_effect.tag & 4U) && !Pulse(s, p))
            return false;
    }
    a->olrox_laser.timer = Word(a->olrox_laser.timer - 1);
    if (!a->olrox_laser.timer)
    {
        a->olrox_laser.timer = 2;
        if (!Find(s, a, 19, 48, 1, &index))
            return false;
        if (index != SDL_MAX_UINT16)
        {
            ActorPrimitive *p = Part(s, a, index);
            if (!p)
                return false;
            p->olrox_effect.tag = 4;
            p->olrox_effect.phase = 0;
        }
    }
    for (Uint16 i = 19; i < 27; ++i)
    {
        ActorPrimitive *p = Part(s, a, i);
        if (!p)
            return false;
        if (p->olrox_effect.tag & 4U)
            Stretch(p);
    }
    a->olrox_laser.delay = Word(a->olrox_laser.delay - 1);
    if (!a->olrox_laser.delay)
    {
        a->olrox_laser.delay = 12;
        if (!Find(s, a, 27, 48, 2, &index))
            return false;
        if (index != SDL_MAX_UINT16)
        {
            ActorPrimitive *p = NULL;
            if (!Pair(s, a, index, &p))
                return false;
            const Sint16 x0 = Word(Pixel(a->state.x) - s->camera_x),
                         x1 = Word(x0 + a->olrox_laser.width), step = (Sint16)((x0 - x1) / 16);
            p->olrox_effect.x = Word(step * (Sint32)(ActorRandom(s) & 15U) + x1) * 65536;
            p->olrox_effect.y = (Pixel(a->state.y) - s->camera_y) * 65536;
        }
    }
    for (Uint16 i = 27; i < 48; ++i)
    {
        ActorPrimitive *p = Part(s, a, i);
        if (!p)
            return false;
        if ((p->olrox_effect.tag & 8U) && !Spark(s, a, i))
            return false;
    }
    ActorPrimitive *p = Part(s, a, 0);
    if (!p)
        return false;
    if (!a->olrox_laser.lifetime)
    {
        a->state.hitbox_state = 0;
        const Sint32 delta = a->olrox_laser.width > 0 ? 2 : -2;
        p->state.offset_x[0] = p->state.offset_x[2] = Word(p->state.offset_x[0] + delta);
        p->state.offset_x[1] = p->state.offset_x[3] = Word(p->state.offset_x[1] - delta);
        if (a->olrox_laser.width > 0 ? p->state.offset_x[0] >= p->state.offset_x[1]
                                     : p->state.offset_x[0] <= p->state.offset_x[1])
            ++a->state.step;
    }
    else
    {
        a->olrox_laser.lifetime = Word(a->olrox_laser.lifetime - 1);
        if (!a->olrox_laser.lifetime && !LaserExplosion(s, a, 16))
            return false;
    }
    return true;
}
static bool Laser(SotnActors *s, Actor *a)
{
    Actor *head = Head(s, a);
    if (!head || (head->state.flags & 0x100U))
    {
        a->remove = true;
        return true;
    }
    if (a->state.parameters > 1)
        return SDL_SetError("Olrox laser requires its original side parameter");
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0x720))
            return false;
        a->state.hitbox_height = 56;
        a->state.hitbox_y = -56;
        if (!ActorPrimitives(s, a, 48))
        {
            SDL_ClearError();
            a->remove = true;
            return true;
        }
        a->state.flags |= 0x800000U;
        ActorPrimitive *p = Part(s, a, 0), *q = Part(s, a, 1);
        if (!p || !q)
            return false;
        p->state = (SotnActorPrimitive){.active = true,
                                        .textured = true,
                                        .quad = true,
                                        .screen_space = true,
                                        .gouraud = true,
                                        .modulate = true,
                                        .blend = true,
                                        .page = 0x3a,
                                        .palette = 0x15f,
                                        .priority_set = true,
                                        .priority = 158,
                                        .u = {16, 16, 20, 20},
                                        .v = {16, 20, 20, 20},
                                        .vertex_red = {96, 128, 0, 0},
                                        .vertex_green = {64, 128, 0, 0},
                                        .vertex_blue = {64, 96, 0, 0}};
        q->state = (SotnActorPrimitive){.quad = true,
                                        .screen_space = true,
                                        .gouraud = true,
                                        .blend = true,
                                        .page = 0x3a,
                                        .palette = 0x15f,
                                        .priority_set = true,
                                        .priority = 158,
                                        .u = {232, 248, 232, 248},
                                        .v = {128, 128, 134, 134},
                                        .vertex_red = {0, 0, 128, 128},
                                        .vertex_green = {0, 0, 128, 128},
                                        .vertex_blue = {0, 0, 48, 48}};
        Sint16 x = 0, y = 0, angle = 0;
        if (!Muzzle(s, a, head, &x, &y, &angle))
            return false;
        SotnMathTrig muzzle = {0};
        if (!Trig(s, angle, &muzzle))
            return false;
        Whole(&a->state.x, Pixel(a->state.x) + ActorShift((Sint64)muzzle.cosine * 16, 12));
        Whole(&a->state.y, Pixel(a->state.y) + ActorShift(-(Sint64)muzzle.sine * 16, 12));
        return ActorPositionalSound(s, a, 0x81a);
    }
    ActorPrimitive *p = Part(s, a, 0), *q = Part(s, a, 1);
    if (!p || !q)
        return false;
    SotnActorPrimitive *beam = &p->state, *floor = &q->state;
    Sint16 angle = 0, x = 0, y = 0;
    SotnMathTrig t = {0};
    const Sint32 direction = (a->state.parameters ^ (a->state.facing_left ? 1U : 0U)) ? 2 : -2;
    switch (a->state.step)
    {
    case 1:
        if (!Muzzle(s, a, head, &x, &y, &angle) || !Trig(s, angle, &t))
            return false;
        a->vx = t.cosine * 128;
        a->vy = -t.sine * 128;
        for (Uint16 i = 0; i < 4; ++i)
        {
            beam->offset_x[i] = Word(Pixel(a->state.x) - s->camera_x);
            beam->offset_y[i] = Word(Pixel(a->state.y) - s->camera_y);
        }
        ++a->state.step;
        break;
    case 2:
        if (!Move(a))
            return false;
        x = Word(Pixel(a->state.x));
        if (a->state.facing_left)
            x = Word(512 - x);
        if (x < 48)
        {
            head->olrox.laser_busy = 0;
            a->remove = true;
            return true;
        }
        if (Word(Pixel(a->state.y)) > 464)
        {
            Whole(&a->state.y, 464);
            a->olrox_laser.timer = 32;
            ++a->state.step;
            floor->active = true;
            Quad(floor, Pixel(a->state.x) - s->camera_x, Pixel(a->state.x) - s->camera_x,
                 Pixel(a->state.y) - s->camera_y - 8, Pixel(a->state.y) - s->camera_y);
        }
        beam->offset_x[1] = Word(Pixel(a->state.x) - s->camera_x);
        beam->offset_x[2] = Word(beam->offset_x[1] + 2);
        beam->offset_y[1] = beam->offset_y[2] = Word(Pixel(a->state.y) - s->camera_y);
        Triangle(beam);
        break;
    case 3: {
        if (!(s->ticks & 15U) && !ActorSound(s, 0x61f))
            return false;
        beam->offset_x[1] = Word(beam->offset_x[1] + direction);
        if (!Muzzle(s, a, head, &x, &y, &angle))
            return false;
        beam->offset_x[0] = x;
        beam->offset_y[0] = y;
        const Sint16 dx = Word(x - beam->offset_x[1]), dy = Word(y - beam->offset_y[1]);
        Sint16 aimed = 0;
        if (!SotnMath_Atan2(s->executable, -dy, dx, &aimed))
            return false;
        angle = Word(-aimed);
        head->state.rotation = Word(a->state.facing_left ? -angle - 1792 : angle + 256);
        if (a->olrox_laser.timer < 16)
            beam->offset_x[2] = Word(beam->offset_x[2] + direction);
        floor->offset_x[0] = floor->offset_x[2] = beam->offset_x[1];
        floor->page = s->ticks & 1U ? 0x3a : 0x7a;
        const Sint16 world_x = Word(s->camera_x + beam->offset_x[1]);
        if (world_x < 48 || world_x > 464)
            a->olrox_laser.timer = 1;
        a->olrox_laser.timer = Word(a->olrox_laser.timer - 1);
        if (!a->olrox_laser.timer)
        {
            Whole(&a->state.x, beam->offset_x[0] + s->camera_x);
            Whole(&a->state.y, beam->offset_y[0] + s->camera_y);
            angle = Word(2048 - angle);
            if (!Trig(s, angle, &t))
                return false;
            a->vx = t.cosine * 128;
            a->vy = -t.sine * 128;
            ++a->state.step;
        }
        Triangle(beam);
        break;
    }
    case 4:
        if (beam->offset_x[2] != beam->offset_x[1])
            beam->offset_x[2] = Word(beam->offset_x[2] + direction);
        if (!Move(a))
            return false;
        if (Word(Pixel(a->state.y)) > 464)
        {
            beam->active = false;
            Whole(&a->state.y, 464);
            a->olrox_laser.timer = 32;
            ++a->state.step;
        }
        beam->offset_x[0] = Word(Pixel(a->state.x) - s->camera_x);
        beam->offset_y[0] = Word(Pixel(a->state.y) - s->camera_y);
        Triangle(beam);
        break;
    case 5:
        floor->offset_y[0] = floor->offset_y[1] = Word(floor->offset_y[0] + 1);
        if (floor->offset_y[0] >= floor->offset_y[2])
        {
            floor->active = false;
            if (!ActorSound(s, 0x660))
                return false;
            ++a->state.step;
        }
        break;
    case 6:
        return LaserTarget(s, a, head);
    case 7:
        return LaserEffects(s, a);
    case 8:
        for (Uint16 i = 0; i < 27; ++i)
        {
            ActorPrimitive *part = Part(s, a, i);
            if (!part)
                return false;
            part->state.active = false;
        }
        ++a->state.step;
        /* fall through */
    case 9: {
        Uint16 active = 0;
        for (Uint16 i = 27; i < 48; ++i)
        {
            ActorPrimitive *part = Part(s, a, i);
            if (!part)
                return false;
            if (part->olrox_effect.tag & 8U)
            {
                ++active;
                if (!Spark(s, a, i))
                    return false;
            }
        }
        if (!active)
            a->remove = true;
        break;
    }
    default:
        return SDL_SetError("Olrox laser entered unsupported original state %u", a->state.step);
    }
    return true;
}
bool ActorOlroxMonsterAttack(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 30 || !a->state.generation)
        return SDL_SetError("Olrox monster attack requires its active original BO0 owner");
    switch (a->state.id)
    {
    case 0x39:
        return Laser(s, a);
    case 0x3a:
        return Fireball(s, a);
    case 0x3d:
        return Drool(s, a);
    default:
        return SDL_SetError("Unsupported original Olrox monster attack identity %u", a->state.id);
    }
}
