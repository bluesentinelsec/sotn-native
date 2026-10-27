#ifndef SOTN_PLAYER_FACTORY_H
#define SOTN_PLAYER_FACTORY_H

#include <sotn/game_data.h>

enum
{
    SOTN_PLAYER_FACTORY_CHILDREN = 63
};

/** @brief Copied real factory source; x/y are room-local 16.16 positions. */
typedef struct SotnPlayerFactorySource
{
    Sint32 x, y;
    Uint32 flags;
    Uint16 priority;
    bool facing_left;
} SotnPlayerFactorySource;

/** @brief One actual DRA factory cursor; allocation of its real slot 8..15 belongs to the owner.
 * Zero-init through Begin, retain until removed, never ordinary save data. */
typedef struct SotnPlayerFactory
{
    SotnPlayerFactorySource source;
    Uint16 blueprint, parameter, entity_id, entity_modifier, spawn_index;
    Sint16 argument;
    Uint8 child, amount, per_cycle, cycle, delay, kind, origin;
    bool initialized, removed, noncritical, increment_parameter;
} SotnPlayerFactory;

/** @brief One original slot write, not a live or initialized gameplay actor.
 * Entity identity zero clears the slot and leaves it available to subsequent allocations. */
typedef struct SotnPlayerFactoryChild
{
    SotnPlayerFactorySource source;
    Uint16 slot, entity_id, parameter, argument_low;
    Sint16 argument_high;
    bool replace;
} SotnPlayerFactoryChild;

/** @brief Real original slot availability and player state at this factory's update point. */
typedef struct SotnPlayerFactoryInput
{
    Uint64
        occupied; /**< Actual DRA entity slots 0..63, including existing other ability children. */
    Sint32 player_x, player_y;
    Uint16 player_step, player_substep;
} SotnPlayerFactoryInput;

/** @brief Original ordered allocations; caller initializes these actual slots in this order.
 * The updated mask includes allocations/explicit empty writes. Only the original forced
 * weapon-slot replacement may retire a previously occupied slot. */
typedef struct SotnPlayerFactoryEvents
{
    SotnPlayerFactoryChild children[SOTN_PLAYER_FACTORY_CHILDREN];
    Uint64 occupied;
    Uint16 count;
    bool clear_weapon_state;
} SotnPlayerFactoryEvents;

/** @brief Initialize one already-reserved real factory with original packed request fields.
 * @param factory Owned output. @param source Copied actual source.
 * @param packed Original blueprint/parameter/family word. @param argument Original signed arg2.
 * @return Success or SDL error without mutation for an invalid blueprint/priority/output.
 * This operation never asserts that a factory slot was available or reserves one itself. */
bool SotnPlayerFactory_Begin(SotnPlayerFactory *factory, SotnPlayerFactorySource source,
                             Uint32 packed, Sint16 argument);

/** @brief Advance original private blueprint scheduling and real-slot selection once.
 * @param factory Owned actual factory. @param dra Borrowed private blueprint/range definitions.
 * @param input Actual occupied mask/current player at this update point.
 * @param out Receives ordered allocations, unchanged with factory on malformed data/state.
 * @return Success, including exhausted optional allocation, or SDL error.
 * Critical factories retry; noncritical ones retire on exhaustion. Kind three explicitly
 * replaces its original weapon slot. The caller owns generations, child initialization,
 * primitive capacity and original shared update order; this is not a second live actor pool. */
bool SotnPlayerFactory_Tick(SotnPlayerFactory *factory, SotnDataView dra,
                            SotnPlayerFactoryInput input, SotnPlayerFactoryEvents *out);

#endif
