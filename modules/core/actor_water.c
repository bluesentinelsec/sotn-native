// SPDX-License-Identifier: AGPL-3.0-or-later
// st/water_effects.h, entity_splash_water.h and entity_water_drop.h, pinned
// sotn-decomp7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Stage data stays private.
#include "actors_internal.h"

static Sint32 Fixed(Uint32 bits)
{
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
static Sint32 Screen(Sint32 value, Sint32 camera)
{
    return Fixed((Uint32)value - (Uint32)camera * 65536U);
}
static Sint32 World(Sint32 value, Sint32 camera)
{
    return Fixed((Uint32)value + (Uint32)camera * 65536U);
}
static Uint16 Base(const SotnActors *s)
{
    return s->stage_id == 65 ? 0x34 : 0x2c;
}
static bool Occupancy(SotnActors *s, Uint32 *out)
{
    Uint32 used = 0;
    for (Uint16 i = 160; i < 192; ++i)
    {
        SotnActorPlacement placement = {0};
        Uint32 generation = 0;
        if (!SotnRoomActors_Get(s->spawns, i, &placement, &generation))
            return false;
        if (generation || s->slots[i].state.generation)
            used |= (Uint32)1 << (i - 160U);
    }
    *out = used;
    return true;
}
static bool Controller(SotnActors *s, Actor *a, SotnDataView tables, SotnDataView volumes)
{
    SotnWaterActor next = a->water.controller;
    Uint32 occupied = 0;
    if (!Occupancy(s, &occupied))
        return false;
    const Sint32 x = Screen(s->player.x, s->camera_x), y = Screen(s->player.y, s->camera_y);
    SotnWaterActorEvents events = {0};
    if (!SotnWaterActor_Tick(&next,
                             (SotnWaterActorInput){.volume = {.volumes = volumes,
                                                              .aspects = {tables.bytes, 16},
                                                              .player_x = (Sint16)Pixel(x),
                                                              .player_y = (Sint16)Pixel(y),
                                                              .scroll_x = (Sint16)s->camera_x,
                                                              .scroll_y = (Sint16)s->camera_y,
                                                              .parameters = a->state.parameters,
                                                              .status = s->player.status},
                                                   .x = x,
                                                   .y = y,
                                                   .velocity_x = s->player.velocity_x,
                                                   .velocity_y = s->player.velocity_y,
                                                   .contacts = s->player.contacts,
                                                   .occupied = occupied,
                                                   .priority = s->player.priority},
                             &events))
        return false;
    /* Preflight every fallible shared-owner boundary before creating any child. */
    if (s->effect_serial > 0x7fffffffU - events.count ||
        (Sint64)s->motion.x + events.current_x < -64LL * 65536 ||
        (Sint64)s->motion.x + events.current_x > 64LL * 65536 ||
        (Sint64)s->player.x + events.current_x < SDL_MIN_SINT32 ||
        (Sint64)s->player.x + events.current_x > SDL_MAX_SINT32)
        return SDL_SetError("Water controller cannot commit actual child/current ownership");
    for (Uint16 i = 0; i < events.count; ++i)
    {
        const SotnWaterChildPlan *p = &events.children[i];
        if (p->slot < 224 || p->slot > 255 || (occupied & ((Uint32)1 << (p->slot - 224U))) ||
            p->kind > SOTN_WATER_SIDE_SPLASH)
            return SDL_SetError("Water controller plan no longer matches actual auxiliary slots");
    }
    if (!a->state.step && !ActorInitialize(s, a, EntranceTable(s, 0xad0, 0xa60)))
        return false;
    for (Uint16 i = 0; i < events.count; ++i)
    {
        const SotnWaterChildPlan *p = &events.children[i];
        Actor *child = NULL;
        if (!ActorSpawnAt(s, a, (Uint16)(Base(s) + 1U + (Uint16)p->kind), p->parameters,
                          p->slot - 64U, &child))
            return false;
        child->state.x = World(p->x, s->camera_x);
        child->state.y = World(p->y, s->camera_y);
        child->state.priority = p->priority;
        child->state.facing_left = s->player.facing_left;
        child->water.effect.water_row = p->row;
        child->water.effect.current = p->current;
    }
    if (!ActorDisplace(s, events.current_x, 0, 0))
        return false;
    a->water.controller = next;
    if (events.depths_changed)
        s->water = events.volume;
    return true;
}
static void Quad(SotnActorPrimitive *p, const SotnPlayerEffectQuad *q)
{
    *p = (SotnActorPrimitive){.page = (Uint16)(q->page + (q->draw_mode & 0x60U)),
                              .palette = q->palette,
                              .priority = q->priority,
                              .priority_set = true,
                              .screen_space = true,
                              .active = !(q->draw_mode & 8U),
                              .textured = true,
                              .quad = true,
                              .blend = (q->draw_mode & 1U) != 0,
                              .modulate = (q->draw_mode & 4U) != 0,
                              .gouraud = (q->draw_mode & 4U) != 0};
    for (Uint16 v = 0; v < 4; ++v)
    {
        p->offset_x[v] = q->x[v];
        p->offset_y[v] = q->y[v];
        p->u[v] = q->u[v];
        p->v[v] = q->v[v];
        p->vertex_red[v] = q->red[v];
        p->vertex_green[v] = q->green[v];
        p->vertex_blue[v] = q->blue[v];
    }
}
static bool Effects(SotnActors *s, Actor *a, Uint16 role, SotnDataView tables, SotnDataView volumes,
                    SotnDataView sounds)
{
    const bool birth = !a->state.step;
    const bool drops = role >= 4;
    Uint16 rows = drops ? (role == 4 ? 1 : 33) : 0;
    if (!drops &&
        !SotnWaterEffect_Rows((SotnWaterChildKind)(role - 1U), a->state.parameters, &rows))
        return false;
    if (birth)
    {
        if (!(role == 5 ? ActorPrimitivesContiguous(s, a, rows) : ActorPrimitives(s, a, rows)))
        {
            a->remove = true;
            return true;
        }
        if (!ActorInitialize(s, a, EntranceTable(s, 0xb00, 0xa90)))
            goto fail;
        a->state.flags |= 0x800000U;
    }
    if (a->state.primitive_count != rows)
    {
        SDL_SetError("Water child lost its actual primitive row count");
        goto fail;
    }
    for (Uint16 i = 0; i < rows; ++i)
        if (!ActorPrimitiveGet(s, a, i))
            goto fail;
    const Sint32 x = Screen(a->state.x, s->camera_x), y = Screen(a->state.y, s->camera_y);
    Uint32 rng = role == 4 ? s->random : s->combat_random;
    if (drops)
    {
        SotnWaterDrops next = birth ? (SotnWaterDrops){.x = x,
                                                       .y = y,
                                                       .velocity_y = a->vy,
                                                       .parameter = a->state.parameters,
                                                       .small = role == 4}
                                    : a->water.drops;
        if ((!birth && !SotnWaterDrops_Translate(&next, Wrap16(Pixel(x) - Pixel(next.x)),
                                                 Wrap16(Pixel(y) - Pixel(next.y)))) ||
            !SotnWaterDrops_Tick(&next,
                                 (SotnWaterDropsInput){.tables = tables,
                                                       .priority = a->state.priority,
                                                       .allocated_rows = rows},
                                 &rng))
            goto fail;
        a->water.drops = next;
        a->remove = next.removed;
        a->state.x = World(next.x, s->camera_x);
        a->state.y = World(next.y, s->camera_y);
        a->vx = next.velocity_x;
        a->vy = next.velocity_y;
        for (Uint16 i = 0; i < rows; ++i)
        {
            const SotnWaterDropTile *q = &next.tiles[i];
            ActorPrimitiveGet(s, a, i)->state = (SotnActorPrimitive){
                .x = (Sint32)q->x * 65536,
                .y = (Sint32)q->y * 65536,
                .width = q->width,
                .height = q->height,
                .red = q->red,
                .green = q->green,
                .blue = q->blue,
                .page = (Uint16)(q->draw_mode & 0x60U),
                .priority = q->priority,
                .priority_set = true,
                .screen_space = true,
                .blend = (q->draw_mode & 1U) != 0,
                .active = !next.removed && q->width && q->height && !(q->draw_mode & 8U)};
        }
    }
    else
    {
        SotnWaterEffect next = birth ? (SotnWaterEffect){.x = x,
                                                         .y = y,
                                                         .velocity_x = a->vx,
                                                         .velocity_y = a->vy,
                                                         .kind = (SotnWaterChildKind)(role - 1U),
                                                         .parameter = a->state.parameters,
                                                         .water_row = a->water.effect.water_row,
                                                         .current = a->water.effect.current}
                                     : a->water.effect;
        SotnWaterEffectEvents events = {0};
        if ((!birth && !SotnWaterEffect_Translate(&next, Wrap16(Pixel(x) - Pixel(next.x)),
                                                  Wrap16(Pixel(y) - Pixel(next.y)))) ||
            !SotnWaterEffect_Tick(&next,
                                  (SotnWaterEffectInput){.tables = tables,
                                                         .volumes = volumes,
                                                         .executable = s->executable,
                                                         .scroll_x = (Sint16)s->camera_x,
                                                         .scroll_y = (Sint16)s->camera_y,
                                                         .priority = a->state.priority,
                                                         .allocated_rows = rows},
                                  &rng, &events))
            goto fail;
        Uint32 occupied = 0;
        if (events.drop && (!Occupancy(s, &occupied) ||
                            (occupied != SDL_MAX_UINT32 && s->effect_serial >= 0x7fffffffU)))
        {
            SDL_SetError("Water splash cannot reserve its actual optional drop generation");
            goto fail;
        }
        if (events.sound && s->combat.sound_count >= SDL_arraysize(s->combat.sounds))
        {
            SDL_SetError("Water splash cannot commit its original sound request");
            goto fail;
        }
        if (events.drop && occupied != SDL_MAX_UINT32)
        {
            Actor *child = NULL;
            if (!ActorSpawn(s, a, (Uint16)(Base(s) + 5U), 0, 160, 192, &child) || !child)
                goto fail;
            child->state.x = World(events.drop_x, s->camera_x);
            child->state.y = World(events.drop_y, s->camera_y);
            child->vy = events.drop_velocity_y;
        }
        if (events.sound)
        {
            const Uint16 id = Read16(sounds.bytes + events.sound_index * 2U);
            if (!(events.positional ? ActorSoundVolumePan(s, id, events.volume, events.pan)
                                    : ActorSound(s, id)))
                goto fail;
        }
        a->water.effect = next;
        a->remove = next.removed;
        a->state.x = World(next.x, s->camera_x);
        a->state.y = World(next.y, s->camera_y);
        a->state.step = next.phase;
        a->vx = next.velocity_x;
        a->vy = next.velocity_y;
        for (Uint16 i = 0; i < rows; ++i)
        {
            Quad(&ActorPrimitiveGet(s, a, i)->state, &next.quads[i]);
            if (next.removed)
                ActorPrimitiveGet(s, a, i)->state.active = false;
        }
    }
    if (role == 4)
        s->random = rng;
    else
        s->combat_random = rng;
    return true;
fail:
    if (birth)
    {
        ActorPrimitivesClear(s, a);
        a->state.primitive_count = 0;
    }
    return false;
}
bool ActorWater(SotnActors *s, Actor *a)
{
    if (!s || !a || (s->stage_id != 65 && s->stage_id != 7) || a->state.id < Base(s) ||
        a->state.id > Base(s) + 5U)
        return SDL_SetError("Ordinary water actor requires its actual Entrance stage identity");
    SotnDataView tables = {0}, volumes = {0}, sounds = {0};
    if (!Slice(s->nz0, EntranceTable(s, 0x3858, 0x2168), 188, &tables) ||
        !Slice(s->nz0, EntranceTable(s, 0x13ac, 0x1230), 48, &volumes) ||
        !Slice(s->nz0, EntranceTable(s, 0x13a8, 0x122c), 4, &sounds))
        return false;
    const Uint16 role = (Uint16)(a->state.id - Base(s));
    return role ? Effects(s, a, role, tables, volumes, sounds) : Controller(s, a, tables, volumes);
}
