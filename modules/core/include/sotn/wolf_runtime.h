#ifndef SOTN_WOLF_RUNTIME_H
#define SOTN_WOLF_RUNTIME_H
#include <sotn/primitive_slots.h>
#include <sotn/wolf_head.h>
#include <sotn/wolf_root_effect.h>
/** @brief Native tags for original global player rows, distinct from DRA entity identities. */
enum
{
    SOTN_WOLF_ENVIRONMENT_TAG = 0xff01,
    SOTN_PLAYER_AFTERIMAGE_TAG = 0xff02
};
/** @brief Actual selected DRA body entity in original slots 17..30. */
typedef struct SotnWolfRuntimeChild
{
    Uint32 generation, lineage;
    Uint16 slot, identity;
} SotnWolfRuntimeChild;
/** @brief Owned linked Wolf entities; primitive storage stays in the actual stage pool.
 * Zero-initialize once per original full stage initialization. Retain global capture/image
 * cursors across body retirement. This owner does not enable transformation or controls. */
typedef struct SotnWolfRuntime
{
    SotnWolfRuntimeChild children[14];
    SotnWolfArticulation body;
    SotnWolfRoot root;
    SotnWolfHead head;
    SotnWolfRootEffect effect;
    Uint32 generation, serial, environment_generation;
    Uint16 root_rows[6], environment_rows[2];
    bool root_reserved, environment_reserved;
} SotnWolfRuntime;
/** @brief Actual shared pool, collision service and current camera/room rebase. */
typedef struct SotnWolfRuntimeInput
{
    SotnDataView dra, executable;
    SotnPrimitiveSlots primitive_slots;
    SotnWolfCollisionQuery query;
    void *context;
    Uint64 other_occupied;
    Sint16 screen_shift_x, screen_shift_y;
    Uint16 palette;
    bool active, entering, attack_requested;
} SotnWolfRuntimeInput;
/** @brief Ordered original attack intents; their actual attack/group owners must apply them. */
typedef struct SotnWolfRuntimeSpell
{
    Uint32 generation;
    Uint16 slot, spell;
} SotnWolfRuntimeSpell;
/** @brief Actual body requests and once-only front hit-feedback MP spending for its owner. */
typedef struct SotnWolfRuntimeEvents
{
    SotnWolfRuntimeSpell spells[2];
    SotnPlayerFactorySource bubble_source;
    Uint16 mp_spent, sound;
    Uint8 spell_count;
    bool bubble;
} SotnWolfRuntimeEvents;
/** @brief Return actual owned original-slot mask, or zero for NULL/retired body. */
Uint64 SotnWolfRuntime_Occupied(const SotnWolfRuntime *runtime);
/** @brief Create all fourteen actual body identities after the real entry factory succeeded.
 * @param runtime Owned stage lifetime, with no live body. @param generation Actual form lineage.
 * @param other_occupied Actual other owners, after original forced replacement of slots17..30.
 * @return Success or SDL error with no mutation. Does not replace unrelated owners silently.
 * The native session must first perform the original forced replacements and commit retirement
 * of their actual primitives. This operation creates bodies, not visual readiness or hitboxes. */
bool SotnWolfRuntime_Begin(SotnWolfRuntime *runtime, Uint32 generation, Uint64 other_occupied);
/** @brief Advance actual linked entities once, reserving root/environment against shared rows.
 * @param runtime Actual owner; unchanged on failure. @param player Actual screen-space copied
 * controller after player animation/collision; root/front changes commit atomically on success.
 * @param input Actual pool/collision/private context. Entering rebases without advancing.
 * @param out Current ordered requests, unchanged on failure. @return Success or SDL error.
 * The stage owner must validate/commit row reservations before any other owner advances. Full
 * stage reset retires this owner; inactive Tick retires only body/root, retaining common env rows.
 * Spell intents and bubble requests confer no successful attack/factory allocation. */
bool SotnWolfRuntime_Tick(SotnWolfRuntime *runtime, SotnWolfControllerFrame *player,
                          SotnWolfRuntimeInput input, SotnWolfRuntimeEvents *out);
/** @brief Deliver actual generation-checked front-part enemy contact for next body update.
 * @param runtime Actual owner. @param generation Observed front slot18 identity.
 * @param flags Original hit flags,1..3. @return Success or SDL error, preserving stale owners. */
bool SotnWolfRuntime_Hit(SotnWolfRuntime *runtime, Uint32 generation, Uint8 flags);
#endif
