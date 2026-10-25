#ifndef SOTN_SAVE_H
#define SOTN_SAVE_H

#include <SDL3/SDL.h>
#include <sotn/character.h>
#include <sotn/world.h>

/** @brief Version-seven ordinary native save, without transient actors or asset bytes. */
typedef struct SotnSaveState
{
    SotnCharacterState character;
    SotnWorldState world;
    Uint32 room;
    Uint16 stage; /**< Original stage identity; legacy records migrate explicitly to Laboratory. */
    Uint64 ticks;
    Uint32 player_random, actor_random, combat_random;
} SotnSaveState;

enum
{
    SOTN_SAVE_BYTES = 4096,
    SOTN_SAVE_V3_BYTES = 768,
    SOTN_SAVE_LEGACY_BYTES = 80,
    SOTN_SAVE_SLOTS = 15
};

/** @brief Encode supported state into an initialized, versioned little-endian record.
 * @param state Copied supported stage state. @param bytes Writable record buffer.
 * @param capacity At least SOTN_SAVE_BYTES. @return Success, or false with SDL_GetError();
 * output remains unchanged on invalid arguments. This is not a PS1 memory-card format. */
bool SotnSave_Encode(SotnSaveState state, Uint8 *bytes, size_t capacity);
/** @brief Validate size, version, region, checksum, reserved fields, and supported state.
 * @param bytes Borrowed record. @param size SOTN_SAVE_BYTES, SOTN_SAVE_V3_BYTES, or
 * SOTN_SAVE_LEGACY_BYTES for explicit v1/v2/v3/v4/v5/v6 migration.
 * @param out Receives copied state, unchanged on failure. @return Success, or false with
 * SDL_GetError(). Unknown versions require explicit migration and are rejected. */
bool SotnSave_Decode(const Uint8 *bytes, size_t size, SotnSaveState *out);

/** @brief Owned save directory and fixed slot names; use on the main thread. */
typedef struct SotnSaveStore SotnSaveStore;
/** @brief Open the native persistent directory returned by SDL_GetPrefPath.
 * @param out Receives ownership, NULL on failure. @return Success, or false with SDL_GetError().
 * @note Browser persistence requires an IDBFS adapter; it currently fails explicitly. */
bool SotnSaveStore_Create(SotnSaveStore **out);
/** @brief Check whether a slot path exists without reading its contents.
 * @param store Borrowed store. @param slot One-based slot 1..SOTN_SAVE_SLOTS.
 * @param out Receives existence, unchanged on failure. @return Success, or false with
 * SDL_GetError(). */
bool SotnSaveStore_Exists(const SotnSaveStore *store, Uint32 slot, bool *out);
/** @brief Write a checked record to an exclusive sibling temporary, then replace its slot.
 * @param store Borrowed store. @param slot One-based slot. @param state Supported state.
 * @return Success, or false with SDL_GetError(); failed writes/renames preserve the old slot.
 * @note Caller obtains user confirmation before replacing a slot. Flush/close precede rename;
 * hardware power-loss durability depends on the platform filesystem. */
bool SotnSaveStore_Write(SotnSaveStore *store, Uint32 slot, SotnSaveState state);
/** @brief Load one exact-size regular slot file and validate its complete record.
 * @param store Borrowed store. @param slot One-based slot. @param out Copied state on success.
 * @return Success, or false with SDL_GetError() and unchanged output; corrupt saves are retained.
 */
bool SotnSaveStore_Read(const SotnSaveStore *store, Uint32 slot, SotnSaveState *out);
/** @brief Release paths without deleting saves. @param store Owned store or NULL. */
void SotnSaveStore_Destroy(SotnSaveStore *store);

#endif
