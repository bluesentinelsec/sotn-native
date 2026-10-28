// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/wolfForm.c original body creation/update ordering, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Tables/artwork remain private.
#include <sotn/wolf_runtime.h>
static Uint16 Identity(Uint16 part)
{
    return part == 0 ? 56 : part == 1 ? 55 : part == 2 ? 57 : part == 3 ? 58 : part == 13 ? 60 : 59;
}
static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
static Sint32 Shift(Sint32 value, Sint16 pixels)
{
    const Uint32 bits = (Uint32)value + (Uint32)(Sint32)pixels * 65536U;
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
static bool Valid(const SotnWolfRuntime *r)
{
    if (!r || (r->root_reserved && !r->generation) || r->body.parts[0].step > 1 ||
        r->root_reserved != (r->body.parts[0].step == 1) ||
        (r->environment_reserved != (r->environment_generation != 0)) ||
        r->environment_generation > r->serial)
        return SDL_SetError("Wolf body lost its actual entity or environment ownership");
    for (Uint16 i = 0; i < 14; ++i)
    {
        const SotnWolfRuntimeChild c = r->children[i];
        if (r->generation
                ? (!c.generation || c.generation > r->serial || c.lineage != r->generation ||
                   c.slot != 17U + i || c.identity != Identity(i))
                : c.generation || c.lineage || c.slot || c.identity)
            return SDL_SetError("Wolf body slot%u lost its actual identity/generation/lineage",
                                17U + i);
        for (Uint16 j = 0; j < i; ++j)
            if (c.generation && c.generation == r->children[j].generation)
                return SDL_SetError("Wolf body has duplicate actual child generations");
    }
    return true;
}
Uint64 SotnWolfRuntime_Occupied(const SotnWolfRuntime *r)
{
    Uint64 mask = 0;
    if (r)
        for (Uint16 i = 0; i < 14; ++i)
            if (r->children[i].generation)
                mask |= (Uint64)1 << (17U + i);
    return mask;
}
bool SotnWolfRuntime_Begin(SotnWolfRuntime *r, Uint32 generation, Uint64 other)
{
    if (!Valid(r) || !generation || r->generation ||
        r->serial > SDL_MAX_UINT32 - 14U - (r->environment_reserved ? 0U : 1U) ||
        (other & (((Uint64)0x3fff) << 17)))
        return SDL_SetError(
            "Wolf entry requires retired original body slots and a bounded actual lineage");
    SotnWolfRuntime next = *r;
    const SotnWolfCaptureBounds capture = next.body.capture;
    const Uint32 wave = next.body.wave;
    const Sint32 angle = next.body.target_angle;
    next.body = (SotnWolfArticulation){.capture = capture, .wave = wave, .target_angle = angle};
    next.head = (SotnWolfHead){0};
    next.effect = (SotnWolfRootEffect){0};
    next.generation = generation;
    for (Uint16 i = 0; i < 14; ++i)
        next.children[i] = (SotnWolfRuntimeChild){.generation = ++next.serial,
                                                  .lineage = generation,
                                                  .slot = (Uint16)(17U + i),
                                                  .identity = Identity(i)};
    *r = next;
    return true;
}
static bool Retain(SotnPrimitiveSlots *slots, SotnPrimitiveSlots *retained, const Uint16 *rows,
                   Uint16 count)
{
    for (Uint16 i = 0; i < count; ++i)
    {
        Uint16 id = rows[i];
        if (id >= SOTN_PRIMITIVE_SLOTS ||
            (retained->occupied[id / 64U] & ((Uint64)1 << (id % 64U))))
            return SDL_SetError("Wolf body retained duplicate or invalid primitive rows");
        retained->occupied[id / 64U] |= (Uint64)1 << (id % 64U);
        slots->occupied[id / 64U] |= (Uint64)1 << (id % 64U);
    }
    return true;
}
static void Rebase(SotnWolfRuntime *r, Sint16 x, Sint16 y)
{
    for (Uint16 i = 0; i < 14; ++i)
    {
        if (r->children[i].generation)
        {
            r->body.parts[i].x = Shift(r->body.parts[i].x, x);
            r->body.parts[i].y = Shift(r->body.parts[i].y, y);
        }
    }
    for (Uint16 i = 0; r->root_reserved && i < 6; ++i)
        for (Uint16 v = 0; v < 4; ++v)
        {
            r->effect.quads[i].x[v] = Word(r->effect.quads[i].x[v] + x);
            r->effect.quads[i].y[v] = Word(r->effect.quads[i].y[v] + y);
        }
}
bool SotnWolfRuntime_Tick(SotnWolfRuntime *r, SotnWolfControllerFrame *player,
                          SotnWolfRuntimeInput in, SotnWolfRuntimeEvents *out)
{
    if (!player || !out || !Valid(r) || (SotnWolfRuntime_Occupied(r) & in.other_occupied) ||
        (in.active && !r->generation))
        return SDL_SetError(
            "Wolf body update requires its actual unshared slot lineage and player");
    SotnWolfRuntime next = *r;
    SotnWolfControllerFrame p = *player;
    SotnWolfRuntimeEvents e = {0};
    SotnPrimitiveSlots retained = {0};
    if ((next.root_reserved && !Retain(&in.primitive_slots, &retained, next.root_rows, 6)) ||
        (next.environment_reserved &&
         !Retain(&in.primitive_slots, &retained, next.environment_rows, 2)))
        return false;
    Rebase(&next, in.screen_shift_x, in.screen_shift_y);
    if (in.entering)
        goto done;
    if (!in.active)
    {
        next.generation = 0;
        next.root_reserved = false;
        SDL_zero(next.root_rows);
        SDL_zero(next.children);
        for (Uint16 i = 0; i < 14; ++i)
            next.body.parts[i] = (SotnWolfPart){0};
        goto done;
    }
    if (!in.query)
        return SDL_SetError("Wolf body requires its actual room collision service");
    if (!next.environment_reserved)
    {
        bool allocated = false;
        if (!SotnPrimitiveSlots_Reserve(&in.primitive_slots, 2, next.environment_rows, &allocated))
            return false;
        if (allocated)
        {
            if (next.serial == SDL_MAX_UINT32)
                return SDL_SetError("Wolf environment generation overflow");
            next.environment_generation = ++next.serial;
            next.environment_reserved = true;
        }
    }
    if (!next.root_reserved &&
        !SotnPrimitiveSlots_Reserve(&in.primitive_slots, 6, next.root_rows, &next.root_reserved))
        return false;
    next.root.player_palette = in.palette;
    SotnWolfRootEvents root = {0};
    if (!SotnWolfRoot_Tick(&next.root, &next.body, &p, next.root_reserved, in.dra, in.executable,
                           in.query, in.context, &root))
        return false;
    if (next.root_reserved &&
        !SotnWolfRootEffect_Tick(
            &next.effect, root,
            Word(p.x >= 0 ? p.x / 65536 : (Sint32)(-((-(Sint64)p.x + 65535) / 65536))),
            Word(p.y >= 0 ? p.y / 65536 : (Sint32)(-((-(Sint64)p.y + 65535) / 65536))),
            p.motion.velocity_x, p.priority, p.motion.facing_left))
        return false;
    for (Uint16 slot = 18; slot <= 30; ++slot)
    {
        if (slot == 20)
        {
            SotnWolfHeadEvents he = {0};
            if (!SotnWolfHead_Tick(&next.head, &next.body, &p, next.root.player_palette,
                                   in.attack_requested, in.dra, in.executable, &he))
                return false;
            if (he.spell)
                e.spells[e.spell_count++] = (SotnWolfRuntimeSpell){
                    .generation = next.children[3].generation, .slot = 20, .spell = he.spell};
            e.sound = he.sound;
            e.bubble = he.bubble;
            e.bubble_source = he.bubble_source;
            if (!SotnWolfHead_Animate(&next.head, &next.body.parts[3], in.dra))
                return false;
        }
        else
        {
            SotnWolfArticulationEvents ae = {0};
            if (!SotnWolfArticulation_Tick(&next.body, &p, slot, next.root.player_palette, in.dra,
                                           in.executable, &ae))
                return false;
            e.mp_spent = (Uint16)(e.mp_spent + ae.mp_spent);
            if (ae.rush_attack)
                e.spells[e.spell_count++] = (SotnWolfRuntimeSpell){
                    .generation = next.children[1].generation, .slot = 18, .spell = 13};
        }
    }
done:
    *r = next;
    *player = p;
    *out = e;
    return true;
}
bool SotnWolfRuntime_Hit(SotnWolfRuntime *r, Uint32 generation, Uint8 flags)
{
    if (!Valid(r) || !generation || r->children[1].generation != generation ||
        !r->body.parts[1].step || flags < 1 || flags > 3)
        return SDL_SetError("Wolf rush contact requires its actual live front generation");
    r->body.parts[1].hit_flags |= flags;
    return true;
}
