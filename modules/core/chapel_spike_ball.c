// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/dai/e_bone_pillar.c: EntityBonePillarSpikeBall at pinned sotn-decomp.
// Owned native records replace its out-of-stage player-attack entities.
#include "actors_internal.h"
static bool Live(const SotnActors *s, Uint16 i)
{
    const Uint16 slot = s->chapel_ball[i].slot;
    return s->chapel_ball[i].generation && slot < SOTN_STAGE_ACTOR_SLOTS &&
           s->slots[slot].state.id == 0x2c &&
           s->slots[slot].state.generation == s->chapel_ball[i].generation;
}
static bool Link(SotnActors *s, const Actor *a)
{
    Uint16 counts[ACTOR_FOREIGN_ATTACK_GROUPS] = {0};
    Uint16 free_slot = ACTOR_CHAPEL_BALL_LINKS;
    for (Uint16 i = 0; i < ACTOR_CHAPEL_BALL_LINKS; ++i)
    {
        if (!Live(s, i))
        {
            SDL_zero(s->chapel_ball[i]);
            if (free_slot == ACTOR_CHAPEL_BALL_LINKS)
                free_slot = i;
            continue;
        }
        if (s->chapel_ball[i].group >= ACTOR_FOREIGN_ATTACK_GROUPS)
            return SDL_SetError("Chapel spike-ball attack has an invalid invulnerability group");
        ++counts[s->chapel_ball[i].group];
    }
    if (free_slot == ACTOR_CHAPEL_BALL_LINKS)
        return true; // Original optional g_Entities[32..47) allocation can be exhausted.
    Uint8 group = 0;
    for (Uint8 i = 1; i < ACTOR_FOREIGN_ATTACK_GROUPS; ++i)
        if (counts[i] < counts[group])
            group = i;
    s->chapel_ball[free_slot].slot = a->slot;
    s->chapel_ball[free_slot].generation = a->state.generation;
    s->chapel_ball[free_slot].group = group;
    return true;
}
bool ActorChapelBallAttack(SotnActors *s, Uint16 index, SotnPlayerState *out, Uint32 *group,
                           bool *active)
{
    if (!s || !out || !group || !active || index >= ACTOR_CHAPEL_BALL_LINKS)
        return SDL_SetError("Chapel linked attack requires bounded borrowed outputs");
    *active = false;
    if (s->stage_id != 6 || !Live(s, index))
    {
        SDL_zero(s->chapel_ball[index]);
        return true;
    }
    if (s->chapel_ball[index].group >= ACTOR_FOREIGN_ATTACK_GROUPS)
        return SDL_SetError("Chapel linked attack lost its original invulnerability group");
    const Actor *ball = &s->slots[s->chapel_ball[index].slot];
    const Sint32 screen_x = Pixel(ball->state.x) - s->camera_x;
    const Sint32 screen_y = Pixel(ball->state.y) - s->camera_y;
    if (screen_x < -32 || screen_x > 288 || screen_y < -32 || screen_y > 256)
        return true;
    out->x = Pixel(ball->state.x) * 65536;
    out->y = Pixel(ball->state.y) * 65536;
    out->facing_left = false;
    out->attack = (SotnPlayerAttack){.width = 13,
                                     .height = 13,
                                     .power = ball->attack,
                                     .element = 0x8000,
                                     .hit_type = 2,
                                     .hit_effect = 1,
                                     .stun_frames = 4,
                                     .invincibility_frames = 16,
                                     .active = true};
    *group = s->chapel_ball[index].group;
    *active = true;
    return true;
}
bool ActorChapelSpikeBall(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 6 || a->state.id != 0x2c)
        return SDL_SetError("Chapel spike ball requires its original stage identity");
    if (a->state.flags & 0x100U)
    {
        if (!ActorPositionalSound(s, a, 0x683) || !ActorExplosionInPool(s, a, 3, 160, 192, 0))
            return false;
        a->remove = true;
        return true;
    }
    SotnDataView palette = {0};
    if (!Slice(s->nz0, 0xa22, 2, &palette))
        return false;
    const Uint16 base = Read16(palette.bytes);
    switch (a->state.step)
    {
    case 0: {
        if (!ActorInitialize(s, a, 0xa1c))
            return false;
        a->state.frame = 22;
        a->state.hitbox_width = a->state.hitbox_height = 13;
        a->state.flags |= 0x02000000U;
        if (!ActorPrimitives(s, a, 1))
        {
            a->remove = true;
            return true;
        }
        ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
        if (!p)
            return false;
        const Uint8 u = (a->state.texture_offset & 1U) ? 128 : 0;
        const Uint8 v = (a->state.texture_offset & 2U) ? 128 : 0;
        p->state = (SotnActorPrimitive){
            .page = (Uint16)(a->state.texture_offset / 4U),
            .palette = (Uint16)(base + 2U),
            .priority = (Uint16)(a->state.priority - 1U),
            .priority_set = true,
            .active = true,
            .textured = true,
            .quad = true,
            .u = {(Uint8)(u + 80), (Uint8)(u + 112), (Uint8)(u + 80), (Uint8)(u + 112)},
            .v = {(Uint8)(v + 48), (Uint8)(v + 48), (Uint8)(v + 80), (Uint8)(v + 80)},
            .offset_x = {-15, 17, -15, 17},
            .offset_y = {-16, -16, 16, 16}};
        SDL_FALLTHROUGH;
    }
    case 1:
        if (a->pillar.bottom_dead)
        {
            a->state.rotated = true;
            a->state.hit_points = 32767;
            a->state.hitbox_state = 1;
            a->vx = -49152;
            a->vy = -65536;
            ++a->state.step;
            if (!Link(s, a))
                return false;
        }
        break;
    case 2: {
        const Sint64 x = (Sint64)a->state.x + a->vx, y = (Sint64)a->state.y + a->vy;
        if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32 ||
            a->vy > SDL_MAX_SINT32 - 16384)
            return SDL_SetError("Chapel spike ball exceeds fixed-point motion bounds");
        a->state.x = (Sint32)x;
        a->state.y = (Sint32)y;
        a->state.rotation = (Sint16)Wrap16((Sint64)a->state.rotation - 64);
        a->vy += 16384;
        SotnCollider c = {0};
        if (!ActorQuery(s, Pixel(a->state.x), Wrap16((Sint64)Pixel(a->state.y) + 8), &c))
            return false;
        if (c.effects & SOTN_EFFECT_SOLID)
        {
            if (!ActorPositionalSound(s, a, a->pillar.bounced ? 0x60e : 0x761))
                return false;
            a->pillar.bounced = true;
            ShiftY(a, c.top);
            a->vy = -a->vy;
            a->vy -= a->vy / 2;
            if (SDL_abs(a->vy) < 32768)
                ++a->state.step;
        }
        break;
    }
    case 3: {
        Uint16 floor = 0;
        if (!ActorWalk(s, a, 0x1c84, &floor))
            return false;
        a->state.rotation = (Sint16)Wrap16((Sint64)a->state.rotation - 128);
        a->vx = -114688;
        break;
    }
    default:
        return SDL_SetError("Chapel spike ball entered an invalid original phase");
    }
    ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
    if (!p)
        return false;
    p->state.x = Pixel(a->state.x) * 65536;
    p->state.y = Pixel(a->state.y) * 65536;
    if (a->pillar.bottom_dead)
    {
        const Uint16 phase = (s->service_ticks & 2U) ? 0 : 1;
        a->state.palette = (Uint16)(base + phase);
        p->state.palette = (Uint16)(base + phase + 2U);
    }
    return true;
}
