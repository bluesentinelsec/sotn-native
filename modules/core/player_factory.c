// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/7879C.c:CreateEntFactoryFromEntity/EntityFactory and get_free_entity.h,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Blueprints/ranges stay private.
#include <SDL3/SDL.h>
#include <sotn/player_factory.h>

bool SotnPlayerFactory_Begin(SotnPlayerFactory *factory, SotnPlayerFactorySource source,
                             Uint32 packed, Sint16 argument)
{
    const Uint16 blueprint = (Uint16)(packed & 0xfffU);
    if (!factory || blueprint >= 123 || source.priority > 511)
        return SDL_SetError("Player factory requires its reserved owner and original request");
    SotnPlayerFactory next = {.source = source,
                              .blueprint = blueprint,
                              .parameter = (Uint16)((packed >> 8) & 0xff00U),
                              .entity_id = 1,
                              .argument = argument};
    next.source.flags &= 0x10000U;
    if (packed & 0x5000U)
        next.entity_modifier = 0xe0;
    if (packed & 0xa000U)
        next.entity_modifier = 0xf0;
    if (packed & 0x1000U)
        next.entity_id = 0xef;
    if (packed & 0x2000U)
        next.entity_id = 0xff;
    if (packed & 0x4000U)
        next.entity_id = 0xed;
    if (packed & 0x8000U)
        next.entity_id = 0xfd;
    *factory = next;
    return true;
}
static bool Read(SotnDataView dra, SotnPlayerFactory *f, Uint16 *begin, Uint16 *end)
{
    const size_t at = 0xd1d4U + f->blueprint * 6U;
    if (!dra.bytes || at > dra.size || dra.size - at < 6)
        return SDL_SetError("Player factory blueprint is truncated");
    if (!f->initialized)
    {
        const Uint8 *p = dra.bytes + at;
        f->child = p[0];
        f->amount = p[1];
        f->per_cycle = p[2] & 63U;
        f->noncritical = (p[2] & 128U) != 0;
        f->increment_parameter = (p[2] & 64U) != 0;
        f->cycle = p[3];
        f->kind = p[4] & 15U;
        f->origin = p[4] >> 4;
        f->delay = p[5];
    }
    if (!f->amount || !f->per_cycle || f->per_cycle > 63 || f->kind >= 10 || f->origin > 15 ||
        f->spawn_index >= f->amount)
        return SDL_SetError("Player factory has unsupported original scheduling or range metadata");
    const size_t range = 0xd4b8U + f->kind * 2U;
    if (range > dra.size || dra.size - range < 2)
        return SDL_SetError("Player factory entity range is truncated");
    *begin = dra.bytes[range];
    *end = dra.bytes[range + 1U];
    if (*begin < 16 || *begin > *end || *end >= 64 || (f->kind == 3 && *begin != *end))
        return SDL_SetError("Player factory entity range exceeds its actual shared slots");
    return true;
}
static bool Find(Uint64 occupied, Uint16 begin, Uint16 end, bool reverse, Uint16 *slot)
{
    for (Uint16 i = 0; i <= end - begin; ++i)
    {
        const Uint16 at = reverse ? (Uint16)(end - i) : (Uint16)(begin + i);
        if (!(occupied & ((Uint64)1 << at)))
        {
            *slot = at;
            return true;
        }
    }
    return false;
}
bool SotnPlayerFactory_Tick(SotnPlayerFactory *factory, SotnDataView dra, SotnPlayerFactoryInput in,
                            SotnPlayerFactoryEvents *out)
{
    if (!factory || !out || factory->removed || factory->blueprint >= 123 ||
        factory->source.priority > 511 || factory->spawn_index > 255)
        return SDL_SetError("Player factory requires its actual live bounded cursor");
    SotnPlayerFactory f = *factory;
    SotnPlayerFactoryEvents e = {.occupied = in.occupied};
    Uint16 begin = 0, end = 0;
    if (!Read(dra, &f, &begin, &end))
        return false;
    if (!f.initialized)
    {
        f.initialized = true;
        f.source.flags |= 0x4000000U;
        if (f.origin == 0 || f.origin == 6)
            f.source.flags |= 0x8000000U;
        if (f.origin == 4 || f.origin == 5)
            f.source.flags |= 0x20000U;
    }
    else if ((f.origin == 4 && in.player_step != 1) ||
             (f.origin == 5 && in.player_substep != 0x70) ||
             (f.origin == 7 && in.player_step != 10))
    {
        f.source.x = in.player_x;
        f.source.y = in.player_y;
        f.removed = true;
        goto done;
    }
    if (f.origin == 2 || f.origin == 4 || f.origin == 5 || f.origin == 7)
    {
        f.source.x = in.player_x;
        f.source.y = in.player_y;
    }
    if (f.delay)
    {
        --f.delay;
        if (f.delay)
            goto done;
        f.delay = f.cycle;
    }
    for (Uint16 i = 0; i < f.per_cycle; ++i)
    {
        Uint16 slot = begin;
        const bool replace = f.kind == 3;
        Uint16 first = begin, last = end;
        if (f.kind == 8)
        {
            const Uint16 part = f.spawn_index % 3U;
            first = part == 0 ? 17 : part == 1 ? 32 : 48;
            last = part == 0 ? 31 : part == 1 ? 47 : 63;
        }
        if (!replace && !Find(e.occupied, first, last, f.kind == 0, &slot))
        {
            f.removed = f.noncritical;
            if (!f.noncritical)
                f.delay = f.cycle;
            goto done;
        }
        if (replace)
            e.clear_weapon_state = true;
        const Uint16 entity_id = (Uint16)(f.child + f.entity_modifier);
        if (entity_id)
            e.occupied |= (Uint64)1 << slot;
        else
            e.occupied &= ~((Uint64)1 << slot);
        SotnPlayerFactorySource source = f.source;
        source.flags = f.source.flags & 0x10000U;
        const Sint32 argument = f.argument;
        e.children[e.count++] = (SotnPlayerFactoryChild){
            .source = source,
            .slot = slot,
            .replace = replace,
            .entity_id = entity_id,
            .parameter = (Uint16)(f.parameter + (f.increment_parameter ? f.spawn_index : i)),
            .argument_low = (Uint16)((Uint16)f.argument & 511U),
            .argument_high = (Sint16)(argument >= 0 ? argument / 512 : -((-argument + 511) / 512))};
        ++f.spawn_index;
        if (f.spawn_index == f.amount)
        {
            f.removed = true;
            goto done;
        }
    }
    f.delay = f.cycle;
done:
    if (f.removed)
        f.entity_id = 0;
    *factory = f;
    *out = e;
    return true;
}
