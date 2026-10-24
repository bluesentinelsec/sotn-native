#ifndef SOTN_ACTOR_LAYOUT_H
#define SOTN_ACTOR_LAYOUT_H

#include <sotn/game_data.h>

/** @brief PS1 stage slots, including room placements and dynamically created actors. */
enum
{
    SOTN_STAGE_ACTOR_SLOTS = 192
};

/** @brief A decoded placement in room-local coordinates; contains no host pointers. */
typedef struct SotnActorPlacement
{
    Sint16 x, y;
    Uint16 id, parameters;
    Uint8 slot, persistence, variant;
    Uint16 mode;
} SotnActorPlacement;

/** @brief Validated paired NZ0 placement lists borrowing immutable overlay bytes. */
typedef struct SotnActorLayout SotnActorLayout;
/** @brief Owned scrolling spawn state, borrowing its current layout. */
typedef struct SotnRoomActors SotnRoomActors;

/**
 * @brief Decode one of the supported US Laboratory's 53 actor layouts.
 * @param image Borrowed immutable NZ0 image, alive until the layout is closed.
 * @param index Original room object-layout ID, below 53.
 * @param out Receives owned metadata, NULL on failure when non-NULL.
 * @return True on success, false with SDL_GetError() for invalid or unsupported data.
 * @note Checks both lists, sorting, terminators, slots, and matching placements.
 * Weighted layouts are rejected explicitly; the supported US NZ0 has only static lists.
 */
bool SotnActorLayout_Open(SotnDataView image, Uint32 index, SotnActorLayout **out);
/** @brief Validate paired placement lists with the original stage's actor-ID boundary.
 * @param image Borrowed overlay, alive until Close(). @param stage_id 12, 65, or 7.
 * @param index Original layout ID, below 53. @param out Receives ownership or NULL.
 * @return Success or SDL error; unsupported stage identities fail explicitly.
 */
bool SotnActorLayout_OpenStage(SotnDataView image, Uint16 stage_id, Uint32 index,
                               SotnActorLayout **out);
/** @brief Release metadata; backing bytes remain owned by the caller. @param layout Owned layout or
 * NULL. */
void SotnActorLayout_Close(SotnActorLayout *layout);
/** @brief Count actual placements, excluding sentinels. @param layout Borrowed layout or NULL.
 * @return Count, or zero. */
Uint32 SotnActorLayout_Count(const SotnActorLayout *layout);
/**
 * @brief Read a placement from the original sorted list.
 * @param layout Borrowed validated layout.
 * @param vertical Select Y-sorted rather than X-sorted order.
 * @param index Zero-based placement index, excluding sentinels.
 * @param out Receives a copy on success; unchanged on failure.
 * @return True on success, false with SDL_GetError() for invalid arguments.
 */
bool SotnActorLayout_Get(const SotnActorLayout *layout, bool vertical, Uint32 index,
                         SotnActorPlacement *out);

/** @brief Allocate initialized spawn state. @param out Receives ownership or NULL. @return Success,
 * or false with SDL_GetError(). */
bool SotnRoomActors_Create(SotnRoomActors **out);
/**
 * @brief Clear room slots and initialize the original scrolling spawn cursors.
 * @param actors Borrowed owned spawn state; temporary destruction flags reset on entry.
 * @param layout Borrowed validated layout, alive until next entry or destruction.
 * @param camera_x Nonnegative room-local scroll X, at most 16384 pixels.
 * @param camera_y Nonnegative room-local scroll Y, at most 16384 pixels.
 * @return True on success; false with SDL_GetError() and unchanged state on failure.
 */
bool SotnRoomActors_Enter(SotnRoomActors *actors, const SotnActorLayout *layout, Sint32 camera_x,
                          Sint32 camera_y);
/**
 * @brief Apply original directional scrolling spawn and persistence rules.
 * @param actors Borrowed initialized spawn state.
 * @param camera_x New room-local scroll X, 0..16384.
 * @param camera_y New room-local scroll Y, 0..16384.
 * @return True on success, false with SDL_GetError() and unchanged state on failure.
 * @note Occupied normal slots are preserved; forced placements replace them.
 * Actors removed inside a stationary view do not spontaneously respawn.
 */
bool SotnRoomActors_Scroll(SotnRoomActors *actors, Sint32 camera_x, Sint32 camera_y);
/**
 * @brief Inspect a slot without exposing mutable state.
 * @param actors Borrowed spawn state.
 * @param slot Original stage-relative slot, below SOTN_STAGE_ACTOR_SLOTS.
 * @param out Receives placement, or zeros for an inactive slot.
 * @param generation Receives zero for an inactive slot or its spawn generation.
 * @return True on success, false with SDL_GetError() for invalid arguments.
 */
bool SotnRoomActors_Get(const SotnRoomActors *actors, Uint32 slot, SotnActorPlacement *out,
                        Uint32 *generation);
/**
 * @brief Release a slot after its behavior destroys an actor.
 * @param actors Borrowed spawn state.
 * @param slot Original stage-relative slot, below SOTN_STAGE_ACTOR_SLOTS.
 * @param permanent Apply the actor's original destruction bit within this room, when nonzero.
 * @return True on success, false with SDL_GetError() for invalid arguments.
 * @note Destruction bits last until room entry. Permanent character/castle progress is owned
 * separately.
 */
bool SotnRoomActors_Release(SotnRoomActors *actors, Uint32 slot, bool permanent);
/** @brief Destroy spawn state, leaving its borrowed layout alive. @param actors Owned state or
 * NULL. */
void SotnRoomActors_Destroy(SotnRoomActors *actors);

#endif
