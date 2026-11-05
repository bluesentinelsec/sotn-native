// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected inventory/order, stat calculations and positive-word SDK root from sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5: dra/5D5BC.c and menu.c.
// Definitions, strings, icons, and progression tables remain private runtime inputs.
#include <SDL3/SDL.h>
#include <sotn/character.h>
#include <sotn/combat.h>
#include <sotn/hud_resources.h>
struct SotnCharacter
{
    SotnDataView dra;
    SotnCharacterState state;
    SotnCharacterStats stats;
    SotnFamiliarProgress familiars;
    Uint16 familiar;
    Uint16 stat_buff_mask;
    SotnPlaytime playtime;
    bool playtime_bound;
    SotnDataView executable; /**< Borrowed SDK image for Muramasa's signed-range SquareRoot0. */
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
static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
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
    /* Additive records and selected implemented armor/head/cape services. */
    return id == 83 || id == 24 || id == 25 || id == 87 || id == 14 || id == 15 || id == 17 ||
           id == 21 || id == 22 || (id >= 38 && id <= 44) || id == 45 || id == 56 || id == 78 ||
           id <= 9 || (id >= 26 && id <= 33) || id == 35 || id == 36 || id == 46 || id == 47 ||
           id == 48 || id == 49 || id == 51 || id == 54 || id == 57 || id == 60 ||
           (id >= 63 && id <= 73) || id == 76 || id == 77 || id == 79 || id == 82 || id == 84 ||
           id == 85 || id == 80 || id == 89;
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
    SotnItem item = {
        .id = id,
        .body = body,
        .attack = Signed16(p + 8),
        .defense = Signed16(p + 10),
        .type = body ? Read16(p + 28) : 0,
        .category = body ? 0 : p[14],
        .icon = Read16(p + (body ? 24 : 44)),
        .palette = Read16(p + (body ? 26 : 46)),
        .supported =
            body ? BodySupported(id)
                 : (id == 0 || SotnEquipment_IsShield(id) || id == 18 || id == 19 || id == 24 ||
                    id == 25 || id == 80 || id == 81 || id == 82 || id == 134 || id == 135 ||
                    id == 143 || id == 119 || id == 133 || id == 163 || id == 164 || id == 165 ||
                    (id != 125 && SotnEquipment_IsSword0(id)) || SotnEquipment_IsBasicFist(id) ||
                    SotnEquipment_IsPlainWeapon(id) || id == 123 || id == 168 || id == 128 ||
                    id == 166 || (id >= 29 && id <= 79) ||
                    (id >= 144 && id <= 162 && id != 146 && id != 147))};
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
        out[i] = item.name_ascii ? (char)c : (char)(c <= 94 ? c + 32 : '?');
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
        s->hearts > s->hearts_max || s->subweapon > 9 || s->level == 0 || s->level > 99 ||
        s->experience > 999999 || s->gold > 999999 || (s->laboratory_items[1] & 0xf8) != 0 ||
        !Order(s->hand_order, SOTN_HAND_ITEMS) || !Order(s->body_order, SOTN_BODY_ITEMS))
        return SDL_SetError("Character progress has invalid bounds or inventory order");
    Uint32 learned = 0;
    bool end = false;
    for (Uint32 i = 0; i < SOTN_SPELLS; ++i)
    {
        const Uint8 encoded = s->spells[i];
        if (!encoded)
        {
            end = true;
            continue;
        }
        if (end || encoded < 0x80 || encoded >= 0x80 + SOTN_SPELLS ||
            (learned & (1U << (encoded & 7U))))
            return SDL_SetError("Known spell acquisition order is invalid");
        learned |= 1U << (encoded & 7U);
    }
    if (learned != s->spells_learned)
        return SDL_SetError("Known spell mask disagrees with acquisition order");
    for (Uint32 i = 0; i < 4; ++i)
        if (s->base[i] == 0 || s->base[i] > 99)
            return SDL_SetError("Character base stat is outside 1..99");
    for (Uint32 i = 0; i < 7; ++i)
        if (s->equipment[i] >= (i < 2 ? SOTN_HAND_ITEMS : SOTN_BODY_ITEMS))
            return SDL_SetError("Equipped item exceeds its table");
    if ((SotnEquipment_IsTwoHand(s->equipment[0]) || SotnEquipment_IsTwoHand(s->equipment[1])) &&
        s->equipment[0] != s->equipment[1])
        return SDL_SetError("Two-hand equipment requires its one owned copy in both actual slots");
    // Original hand stock is a byte. Ordinary pickups cap at99, but a Fairy's later
    // unchecked decrement can wrap after another producer consumes the last spare.
    for (Uint32 i = 0; i < SOTN_BODY_ITEMS; ++i)
        if (s->body_count[i] > 99)
            return SDL_SetError("Body inventory exceeds 99");
    return true;
}
static Uint32 Root(Uint32 value)
{
    if (!value)
        return 0;
    // Original positive-word SDK SquareRoot0 normalization. Its table value is exactly the
    // integer root of normalized<<18; compute that value without embedding a private table.
    Uint32 bits = value;
    Uint16 leading = 0;
    while (!(bits & 0x80000000U))
    {
        bits <<= 1;
        ++leading;
    }
    const Uint16 even = (Uint16)(leading & 0xfffeU);
    const Uint32 normalized = even >= 24 ? value << (even - 24U) : value >> (24U - even);
    Uint32 remaining = normalized << 18, root = 0, bit = 1U << 30;
    while (bit > remaining)
        bit >>= 2;
    while (bit)
    {
        if (remaining >= root + bit)
        {
            remaining -= root + bit;
            root = (root >> 1) + bit;
        }
        else
            root >>= 1;
        bit >>= 2;
    }
    return (root << ((31U - even) / 2U)) >> 12;
}
// SDK SquareRoot0 for Muramasa's u32 counter: the signed input counts leading sign bits and
// shifts arithmetically, so counters from 0x80000000 index entries before the private table
// (0x8002DBE0, file 0x1E3E0). Nonnegative counters use the equivalent table-free Root.
static bool MuramasaRoot(const SotnCharacter *c, Sint32 *out)
{
    const Uint32 value = c->state.muramasa_blood;
    if (value <= (Uint32)SDL_MAX_SINT32)
    {
        *out = (Sint32)Root(value);
        return true;
    }
    Uint32 lzc = 0;
    const Uint32 inverted = ~value;
    while (lzc < 32 && !(inverted & (0x80000000U >> lzc)))
        ++lzc;
    if (lzc == 32)
    {
        *out = 0;
        return true;
    }
    const Uint32 even = lzc & ~1U, shift = (31U - even) >> 1;
    const Sint32 signed_value = (Sint32)(value - 0x80000000U) - SDL_MAX_SINT32 - 1;
    const Sint32 normalized = even >= 24
                                  ? (Sint32)(value << (even - 24U))
                                  : -(Sint32)(((Uint32)(-(signed_value + 1)) >> (24U - even)) + 1U);
    const Sint64 at = 0x1e3e0 + ((Sint64)normalized - 64) * 2;
    if (!c->executable.bytes || at < 0 || (Uint64)at + 2U > c->executable.size)
        return SDL_SetError("Muramasa's signed-range SquareRoot0 requires its bound SDK image");
    const Uint8 *p = c->executable.bytes + at;
    const Sint16 coefficient = (Sint16)(Uint16)(p[0] | (Uint16)p[1] << 8);
    *out = (Sint32)(((Uint32)(Sint32)coefficient << shift) >> 12);
    return true;
}
static bool ReadOwnedItem(const SotnCharacter *c, bool body, Uint16 id, SotnItem *out)
{
    if (!SotnItem_Read(c->dra, body, id, out))
        return false;
    if (!body && id == 125)
        out->supported = c->playtime_bound;
    return true;
}
static bool Calculate(SotnCharacter *c)
{
    SotnCharacterStats result = {0};
    SotnItem items[7] = {0};
    Sint32 bonus[4] = {0}, attack = 0, defense = 0, muramasa = 0;
    if ((c->state.equipment[0] == 141 || c->state.equipment[1] == 141) &&
        !MuramasaRoot(c, &muramasa))
        return false;
    for (Uint32 i = 0; i < 7; ++i)
    {
        if (!ReadOwnedItem(c, i >= 2, c->state.equipment[i], &items[i]))
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
    if ((items[0].category == 5 || items[1].category == 5) &&
        c->state.equipment[0] != c->state.equipment[1])
        return SDL_SetError("Two-hand equipment lost its paired actual slots");
    // IsAlucart: Alucart sword168 and shield167 in the hands with Alucart mail89 add LCK30.
    if ((c->state.equipment[0] == 168 || c->state.equipment[1] == 168) &&
        (c->state.equipment[0] == 167 || c->state.equipment[1] == 167) &&
        c->state.equipment[3] == 89)
        bonus[3] += 30;
    for (Uint32 i = 0; i < 4; ++i)
    {
        if ((i == 0 && (c->stat_buff_mask & (1U << SOTN_BUFF_STRENGTH))) ||
            (i == 2 && (c->stat_buff_mask & (1U << SOTN_BUFF_INTELLIGENCE))) ||
            (i == 3 && (c->stat_buff_mask & (1U << SOTN_BUFF_LUCK))))
            bonus[i] += 20;
        bonus[i] = SDL_min(99, bonus[i]);
        result.bonus[i] = (Sint16)bonus[i];
        const Sint32 multiplier = i == 1 ? 8 : i == 2 ? 4 : 1;
        result.total[i] = (Uint16)SDL_max(0, c->state.base[i] + bonus[i] * multiplier);
    }
    if (c->stat_buff_mask & (1U << SOTN_BUFF_ATTACK))
        attack += 20;
    for (Uint32 i = 0; i < 2; ++i)
        result.attack[i] =
            items[i].category == 6 || items[i].category == 10 || items[i].id == 16 ||
                    (items[i].category == 9 && items[i].attack == 1)
                ? 0
                : (Uint16)SDL_clamp(
                      Word(items[i].attack + attack +
                           (items[i].attack <= result.total[0] ? result.total[0]
                                                               : result.total[0] / 2) +
                           (items[i].id == 125 ? Word(c->playtime.hours) : 0) +
                           (items[i].id == 141 ? Word(muramasa) : 0) +
                           // Shield rod4 beside a shield (category9) gains five.
                           (items[i].id == 4 && items[1 - i].category == 9 ? 5 : 0)),
                      0, 999);
    SotnItem empty = {0};
    if (!SotnItem_Read(c->dra, false, 0, &empty))
        return false;
    result.unarmed_attack = (Uint16)SDL_clamp(
        Word(empty.attack + attack +
             (empty.attack <= result.total[0] ? result.total[0] : result.total[0] / 2)),
        0, 999);
    if (c->stat_buff_mask & (1U << SOTN_BUFF_DEFENSE))
        defense += 20;
    if (c->stat_buff_mask & (1U << SOTN_BUFF_FIRE))
        result.resistance |= 0x8000;
    if (c->stat_buff_mask & (1U << SOTN_BUFF_ICE))
        result.resistance |= 0x2000;
    if (c->stat_buff_mask & (1U << SOTN_BUFF_THUNDER))
        result.resistance |= 0x4000;
    if (c->stat_buff_mask & (1U << SOTN_BUFF_CURSE))
        result.resistance |= 0x100;
    if (c->stat_buff_mask & (1U << SOTN_BUFF_HOLY))
        result.resistance |= 0x1000;
    if (c->stat_buff_mask & (1U << SOTN_BUFF_STONE))
        result.immunity |= 0x200;
    if (c->stat_buff_mask & (1U << SOTN_BUFF_DARK))
        result.resistance |= 0x800;
    result.defense = (Uint16)SDL_clamp(Word(defense + (Sint32)Root(result.total[1]) - 2), 0, 999);
    c->stats = result;
    return true;
}
static bool RetainedWeaponProperties(const SotnCharacter *c, SotnEquipment e, Uint16 other,
                                     bool poison, Uint32 *random, SotnEquipment *out)
{
    if (!c || !random || !out)
        return SDL_SetError("Retained weapon properties require actual character and copied RNG");
    Uint32 next_random = *random;
    const Sint32 critical =
        e.critical_rate +
        (Sint32)Root((Uint16)(c->stats.total[3] * 2U + (SotnCombat_Random(&next_random) & 15U))) -
        5;
    Sint16 bonus = 0;
    for (Uint16 i = 2; i < 7; ++i)
    {
        SotnItem item = {0};
        if (!SotnItem_Read(c->dra, true, c->state.equipment[i], &item))
            return false;
        bonus = Word(bonus + item.attack);
    }
    Sint16 attack = Word(
        e.attack + (e.attack <= c->stats.total[0] ? c->stats.total[0] : c->stats.total[0] / 2));
    attack = Word(attack + bonus);
    if (e.id == 125)
    {
        if (!c->playtime_bound)
            return SDL_SetError("Badelaire requires its actual main-loop playtime owner");
        attack = Word(attack + Word(c->playtime.hours));
    }
    if (e.id == 141)
    {
        Sint32 root = 0;
        if (!MuramasaRoot(c, &root))
            return false;
        attack = Word(attack + Word(root));
    }
    // CalcAttack's Shield rod4 term: five more beside a shield (category9) in the other hand.
    if (e.id == 4)
    {
        SotnItem beside = {0};
        if (!SotnItem_Read(c->dra, false, other, &beside))
            return false;
        if (beside.category == 9)
            attack = Word(attack + 5);
    }
    if (c->stat_buff_mask & (1U << SOTN_BUFF_ATTACK))
        attack = Word(attack + 20);
    const bool no_attack =
        e.id == 16 || (SotnEquipment_IsBasicShield(e.id) && e.kind == 128 && e.attack == 1);
    e.attack = (Sint16)(SDL_clamp(attack, 0, 999) >> (poison ? 1 : 0));
    if (no_attack)
        e.attack = 0;
    e.critical_rate = (Uint16)SDL_clamp(critical, 0, 255);
    *random = next_random;
    *out = e;
    return true;
}
bool SotnCharacter_HandProperties(const SotnCharacter *c, Uint16 item, Uint16 hand, bool poison,
                                  Uint32 *random, SotnEquipment *out)
{
    if (!c || hand > 1 ||
        (item != 0 && !SotnEquipment_IsShield(item) && item != 18 && item != 174 && item != 19 &&
         !SotnEquipment_IsSword0(item) && !SotnEquipment_IsBasicFist(item) &&
         !SotnEquipment_IsPlainWeapon(item) && item != 123 && item != 168 && item != 128 &&
         item != 143 && item != 133 && item != 163 && item != 164 && item != 165 && item != 119))
        return SDL_SetError("Ordinary properties require an actual integrated hand item");
    SotnEquipment e = {0};
    return SotnEquipment_Read(c->dra, item, &e) &&
           RetainedWeaponProperties(c, e, c->state.equipment[1U - hand], poison, random, out);
}
bool SotnCharacter_SwordProperties(const SotnCharacter *c, bool poison, Uint32 *random,
                                   SotnEquipment *out)
{
    if (!c)
        return SDL_InvalidParamError("character");
    SotnEquipment e = {0};
    Uint16 mp = 0;
    return SotnEquipment_ReadSwordSpecial(c->dra, 123, &e, &mp) &&
           RetainedWeaponProperties(c, e, 0, poison, random, out);
}
bool SotnCharacter_ThrowingProperties(const SotnCharacter *c, Uint16 item, bool poison,
                                      Uint32 *random, SotnEquipment *out)
{
    if (!c)
        return SDL_InvalidParamError("character");
    SotnEquipment e = {0};
    return SotnEquipment_ReadThrowing(c->dra, item, &e) &&
           RetainedWeaponProperties(c, e, 0, poison, random, out);
}

bool SotnCharacter_NeutronProperties(const SotnCharacter *c, bool poison, Uint32 *random,
                                     SotnEquipment *out)
{
    if (!c)
        return SDL_InvalidParamError("character");
    SotnEquipment e = {0};
    return SotnEquipment_ReadNeutron(c->dra, &e) &&
           RetainedWeaponProperties(c, e, 0, poison, random, out);
}

bool SotnCharacter_MagicMissileProperties(const SotnCharacter *c, Uint16 item, bool poison,
                                          Uint32 *random, SotnEquipment *out)
{
    if (!c || (item != 0 && item != 24 && item != 25 && (item < 75 || item > 79) && item != 80 &&
               item != 81 && item != 82 && item != 134 && item != 135))
        return SDL_SetError("Magic Missile properties require an actual supported raw parent item");
    SotnEquipment e = {0};
    return (item != 0 ? SotnEquipment_ReadThrowing(c->dra, item, &e)
                      : SotnEquipment_Read(c->dra, 0, &e)) &&
           RetainedWeaponProperties(c, e, 0, poison, random, out);
}
bool SotnCharacter_SireProperties(const SotnCharacter *c, bool poison, Uint32 *random,
                                  SotnEquipment *out)
{
    if (!c)
        return SDL_InvalidParamError("character");
    SotnEquipment e = {0};
    return SotnEquipment_ReadThrowing(c->dra, 72, &e) &&
           RetainedWeaponProperties(c, e, 0, poison, random, out);
}

bool SotnCharacter_SetPlaytime(SotnCharacter *c, SotnPlaytime clock)
{
    if (!c)
        return SDL_InvalidParamError("character");
    SotnCharacter next = *c;
    next.playtime = clock;
    next.playtime_bound = true;
    if ((!c->playtime_bound || c->playtime.hours != clock.hours) && !Calculate(&next))
        return false;
    *c = next;
    return true;
}
bool SotnCharacter_BindExecutable(SotnCharacter *c, SotnDataView executable)
{
    if (!c || !executable.bytes || executable.size < 0x1e3e0 + 192U * 2U)
        return SDL_SetError("Character SDK binding requires the complete private executable");
    SotnCharacter next = *c;
    next.executable = executable;
    if (!Calculate(&next))
        return false;
    *c = next;
    return true;
}
bool SotnCharacter_AddMuramasaBlood(SotnCharacter *c)
{
    if (!c)
        return SDL_InvalidParamError("character");
    if (c->state.muramasa_blood == SDL_MAX_UINT32)
        return true;
    SotnCharacter next = *c;
    ++next.state.muramasa_blood;
    if (!Calculate(&next))
        return false;
    *c = next;
    return true;
}
bool SotnCharacter_CommitMainLoop(SotnCharacter *c, SotnPlaytime clock, Uint32 random)
{
    if (!c)
        return SDL_InvalidParamError("character");
    SotnCharacter next = *c;
    if (!SotnCharacter_SetPlaytime(&next, clock))
        return false;
    next.state.random = random;
    *c = next;
    return true;
}
bool SotnCharacter_SetStatBuffMask(SotnCharacter *c, Uint16 mask)
{
    if (!c)
        return SDL_InvalidParamError("character");
    if (c->stat_buff_mask == mask)
        return true;
    SotnCharacter next = *c;
    next.stat_buff_mask = mask;
    if (!Calculate(&next))
        return false;
    *c = next;
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
    if (!SotnFamiliarProgress_Init(&c->familiars))
    {
        SDL_free(c);
        return false;
    }
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
bool SotnCharacter_OpeningGear(SotnCharacter *c)
{
    if (c == NULL)
        return SDL_InvalidParamError("character");
    SotnCharacter next = *c;
    const Uint16 gear[SOTN_EQUIP_SLOTS] = {123, 16, 45, 15, 56, 78, 57};
    SDL_memcpy(next.state.equipment, gear, sizeof(gear));
    if (!Calculate(&next))
        return false;
    *c = next;
    return true;
}

bool SotnCharacter_GetFamiliars(const SotnCharacter *c, SotnFamiliarProgress *p, Uint16 *selected)
{
    if (!c || (!p && !selected))
        return SDL_SetError("Familiar query requires its actual character and an output");
    if (p)
        *p = c->familiars;
    if (selected)
        *selected = c->familiar;
    return true;
}
bool SotnCharacter_RestoreFamiliars(SotnCharacter *c, const SotnFamiliarProgress *p)
{
    if (!c)
        return SDL_InvalidParamError("character");
    return SotnCharacter_RestoreFamiliarState(c, p, c->familiar);
}
bool SotnCharacter_RestoreFamiliarState(SotnCharacter *c, const SotnFamiliarProgress *p,
                                        Uint16 selected)
{
    if (!c || selected > SOTN_FAMILIAR_COUNT)
        return SDL_SetError("Familiar restoration requires its actual character and identity0..7");
    if (!SotnFamiliarProgress_Validate(p))
        return false;
    c->familiars = *p;
    c->familiar = selected;
    return true;
}
bool SotnCharacter_SelectFamiliar(SotnCharacter *c, Uint16 selected, bool reloaded)
{
    if (!c || selected > SOTN_FAMILIAR_COUNT || (!selected && reloaded))
        return SDL_SetError("Familiar selection requires actual identity0..7 and load ownership");
    if (selected && (selected != c->familiar || reloaded) &&
        !SotnFamiliarProgress_Load(&c->familiars, selected))
        return false;
    c->familiar = selected;
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

// dra/5F60C.c:InitStatsAndGear(true), at the pinned reconstructed US revision.
bool SotnCharacter_DeathTheft(SotnCharacter *c)
{
    if (c == NULL)
        return SDL_InvalidParamError("character");
    SotnCharacter next = *c;
    const Uint16 items[6] = {123, 16, 45, 15, 56, 78};
    for (Uint32 i = 0; i < 6; ++i)
    {
        const bool hand = i < 2;
        const Uint32 first = hand ? 0 : i == 5 ? 5 : i;
        const Uint32 last = hand ? 2 : i == 5 ? 7 : i + 1;
        bool equipped = false;
        for (Uint32 slot = first; slot < last; ++slot)
            if (next.state.equipment[slot] == items[i])
            {
                next.state.equipment[slot] = Empty(slot);
                equipped = true;
                break;
            }
        Uint8 *count = hand ? next.state.hand_count : next.state.body_count;
        if (!equipped && count[items[i]])
            --count[items[i]];
    }
    if (!Calculate(&next))
        return false;
    *c = next;
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
            if (empty == size && counts[order[i]] == 0 && !IsEmpty(body, order[i]))
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
bool SotnCharacter_PlanCollect(const SotnCharacter *c, SotnCharacterState *state, bool body,
                               Uint16 id)
{
    if (!c || !state || !SotnCharacter_Validate(state))
        return SDL_SetError("Inventory planning requires actual character progress");
    SotnCharacter next = *c;
    next.state = *state;
    if (!SotnCharacter_Collect(&next, body, id))
        return false;
    *state = next.state;
    return true;
}
bool SotnCharacter_LearnSpell(SotnCharacterState *s, Uint16 id)
{
    if (id >= SOTN_SPELLS || !SotnCharacter_Validate(s))
        return SDL_SetError("Spell learning requires valid progress and an original spell ID");
    if (s->spells_learned & (1U << id))
        return true;
    for (Uint32 i = 0; i < SOTN_SPELLS; ++i)
        if (!s->spells[i])
        {
            s->spells[i] = (Uint8)(0x80U | id);
            s->spells_learned |= 1U << id;
            return true;
        }
    return SDL_SetError("Known spell order has no free entry");
}
bool SotnCharacter_Trade(SotnCharacter *c, bool body, Uint16 id, Uint16 quantity, Uint32 price,
                         bool sell)
{
    if (!c || !quantity || quantity > 99 || price > 999999)
        return SDL_SetError("Equipment trade requires a character, 1..99 copies and valid price");
    SotnItem item = {0};
    if (!SotnItem_Read(c->dra, body, id, &item))
        return false;
    if (IsEmpty(body, id))
        return SDL_SetError("Virtual empty equipment cannot be traded");
    SotnCharacter next = *c;
    Uint8 *count = body ? next.state.body_count : next.state.hand_count;
    Uint32 owned = count[id];
    for (Uint32 i = body ? 2U : 0U; i < (body ? 7U : 2U); ++i)
        if (next.state.equipment[i] == id)
            ++owned;
    const Uint32 total = quantity * price;
    if (sell)
    {
        if (count[id] < quantity)
            return SDL_SetError("Not enough unequipped copies to sell");
        count[id] = (Uint8)(count[id] - quantity);
        next.state.gold = SDL_min(999999U, next.state.gold + total);
    }
    else
    {
        if (owned > 99 || quantity > 99U - owned || next.state.gold < total)
            return SDL_SetError("Purchase exceeds owned-copy limit or available gold");
        for (Uint16 i = 0; i < quantity; ++i)
            if (!SotnCharacter_Collect(&next, body, id))
                return false;
        next.state.gold -= total;
    }
    *c = next;
    return true;
}
static bool Equip(SotnCharacter *c, Uint32 slot, Uint16 id, bool confirmation)
{
    if (c == NULL || slot >= 7)
        return SDL_SetError("Equipment slot must be 0..6");
    const bool body = slot >= 2;
    if (!body && (id == 146 || id == 147))
        return SDL_SetError("This item cannot be equipped");
    SotnItem item = {0};
    if (!ReadOwnedItem(c, body, id, &item))
        return false;
    if (!item.supported || (body && item.type != Type(slot)))
        return SDL_SetError(
            "This item requires an unintegrated behavior or another equipment slot");
    const Uint16 old = c->state.equipment[slot];
    if (old == id && !confirmation)
        return true;
    SotnItem prior = {0}, other = {0};
    if (!body && (!SotnItem_Read(c->dra, false, old, &prior) ||
                  !SotnItem_Read(c->dra, false, c->state.equipment[1U - slot], &other)))
        return false;
    SotnCharacter next = *c;
    Uint8 *count = body ? next.state.body_count : next.state.hand_count;
    if (!IsEmpty(body, id))
    {
        if (count[id] == 0)
            return SDL_SetError("No unequipped copy of this item is owned");
        --count[id];
    }
    if (!SotnCharacter_Collect(&next, body, old))
        return false;
    next.state.equipment[slot] = id;
    if (!body && prior.category == 5)
        next.state.equipment[1U - slot] = 0;
    if (!body && item.category == 5)
    {
        if (other.category != 5 && !SotnCharacter_Collect(&next, false, other.id))
            return false;
        next.state.equipment[1U - slot] = id;
    }
    if (!Calculate(&next))
        return false;
    *c = next;
    return true;
}
bool SotnCharacter_Equip(SotnCharacter *c, Uint32 slot, Uint16 id)
{
    return Equip(c, slot, id, false);
}
bool SotnCharacter_ConfirmEquipment(SotnCharacter *c, Uint32 slot, Uint16 id)
{
    return Equip(c, slot, id, true);
}
bool SotnCharacter_Item(const SotnCharacter *c, bool body, Uint16 id, SotnItem *out)
{
    if (c == NULL)
        return SDL_InvalidParamError("character");
    return ReadOwnedItem(c, body, id, out);
}

static bool UseHandConsumable(SotnCharacter *c, Uint16 hand, Uint16 item, Uint16 *equipped,
                              Uint16 kind)
{
    if (!c || hand > 1 || !equipped || c->state.equipment[hand] != item ||
        (kind == 132 ? item < 29 || item > 70
         : kind == 130
             ? item != 24 && item != 25 && (item < 75 || item > 79) && item != 80 && item != 81 &&
                   item != 82 && item != 134 && item != 135 && (item < 71 || item > 74)
             : item != 166 && (item < 144 || item > 162 || item == 146 || item == 147)))
        return SDL_SetError("Consumption requires its actual supported equipped hand item");
    SotnEquipment medicine = {0};
    if (!SotnEquipment_Read(c->dra, item, &medicine))
        return false;
    SotnCharacter next = *c;
    if (next.state.equipment[5] != 84 && next.state.equipment[6] != 84)
    {
        if (next.state.hand_count[item])
            --next.state.hand_count[item];
        else
        {
            next.state.equipment[hand] = 0;
            if (!Calculate(&next))
                return false;
        }
    }
    *equipped = next.state.equipment[hand];
    *c = next;
    return true;
}
bool SotnCharacter_UseFood(SotnCharacter *c, Uint16 hand, Uint16 item, Uint16 *equipped)
{
    return UseHandConsumable(c, hand, item, equipped, 132);
}
bool SotnCharacter_UseThrowing(SotnCharacter *c, Uint16 hand, Uint16 item, Uint16 *equipped)
{
    return UseHandConsumable(c, hand, item, equipped, 130);
}

bool SotnCharacter_UseNeutron(SotnCharacter *c, Uint16 hand, Uint16 *equipped)
{
    return UseHandConsumable(c, hand, 71, equipped, 130);
}
bool SotnCharacter_UseMedicine(SotnCharacter *c, Uint16 hand, Uint16 item, Uint16 *equipped)
{
    return UseHandConsumable(c, hand, item, equipped, 131);
}
bool SotnCharacter_UseLibraryCard(SotnCharacter *c, Uint16 hand, Uint16 *equipped)
{
    return SotnCharacter_UseMedicine(c, hand, 166, equipped);
}
bool SotnCharacter_UseFamiliarItem(SotnCharacter *c, Uint16 item)
{
    if (!c || item < 144 || item > 162)
        return SDL_SetError("Familiar consumption requires its actual mapped medicine-family item");
    // faerie.c decrements the bag byte at the committed pose, not at its earlier stock check.
    // Duplicator and the equipped copies are deliberately not involved in this operation.
    --c->state.hand_count[item];
    return true;
}

static bool Progression(const SotnCharacter *c, SotnDataView *thresholds, SotnDataView *health)
{
    if (c == NULL || !Resolve(c->dra, 0x800ac780, 100 * 4, thresholds) ||
        !Resolve(c->dra, 0x800a2ec0, 10 * 4, health))
        return SDL_SetError("Progression requires bounded private level and HP tables");
    Uint32 previous = 0;
    for (Uint32 i = 0; i < 100; ++i)
    {
        const Uint32 value = Read32(thresholds->bytes + i * 4);
        if (value > 999999 || (i < 2 && value != 0) || (i >= 2 && value <= previous))
            return SDL_SetError("Private experience thresholds are invalid");
        previous = value;
    }
    for (Uint32 i = 0; i < 10; ++i)
        if (Read32(health->bytes + i * 4) > 9999)
            return SDL_SetError("Private level HP gain exceeds the character limit");
    return true;
}
bool SotnCharacter_NextLevel(const SotnCharacter *c, Uint32 *remaining)
{
    if (c == NULL || remaining == NULL)
        return SDL_SetError("Next-level query requires character and output");
    SotnDataView thresholds = {0}, health = {0};
    if (!Progression(c, &thresholds, &health))
        return false;
    const Uint32 target = c->state.level == 99
                              ? c->state.experience
                              : Read32(thresholds.bytes + (c->state.level + 1U) * 4);
    if (target < c->state.experience)
        return SDL_SetError("Character EXP exceeds an unprocessed level threshold");
    *remaining = target - c->state.experience;
    return true;
}
bool SotnCharacter_GainExperience(SotnCharacter *c, Uint16 experience, Uint16 enemy_level,
                                  Uint16 *levels)
{
    if (c == NULL || levels == NULL || enemy_level > 99)
        return SDL_SetError("Enemy reward requires a character, level 0..99, and output");
    *levels = 0;
    if (experience == 0 || c->state.level == 99)
        return true;
    SotnDataView thresholds = {0}, health = {0};
    if (!Progression(c, &thresholds, &health))
        return false;
    SotnCharacter next = *c;
    Uint32 reward = experience;
    if (!SotnFamiliarProgress_Reward(&next.familiars, next.familiar, experience, next.state.level))
        return false;
    if (next.state.level > enemy_level)
    {
        for (Uint32 i = enemy_level; i < next.state.level; ++i)
            reward = reward * 2 / 3;
        if (reward == 0)
            reward = 1;
    }
    else
    {
        const Uint32 difference = SDL_min(5U, (Uint32)enemy_level - next.state.level);
        for (Uint32 i = 0; i < difference; ++i)
            reward += reward / 4;
    }
    next.state.experience =
        SDL_min(Read32(thresholds.bytes + 99 * 4), next.state.experience + reward);
    Uint16 gained = 0;
    while (next.state.level < 99 &&
           Read32(thresholds.bytes + (next.state.level + 1U) * 4) <= next.state.experience)
    {
        ++next.state.level;
        ++gained;
        const Uint32 hp = Read32(health.bytes + (next.state.level / 10U) * 4);
        const Uint32 mp = 4 + (SotnCombat_Random(&next.state.random) & 1);
        if (next.state.hp_max + hp > 9999 || next.state.mp_max + mp > 9999 ||
            next.state.hearts_max + 2U > 9999)
            return SDL_SetError("Level gain exceeds the integrated character resource bounds");
        next.state.hp = (Uint16)(next.state.hp + hp);
        next.state.hp_max = (Uint16)(next.state.hp_max + hp);
        next.state.mp_max = (Uint16)(next.state.mp_max + mp);
        next.state.hearts_max += 2;
    }
    /* The original recursion finishes all MP/HP gains before unwinding stat gains. */
    for (Uint32 level = 0; level < gained; ++level)
    {
        Uint32 total = 0;
        for (Uint32 stat = 0; stat < 4; ++stat)
        {
            const Uint16 gain = SotnCombat_Random(&next.state.random) & 1;
            if (next.state.base[stat] + gain <= 99)
            {
                next.state.base[stat] += gain;
                total += gain;
            }
        }
        if (total < 2)
        {
            const Uint16 stat = SotnCombat_Random(&next.state.random) & 3;
            next.state.base[stat] = (Uint16)SDL_min(99, next.state.base[stat] + 1);
        }
    }
    if (!Calculate(&next))
        return false;
    *c = next;
    *levels = gained;
    return true;
}
bool SotnCharacter_CollectVessel(SotnCharacter *c, bool life)
{
    if (c == NULL)
        return SDL_InvalidParamError("character");
    if (life)
    {
        if (c->state.hp_max == 9999)
            return true;
        c->state.hp_max = (Uint16)SDL_min(9999, c->state.hp_max + 5);
        c->state.hp = c->state.hp_max;
    }
    else
    {
        const Uint16 old = c->state.hearts_max;
        c->state.hearts_max = (Uint16)SDL_min(9999, c->state.hearts_max + 5);
        c->state.hearts = (Uint16)(c->state.hearts + c->state.hearts_max - old);
    }
    return true;
}

bool SotnCharacter_TickResources(SotnCharacter *c, Uint64 ticks)
{
    if (c == NULL)
        return SDL_InvalidParamError("character");
    const bool pendant = c->state.equipment[5] == 76 || c->state.equipment[6] == 76;
    SotnHudResources state = {0};
    SotnHudResourcesEvents events = {0};
    if (!SotnHudResources_Tick(&state,
                               (SotnHudResourcesInput){.game_timer = (Uint32)ticks,
                                                       .mp = c->state.mp,
                                                       .mp_max = c->state.mp_max,
                                                       .mystic_pendant = pendant},
                               &events))
        return false;
    c->state.mp = events.mp;
    return true;
}

bool SotnCharacter_AddHearts(SotnCharacter *c, Uint16 amount)
{
    if (!c || amount > 9999)
        return SDL_SetError("Heart gain requires actual character progress and bounded damage");
    c->state.hearts = (Uint16)SDL_min(c->state.hearts_max, (Uint32)c->state.hearts + amount);
    return true;
}
bool SotnCharacter_SpendMP(SotnCharacter *c, Uint16 amount)
{
    if (c == NULL || amount > 9999 || amount > c->state.mp)
        return SDL_SetError("Ability cost exceeds available character MP");
    c->state.mp = (Uint16)(c->state.mp - amount);
    return true;
}

void SotnCharacter_Destroy(SotnCharacter *c)
{
    SDL_free(c);
}
