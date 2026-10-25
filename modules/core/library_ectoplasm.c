// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/lib/e_ectoplasm.c at the pinned reconstruction. Native owned
// anchors replace primitive packet pointers; animation/recipes and SDK tables stay private.
#include "actors_internal.h"
static Actor *Owner(SotnActors *s, const Actor *a)
{
    if (a->enemy_state.owner_slot >= SOTN_STAGE_ACTOR_SLOTS)
        return NULL;
    Actor *p = &s->slots[a->enemy_state.owner_slot];
    return p->state.id == 0x37 && p->state.generation != 0 &&
                   p->state.generation == a->enemy_state.owner_generation
               ? p
               : NULL;
}
static bool TailEmitter(SotnActors *s, Actor *a)
{
    if (!a->state.step)
    {
        SotnDataView recipe = {0};
        if (a->state.parameters >= 2 ||
            !Slice(s->nz0, 0x2ae0 + a->state.parameters * 8U, 8, &recipe) ||
            !ActorInitialize(s, a, 0x830))
            return false;
        const Uint16 id = Read16(recipe.bytes), count = Read16(recipe.bytes + 2),
                     mode = Read16(recipe.bytes + 4), delay = Read16(recipe.bytes + 6);
        a->rose.spawn_id = id & 255U;
        a->rose.parameter = id & 0xff00U;
        a->rose.total = count & 255U;
        a->rose.drop_if_full = (count >> 8) == 1;
        a->rose.per_tick = mode & 127U;
        a->rose.mode = mode >> 8;
        a->rose.increment = (mode & 128U) != 0;
        a->rose.pool = delay >> 8;
        a->rose.interval = delay & 127U;
        a->rose.delay = (delay & 128U) ? a->rose.interval : 0;
        if (a->rose.spawn_id != 0x38 || !a->rose.total || !a->rose.per_tick ||
            a->rose.per_tick > 32 || a->rose.mode > 2 || a->rose.pool > 1 || (count >> 8) > 1)
            return SDL_SetError("Ectoplasm emitter recipe exceeds its original bounded family");
        if (!a->rose.mode)
            a->state.flags |= 0x08000000U;
    }
    if (a->rose.mode == 2)
    {
        Actor *root = Owner(s, a);
        if (!root)
        {
            a->remove = true;
            return true;
        }
        a->state.x = root->state.x;
        a->state.y = root->state.y;
    }
    if (a->rose.delay && --a->rose.delay)
        return true;
    for (Uint16 i = 0; i < a->rose.per_tick; ++i)
    {
        Actor *child = NULL;
        const Uint16 parameter =
            (Uint16)(a->rose.parameter + (a->rose.increment ? a->rose.spawned : i));
        if (!ActorSpawn(s, a, a->rose.spawn_id, parameter, a->rose.pool ? 160 : 96,
                        a->rose.pool ? 192 : 128, &child))
            return false;
        if (!child)
        {
            if (a->rose.drop_if_full)
                a->remove = true;
            return true;
        }
        child->enemy_state.owner_slot = a->enemy_state.owner_slot;
        child->enemy_state.owner_generation = a->enemy_state.owner_generation;
        if (++a->rose.spawned == a->rose.total)
        {
            a->remove = true;
            return true;
        }
    }
    a->rose.delay = a->rose.interval;
    return true;
}
static bool Emit(SotnActors *s, Actor *a)
{
    Actor *emitter = NULL;
    if (!ActorSpawn(s, a, 0x39, a->state.parameters ? 1 : 0, 160, 192, &emitter))
        return false;
    if (emitter)
    {
        emitter->enemy_state.owner_slot = a->slot;
        emitter->enemy_state.owner_generation = a->state.generation;
    }
    return true;
}
static bool Afterimage(SotnActors *s, Actor *a)
{
    Actor *root = Owner(s, a);
    bool ended = false;
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0x8c0))
            return false;
        a->state.flags |= 0x80202000U;
        a->state.flags &= ~0x20000000U;
        a->state.hitbox_state = 0;
        a->vy = -65536;
        a->animation_stream = a->state.parameters < 128 ? 0x2b04 : 0x2b4c;
        const Uint16 angle = SotnCombat_Random(&s->combat_random) & 2047U;
        SotnMathTrig trig = {0};
        if (!SotnMath_Trig(s->executable, angle, &trig))
            return false;
        const Sint64 x = (Sint64)a->state.x + trig.cosine * 192,
                     y = (Sint64)a->state.y - trig.sine * 192;
        if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32)
            return SDL_SetError("Ectoplasm afterimage spawn exceeds native fixed-point bounds");
        a->state.x = (Sint32)x;
        a->state.y = (Sint32)y;
    }
    else if (a->state.step == 1)
    {
        if (a->state.y < SDL_MIN_SINT32 + 65536)
            return SDL_SetError("Ectoplasm afterimage motion exceeds fixed-point bounds");
        a->state.y += a->vy;
        if (a->remaining == SDL_MAX_UINT16)
        {
            a->remove = true;
            return true;
        }
    }
    else
        return SDL_SetError("Ectoplasm afterimage entered an invalid step");
    if (!ActorWideAnimation(s, a, a->animation_stream, &ended))
        return false;
    if (root && root->state.step == 4)
    {
        a->ecto.counter = (Sint16)Wrap16((Sint32)a->ecto.counter - 1);
        a->state.blend_mode = (a->ecto.counter & 1) ? 0x10 : 0x30;
    }
    return true;
}
static bool Flame(SotnActors *s, Actor *a)
{
    Actor *p = NULL;
    if (!ActorSpawn(s, a, 2, 0, 160, 192, &p))
        return false;
    if (p)
    {
        if (a->state.priority > 509)
            return SDL_SetError("Ectoplasm flame priority exceeds original ordering table");
        p->state.x =
            Wrap16((Sint64)Pixel(a->state.x) + (SotnCombat_Random(&s->combat_random) & 31U) - 16) *
            65536;
        p->state.y =
            Wrap16((Sint64)Pixel(a->state.y) + (SotnCombat_Random(&s->combat_random) & 31U) - 16) *
            65536;
        p->state.priority = (Uint16)(a->state.priority + 2);
        p->state.facing_left = false;
        p->state.blend_mode = 0x10;
    }
    return true;
}
static bool Body(SotnActors *s, Actor *a)
{
    if ((a->state.parameters & 16U) && (s->library_enemy_flags & 2U))
    {
        a->hit_flags |= 3U;
        a->state.flags |= 0x100U;
    }
    if ((a->hit_flags & 3U) && a->state.step < 4)
    {
        if (a->state.flags & 0x100U)
        {
            if (!ActorLibrarySound(s, a, 0x73b))
                return false;
            a->state.hitbox_state = 0;
            a->ecto.squash = 0;
            a->ecto.scale = 256;
            a->state.blend_mode |= 0x10;
            a->state.step = 4;
            a->ecto.counter = 0;
        }
        else if (a->state.step != 3)
        {
            if (!ActorLibrarySound(s, a, 0x73a))
                return false;
            a->ecto.squash = 0;
            a->substep = a->state.step;
            a->state.step = 3;
        }
    }
    SotnMathTrig trig = {0};
    switch (a->state.step)
    {
    case 0: {
        if (!ActorInitialize(s, a, 0x8c0))
            return false;
        a->animation_stream = a->state.parameters ? 0x2b38 : 0x2af0;
        if (!ActorPrimitives(s, a, 1))
        {
            a->remove = true;
            return true;
        }
        ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
        if (!p)
            return false;
        p->state.x = Pixel(a->state.x) * 65536;
        p->state.y = Pixel(a->state.y) * 65536;
        a->state.scale_x = a->state.scale_y = 256;
        a->ecto.radius_x = a->state.parameters ? 16384 : 14336;
        a->ecto.radius_y = a->state.parameters ? 6144 : 8192;
        a->ecto.speed = a->state.parameters ? -10 : 8;
        a->ecto.limit = a->state.parameters ? 10 : 8;
        if (a->state.parameters & 16U)
        {
            a->ecto.radius_x = 0;
            a->state.flags |= 0x2000U;
            a->state.hitbox_state = 0;
            a->state.frame = 0;
            a->state.step = 32;
        }
        break;
    }
    case 32:
        if (!ActorLibrarySummon(s, a, 0))
            return false;
        break;
    case 1:
        a->ecto.phase_x = (Sint16)Wrap16((Sint32)a->ecto.phase_x + a->ecto.speed);
        a->ecto.phase_y = (Sint16)Wrap16((Sint32)a->ecto.phase_y + 64);
        if (s->tapped_buttons & SOTN_BUTTON_LEFT_HAND)
        {
            ++a->state.step;
            a->ecto.speed = (Sint16)Wrap16((Sint32)a->ecto.speed * 2);
            a->ecto.excited = 0;
        }
        if (SDL_abs(a->ecto.speed) > a->ecto.limit)
            a->ecto.speed += a->ecto.speed > 0 ? -1 : 1;
        break;
    case 2:
        a->ecto.phase_x = (Sint16)Wrap16((Sint32)a->ecto.phase_x + a->ecto.speed);
        a->ecto.phase_y = (Sint16)Wrap16((Sint32)a->ecto.phase_y + 64);
        if (!SotnMath_Trig(s->executable, a->ecto.excited, &trig))
            return false;
        if (a->ecto.excited < 4096)
        {
            a->ecto.excited = (Sint16)(a->ecto.excited + 32);
            const Sint32 change = ActorShift(trig.sine, 4);
            a->ecto.radius_x += change;
            a->ecto.radius_y += change;
        }
        else
        {
            a->ecto.excited = (Sint16)(a->ecto.excited + 64);
            const Sint32 change = ActorShift(trig.sine, 5);
            a->ecto.radius_x -= change;
            a->ecto.radius_y -= change;
        }
        if (a->ecto.excited == 8192)
        {
            --a->state.step;
            a->ecto.speed /= 2;
        }
        break;
    case 3:
        if (!SotnMath_Trig(s->executable, a->ecto.squash & 2047, &trig))
            return false;
        a->state.scale_y = (Uint16)(ActorShift(trig.sine, 4) + 256);
        a->state.scale_x = (Uint16)(256 - ActorShift(trig.sine, 6));
        a->ecto.squash = (Sint16)Wrap16((Sint32)a->ecto.squash + 128);
        if (a->ecto.squash == 4096)
        {
            a->state.scale_x = a->state.scale_y = 256;
            a->state.step = a->substep;
            a->ecto.speed = (Sint16)Wrap16(-(Sint32)a->ecto.speed * 2);
        }
        break;
    case 4: {
        if (a->ecto.counter == 32 && !ActorLibrarySound(s, a, 0x672))
            return false;
        if (!SotnMath_Trig(s->executable, a->ecto.squash & 2047, &trig))
            return false;
        a->state.scale_y = (Uint16)(a->ecto.scale + ActorShift(trig.sine, 4));
        a->state.scale_x = (Uint16)(a->ecto.scale - ActorShift(trig.sine, 6));
        a->ecto.squash = (Sint16)Wrap16((Sint32)a->ecto.squash + 128);
        const Sint64 y = (Sint64)a->state.y + a->vy;
        if (y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32 || a->vy > SDL_MAX_SINT32 - 1024)
            return SDL_SetError("Ectoplasm death exceeds fixed-point bounds");
        a->state.y = (Sint32)y;
        a->vy += 1024;
        a->ecto.scale = (Sint16)SDL_max(96, a->ecto.scale - 1);
        if (!(a->ecto.counter & 1) && !Flame(s, a))
            return false;
        if (Pixel(a->state.y) - s->camera_y > 256)
        {
            a->remove = true;
            return true;
        }
        break;
    }
    default:
        return SDL_SetError("Ectoplasm entered an unsupported gameplay step");
    }
    bool ended = false;
    if (!ActorWideAnimation(s, a, a->animation_stream, &ended))
        return false;
    a->ecto.counter = (Sint16)Wrap16((Sint32)a->ecto.counter + 1);
    if (!(a->ecto.counter & 7) && !Emit(s, a))
        return false;
    if (a->state.step != 4)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
        if (!p)
            return false;
        if (a->ecto.radius_x < -65536 || a->ecto.radius_x > 65536 || a->ecto.radius_y < -65536 ||
            a->ecto.radius_y > 65536)
            return SDL_SetError("Ectoplasm radius exceeds its original bounded motion");
        if (!SotnMath_Trig(s->executable, a->ecto.phase_x, &trig))
            return false;
        const Sint32 x = ActorShift((Sint64)trig.cosine * a->ecto.radius_x, 8) * 16;
        Sint32 y = -ActorShift((Sint64)trig.sine * a->ecto.radius_y, 8) * 16;
        if (!SotnMath_Trig(s->executable, a->ecto.phase_y, &trig))
            return false;
        y -= trig.sine * 32;
        a->state.x = Wrap16((Sint64)Pixel(p->state.x) + Pixel(x)) * 65536 +
                     (Sint32)((Uint32)a->state.x & 65535U);
        a->state.y = Wrap16((Sint64)Pixel(p->state.y) + Pixel(y)) * 65536 +
                     (Sint32)((Uint32)a->state.y & 65535U);
    }
    return true;
}
bool ActorLibraryEctoplasm(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 2 || a->state.id < 0x37 || a->state.id > 0x39)
        return SDL_SetError("Ectoplasm dispatch requires original Library identities");
    if (a->state.id == 0x38)
        return Afterimage(s, a);
    return a->state.id == 0x39 ? TailEmitter(s, a) : Body(s, a);
}
