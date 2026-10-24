#include <sotn/save.h>

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

static Uint32 Checksum(const Uint8 *bytes)
{
    Uint32 crc = SDL_MAX_UINT32;
    for (Uint32 i = 0; i < 76; ++i)
    {
        crc ^= bytes[i];
        for (Uint32 bit = 0; bit < 8; ++bit)
            crc = (crc >> 1) ^ ((crc & 1) ? 0xedb88320U : 0);
    }
    return ~crc;
}

static bool HandValid(Uint16 hand)
{
    return hand == 0 || hand == 86 || hand == 128;
}

static bool Valid(SotnSaveState state)
{
    return (state.room == 23 || state.room == 24 || state.room == 25) && state.hp != 0 &&
           state.hp <= 70 && state.mp <= 20 && state.hearts <= 50 && state.subweapon < 10 &&
           state.gold <= 999999 && HandValid(state.left_hand) && HandValid(state.right_hand) &&
           state.ticks <= 99999ULL * 3600 * 60;
}

bool SotnSave_Encode(SotnSaveState state, Uint8 *bytes, size_t capacity)
{
    if (bytes == NULL || capacity < SOTN_SAVE_BYTES || !Valid(state))
        return SDL_SetError("Save requires supported Laboratory state and a full record buffer");
    Uint8 record[SOTN_SAVE_BYTES] = {0};
    SDL_memcpy(record, "SOTNNSAV", 8);
    Put32(record + 8, 1);
    Put32(record + 12, SOTN_SAVE_BYTES);
    Put32(record + 16, 0x00067);
    Put32(record + 20, state.room);
    Put16(record + 24, state.hp);
    Put16(record + 26, state.mp);
    Put16(record + 28, state.hearts);
    Put16(record + 30, state.subweapon);
    Put16(record + 32, state.left_hand);
    Put16(record + 34, state.right_hand);
    Put32(record + 36, state.gold);
    Put32(record + 40, state.kills);
    Put32(record + 44, (Uint32)state.ticks);
    Put32(record + 48, (Uint32)(state.ticks >> 32));
    Put32(record + 76, Checksum(record));
    SDL_memcpy(bytes, record, sizeof(record));
    return true;
}

bool SotnSave_Decode(const Uint8 *bytes, size_t size, SotnSaveState *out)
{
    if (bytes == NULL || size != SOTN_SAVE_BYTES || out == NULL)
        return SDL_SetError("Save file is missing or has an invalid record size");
    if (SDL_memcmp(bytes, "SOTNNSAV", 8) != 0)
        return SDL_SetError(
            "Save is not a native Laboratory record; PS1 memory cards are unsupported");
    if (Read32(bytes + 8) != 1)
        return SDL_SetError("Save version %u is unsupported; explicit migration is required",
                            Read32(bytes + 8));
    if (Read32(bytes + 12) != SOTN_SAVE_BYTES || Read32(bytes + 16) != 0x00067 ||
        Read32(bytes + 76) != Checksum(bytes))
        return SDL_SetError("Save region, length, or checksum is invalid; original file retained");
    for (Uint32 i = 52; i < 76; ++i)
        if (bytes[i] != 0)
            return SDL_SetError("Save contains unsupported reserved state");
    const SotnSaveState state = {.room = Read32(bytes + 20),
                                 .hp = Read16(bytes + 24),
                                 .mp = Read16(bytes + 26),
                                 .hearts = Read16(bytes + 28),
                                 .subweapon = Read16(bytes + 30),
                                 .left_hand = Read16(bytes + 32),
                                 .right_hand = Read16(bytes + 34),
                                 .gold = Read32(bytes + 36),
                                 .kills = Read32(bytes + 40),
                                 .ticks = Read32(bytes + 44) | (Uint64)Read32(bytes + 48) << 32};
    if (!Valid(state))
        return SDL_SetError("Save contains state outside this supported Laboratory checkpoint");
    *out = state;
    return true;
}
