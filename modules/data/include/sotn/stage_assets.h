#ifndef SOTN_STAGE_ASSETS_H
#define SOTN_STAGE_ASSETS_H
#include <sotn/resource.h>
/** @brief Public identity metadata; loading assets does not implement a stage's game logic. */
typedef struct SotnStageIdentity
{
    Uint16 id;
    const char *code;
} SotnStageIdentity;
/** @brief Owned, independently verified immutable stage overlay and artwork. */
typedef struct SotnStageAssets SotnStageAssets;
/** @brief Number of canonical US stages in the pinned catalog, excluding aliases/cinematics. */
Uint32 SotnStageAssets_Count(void);
/** @brief Get catalog identity by index.
 * @param index Less than Count(). @param out Unchanged on error; code has static lifetime.
 * @return Success or SDL error. */
bool SotnStageAssets_Identity(Uint32 index, SotnStageIdentity *out);
/** @brief Resolve a case-insensitive canonical code; reject paths and aliases.
 * @param code Borrowed code such as NZ0, NO3, or RNO0. @param out Unchanged on error.
 * @return Success or SDL error. */
bool SotnStageAssets_Find(const char *code, SotnStageIdentity *out);
/** @brief Load a verified pair on demand with partial-failure cleanup.
 * @param directory User's private extracted directory. @param id Canonical original stage ID.
 * @param out Receives ownership; NULL on failure. @return Success or SDL error.
 * @note No core snapshot, graphics initialization, or executable code loading is required.
 * Room parsing and native actor support must be established separately. */
bool SotnStageAssets_Open(const char *directory, Uint16 id, SotnStageAssets **out);
/** @brief Borrow the overlay until Close(); empty for NULL. Never execute its machine code. */
SotnDataView SotnStageAssets_Overlay(const SotnStageAssets *assets);
/** @brief Borrow stage artwork until Close(); empty for NULL. */
SotnDataView SotnStageAssets_Graphics(const SotnStageAssets *assets);
/** @brief Release both resources. @param assets Owned pair or NULL. */
void SotnStageAssets_Close(SotnStageAssets *assets);
#endif
