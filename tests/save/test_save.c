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

static bool Codec(void)
{
    const SotnSaveState state = {.room = 23,
                                 .hp = 70,
                                 .mp = 20,
                                 .hearts = 11,
                                 .subweapon = 3,
                                 .left_hand = 86,
                                 .right_hand = 128,
                                 .gold = 12345,
                                 .kills = 7,
                                 .ticks = 0x100000002ULL};
    Uint8 record[SOTN_SAVE_BYTES] = {0};
    CHECK(SotnSave_Encode(state, record, sizeof(record)));
    CHECK(record[24] == 70 && record[36] == 57 && record[37] == 48 && record[44] == 2 &&
          record[48] == 1);
    SotnSaveState decoded = {0};
    CHECK(SotnSave_Decode(record, sizeof(record), &decoded));
    CHECK(decoded.room == state.room && decoded.hp == 70 && decoded.gold == state.gold &&
          decoded.ticks == state.ticks && decoded.right_hand == 128 && decoded.hearts == 11 &&
          decoded.subweapon == 3);
    record[8] = 2;
    decoded.room = 99;
    CHECK(!SotnSave_Decode(record, sizeof(record), &decoded) && decoded.room == 99);
    CHECK(SDL_strstr(SDL_GetError(), "migration") != NULL);
    record[8] = 1;
    record[28] ^= 1;
    CHECK(!SotnSave_Decode(record, sizeof(record), &decoded) && decoded.room == 99);
    CHECK(!SotnSave_Decode(record, sizeof(record) - 1, &decoded));
    SDL_memset(record, 0xa5, sizeof(record));
    CHECK(!SotnSave_Encode((SotnSaveState){0}, record, sizeof(record)) && record[0] == 0xa5);
    CHECK(!SotnSave_Encode(state, record, 1));
    CHECK(!SotnSave_Decode(NULL, sizeof(record), &decoded));
    CHECK(!SotnSave_Decode(record, sizeof(record), NULL));
    return true;
}

static bool Storage(const char *directory)
{
    SotnSaveStore *store = NULL;
    CHECK(SotnSaveStore_OpenAt(directory, &store));
    SotnSaveState state = {.room = 23, .hp = 70, .mp = 20, .hearts = 10, .left_hand = 86};
    SotnSaveState decoded = {0};
    bool found = true;
    CHECK(SotnSaveStore_Exists(store, 1, &found) && !found);
    CHECK(!SotnSaveStore_Read(store, 1, &decoded));
    for (Uint32 slot = 1; slot <= SOTN_SAVE_SLOTS; ++slot)
    {
        state.room = 23 + (slot - 1) % 3;
        state.gold = slot * 100;
        CHECK(SotnSaveStore_Write(store, slot, state));
        CHECK(SotnSaveStore_Exists(store, slot, &found) && found);
        CHECK(SotnSaveStore_Read(store, slot, &decoded));
        CHECK(decoded.room == state.room && decoded.gold == state.gold);
    }
    state.room = 23;
    state.gold = 777;
    CHECK(SotnSaveStore_Write(store, 1, state));
    CHECK(SotnSaveStore_Read(store, 1, &decoded) && decoded.gold == 777);
    state.hp = 0;
    CHECK(!SotnSaveStore_Write(store, 1, state));
    CHECK(SotnSaveStore_Read(store, 1, &decoded) && decoded.gold == 777);
    state.hp = 70;
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
    decoded.gold = 999;
    CHECK(!SotnSaveStore_Read(store, 2, &decoded) && decoded.gold == 999);
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
    if (argc != 2 || !Codec() || !Storage(argv[1]))
        return 1;
    SDL_Log("SOTN_SAVE_TEST: PASS");
    return 0;
}
