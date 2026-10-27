// SPDX-License-Identifier: AGPL-3.0-or-later
// Original US stage dimensions/addresses checked against the pinned stage headers.
// Metadata only: definitions, artwork, animation and prize tables remain private.
#include <sotn/stage_profile.h>
const SotnStageProfile *SotnStageProfile_Get(Uint16 stage)
{
    static const SotnStageProfile profiles[] = {
        {12, 0x4e, 24, 11, 0xf8, 3, 2, {23, 24, 25}, true},
        {65, 0x63, 24, 10, 0x30, 2, 0, {25, 26, 27}, true},
        {7, 0x5c, 24, 10, 0x30, 2, 0, {22, 23, 24}, true},
        {0, 0x52, 25, 14, 0, 3, 6, {31, 32, SDL_MAX_UINT16}, true},
        {1, 0x5f, 25, 7, 0x10, 6, 7, {14, 17, SDL_MAX_UINT16}, true},
        {2, 0x4f, 24, 11, 0x70, 1, 7, {14, SDL_MAX_UINT16, SDL_MAX_UINT16}, true},
        {4, 0x42, 24, 13, 0x20, 4, 7, {16, SDL_MAX_UINT16, SDL_MAX_UINT16}, true},
        {6, 0x49, 24, 16, 0x98, 3, 7, {16, 18, SDL_MAX_UINT16}, true},
        {30, 0x4d, 50, 10, 0, 4, 7, {SDL_MAX_UINT16, SDL_MAX_UINT16, SDL_MAX_UINT16}, true},
        {25, 0x2f, 24, 1, 0, 3, 7, {SDL_MAX_UINT16, SDL_MAX_UINT16, SDL_MAX_UINT16}, true},
        {26, 0x1e, 24, 0, 0, 2, 7, {SDL_MAX_UINT16, SDL_MAX_UINT16, SDL_MAX_UINT16}, true}};
    for (Uint32 i = 0; i < SDL_arraysize(profiles); ++i)
        if (profiles[i].id == stage)
            return &profiles[i];
    return NULL;
}
bool SotnStageProfile_SaveRoom(Uint16 stage, Uint32 room)
{
    const SotnStageProfile *profile = SotnStageProfile_Get(stage);
    if (profile == NULL || room == SDL_MAX_UINT16)
        return false;
    for (Uint32 i = 0; i < SDL_arraysize(profile->save_rooms); ++i)
        if (room == profile->save_rooms[i])
            return true;
    return false;
}

bool SotnStageProfile_EntranceAfterDeath(Uint32 room)
{
    // Explicit development-entry policy, not a substitute for the live event flag.
    return (room >= 12 && room <= 17) || (room >= 21 && room <= 23) || room == 27;
}
