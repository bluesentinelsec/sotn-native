// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/e_bell.h at sotn-decomp 7bc54d8157b6.
// Bell profiles, model pointers/vertices and SDK tables remain private inputs.
#include "actors_internal.h"
#include "boss_projection.h"
typedef struct BellProfile
{
    Uint16 segments, length, priority;
} BellProfile;
static bool Profile(SotnActors *s, Uint16 index, BellProfile *out)
{
    SotnDataView data = {0};
    if (index > 1 || !Slice(s->nz0, 0xd2cU + index * 6U, 6, &data))
        return SDL_SetError("Chapel bell profile exceeds its two private variants");
    const Uint16 segments = Read16(data.bytes), length = Read16(data.bytes + 2),
                 priority = Read16(data.bytes + 4);
    if ((segments != 2 && segments != 4) || !length || length > 64 || priority < 26 ||
        priority > 511)
        return SDL_SetError("Chapel bell rope or priority exceeds its selected profile");
    *out = (BellProfile){segments, length, priority};
    return true;
}
static bool Spawner(SotnActors *s, Actor *a)
{
    if (a->state.step)
        return a->state.step == 1 || SDL_SetError("Chapel bell spawner entered an invalid state");
    if ((Uint32)a->slot + 2U >= SOTN_STAGE_ACTOR_SLOTS || s->effect_serial > 0x7fffffffU - 2U)
        return SDL_SetError("Chapel bell spawner cannot reserve two adjacent children");
    SotnDataView records = {0};
    if (!Slice(s->nz0, 0xd38, 12, &records))
        return false;
    for (Uint16 i = 0; i < 2; ++i)
    {
        const Actor *old = &s->slots[a->slot + i + 1U];
        SotnActorPlacement placement = {0};
        Uint32 placed = 0;
        const Uint8 *record = records.bytes + i * 6U;
        BellProfile profile = {0};
        if (Signed16(record) < 0 || Signed16(record) > 16384 || Signed16(record + 2) < 0 ||
            Signed16(record + 2) > 16384 || !Profile(s, Read16(record + 4), &profile) ||
            !SotnRoomActors_Get(s->spawns, a->slot + i + 1U, &placement, &placed))
            return SDL_SetError(
                "Chapel bell spawner has invalid private positions or reservations");
        if (placed || (old->state.generation &&
                       (old->state.id != 0x1b || old->enemy_state.owner_slot != a->slot)))
            return SDL_SetError("Chapel bell spawner would overwrite an unrelated actor");
    }
    if (!ActorInitialize(s, a, 0x968))
        return false;
    for (Uint16 i = 0; i < 2; ++i)
    {
        const Uint8 *record = records.bytes + i * 6U;
        Actor *child = NULL;
        if (!ActorOwnedSpawnAt(s, a, 0x1b, Read16(record + 4), a->slot + i + 1U, &child))
            return false;
        child->state.x = Signed16(record) * 65536;
        child->state.y = Signed16(record + 2) * 65536;
        child->state.facing_left = false;
    }
    return true;
}
static bool Initialize(SotnActors *s, Actor *a, BellProfile profile)
{
    if (!ActorInitialize(s, a, 0x968))
        return false;
    a->state.priority = 0x9c;
    a->bell.ring_timer = 0;
    if (!ActorPrimitives(s, a, (Uint16)(profile.segments + 6U)))
    {
        a->state.step = 0;
        return true;
    }
    a->state.flags |= 0x800000U;
    for (Uint16 i = 0; i < profile.segments + 6U; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (!p)
            return false;
        p->state = (SotnActorPrimitive){.page = 15,
                                        .palette = 0x76,
                                        .red = 128,
                                        .green = 128,
                                        .blue = 128,
                                        .priority_set = true,
                                        .priority = profile.priority,
                                        .active = true,
                                        .textured = true,
                                        .quad = true};
        if (!i)
        {
            p->state.x = Pixel(a->state.x) * 65536;
            p->state.y = Pixel(a->state.y) * 65536;
            p->state.offset_x[0] = p->state.offset_x[2] = -16;
            p->state.offset_x[1] = p->state.offset_x[3] = 16;
            p->state.offset_y[0] = p->state.offset_y[1] = -10;
            p->state.offset_y[2] = p->state.offset_y[3] = 14;
            p->state.u[0] = p->state.u[2] = 96;
            p->state.u[1] = p->state.u[3] = 128;
            p->state.v[2] = p->state.v[3] = 24;
        }
        else if (i <= profile.segments)
        {
            p->state.u[0] = p->state.u[2] = 120;
            p->state.u[1] = p->state.u[3] = 128;
            p->state.v[0] = p->state.v[1] = 65;
            p->state.v[2] = p->state.v[3] = (Uint8)(65U + profile.length);
        }
        else if (i == profile.segments + 1U)
        {
            p->state.u[0] = p->state.u[2] = 96;
            p->state.u[1] = p->state.u[3] = 128;
            p->state.v[0] = p->state.v[1] = 36;
            p->state.v[2] = p->state.v[3] = 62;
        }
        else if (i <= profile.segments + 3U)
        {
            p->state.palette = 0x75;
            p->state.u[0] = p->state.u[2] = 177;
            p->state.u[1] = p->state.u[3] = 224;
            p->state.v[0] = p->state.v[1] = 1;
            p->state.v[2] = p->state.v[3] = 97;
        }
        else
            p->state = (SotnActorPrimitive){.clip = true,
                                            .clip_left = 0,
                                            .clip_top = 20,
                                            .clip_right = 256,
                                            .clip_bottom = 227,
                                            .priority_set = true,
                                            .priority =
                                                (Uint16)(i == profile.segments + 4U ? 0x93 : 0x95)};
    }
    return true;
}
static bool Swing(Actor *a)
{
    if (a->bell.distance < -64 * 65536 || a->bell.distance > 64 * 65536 ||
        a->bell.velocity < -16 * 65536 || a->bell.velocity > 16 * 65536 ||
        a->bell.target < -20 * 65536 || a->bell.target > 20 * 65536)
        return SDL_SetError("Chapel bell swing exceeds its original bounded controls");
    a->bell.distance += a->bell.velocity;
    if (a->bell.distance > a->bell.target)
        a->bell.velocity -= 16384;
    if (a->bell.distance < a->bell.target)
        a->bell.velocity += 16384;
    a->bell.velocity -= a->bell.velocity / 8;
    const Sint32 delta = a->bell.distance - a->bell.target;
    if (a->bell.velocity > -24576 && a->bell.velocity < 24576 && delta > -24576 && delta < 24576)
    {
        a->bell.velocity = 0;
        a->bell.distance = a->bell.target;
    }
    a->bell.distance = SDL_clamp(a->bell.distance, -64 * 65536, 64 * 65536);
    return true;
}
static bool Corners(ActorPrimitive *p, const Sint32 x[4], const Sint32 y[4])
{
    for (Uint16 i = 0; i < 4; ++i)
    {
        if (x[i] < SDL_MIN_SINT16 || x[i] > SDL_MAX_SINT16 || y[i] < SDL_MIN_SINT16 ||
            y[i] > SDL_MAX_SINT16)
            return SDL_SetError("Chapel bell rope exceeds its original vertex coordinates");
        p->state.offset_x[i] = (Sint16)x[i];
        p->state.offset_y[i] = (Sint16)y[i];
    }
    p->state.screen_space = true;
    return true;
}
static bool Geometry(SotnActors *s, Actor *a, BellProfile profile, Sint32 *bottom_x,
                     Sint32 *bottom_y)
{
    const Uint16 priority = (Uint16)(profile.priority - ((s->player.status & 2U) ? 25U : 0U));
    ActorPrimitive *anchor = ActorPrimitiveGet(s, a, 0);
    if (!anchor)
        return false;
    anchor->state.priority = (Uint16)(priority - 1U);
    Sint32 x = Pixel(a->state.x) - s->camera_x, y = Pixel(a->state.y) - s->camera_y;
    Sint32 left_x = x - 4, left_y = y, right_x = x + 4, right_y = y;
    Sint16 angle = 0;
    for (Uint16 i = 0; i < profile.segments; ++i)
    {
        SotnMathTrig trig = {0}, plus = {0}, minus = {0};
        if (!SotnMath_Trig(s->executable, angle, &trig))
            return false;
        x -= ActorShift((Sint64)profile.length * trig.sine, 12);
        y += ActorShift((Sint64)profile.length * trig.cosine, 12);
        angle = (Sint16)((a->bell.distance / profile.segments * i) / 65536);
        if (!SotnMath_Trig(s->executable, angle + 1024, &plus) ||
            !SotnMath_Trig(s->executable, angle - 1024, &minus))
            return false;
        const Sint32 x2 = x - ActorShift((Sint64)plus.sine * 4, 12),
                     y2 = y + ActorShift((Sint64)plus.cosine * 4, 12),
                     x3 = x - ActorShift((Sint64)minus.sine * 4, 12),
                     y3 = y + ActorShift((Sint64)minus.cosine * 4, 12);
        ActorPrimitive *p = ActorPrimitiveGet(s, a, (Uint16)(i + 1U));
        if (!p ||
            !Corners(p, (Sint32[4]){left_x, right_x, x2, x3}, (Sint32[4]){left_y, right_y, y2, y3}))
            return false;
        p->state.priority = priority;
        left_x = x2;
        left_y = y2;
        right_x = x3;
        right_y = y3;
    }
    for (Uint16 i = 0; i < 3; ++i)
    {
        SotnProjectedPoint point[4] = {0};
        ActorPrimitive *p = ActorPrimitiveGet(s, a, (Uint16)(profile.segments + 1U + i));
        if (!p || !SotnChapel_ProjectBell(s->executable, s->nz0, i, angle, x, y, point))
            return false;
        Sint32 px[4] = {0}, py[4] = {0};
        for (Uint16 v = 0; v < 4; ++v)
        {
            px[v] = point[v].x;
            py[v] = point[v].y;
        }
        if (!Corners(p, px, py))
            return false;
        p->state.priority = priority;
    }
    *bottom_x = x + s->camera_x;
    *bottom_y = y + s->camera_y;
    return true;
}
static bool Interact(SotnActors *s, Actor *a, BellProfile profile, Sint32 x, Sint32 y)
{
    const Sint32 dx = Pixel(s->player.x) - x, dy = y + 124 - Pixel(s->player.y);
    const bool inside = dy >= 0 && dy < 80 && SDL_abs(dx) < 24;
    ActorPrimitive *clip = ActorPrimitiveGet(s, a, (Uint16)(profile.segments + 4U)),
                   *restore = ActorPrimitiveGet(s, a, (Uint16)(profile.segments + 5U));
    if (!clip || !restore || y - s->camera_y + 120 < SDL_MIN_SINT16 ||
        y - s->camera_y + 120 > SDL_MAX_SINT16)
        return SDL_SetError("Chapel bell clipping exceeds its original screen coordinates");
    clip->state.active = restore->state.active = inside;
    clip->state.clip_top = (Sint16)(y - s->camera_y + 120);
    clip->state.clip_bottom = 207;
    if (inside)
    {
        s->motion.protect = true;
        s->motion.bell_inside = true;
        s->player.status |= 0x100008U;
        if (!(s->player.status & 3U))
        {
            s->control.timer = 2;
            s->control.buttons = 0;
        }
    }
    a->state.hitbox_width = 56;
    a->state.hitbox_height = 64;
    a->state.hitbox_y = (Sint16)(y - Pixel(a->state.y) + 56);
    a->state.hitbox_state = 1;
    const Sint32 side = Pixel(s->player.x) - Pixel(a->state.x),
                 height = Pixel(s->player.y) - (y - 8);
    bool sound = false;
    switch (a->bell.phase)
    {
    case 0:
        a->bell.target = 0;
        if (height >= 0 && height < 128 && SDL_abs(side) < 56)
        {
            if (s->player.contacts & 1U)
            {
                sound = true;
                a->bell.phase = 1;
                a->bell.velocity = side > 0 ? 10 * 65536 : -10 * 65536;
            }
            if (s->player.contacts & 2U)
            {
                sound = true;
                a->bell.phase = 2;
                a->bell.velocity = side > 0 ? -8 * 65536 : 8 * 65536;
            }
        }
        break;
    case 1:
        if (!(s->player.contacts & 1U))
        {
            sound = true;
            a->bell.phase = 0;
            a->bell.velocity = side > 0 ? 10 * 65536 : -10 * 65536;
        }
        else
            a->bell.target = side > 0 ? 20 * 65536 : -20 * 65536;
        break;
    case 2:
        a->bell.target = 0;
        if (!(s->player.contacts & 2U))
        {
            sound = true;
            a->bell.phase = 0;
        }
        break;
    default:
        return SDL_SetError("Chapel bell entered an invalid interaction phase");
    }
    if (a->bell.ring_timer)
        --a->bell.ring_timer;
    else if (sound)
    {
        if (!ActorSound(s, 0x7bc))
            return false;
        a->bell.ring_timer = 64;
    }
    return true;
}
bool ActorChapelBell(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 6 || (a->state.id != 0x1b && a->state.id != 0x26))
        return SDL_SetError("Chapel bells require their original stage identity");
    if (a->state.id == 0x26)
        return Spawner(s, a);
    BellProfile profile = {0};
    if (!Profile(s, a->state.parameters, &profile))
        return false;
    if (!a->state.step)
    {
        if (!Initialize(s, a, profile))
            return false;
        if (!a->state.step)
            return true;
    }
    if (a->state.step != 1 || a->state.primitive_count != profile.segments + 6U ||
        a->bell.phase > 2 || a->bell.ring_timer > 64)
        return SDL_SetError("Chapel bell has invalid interaction, timer or primitive state");
    if (!Swing(a))
        return false;
    Sint32 x = 0, y = 0;
    return Geometry(s, a, profile, &x, &y) && Interact(s, a, profile, x, y);
}
