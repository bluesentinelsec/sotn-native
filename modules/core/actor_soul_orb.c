// SPDX-License-Identifier: AGPL-3.0-or-later
// st/e_particles.h:EntitySoulStealOrb, pinned sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. All stage tables/artwork stay private.
#include "actors_internal.h"

static Sint32 Fixed(Uint32 bits)
{
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
static size_t Table(const SotnActors *s)
{
    switch (s->stage_id)
    {
    case 12:
        return 0x1f80;
    case 65:
        return 0x26d0;
    case 7:
        return 0x205c;
    case 0:
        return 0x15cc;
    case 1:
        return 0x2610;
    case 2:
        return 0x28d4;
    case 6:
        return 0x1a80;
    case 4:
        return 0x1f1c;
    case 26:
        return 0x10c0;
    case 25:
        return 0x1770;
    case 30:
        return 0x2508;
    default:
        return s->nz0.size;
    }
}
bool ActorSoulOrb(SotnActors *s, Actor *a, SotnPlayerState player)
{
    if (!s || !a || a->state.id != 7 || a->state.step > 1 ||
        (a->state.step && (a->state.primitive_count != 1 || a->soul_orb.phase != 1)))
        return SDL_SetError("Soul orb requires its actual common actor and owned GT4 row");
    const bool birth = !a->state.step;
    if (birth)
    {
        if (!ActorPrimitives(s, a, 1))
        {
            a->remove = true;
            return true;
        }
        if (!ActorInitialize(s, a, ActorTable(s, 0xbd4, 0xab8, 0xa48)))
            goto fail_birth;
        a->state.flags |= 0x800000U;
        a->state.animation_set = 0;
    }
    ActorPrimitive *row = ActorPrimitiveGet(s, a, 0);
    SotnDataView tables = {0};
    if (!row || !Slice(s->nz0, Table(s), 126, &tables))
        goto fail_birth;
    SotnSoulOrb orb = a->soul_orb;
    orb.x = Fixed((Uint32)a->state.x - (Uint32)s->camera_x * 65536U);
    orb.y = Fixed((Uint32)a->state.y - (Uint32)s->camera_y * 65536U);
    SotnSoulOrbEvents out = {0};
    if (!SotnSoulOrb_Tick(
            &orb,
            (SotnSoulOrbInput){.executable = s->executable,
                               .tables = tables,
                               .x = orb.x,
                               .y = orb.y,
                               .player_x = (Sint16)Wrap16(Pixel(player.x) - s->camera_x),
                               .player_y = (Sint16)Wrap16(Pixel(player.y) - s->camera_y),
                               .allocated = true,
                               .hit = a->hit_flags != 0,
                               .heal_pending = player.healing.pending || s->combat.blood_heal},
            &s->random, &out))
        goto fail_birth;
    if (out.contact)
    {
        if (s->combat.blood_contacts >= 32)
            return SDL_SetError("Soul orb exceeded its actual common healing actor pool");
        ++s->combat.blood_contacts;
        if (!s->combat.blood_heal)
            s->combat.blood_heal = out.heal;
    }
    a->soul_orb = orb;
    a->remove = orb.removed;
    a->state.x = Fixed((Uint32)orb.x + (Uint32)s->camera_x * 65536U);
    a->state.y = Fixed((Uint32)orb.y + (Uint32)s->camera_y * 65536U);
    a->vx = orb.velocity_x;
    a->vy = orb.velocity_y;
    a->state.scale_x = orb.scale_x;
    a->state.scale_y = orb.scale_y;
    a->state.frame = orb.frame;
    a->pose = orb.pose;
    a->remaining = orb.remaining;
    a->state.hitbox_state = orb.hitbox ? 1 : 0;
    row->state = (SotnActorPrimitive){.page = 0x18,
                                      .palette = 0x194,
                                      .priority = a->state.priority,
                                      .priority_set = true,
                                      .active = out.draw && !orb.removed,
                                      .textured = true,
                                      .quad = true};
    for (Uint16 i = 0; i < 4; ++i)
    {
        row->state.offset_x[i] = (Sint16)Wrap16(out.x[i] + s->camera_x);
        row->state.offset_y[i] = (Sint16)Wrap16(out.y[i] + s->camera_y);
        row->state.u[i] = out.u[i];
        row->state.v[i] = out.v[i];
    }
    return true;
fail_birth:
    if (birth)
    {
        ActorPrimitivesClear(s, a);
        a->state.primitive_count = 0;
    }
    return false;
}
