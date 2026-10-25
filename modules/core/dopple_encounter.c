// SPDX-License-Identifier: AGPL-3.0-or-later
// Original BO4 root/child orchestration, adapted from sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5, bo4/doppleganger.c and unk_49A6C.c.
// Runtime profiles and artwork remain private. See docs/doppleganger-runtime.md.
#include "actors_internal.h"

bool ActorDoppleHitEffect(SotnActors *s, Uint16 element, Uint8 *out)
{
    SotnDataView masks = {0}, selectors = {0};
    if (s == NULL || out == NULL || s->stage_id != 26 || !Slice(s->nz0, 0xa10, 20, &masks) ||
        !Slice(s->nz0, 0xa24, 10, &selectors))
        return SDL_SetError("BO4 hit effect requires its original stage collision tables");
    Uint8 next = 0;
    if (element & 0xffc0U)
        for (Uint32 i = 0; i < 10; ++i)
            if (element & Read16(masks.bytes + i * 2))
            {
                next = selectors.bytes[i];
                break;
            }
    if (next >= 16)
        return SDL_SetError("BO4 stage hit-effect selector exceeds its private reaction table");
    *out = next;
    return true;
}
static bool Initialize(SotnActors *s, Actor *a)
{
    if (a->slot != 0 || s->dopple.active)
        return SDL_SetError("BO4 root requires its original reserved slot and fresh encounter");
    if (!ActorInitialize(s, a, 0x410) ||
        !SotnDoppleBody_Init(s->nz0, a->state.x, a->state.y,
                             Pixel(a->state.x) >= Pixel(s->player.x), &s->dopple.body) ||
        !SotnDoppleAI_Init(&s->dopple.ai))
        return false;
    for (Uint16 i = 4; i < 80; ++i)
    {
        ActorPrimitivesClear(s, &s->slots[i]);
        if (!SotnRoomActors_Release(s->spawns, i, false))
            return false;
        s->slots[i] = (Actor){0};
    }
    for (Uint16 i = 0; i < 32; ++i)
    {
        const Sint32 angle = (Sint32)(SotnCombat_Random(&s->combat_random) & 1023U) + 256;
        const Sint32 scale = (Sint32)(SotnCombat_Random(&s->combat_random) & 255U) + 256;
        SotnMathTrig trig = {0};
        if (!SotnMath_Trig(s->executable, angle, &trig))
            return false;
        s->dopple.debris_x[i] = ActorShift((Sint64)trig.cosine * 16 * scale, 8);
        s->dopple.debris_y[i] = -ActorShift((Sint64)trig.sine * 16 * scale, 7);
    }
    a->state.flags = 0x18400000U;
    a->state.priority = 0x9c;
    s->dopple.hitbox_state = a->state.hitbox_state;
    s->dopple.previous_hp = a->state.hit_points;
    s->dopple.active = true;
    return true;
}
static bool Requests(SotnActors *s, Actor *a)
{
    const SotnDoppleBody *body = &s->dopple.body;
    for (Uint8 i = 0; i < body->request_count; ++i)
    {
        const SotnDoppleRequest *request = &body->requests[i];
        switch (request->kind)
        {
        case SOTN_DOPPLE_FACTORY:
            if (!ActorDoppleFactoryRequest(s, a, request))
                return false;
            break;
        case SOTN_DOPPLE_SOUND:
            if (!ActorSound(s, request->id))
                return false;
            break;
        case SOTN_DOPPLE_CLEAR_ATTACK:
            ActorPrimitivesClear(s, &s->slots[16]);
            if (!SotnRoomActors_Release(s->spawns, 16, false))
                return false;
            s->slots[16] = (Actor){0};
            break;
        case SOTN_DOPPLE_DEATH_PROFILE: {
            SotnDataView profile = {0};
            if (request->id >= 13 || !Slice(s->nz0, 0x136c + request->id * 4U, 4, &profile))
                return SDL_SetError("BO4 death profile exceeds its original private records");
            SDL_memcpy(s->dopple.death_profile, profile.bytes, 4);
            break;
        }
        case SOTN_DOPPLE_PALETTE_BEGIN:
        case SOTN_DOPPLE_PALETTE_CLEAR:
            return SDL_SetError("BO4 mist controls require the unintegrated palette service");
        case SOTN_DOPPLE_SHAKE:
            // Original camera shake remains a presentation effect, not body motion.
            s->dopple.shake = request->id;
            break;
        default:
            return SDL_SetError("BO4 encounter received unknown body request %u", request->kind);
        }
    }
    return true;
}
static bool Root(SotnActors *s, Actor *a)
{
    SotnDoppleBody *body = &s->dopple.body;
    if (a->state.step == 0)
    {
        if (!Initialize(s, a))
            return false;
    }
    else
    {
        const SotnDoppleObservation observation = {
            .boss_x = (Sint16)(Pixel(body->motion.x) - s->camera_x),
            .boss_y = (Sint16)(Pixel(body->motion.y) - s->camera_y),
            .player_x = (Sint16)(Pixel(s->player.x) - s->camera_x),
            .player_y = (Sint16)(Pixel(s->player.y) - s->camera_y),
            .world_x = Pixel(body->motion.x),
            .velocity_y = body->motion.vy,
            .boss_status = body->motion.status,
            .player_status = s->player.status,
            .contact = body->motion.contact,
            .tick = s->ticks,
            .step = body->step,
            .substep = body->substep,
            .facing_left = body->facing_left,
            .player_weapon_starting = s->player.weapon_starting,
            .player_subweapon_starting = s->player.subweapon_starting};
        if (!SotnDoppleAI_Tick(&observation, &s->combat_random, &s->dopple.ai))
            return false;
        Uint8 knives = 0;
        for (Uint16 i = 16; i < 80; ++i)
            if (s->slots[i].state.generation && s->slots[i].state.id == 0x104)
                ++knives;
        const Sint32 lost = (Sint32)s->dopple.previous_hp - a->state.hit_points;
        const SotnDoppleInput input = {.buttons = s->dopple.ai.buttons,
                                       .knives = knives,
                                       .hp_lost = (Uint16)SDL_max(0, lost),
                                       .effect_index = a->damage_effect,
                                       .player_x = Pixel(s->player.x),
                                       .hit = a->hit_flags > 0 && a->hit_flags < 4,
                                       .lethal = (a->state.flags & 0x100U) != 0};
        body->tick = s->ticks;
        if (!SotnDoppleBody_Update(s->nz0, &s->map, input, &s->combat_random, body) ||
            !Requests(s, a))
            return false;
        if (body->motion.status & 0x40000U)
            s->boss_flags |= 2;
        if (body->death.complete)
            s->boss_flags |= 4;
    }
    s->dopple.previous_hp = a->state.hit_points;
    a->state.step = body->step;
    a->state.x = body->motion.x;
    a->state.y = body->motion.y;
    a->vx = body->motion.vx;
    a->vy = body->motion.vy;
    a->state.animation_set = body->animation_set;
    a->state.frame = body->animation.sprite;
    a->state.palette = body->palette;
    a->state.facing_left = body->facing_left;
    a->state.rotation = body->rotation;
    a->state.rotated = body->rotated;
    a->state.bat_offset = body->bat_offset;
    a->state.blend_mode = body->blend_mode;
    a->state.tinted = body->death.shading;
    a->state.red = body->death.red;
    a->state.green = body->death.green;
    a->state.blue = body->death.blue;
    if (body->animation.sprite)
    {
        a->state.hitbox_x = body->animation.hitbox_x;
        a->state.hitbox_y = body->animation.hitbox_y;
        a->state.hitbox_width = body->animation.hitbox_width;
        a->state.hitbox_height = body->animation.hitbox_height;
    }
    a->state.hitbox_state = (body->motion.status & 0x40000U) || body->timers[13] || body->timers[14]
                                ? 0
                                : s->dopple.hitbox_state;
    return true;
}
bool ActorDoppleEncounter(SotnActors *s, Actor *a)
{
    if (s == NULL || a == NULL || s->stage_id != 26)
        return SDL_SetError("BO4 encounter dispatch requires its own stage");
    switch (a->state.id)
    {
    case 0x16:
        return Root(s, a);
    case 0x101:
        return ActorDoppleFactory(s, a);
    case 0x102:
        return ActorDoppleSmoke(s, a);
    case 0x104:
        return ActorDoppleKnife(s, a);
    case 0x105:
        return ActorDoppleParticles(s, a);
    case 0x10b:
        return ActorDoppleBlink(s, a);
    case 0x123:
        return ActorDoppleTrail(s, a);
    case 0x124:
        return ActorDoppleWingHitbox(s, a);
    case 0x137:
        return ActorDoppleUnarmed(s, a);
    default:
        if (a->state.id >= 0x100 && a->state.id < 0x138)
            return SDL_SetError("BO4 requested unintegrated child 0x%x", a->state.id & 255U);
        a->state.unsupported = true;
        return true;
    }
}
