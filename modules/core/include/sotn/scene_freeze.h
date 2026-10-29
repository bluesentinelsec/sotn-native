#ifndef SOTN_SCENE_FREEZE_H
#define SOTN_SCENE_FREEZE_H
#include <sotn/game_data.h>
/** @brief Selected original engine-five spell freeze, separate from ordinary pause.
 * Zero-initialize on full stage initialization. No state is saved to ordinary slots. */
typedef struct SotnSceneFreeze
{
    Uint16 request;
    bool active;
} SotnSceneFreeze;
typedef enum SotnSceneFreezeFrame
{
    SOTN_SCENE_NORMAL,
    SOTN_SCENE_FROZEN,
    SOTN_SCENE_RELEASE
} SotnSceneFreezeFrame;
/** @brief Select this frame's original engine path before any child can change its request.
 * @param freeze Actual engine owner. @param out Receives stable mode for the entire frame.
 * @return Success or SDL error preserving state/output. A cleared request still uses a
 * separate release-only frame: apply original protection/audio resume then return, with no
 * player/child/stage update. Normal simulation resumes on the following frame. */
bool SotnSceneFreeze_BeginFrame(SotnSceneFreeze *freeze, SotnSceneFreezeFrame *out);
/** @brief Apply actual portal birth(3) or retirement(0), without changing this frame's mode.
 * @param freeze Actual engine owner. @param request Selected original request0/3.
 * @return Success or SDL error preserving state. No capture, allocation or readiness is implied. */
bool SotnSceneFreeze_Request(SotnSceneFreeze *freeze, Uint16 request);
/** @brief Select engine five at the END of the ordinary birth frame.
 * @param freeze Actual engine owner. @param out True only for the real entry edge.
 * @return Success or SDL error preserving state/output. On the edge the native owner must
 * reset actual afterimages(1,1) and pause sound scripts. The birth frame's ordinary stage
 * update must finish first. Frozen frames use this same call and never repeat the edge. */
bool SotnSceneFreeze_EndFrame(SotnSceneFreeze *freeze, bool *out);
#endif
