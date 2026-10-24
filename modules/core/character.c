// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected inventory/order and unbuffed stat calculations from sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5: dra/5D5BC.c and menu.c.
// Definitions, strings, icons, and progression tables remain private runtime inputs.
#include <SDL3/SDL.h>
#include <sotn/character.h>
struct SotnCharacter
{
    SotnDataView dra;
    SotnCharacterState state;
    SotnCharacterStats stats;
};
static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
static Uint32 Read32(const Uint8 *p)
{
    return p[0] | (Uint32)p[1] << 8 | (Uint32)p[2] << 16 | (Uint32)p[3] << 24;
}
static Sint16 Signed16(const Uint8 *p)
{
    const Uint16 v = Read16(p);
    return (Sint16)(v < 32768 ? v : (Sint32)v - 65536);
}
static bool Resolve(SotnDataView dra, Uint64 address, size_t size, SotnDataView *out)
{
    if (dra.bytes == NULL || address < 0x800a0000 || address > SDL_MAX_UINT32 ||
        address - 0x800a0000 > dra.size || size > dra.size - (address - 0x800a0000))
        return SDL_SetError("Character metadata references invalid private DRA data");
    *out = (SotnDataView){dra.bytes + (size_t)(address - 0x800a0000), size};
    return true;
}
static bool Text(SotnDataView dra, Uint32 address, SotnDataView *out)
{
    SotnDataView first = {0};
    if (!Resolve(dra, address, 1, &first))
        return false;
    const size_t available = SDL_min((size_t)160, dra.size - (address - 0x800a0000));
    for (size_t i = 0; i < available; ++i)
        if (first.bytes[i] == 255)
        {
            *out = (SotnDataView){first.bytes, i};
            return true;
        }
    return SDL_SetError("Item text lacks a bounded original terminator");
}
static Uint16 Empty(Uint32 slot)
{
    return slot < 2 ? 0 : slot == 2 ? 26 : slot == 3 ? 0 : slot == 4 ? 48 : 57;
}
static Uint16 Type(Uint32 slot)
{
    return slot < 2 ? 0 : slot < 5 ? (Uint16)(slot - 2) : 3;
}
static bool IsEmpty(bool body, Uint16 id)
{
    return body ? (id == 0 || id == 26 || id == 48 || id == 57) : id == 0;
}
static bool BodySupported(Uint16 id)
{
    /* These ordinary records need no unintegrated passive/transform services. */
    return id <= 9 || (id >= 26 && id <= 33) || id == 35 || id == 46 || id == 47 || id == 48 ||
           id == 49 || id == 51 || id == 57 || (id >= 63 && id <= 73) || id == 77 || id == 79 ||
           id == 82 || id == 85;
}
bool SotnItem_Read(SotnDataView dra, bool body, Uint16 id, SotnItem *out)
{
    if (out == NULL || id >= (body ? SOTN_BODY_ITEMS : SOTN_HAND_ITEMS))
        return SDL_SetError("Item ID exceeds its original table");
    SotnDataView pointer = {0}, record = {0};
    if (!Resolve(dra, body ? 0x800a00c4 : 0x800a00c0, 4, &pointer) ||
        !Resolve(dra, (Uint64)Read32(pointer.bytes) + (Uint32)id * (body ? 32U : 52U),
                 body ? 30U : 50U, &record))
        return false;
    const Uint8 *p = record.bytes;
    SotnItem item = {.id = id,
                     .body = body,
                     .attack = Signed16(p + 8),
                     .defense = Signed16(p + 10),
                     .type = body ? Read16(p + 28) : 0,
                     .icon = Read16(p + (body ? 24 : 44)),
                     .palette = Read16(p + (body ? 26 : 46)),
                     .supported = body ? BodySupported(id) : (id == 0 || id == 86 || id == 128)};
    if (body)
    {
        if (item.type > 3)
            return SDL_SetError("Body item references an invalid equipment category");
        for (Uint32 i = 0; i < 4; ++i)
            item.bonus[i] = (Sint16)(p[12 + i] > 128 ? (Sint32)p[12 + i] - 256 : p[12 + i]);
        item.weakness = Read16(p + 16);
        item.resistance = Read16(p + 18);
        item.immunity = Read16(p + 20);
        item.absorption = Read16(p + 22);
    }
    if (item.icon >= 320 || item.palette >= 320)
        return SDL_SetError("Item icon or palette exceeds the original private table");
    if (!Text(dra, Read32(p), &item.name))
        return false;
    /* Descriptions use their own encoding/zero terminator; keep a bounded borrowed view. */
    SotnDataView description = {0};
    if (!Resolve(dra, Read32(p + 4), 1, &description))
        return false;
    const size_t length = SDL_min((size_t)160, dra.size - (Read32(p + 4) - 0x800a0000));
    size_t n = 0;
    while (n < length && description.bytes[n] != 0)
        ++n;
    if (n == length)
        return SDL_SetError("Item description lacks a bounded terminator");
    item.description = (SotnDataView){description.bytes, n};
    *out = item;
    return true;
}
bool SotnItem_Name(SotnItem item, char *out, size_t capacity)
{
    if (out == NULL || item.name.bytes == NULL || item.name.size + 1 > capacity)
        return SDL_SetError("Item name output is too small");
    for (size_t i = 0; i < item.name.size; ++i)
    {
        const Uint8 c = item.name.bytes[i];
        out[i] = (char)(c <= 94 ? c + 32 : '?');
    }
    out[item.name.size] = '\0';
    return true;
}
static bool Order(const Uint8 *order, Uint32 count)
{
    Uint8 seen[SOTN_HAND_ITEMS] = {0};
    for (Uint32 i = 0; i < count; ++i)
        if (order[i] >= count || seen[order[i]]++)
            return false;
    return true;
}
bool SotnCharacter_Validate(const SotnCharacterState *s)
{
    if (s == NULL || s->hp_max == 0 || s->hp_max > 9999 || s->hp > s->hp_max || s->mp_max == 0 ||
        s->mp_max > 9999 || s->mp > s->mp_max || s->hearts_max == 0 || s->hearts_max > 9999 ||
        s->hearts > s->hearts_max || s->level == 0 || s->level > 99 || s->experience > 999999 ||
        s->gold > 999999 || (s->laboratory_items[1] & 0xf8) != 0 ||
        !Order(s->hand_order, SOTN_HAND_ITEMS) || !Order(s->body_order, SOTN_BODY_ITEMS))
        return SDL_SetError("Character progress has invalid bounds or inventory order");
    for (Uint32 i = 0; i < 4; ++i)
        if (s->base[i] > 99)
            return SDL_SetError("Character base stat exceeds 99");
    for (Uint32 i = 0; i < 7; ++i)
        if (s->equipment[i] >= (i < 2 ? SOTN_HAND_ITEMS : SOTN_BODY_ITEMS))
            return SDL_SetError("Equipped item exceeds its table");
    for (Uint32 i = 0; i < SOTN_HAND_ITEMS; ++i)
        if (s->hand_count[i] > 99)
            return SDL_SetError("Hand inventory exceeds 99");
    for (Uint32 i = 0; i < SOTN_BODY_ITEMS; ++i)
        if (s->body_count[i] > 99)
            return SDL_SetError("Body inventory exceeds 99");
    return true;
}
static Uint16 Root(Uint16 v)
{
    Uint16 r = 0;
    while ((Uint32)(r + 1) * (r + 1) <= v)
        ++r;
    return r;
}
static bool Calculate(SotnCharacter *c)
{
    SotnCharacterStats result = {0};
    SotnItem items[7] = {0};
    Sint32 bonus[4] = {0}, attack = 0, defense = 0;
    for (Uint32 i = 0; i < 7; ++i)
    {
        if (!SotnItem_Read(c->dra, i >= 2, c->state.equipment[i], &items[i]))
            return false;
        if (!items[i].supported || (i >= 2 && items[i].type != Type(i)))
            return SDL_SetError(
                "Equipped item requires an unintegrated behavior or a different slot");
        defense += items[i].defense;
        if (i >= 2)
        {
            attack += items[i].attack;
            for (Uint32 j = 0; j < 4; ++j)
                bonus[j] += items[i].bonus[j];
            result.weakness |= items[i].weakness;
            result.resistance |= items[i].resistance;
            result.immunity |= items[i].immunity;
            result.absorption |= items[i].absorption;
        }
    }
    for (Uint32 i = 0; i < 4; ++i)
    {
        bonus[i] = SDL_min(99, bonus[i]);
        result.bonus[i] = (Sint16)bonus[i];
        const Sint32 multiplier = i == 1 ? 8 : i == 2 ? 4 : 1;
        result.total[i] = (Uint16)SDL_max(0, c->state.base[i] + bonus[i] * multiplier);
    }
    for (Uint32 i = 0; i < 2; ++i)
        result.attack[i] = (Uint16)SDL_clamp(
            items[i].attack + attack +
                (items[i].attack <= result.total[0] ? result.total[0] : result.total[0] / 2),
            0, 999);
    result.defense = (Uint16)SDL_clamp(defense + Root(result.total[1]) - 2, 0, 999);
    c->stats = result;
    return true;
}
bool SotnCharacter_Create(SotnDataView dra, SotnCharacter **out)
{
    if (out == NULL)
        return SDL_InvalidParamError("out");
    *out = NULL;
    SotnCharacter *c = SDL_calloc(1, sizeof(*c));
    if (c == NULL)
        return false;
    c->dra = dra;
    c->state.hp = c->state.hp_max = 70;
    c->state.mp = c->state.mp_max = 20;
    c->state.hearts = 10;
    c->state.hearts_max = 50;
    c->state.level = 1;
    for (Uint32 i = 0; i < 4; ++i)
        c->state.base[i] = 6;
    for (Uint32 i = 0; i < 7; ++i)
        c->state.equipment[i] = Empty(i);
    for (Uint32 i = 0; i < SOTN_HAND_ITEMS; ++i)
        c->state.hand_order[i] = (Uint8)i;
    for (Uint32 i = 0; i < SOTN_BODY_ITEMS; ++i)
        c->state.body_order[i] = (Uint8)i;
    if (!Calculate(c))
    {
        SDL_free(c);
        return false;
    }
    *out = c;
    return true;
}
bool SotnCharacter_Get(const SotnCharacter *c, SotnCharacterState *s, SotnCharacterStats *stats)
{
    if (c == NULL || (s == NULL && stats == NULL))
        return SDL_SetError("Character query requires state or stats output");
    if (s != NULL)
        *s = c->state;
    if (stats != NULL)
        *stats = c->stats;
    return true;
}
bool SotnCharacter_Restore(SotnCharacter *c, const SotnCharacterState *s)
{
    if (c == NULL || !SotnCharacter_Validate(s))
        return false;
    SotnCharacter next = *c;
    next.state = *s;
    if (!Calculate(&next))
        return false;
    *c = next;
    return true;
}
bool SotnCharacter_Collect(SotnCharacter *c, bool body, Uint16 id)
{
    if (c == NULL)
        return SDL_InvalidParamError("character");
    SotnItem item = {0};
    if (!SotnItem_Read(c->dra, body, id, &item))
        return false;
    if (IsEmpty(body, id))
        return true;
    Uint8 *counts = body ? c->state.body_count : c->state.hand_count;
    Uint8 *order = body ? c->state.body_order : c->state.hand_order;
    if (counts[id] >= 99)
        return true;
    if (counts[id] == 0)
    {
        const Uint32 size = body ? SOTN_BODY_ITEMS : SOTN_HAND_ITEMS;
        Uint32 existing = size, empty = size;
        for (Uint32 i = 0; i < size; ++i)
        {
            if (order[i] == id)
                existing = i;
            if (empty == size && counts[order[i]] == 0)
            {
                SotnItem other = {0};
                if (!SotnItem_Read(c->dra, body, order[i], &other))
                    return false;
                if (!body || other.type == item.type)
                    empty = i;
            }
        }
        if (existing == size || empty == size)
            return SDL_SetError("Inventory order lacks the collected item");
        if (existing > empty)
        {
            const Uint8 swap = order[empty];
            order[empty] = (Uint8)id;
            order[existing] = swap;
        }
    }
    ++counts[id];
    return true;
}
bool SotnCharacter_Equip(SotnCharacter *c, Uint32 slot, Uint16 id)
{
    if (c == NULL || slot >= 7)
        return SDL_SetError("Equipment slot must be 0..6");
    const bool body = slot >= 2;
    SotnItem item = {0};
    if (!SotnItem_Read(c->dra, body, id, &item))
        return false;
    if (!item.supported || (body && item.type != Type(slot)))
        return SDL_SetError(
            "This item requires an unintegrated behavior or another equipment slot");
    const Uint16 old = c->state.equipment[slot];
    if (old == id)
        return true;
    SotnCharacter next = *c;
    Uint8 *count = body ? next.state.body_count : next.state.hand_count;
    if (!IsEmpty(body, id))
    {
        if (count[id] == 0)
            return SDL_SetError("No unequipped copy of this item is owned");
        --count[id];
    }
    if (!IsEmpty(body, old))
        count[old] = (Uint8)SDL_min(99, count[old] + 1);
    next.state.equipment[slot] = id;
    if (!Calculate(&next))
        return false;
    *c = next;
    return true;
}
void SotnCharacter_Destroy(SotnCharacter *c)
{
    SDL_free(c);
}
