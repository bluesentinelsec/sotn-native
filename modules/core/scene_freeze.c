// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/5087C.c Engine_Normal/Engine_5 edges, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. No game data is embedded.
#include <SDL3/SDL_error.h>
#include <sotn/scene_freeze.h>
static bool Valid(const SotnSceneFreeze *f)
{
    return f && (f->request == 0 || f->request == 3);
}
bool SotnSceneFreeze_BeginFrame(SotnSceneFreeze *f, SotnSceneFreezeFrame *out)
{
    if (!Valid(f) || !out)
        return SDL_SetError("Spell freeze requires its actual selected engine state/output");
    *out = !f->active ? SOTN_SCENE_NORMAL : f->request ? SOTN_SCENE_FROZEN : SOTN_SCENE_RELEASE;
    if (*out == SOTN_SCENE_RELEASE)
        f->active = false;
    return true;
}
bool SotnSceneFreeze_Request(SotnSceneFreeze *f, Uint16 request)
{
    if (!Valid(f) || (request != 0 && request != 3) || (!request && !f->active))
        return SDL_SetError("Spell freeze requires actual portal birth or active retirement");
    f->request = request;
    return true;
}
bool SotnSceneFreeze_EndFrame(SotnSceneFreeze *f, bool *out)
{
    if (!Valid(f) || !out)
        return SDL_SetError("Spell freeze requires its actual selected engine state/output");
    *out = !f->active && f->request;
    if (*out)
        f->active = true;
    return true;
}
