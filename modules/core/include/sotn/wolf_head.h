#ifndef SOTN_WOLF_HEAD_H
#define SOTN_WOLF_HEAD_H
#include <sotn/wolf_articulation.h>
/** @brief Original retained head animation and underwater-breath timer.
 * The actual head entity is body.parts[3], slot20. Ownership remains with the session. */
typedef struct SotnWolfHead
{
    Uint16 pose;
    Sint16 remaining;
    Uint32 breath_timer;
} SotnWolfHead;
/** @brief Original ordered sound, attack-initialization and bubble requests.
 * The owner applies sound first, spell second, then the bubble factory; these reports
 * confer no successful allocation, attack initialization or collision feedback. */
typedef struct SotnWolfHeadEvents
{
    SotnPlayerFactorySource bubble_source;
    Uint16 sound, spell;
    bool bubble;
} SotnWolfHeadEvents;
/** @brief Advance original Wolf head attachment, bite intent and underwater breathing.
 * @param head Actual copied head cursor. @param body Actual copied parts; only slot20 changes.
 * @param player Actual copied active or entry Wolf snapshot. @param palette Current palette.
 * @param attack_requested This frame's original controller attack intent, not held-button state.
 * @param dra Borrowed private head/bob tables. @param executable Borrowed SDK tables.
 * @param out Requests, unchanged on failure. @return Success or SDL error, atomically.
 * Run after slot19 and before spine21. Native owner still owns generation, animation order,
 * factories, attack power/groups, combat and rendering; this enables no Wolf controls. */
bool SotnWolfHead_Tick(SotnWolfHead *head, SotnWolfArticulation *body,
                       const SotnWolfControllerFrame *player, Uint16 palette, bool attack_requested,
                       SotnDataView dra, SotnDataView executable, SotnWolfHeadEvents *out);
/** @brief Advance the original six-record private head animation after its entity update.
 * @param head Actual head cursor. @param part Actual slot20; nonanimated turning poses retain.
 * @param dra Borrowed private animation. @return Success or SDL error, atomically.
 * Separate from Tick to preserve original behavior/animation ordering. */
bool SotnWolfHead_Animate(SotnWolfHead *head, SotnWolfPart *part, SotnDataView dra);
#endif
