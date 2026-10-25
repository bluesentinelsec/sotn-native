#include "save_internal.h"

#define CHECK(value)                                                                               \
    do                                                                                             \
    {                                                                                              \
        if (!(value))                                                                              \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #value, SDL_GetError());                          \
            return false;                                                                          \
        }                                                                                          \
    } while (0)

static SotnSaveState Profile(void)
{
    SotnSaveState s = {.stage = 12,
                       .room = 23,
                       .ticks = 0x100000002ULL,
                       .player_random = 42,
                       .actor_random = 999,
                       .combat_random = 123,
                       .character = {.hp = 87,
                                     .hp_max = 95,
                                     .mp = 29,
                                     .mp_max = 40,
                                     .hearts = 64,
                                     .hearts_max = 100,
                                     .level = 5,
                                     .base = {8, 9, 10, 11},
                                     .equipment = {18, 128, 27, 2, 49, 77, 57},
                                     .subweapon = 3,
                                     .experience = 1234,
                                     .gold = 12345,
                                     .kills = 7,
                                     .random = 0xdeadbeef,
                                     .laboratory_items = {0x81, 4}}};
    for (Uint32 i = 0; i < SOTN_HAND_ITEMS; ++i)
    {
        s.character.hand_count[i] = (Uint8)(i % 100);
        s.character.hand_order[i] = (Uint8)(SOTN_HAND_ITEMS - 1 - i);
    }
    for (Uint32 i = 0; i < SOTN_BODY_ITEMS; ++i)
    {
        s.character.body_count[i] = (Uint8)(i % 100);
        s.character.body_order[i] = (Uint8)(SOTN_BODY_ITEMS - 1 - i);
    }
    s.world.flags[SOTN_LAB_SECRET_WALL] = 1;
    s.world.flags[SOTN_LAB_SECRET_FLOOR] = 1;
    s.world.flags[SOTN_LAB_CANNON] = 1;
    s.world.flags[SOTN_LAB_BOSSES] = 1;
    s.world.laboratory_boss_visited = true;
    s.world.laboratory_boss_time = 12345;
    // Independent expected packed cells: (0,0), (3,0), and inverted (63,63).
    s.world.map[0] = 65;
    s.world.map[2047] = 1;
    s.world.rooms = 3;
    return s;
}
static void Rechecksum(Uint8 *p, size_t size)
{
    Uint32 crc = SDL_MAX_UINT32;
    for (size_t i = 0; i < size - 4; ++i)
    {
        crc ^= p[i];
        for (Uint32 bit = 0; bit < 8; ++bit)
            crc = (crc >> 1) ^ ((crc & 1) ? 0xedb88320U : 0);
    }
    crc = ~crc;
    for (Uint32 i = 0; i < 4; ++i)
        p[size - 4 + i] = (Uint8)(crc >> (i * 8));
}
static bool Codec(void)
{
    SotnSaveState state = Profile(), decoded = {0};
    Uint8 record[SOTN_SAVE_BYTES] = {0};
    CHECK(SotnSave_Encode(state, record, sizeof(record)));
    // Independently produced with Python struct and zlib for Profile().
    CHECK(record[4092] == 0x26 && record[4093] == 0xf4 && record[4094] == 0x0f &&
          record[4095] == 0xe5);
    CHECK(record[8] == 5 && record[12] == 0 && record[13] == 16 && record[24] == 2 &&
          record[28] == 1 && record[48] == 87);
    CHECK(record[92] == 57 && record[93] == 48 && record[104 + 99] == 99 && record[363] == 168 &&
          record[622] == 0x81);
    CHECK(SotnSave_Decode(record, sizeof(record), &decoded));
    CHECK(decoded.room == 23 && decoded.character.hp == 87 && decoded.character.hp_max == 95 &&
          decoded.character.level == 5);
    CHECK(decoded.character.mp_max == 40 && decoded.character.hearts == 64 &&
          decoded.character.hearts_max == 100);
    CHECK(decoded.character.experience == 1234 && decoded.character.gold == 12345 &&
          decoded.character.kills == 7);
    CHECK(decoded.character.equipment[0] == 18 && decoded.character.equipment[4] == 49 &&
          decoded.character.equipment[5] == 77);
    CHECK(SDL_memcmp(decoded.character.hand_count, state.character.hand_count, SOTN_HAND_ITEMS) ==
          0);
    CHECK(SDL_memcmp(decoded.character.body_count, state.character.body_count, SOTN_BODY_ITEMS) ==
          0);
    CHECK(SDL_memcmp(decoded.character.hand_order, state.character.hand_order, SOTN_HAND_ITEMS) ==
          0);
    CHECK(SDL_memcmp(decoded.character.body_order, state.character.body_order, SOTN_BODY_ITEMS) ==
          0);
    CHECK(decoded.character.laboratory_items[0] == 0x81 &&
          decoded.character.laboratory_items[1] == 4);
    CHECK(decoded.character.random == 0xdeadbeef && decoded.player_random == 42 &&
          decoded.actor_random == 999 && decoded.combat_random == 123);
    CHECK(decoded.world.rooms == 3 && decoded.world.map[0] == 65 && decoded.world.map[2047] == 1 &&
          decoded.world.flags[SOTN_LAB_CANNON] == 1 &&
          decoded.world.flags[SOTN_LAB_SECRET_WALL] == 1 &&
          decoded.world.flags[SOTN_LAB_SECRET_FLOOR] == 1 &&
          decoded.world.flags[SOTN_LAB_BOSSES] == 1 && decoded.world.laboratory_boss_visited &&
          decoded.world.laboratory_boss_time == 12345);
    const Uint32 corrupt_offsets[] = {3440, 3442, 3443, 3447, 624 + SOTN_LAB_CANNON};
    for (Uint32 i = 0; i < SDL_arraysize(corrupt_offsets); ++i)
    {
        CHECK(SotnSave_Encode(state, record, sizeof(record)));
        record[corrupt_offsets[i]] = 255;
        Rechecksum(record, sizeof(record));
        decoded.room = 99;
        CHECK(!SotnSave_Decode(record, sizeof(record), &decoded) && decoded.room == 99);
    }
    CHECK(SotnSave_Encode(state, record, sizeof(record)));
    Uint8 version_four[SOTN_SAVE_BYTES] = {0};
    SDL_memcpy(version_four, record, sizeof(version_four));
    version_four[8] = 4;
    SDL_memset(version_four + 44, 0, 4);
    Rechecksum(version_four, sizeof(version_four));
    CHECK(version_four[4092] == 0x9c && version_four[4093] == 0x24 && version_four[4094] == 0x3d &&
          version_four[4095] == 0xe4);
    CHECK(SotnSave_Decode(version_four, sizeof(version_four), &decoded) && decoded.stage == 12);
    version_four[44] = 7;
    Rechecksum(version_four, sizeof(version_four));
    CHECK(!SotnSave_Decode(version_four, sizeof(version_four), &decoded));
    Uint8 version_three[SOTN_SAVE_V3_BYTES] = {0};
    SDL_memcpy(version_three, record, 624);
    SDL_memset(version_three + 44, 0, 4);
    version_three[8] = 3;
    version_three[13] = 3;
    Rechecksum(version_three, sizeof(version_three));
    // Frozen independent v3 checksum verifies character migration has not changed.
    CHECK(version_three[764] == 0xaf && version_three[765] == 0x4e && version_three[766] == 0xe9 &&
          version_three[767] == 0x03);
    CHECK(SotnSave_Decode(version_three, sizeof(version_three), &decoded));
    CHECK(decoded.character.hp_max == 95 && decoded.character.hand_order[0] == 168 &&
          decoded.stage == 12 && decoded.world.rooms == 0 &&
          decoded.world.laboratory_boss_time == 0 && !decoded.world.laboratory_boss_visited &&
          decoded.world.flags[SOTN_LAB_BOSSES] == 0);
    version_three[624] = 1;
    Rechecksum(version_three, sizeof(version_three));
    CHECK(!SotnSave_Decode(version_three, sizeof(version_three), &decoded));
    record[8] = 6;
    decoded.room = 99;
    CHECK(!SotnSave_Decode(record, sizeof(record), &decoded) && decoded.room == 99);
    CHECK(SDL_strstr(SDL_GetError(), "migration") != NULL);
    record[8] = 5;
    record[104] = 100;
    Rechecksum(record, sizeof(record));
    CHECK(!SotnSave_Decode(record, sizeof(record), &decoded) && decoded.room == 99);
    CHECK(SotnSave_Encode(state, record, sizeof(record)));
    record[364] = record[363];
    Rechecksum(record, sizeof(record));
    CHECK(!SotnSave_Decode(record, sizeof(record), &decoded) && decoded.room == 99);
    CHECK(SotnSave_Encode(state, record, sizeof(record)));
    record[3448] = 1;
    Rechecksum(record, sizeof(record));
    CHECK(!SotnSave_Decode(record, sizeof(record), &decoded) && decoded.room == 99);
    CHECK(SotnSave_Encode(state, record, sizeof(record)));
    record[623] = 8;
    Rechecksum(record, sizeof(record));
    CHECK(!SotnSave_Decode(record, sizeof(record), &decoded) && decoded.room == 99);
    CHECK(SotnSave_Encode(state, record, sizeof(record)));
    record[48] ^= 1;
    CHECK(!SotnSave_Decode(record, sizeof(record), &decoded) && decoded.room == 99);
    CHECK(!SotnSave_Decode(record, sizeof(record) - 1, &decoded));
    SDL_memset(record, 0xa5, sizeof(record));
    ++state.world.rooms;
    CHECK(!SotnSave_Encode(state, record, sizeof(record)) && record[0] == 0xa5);
    --state.world.rooms;
    state.character.hp = 0;
    CHECK(!SotnSave_Encode(state, record, sizeof(record)) && record[0] == 0xa5);
    CHECK(!SotnSave_Encode(Profile(), record, 1));
    CHECK(!SotnSave_Decode(NULL, sizeof(record), &decoded));
    CHECK(!SotnSave_Decode(record, sizeof(record), NULL));
    /* Independent Python struct/zlib fixture for the explicit version-one migration. */
    const Uint8 version_one[80] = {
        0x53, 0x4f, 0x54, 0x4e, 0x4e, 0x53, 0x41, 0x56, 0x01, 0x00, 0x00, 0x00, 0x50, 0x00,
        0x00, 0x00, 0x67, 0x00, 0x00, 0x00, 0x17, 0x00, 0x00, 0x00, 0x46, 0x00, 0x14, 0x00,
        0x0a, 0x00, 0x00, 0x00, 0x56, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0x01, 0x00,
        0x00, 0x00, 0x7b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0x90, 0x1e, 0xcc};
    CHECK(SotnSave_Decode(version_one, sizeof(version_one), &decoded));
    CHECK(decoded.character.hp == 70 && decoded.character.gold == 100 && decoded.ticks == 123 &&
          decoded.stage == 12 && decoded.player_random == 0 && decoded.actor_random == 0 &&
          decoded.combat_random == 0);

    CHECK(decoded.character.hp_max == 70 && decoded.character.mp_max == 20 &&
          decoded.character.hearts_max == 50 && decoded.character.level == 1);
    CHECK(decoded.character.equipment[0] == 86 && decoded.character.equipment[2] == 26 &&
          decoded.character.equipment[4] == 48);
    CHECK(decoded.character.hand_count[86] == 0 && decoded.character.experience == 0 &&
          decoded.character.random == 0);
    CHECK(decoded.character.hand_order[168] == 168 && decoded.character.body_order[89] == 89);
    Uint8 version_two[80] = {0};
    SDL_memcpy(version_two, version_one, 80);
    version_two[8] = 2;
    version_two[52] = 42;
    version_two[56] = 43;
    version_two[60] = 44;
    Rechecksum(version_two, 80);
    CHECK(SotnSave_Decode(version_two, 80, &decoded) && decoded.player_random == 42 &&
          decoded.actor_random == 43 && decoded.combat_random == 44);
    CHECK(SotnSave_Encode(decoded, record, sizeof(record)) && record[8] == 5);
    CHECK(SotnSave_Decode(record, sizeof(record), &decoded) && decoded.character.hp_max == 70);
    version_two[12] = 0;
    Rechecksum(version_two, 80);
    CHECK(!SotnSave_Decode(version_two, 80, &decoded));
    return true;
}

static bool Stages(void)
{
    const Uint16 stages[] = {12, 65, 7};
    const Uint32 first_rooms[] = {23, 25, 22};
    for (Uint32 i = 0; i < SDL_arraysize(stages); ++i)
    {
        SotnSaveState state = Profile(), decoded = {0};
        state.stage = stages[i];
        state.world.flags[SOTN_ENTRANCE_DEATH] = 1;
        state.world.flags[SOTN_ENTRANCE_ON] = 1;
        Uint8 record[SOTN_SAVE_BYTES] = {0};
        for (Uint32 room = first_rooms[i]; room < first_rooms[i] + 3; ++room)
        {
            state.room = room;
            CHECK(SotnSave_Encode(state, record, sizeof(record)));
            CHECK(SotnSave_Decode(record, sizeof(record), &decoded));
            CHECK(decoded.stage == stages[i] && decoded.room == room &&
                  decoded.world.flags[SOTN_ENTRANCE_DEATH] == 1 &&
                  decoded.world.flags[SOTN_ENTRANCE_ON] == 1);
        }
        record[46] = 1;
        Rechecksum(record, sizeof(record));
        decoded.stage = 999;
        CHECK(!SotnSave_Decode(record, sizeof(record), &decoded) && decoded.stage == 999);
        state.room = first_rooms[i] - 1;
        CHECK(!SotnSave_Encode(state, record, sizeof(record)));
        state.room = first_rooms[i] + 3;
        CHECK(!SotnSave_Encode(state, record, sizeof(record)));
    }
    SotnSaveState invalid = Profile();
    invalid.stage = 8;
    Uint8 record[SOTN_SAVE_BYTES] = {0};
    CHECK(!SotnSave_Encode(invalid, record, sizeof(record)));
    invalid = Profile();
    CHECK(SotnSave_Encode(invalid, record, sizeof(record)));
    record[44] = 8;
    Rechecksum(record, sizeof(record));
    CHECK(!SotnSave_Decode(record, sizeof(record), &invalid));
    return true;
}

static bool Storage(const char *directory)
{
    SotnSaveStore *store = NULL;
    CHECK(SotnSaveStore_OpenAt(directory, &store));
    SotnSaveState state = Profile();
    SotnSaveState decoded = {0};
    bool found = true;
    CHECK(SotnSaveStore_Exists(store, 1, &found) && !found);
    CHECK(!SotnSaveStore_Read(store, 1, &decoded));
    for (Uint32 slot = 1; slot <= SOTN_SAVE_SLOTS; ++slot)
    {
        state.room = 23 + (slot - 1) % 3;
        state.character.gold = slot * 100;
        CHECK(SotnSaveStore_Write(store, slot, state));
        CHECK(SotnSaveStore_Exists(store, slot, &found) && found);
        CHECK(SotnSaveStore_Read(store, slot, &decoded));
        CHECK(decoded.stage == state.stage && decoded.room == state.room &&
              decoded.character.gold == state.character.gold);
        CHECK(decoded.world.rooms == 3 && decoded.world.laboratory_boss_time == 12345 &&
              decoded.world.flags[SOTN_LAB_BOSSES] == 1);
    }
    state.room = 23;
    state.character.gold = 777;
    CHECK(SotnSaveStore_Write(store, 1, state));
    CHECK(SotnSaveStore_Read(store, 1, &decoded) && decoded.character.gold == 777);
    char *migration_path = NULL;
    CHECK(SDL_asprintf(&migration_path, "%s/laboratory-01.sav", directory) >= 0);
    Uint8 old[80] = {0};
    SDL_memcpy(old, "SOTNNSAV", 8);
    old[8] = 2;
    old[12] = 80;
    old[16] = 0x67;
    old[20] = 23;
    old[24] = 70;
    old[26] = 20;
    old[28] = 11;
    old[32] = 86;
    old[34] = 128;
    old[36] = 9;
    old[37] = 3;
    old[52] = 42;
    old[56] = 43;
    old[60] = 44;
    Rechecksum(old, sizeof(old));
    CHECK(SDL_SaveFile(migration_path, old, sizeof(old)));
    CHECK(SotnSaveStore_Read(store, 1, &decoded) && decoded.character.gold == 777 &&
          decoded.character.hp_max == 70 && decoded.actor_random == 43);
    SDL_PathInfo migration_info = {0};
    CHECK(SDL_GetPathInfo(migration_path, &migration_info) && migration_info.size == 80);
    CHECK(SotnSaveStore_Write(store, 1, state));
    CHECK(SDL_GetPathInfo(migration_path, &migration_info) &&
          migration_info.size == SOTN_SAVE_BYTES);
    SDL_free(migration_path);
    CHECK(SDL_asprintf(&migration_path, "%s/laboratory-01.sav", directory) >= 0);
    Uint8 current[SOTN_SAVE_BYTES] = {0}, v3[SOTN_SAVE_V3_BYTES] = {0};
    CHECK(SotnSave_Encode(state, current, sizeof(current)));
    SDL_memcpy(v3, current, 624);
    SDL_memset(v3 + 44, 0, 4);
    v3[8] = 3;
    v3[13] = 3;
    Rechecksum(v3, sizeof(v3));
    CHECK(SDL_SaveFile(migration_path, v3, sizeof(v3)));
    CHECK(SotnSaveStore_Read(store, 1, &decoded) && decoded.character.gold == 777 &&
          decoded.world.rooms == 0);
    CHECK(SDL_GetPathInfo(migration_path, &migration_info) && migration_info.size == 768);
    CHECK(SotnSaveStore_Write(store, 1, state));
    SDL_free(migration_path);
    state.character.hp = 0;
    CHECK(!SotnSaveStore_Write(store, 1, state));
    CHECK(SotnSaveStore_Read(store, 1, &decoded) && decoded.character.gold == 777);
    state.character.hp = 70;
    char *slot_path = NULL, *marker = NULL;
    CHECK(SDL_asprintf(&slot_path, "%s/laboratory-03.sav", directory) >= 0);
    CHECK(SDL_RemovePath(slot_path) && SDL_CreateDirectory(slot_path));
    CHECK(SDL_asprintf(&marker, "%s/retained.txt", slot_path) >= 0);
    CHECK(SDL_SaveFile(marker, "retained", 8));
    CHECK(!SotnSaveStore_Write(store, 3, state));
    SDL_PathInfo info = {0};
    CHECK(SDL_GetPathInfo(marker, &info) && info.size == 8);
    CHECK(SDL_RemovePath(marker) && SDL_RemovePath(slot_path));
    SDL_free(marker);
    SDL_free(slot_path);
    CHECK(SDL_asprintf(&slot_path, "%s/laboratory-02.sav", directory) >= 0);
    CHECK(SDL_SaveFile(slot_path, "corrupt", 7));
    decoded.character.gold = 999;
    CHECK(!SotnSaveStore_Read(store, 2, &decoded) && decoded.character.gold == 999);
    CHECK(SDL_GetPathInfo(slot_path, &info) && info.size == 7);
    CHECK(SDL_RemovePath(slot_path));
    SDL_free(slot_path);
    CHECK(SDL_asprintf(&slot_path, "%s/laboratory-01.sav", directory) >= 0);
    CHECK(SDL_RemovePath(slot_path));
    SDL_free(slot_path);
    CHECK(!SotnSaveStore_Write(store, 0, state));
    CHECK(!SotnSaveStore_Read(store, 16, &decoded));
    CHECK(!SotnSaveStore_Exists(store, 16, &found));
    for (Uint32 slot = 4; slot <= SOTN_SAVE_SLOTS; ++slot)
    {
        CHECK(SDL_asprintf(&slot_path, "%s/laboratory-%02u.sav", directory, slot) >= 0);
        CHECK(SDL_RemovePath(slot_path));
        SDL_free(slot_path);
    }
    SotnSaveStore_Destroy(store);
    SotnSaveStore_Destroy(NULL);
    return true;
}

int main(int argc, char **argv)
{
    if (argc != 2 || !Codec() || !Stages() || !Storage(argv[1]))
        return 1;
    SDL_Log("SOTN_SAVE_TEST: PASS");
    return 0;
}
