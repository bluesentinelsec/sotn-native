#include <sotn/save.h>
#include <sotn/stage_profile.h>

static Uint32 Read32(const Uint8 *p)
{
    return p[0] | (Uint32)p[1] << 8 | (Uint32)p[2] << 16 | (Uint32)p[3] << 24;
}
static void Put32(Uint8 *p, Uint32 value)
{
    for (Uint32 i = 0; i < 4; ++i)
        p[i] = (Uint8)(value >> (i * 8));
}
static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
static void Put16(Uint8 *p, Uint16 value)
{
    p[0] = (Uint8)value;
    p[1] = (Uint8)(value >> 8);
}
static Uint32 Checksum(const Uint8 *bytes, size_t size)
{
    Uint32 crc = SDL_MAX_UINT32;
    for (size_t i = 0; i < size; ++i)
    {
        crc ^= bytes[i];
        for (Uint32 bit = 0; bit < 8; ++bit)
            crc = (crc >> 1) ^ ((crc & 1) ? 0xedb88320U : 0);
    }
    return ~crc;
}
static bool Zeroes(const Uint8 *bytes, size_t begin, size_t end)
{
    for (size_t i = begin; i < end; ++i)
        if (bytes[i] != 0)
            return SDL_SetError("Save contains unsupported reserved state");
    return true;
}
static bool Valid(SotnSaveState state)
{
    const bool save_room = SotnStageProfile_SaveRoom(state.stage, state.room);
    if (!save_room || state.character.hp == 0 || state.ticks > 99999ULL * 3600 * 60)
        return SDL_SetError("Save requires a living character in a supported native save room");
    return SotnCharacter_Validate(&state.character) && SotnWorld_Validate(&state.world);
}
static void WriteCharacter(Uint8 *p, const SotnCharacterState *s)
{
    const Uint16 resources[] = {s->hp,     s->hp_max,     s->mp,    s->mp_max,
                                s->hearts, s->hearts_max, s->level, s->subweapon};
    for (Uint32 i = 0; i < SDL_arraysize(resources); ++i)
        Put16(p + 48 + i * 2, resources[i]);
    for (Uint32 i = 0; i < 4; ++i)
        Put16(p + 64 + i * 2, s->base[i]);
    for (Uint32 i = 0; i < SOTN_EQUIP_SLOTS; ++i)
        Put16(p + 72 + i * 2, s->equipment[i]);
    Put32(p + 88, s->experience);
    Put32(p + 92, s->gold);
    Put32(p + 96, s->kills);
    Put32(p + 100, s->random);
    SDL_memcpy(p + 104, s->hand_count, SOTN_HAND_ITEMS);
    SDL_memcpy(p + 273, s->body_count, SOTN_BODY_ITEMS);
    SDL_memcpy(p + 363, s->hand_order, SOTN_HAND_ITEMS);
    SDL_memcpy(p + 532, s->body_order, SOTN_BODY_ITEMS);
    SDL_memcpy(p + 622, s->laboratory_items, 2);
}
static SotnCharacterState ReadCharacter(const Uint8 *p)
{
    SotnCharacterState s = {.hp = Read16(p + 48),
                            .hp_max = Read16(p + 50),
                            .mp = Read16(p + 52),
                            .mp_max = Read16(p + 54),
                            .hearts = Read16(p + 56),
                            .hearts_max = Read16(p + 58),
                            .level = Read16(p + 60),
                            .subweapon = Read16(p + 62),
                            .experience = Read32(p + 88),
                            .gold = Read32(p + 92),
                            .kills = Read32(p + 96),
                            .random = Read32(p + 100)};
    for (Uint32 i = 0; i < 4; ++i)
        s.base[i] = Read16(p + 64 + i * 2);
    for (Uint32 i = 0; i < SOTN_EQUIP_SLOTS; ++i)
        s.equipment[i] = Read16(p + 72 + i * 2);
    SDL_memcpy(s.hand_count, p + 104, SOTN_HAND_ITEMS);
    SDL_memcpy(s.body_count, p + 273, SOTN_BODY_ITEMS);
    SDL_memcpy(s.hand_order, p + 363, SOTN_HAND_ITEMS);
    SDL_memcpy(s.body_order, p + 532, SOTN_BODY_ITEMS);
    SDL_memcpy(s.laboratory_items, p + 622, 2);
    return s;
}
bool SotnSave_Encode(SotnSaveState state, Uint8 *bytes, size_t capacity)
{
    if (bytes == NULL || capacity < SOTN_SAVE_BYTES)
        return SDL_SetError("Save requires a full record buffer");
    if (!Valid(state))
        return false;
    Uint8 record[SOTN_SAVE_BYTES] = {0};
    SDL_memcpy(record, "SOTNNSAV", 8);
    Put32(record + 8, 9);
    Put32(record + 12, SOTN_SAVE_BYTES);
    Put32(record + 16, 0x67);
    Put32(record + 20, state.room);
    Put32(record + 24, (Uint32)state.ticks);
    Put32(record + 28, (Uint32)(state.ticks >> 32));
    Put32(record + 32, state.player_random);
    Put32(record + 36, state.actor_random);
    Put32(record + 40, state.combat_random);
    Put16(record + 44, state.stage);
    WriteCharacter(record, &state.character);
    SDL_memcpy(record + 624, state.world.flags, SOTN_CASTLE_FLAGS);
    SDL_memcpy(record + 1392, state.world.map, SOTN_CASTLE_MAP);
    Put16(record + 3440, state.world.rooms);
    for (Uint32 i = 0; i < SOTN_TIME_ATTACK_EVENTS; ++i)
        Put32(record + 3480 + i * 4U, state.world.time_attack[i]);
    Put32(record + 3608, state.world.time_attack_visited);
    SDL_memcpy(record + 3448, state.world.relics, SOTN_RELICS);
    Put32(record + 3612, state.character.spells_learned);
    SDL_memcpy(record + 3616, state.character.spells, SOTN_SPELLS);
    record[3624] = state.world.game_clear;
    Put32(record + 3628, state.world.cutscenes_seen);
    Put32(record + SOTN_SAVE_BYTES - 4, Checksum(record, SOTN_SAVE_BYTES - 4));
    SDL_memcpy(bytes, record, sizeof(record));
    return true;
}
static bool Legacy(const Uint8 *p, Uint32 version, SotnSaveState *out)
{
    if (!Zeroes(p, version == 1 ? 52U : 64U, 76))
        return false;
    const Uint16 left = Read16(p + 32), right = Read16(p + 34);
    if (Read16(p + 24) == 0 || Read16(p + 24) > 70 || Read16(p + 26) > 20 || Read16(p + 28) > 50 ||
        Read16(p + 30) > 9 || (left != 0 && left != 86 && left != 128) ||
        (right != 0 && right != 86 && right != 128))
        return SDL_SetError("Legacy save exceeds its supported development profile");
    SotnSaveState state = {.stage = 12,
                           .room = Read32(p + 20),
                           .ticks = Read32(p + 44) | (Uint64)Read32(p + 48) << 32,
                           .player_random = version == 2 ? Read32(p + 52) : 0,
                           .actor_random = version == 2 ? Read32(p + 56) : 0,
                           .combat_random = version == 2 ? Read32(p + 60) : 0,
                           .character = {.hp = Read16(p + 24),
                                         .hp_max = 70,
                                         .mp = Read16(p + 26),
                                         .mp_max = 20,
                                         .hearts = Read16(p + 28),
                                         .hearts_max = 50,
                                         .level = 1,
                                         .base = {6, 6, 6, 6},
                                         .equipment = {left, right, 26, 0, 48, 57, 57},
                                         .subweapon = Read16(p + 30),
                                         .gold = Read32(p + 36),
                                         .kills = Read32(p + 40)}};
    for (Uint32 i = 0; i < SOTN_HAND_ITEMS; ++i)
        state.character.hand_order[i] = (Uint8)i;
    for (Uint32 i = 0; i < SOTN_BODY_ITEMS; ++i)
        state.character.body_order[i] = (Uint8)i;
    if (version < 6)
    {
        state.world.relics[SOTN_RELIC_CUBE] = 3;
        for (Uint32 i = 0; i < 11; ++i)
            if (state.character.laboratory_items[i / 8] & (1U << (i & 7)))
                state.world.flags[0x100 + (0xf8 + i) / 8] |= (Uint8)(1U << ((0xf8 + i) & 7));
    }
    if (!Valid(state))
        return false;
    *out = state;
    return true;
}
bool SotnSave_Decode(const Uint8 *bytes, size_t size, SotnSaveState *out)
{
    if (bytes == NULL || out == NULL ||
        (size != SOTN_SAVE_BYTES && size != SOTN_SAVE_V3_BYTES && size != SOTN_SAVE_LEGACY_BYTES))
        return SDL_SetError("Save file is missing or has an invalid record size");
    if (SDL_memcmp(bytes, "SOTNNSAV", 8) != 0)
        return SDL_SetError("Save is not a native record; PS1 memory cards are unsupported");
    const Uint32 version = Read32(bytes + 8);
    if (version != 1 && version != 2 && version != 3 && version != 4 && version != 5 &&
        version != 6 && version != 7 && version != 8 && version != 9)
        return SDL_SetError("Save version %u is unsupported; explicit migration is required",
                            version);
    const size_t expected = version >= 4   ? SOTN_SAVE_BYTES
                            : version == 3 ? SOTN_SAVE_V3_BYTES
                                           : SOTN_SAVE_LEGACY_BYTES;
    if (size != expected || Read32(bytes + 12) != expected || Read32(bytes + 16) != 0x67 ||
        Read32(bytes + size - 4) != Checksum(bytes, size - 4))
        return SDL_SetError("Save region, length, or checksum is invalid; original file retained");
    if (version < 3)
        return Legacy(bytes, version, out);
    if (!Zeroes(bytes, version >= 5 ? 46U : 44U, 48) || !Zeroes(bytes, 86, 88) ||
        (version == 3 && !Zeroes(bytes, 624, 764)))
        return false;
    SotnSaveState state = {.stage = version >= 5 ? Read16(bytes + 44) : 12,
                           .room = Read32(bytes + 20),
                           .ticks = Read32(bytes + 24) | (Uint64)Read32(bytes + 28) << 32,
                           .player_random = Read32(bytes + 32),
                           .actor_random = Read32(bytes + 36),
                           .combat_random = Read32(bytes + 40),
                           .character = ReadCharacter(bytes)};
    if (version >= 4)
    {
        if (version >= 7)
        {
            if (!Zeroes(bytes, 3442, 3448) || !Zeroes(bytes, 3478, 3480) ||
                !Zeroes(bytes,
                        version >= 9   ? 3632U
                        : version >= 8 ? 3625U
                                       : 3612U,
                        4092) ||
                (version >= 9 && !Zeroes(bytes, 3625, 3628)))
                return SDL_SetError("Saved world contains unsupported reserved state");
            for (Uint32 i = 0; i < SOTN_TIME_ATTACK_EVENTS; ++i)
                state.world.time_attack[i] = Read32(bytes + 3480 + i * 4U);
            state.world.time_attack_visited = Read32(bytes + 3608);
            if (version >= 8)
            {
                state.character.spells_learned = Read32(bytes + 3612);
                SDL_memcpy(state.character.spells, bytes + 3616, SOTN_SPELLS);
                state.world.game_clear = bytes[3624];
                if (version >= 9)
                    state.world.cutscenes_seen = Read32(bytes + 3628);
            }
        }
        else
        {
            if (bytes[3442] > 1 || !Zeroes(bytes, 3443, 3444) ||
                !Zeroes(bytes, version >= 6 ? 3448 + SOTN_RELICS : 3448, 4092))
                return SDL_SetError("Legacy world contains unsupported reserved state");
            if (!SotnWorld_RecordTime(&state.world, SOTN_TIME_LAB_BOSSES, Read32(bytes + 3444)))
                return false;
            if (bytes[3442] && !SotnWorld_VisitEvent(&state.world, SOTN_TIME_LAB_BOSSES))
                return false;
        }
        SDL_memcpy(state.world.flags, bytes + 624, SOTN_CASTLE_FLAGS);
        SDL_memcpy(state.world.map, bytes + 1392, SOTN_CASTLE_MAP);
        state.world.rooms = Read16(bytes + 3440);
        if (version >= 6)
            SDL_memcpy(state.world.relics, bytes + 3448, SOTN_RELICS);
    }
    if (version < 6)
    {
        state.world.relics[SOTN_RELIC_CUBE] = 3;
        for (Uint32 i = 0; i < 11; ++i)
            if (state.character.laboratory_items[i / 8] & (1U << (i & 7)))
                state.world.flags[0x100 + (0xf8 + i) / 8] |= (Uint8)(1U << ((0xf8 + i) & 7));
    }
    if (!Valid(state))
        return false;
    *out = state;
    return true;
}
