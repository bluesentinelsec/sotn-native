// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/{dai,no2}/e_breakable.c, st/e_breakable_with_debris.h and st_common.h
// at the pinned sotn-decomp revision. Original tables remain private inputs.
#include "actors_internal.h"
#include "boss_projection.h"
static size_t Table(const SotnActors *s, size_t chapel)
{
    return s->stage_id == 4 ? chapel - 0x128U : chapel;
}
static Uint16 DebrisId(const SotnActors *s)
{
    return s->stage_id == 4 ? 0x41 : 0x44;
}
static bool Spawn(SotnActors *s, const Actor *a, Uint16 id, Uint16 params, Uint32 first, Sint32 y,
                  bool facing)
{
    Actor *child = NULL;
    if (!ActorSpawn(s, a, id, params, first, first + 32U, &child))
        return false;
    if (child)
    {
        child->state.x = Pixel(a->state.x) * 65536;
        child->state.y = Wrap16(y) * 65536;
        child->state.facing_left = facing;
    }
    return true;
}
static bool Drop(SotnActors *s, Actor *a)
{
    a->permanent = true;
    if (!(s->world->relics[SOTN_RELIC_CUBE] & 2U))
    {
        a->remove = true;
        return true;
    }
    const Uint16 item = a->state.parameters & 0xfffU;
    a->state.id = item < 128 ? 3 : 0xa;
    a->state.parameters = item < 128 ? item : (Uint16)(item - 128);
    a->state.step = a->pose = a->remaining = a->state.frame = 0;
    a->dynamic = true;
    a->timer = 16;
    return true;
}
static bool Breakable(SotnActors *s, Actor *a)
{
    const Uint16 type = a->state.parameters >> 12;
    const Uint16 persistent = a->state.parameters & 0x1ffU;
    SotnDataView animation = {0};
    if (type >= 10 ||
        ((type == 7 || type == 8) &&
         persistent >= SotnStageProfile_Get(s->stage_id)->prize_count) ||
        !Slice(s->nz0, Table(s, 0xbf0U) + type * 4U, 4, &animation))
        return SDL_SetError("Castle breakable type or persistent drop is invalid");
    const Uint32 address = Read32(animation.bytes);
    if (address < 0x80180000 || (address & 1U))
        return SDL_SetError("Castle breakable animation pointer is invalid");
    if (!a->state.step)
    {
        SotnDataView height = {0}, palette = {0}, bank = {0}, texture = {0}, blend = {0}, y = {0};
        if (!Slice(s->nz0, Table(s, 0xc18U) + type, 1, &height) ||
            !Slice(s->nz0, Table(s, 0xc30U) + type * 2U, 2, &palette) ||
            !Slice(s->nz0, Table(s, 0xc44U) + type * 2U, 2, &bank) ||
            !Slice(s->nz0, Table(s, 0xc58U) + type * 2U, 2, &texture) ||
            !Slice(s->nz0, Table(s, 0xc6cU) + type, 1, &blend) ||
            !Slice(s->nz0, Table(s, 0xc78U) + type * 2U, 2, &y) ||
            !ActorInitialize(s, a, ActorStageOffset(s, 0xbc8)))
            return false;
        if (height.bytes[0] > 127)
            return SDL_SetError("Castle breakable hitbox exceeds the signed original range");
        a->state.priority = 128;
        a->state.blend_mode = blend.bytes[0];
        a->state.hitbox_height = height.bytes[0];
        a->state.animation_set = Read16(bank.bytes);
        a->state.texture_offset = Read16(texture.bytes);
        a->state.palette = Read16(palette.bytes);
        a->state.hitbox_y = (Sint16)Signed16(y.bytes);
    }
    Uint8 animated = 0;
    if (!ActorAnimate(s, a, address - 0x80180000, &animated))
        return false;
    if (!a->hit_params)
        return true;
    SotnDataView explosion = {0};
    if (!Slice(s->nz0, Table(s, 0xc24U) + type, 1, &explosion) || explosion.bytes[0] >= 5)
        return SDL_SetError("Castle breakable explosion profile is invalid");
    if (!ActorExplosionInPool(s, a, explosion.bytes[0], 160, 192, 0))
        return false;
    if (type == 2 || type == 3)
    {
        const Uint16 count = type == 2 ? 4 : 3;
        SotnDataView offsets = {0};
        if (!Slice(s->nz0, Table(s, 0xc90U) + (type == 3 ? 10U : 0U), count * 2U, &offsets))
            return false;
        a->state.facing_left = Side(a, s->player) != 0;
        for (Uint16 i = 0; i < count; ++i)
        {
            const Sint32 piece = Signed16(offsets.bytes + i * 2U);
            const Sint32 y = Pixel(a->state.y) - 40 + i * 16;
            if (piece < 0 || piece > 3)
                return SDL_SetError("Castle candelabra debris index is outside its texture strip");
            if (!Spawn(s, a, DebrisId(s), (Uint16)piece, 160, y, a->state.facing_left) ||
                !Spawn(s, a, 2, 0, 160, y, false))
                return false;
        }
    }
    if (type == 9 && !Spawn(s, a, DebrisId(s), 256, 96, Pixel(a->state.y), false))
        return false;
    if (!ActorSound(s, type >= 7 ? 0x61d
                                 : (s->stage_id == 4 && (type == 2 || type == 3) ? 0x67f : 0x634)))
        return false;
    if (type == 7 || type == 8)
    {
        if (!Spawn(s, a, 0xc, persistent, 96, Pixel(a->state.y), false))
            return false;
        a->permanent = a->remove = true;
        return true;
    }
    return Drop(s, a);
}
static bool Debris(SotnActors *s, Actor *a)
{
    if (a->state.parameters != 256 && a->state.parameters > 3)
        return SDL_SetError("Castle breakable debris index is invalid");
    if (!a->state.step)
    {
        if (a->state.parameters == 256)
        {
            SotnDataView bank = {0}, palette = {0}, texture = {0};
            if (!Slice(s->nz0, Table(s, 0xc44U) + 9U * 2U, 2, &bank) ||
                !Slice(s->nz0, Table(s, 0xc30U) + 9U * 2U, 2, &palette) ||
                !Slice(s->nz0, Table(s, 0xc58U) + 9U * 2U, 2, &texture) ||
                !ActorInitialize(s, a, ActorStageOffset(s, 0xbf8)))
                return false;
            a->state.animation_set = Read16(bank.bytes);
            a->state.palette = Read16(palette.bytes);
            a->state.texture_offset = Read16(texture.bytes);
            a->state.frame = 21;
            a->state.priority = 106;
            a->state.step = 256;
            return true;
        }
        if (!ActorInitialize(s, a, ActorStageOffset(s, 0xbe0)))
            return false;
        if (!ActorPrimitives(s, a, 2))
        {
            a->remove = true;
            return true;
        }
        a->state.flags |= 0x800000U;
        ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
        if (!p)
            return false;
        const Uint8 v = (Uint8)(132U + a->state.parameters * 16U);
        p->state = (SotnActorPrimitive){.page = 22,
                                        .palette = s->stage_id == 4 ? 0x255 : 0x24d,
                                        .u = {152, 167, 152, 167},
                                        .v = {v, v, (Uint8)(v + 15), (Uint8)(v + 15)},
                                        .red = 128,
                                        .green = 128,
                                        .blue = 128,
                                        .textured = true,
                                        .quad = true};
        a->vx = (Sint32)(ActorRandom(s) & 7U) * 4096 + 32768;
        if (!a->state.facing_left)
            a->vx = -a->vx;
        a->vy = (Sint32)(ActorRandom(s) & 7U) * 4096 - 32768;
    }
    if (a->state.step == 256)
        return true;
    const Sint64 x = (Sint64)a->state.x + a->vx, y = (Sint64)a->state.y + a->vy;
    if (a->state.step != 1 || x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 ||
        y > SDL_MAX_SINT32 || a->vy > SDL_MAX_SINT32 - 8192)
        return SDL_SetError("Castle breakable debris motion exceeds fixed-point bounds");
    a->state.x = (Sint32)x;
    a->state.y = (Sint32)y;
    a->vy += 8192;
    a->state.rotation =
        (Sint16)Wrap16((Sint64)a->state.rotation + (a->state.facing_left ? 16 : -16));
    ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
    SotnProjectedPoint points[4] = {0};
    if (!p || !SotnDemon_ProjectQuad(
                  s->executable,
                  (SotnDemonQuadPose){
                      .angle = a->state.rotation, .width = 16, .height = 16, .scale = 4096},
                  points))
        return false;
    p->state.x = Pixel(a->state.x) * 65536;
    p->state.y = Pixel(a->state.y) * 65536;
    p->state.active = true;
    for (Uint16 i = 0; i < 4; ++i)
    {
        p->state.offset_x[i] = (Sint16)points[i].x;
        p->state.offset_y[i] = (Sint16)points[i].y;
    }
    SotnCollider c = {0};
    if (!ActorQuery(s, Pixel(a->state.x), Pixel(a->state.y) + 8, &c))
        return false;
    if (c.effects & SOTN_EFFECT_SOLID)
    {
        if (!ActorSound(s, 0x683) || !ActorExplosionInPool(s, a, 0, 160, 192, 0))
            return false;
        a->remove = true;
    }
    return true;
}
bool ActorChapelBreakable(SotnActors *s, Actor *a)
{
    if (!s || !a || !s->world || (s->stage_id != 4 && s->stage_id != 6) ||
        (a->state.id != 1 && a->state.id != DebrisId(s)))
        return SDL_SetError("Castle breakables require their original stage/world owners");
    return a->state.id == 1 ? Breakable(s, a) : Debris(s, a);
}
