#include "input.h"
#include <sotn/alucard_sprite.h>
#include <sotn/laboratory.h>
#include <sotn/player.h>
#include <sotn/presenter.h>
#include <sotn/resource.h>

struct SotnLaboratory
{
    SotnStage *stage;
    SotnStageRoom room;
    SotnDataView dra;
    SotnResource *tiles, *common, *arc;
    SotnPlayer *player;
    SotnPresenter *presenter;
    LaboratoryInput input;
    Uint16 frame[SOTN_FRAME_PIXELS];
    Uint8 rgba[SOTN_FRAME_PIXELS * 4];
    Uint32 room_index;
    Uint64 last_time, clock_units;
    bool dirty, paused;
};

static Sint32 Pixel(Sint32 fixed)
{
    return fixed >= 0 ? fixed / 65536 : (Sint32)(-((-(Sint64)fixed + 65535) / 65536));
}

static bool Transition(SotnLaboratory *session, const SotnPlayerState *state)
{
    const Sint32 local_x = Pixel(state->x), local_y = Pixel(state->y);
    const Sint32 width = (Sint32)session->room.foreground.collision.width * 16;
    const Sint32 height = (Sint32)session->room.foreground.collision.height * 16;
    if (local_x >= 0 && local_y >= 0 && local_x < width && local_y < height)
        return true;
    const Sint32 world_x = (Sint32)session->room.left * 256 + local_x;
    const Sint32 world_y = (Sint32)session->room.top * 256 + local_y;
    for (Uint32 i = 0; i < SotnStage_RoomCount(session->stage); ++i)
    {
        SotnStageRoom room = {0};
        if (!SotnStage_GetRoom(session->stage, i, &room))
            return false;
        if (world_x < (Sint32)room.left * 256 || world_x >= ((Sint32)room.right + 1) * 256 ||
            world_y < (Sint32)room.top * 256 || world_y >= ((Sint32)room.bottom + 1) * 256)
            continue;
        if (room.transition)
            return SDL_SetError(
                "This development build cannot leave Laboratory for an unintegrated stage");
        const Sint32 offset_x = ((Sint32)session->room.left - room.left) * 256;
        const Sint32 offset_y = ((Sint32)session->room.top - room.top) * 256;
        if (!SotnPlayer_ChangeRoom(session->player, room.foreground.collision, offset_x, offset_y))
            return false;
        session->room = room;
        session->room_index = i;
        SDL_Log("Laboratory movement checkpoint: room %u", (unsigned int)i);
        return true;
    }
    return SDL_SetError("Player reached an unsupported Laboratory room boundary");
}

bool SotnLaboratory_Create(const SotnGameData *data, const char *directory, SotnLaboratory **out)
{
    if (out == NULL)
        return SDL_InvalidParamError("out");
    *out = NULL;
    SotnLaboratory *session = SDL_calloc(1, sizeof(*session));
    if (session == NULL)
        return false;
    SotnDataView stage_image = {0};
    session->room_index = 1;
    if (!SotnGameData_GetFile(data, SOTN_GAME_DRA, &session->dra) ||
        !SotnGameData_GetFile(data, SOTN_GAME_NZ0, &stage_image) ||
        !SotnStage_Open(stage_image, &session->stage) ||
        !SotnStage_GetRoom(session->stage, session->room_index, &session->room) ||
        session->room.transition ||
        !SotnResource_Open(directory, SOTN_RESOURCE_LAB_GRAPHICS, &session->tiles) ||
        !SotnResource_Open(directory, SOTN_RESOURCE_COMMON_GRAPHICS, &session->common) ||
        !SotnResource_Open(directory, SOTN_RESOURCE_ALUCARD_GRAPHICS, &session->arc) ||
        !SotnPlayer_Create(session->dra, session->room.foreground.collision, 32, 135,
                           &session->player) ||
        !SotnPlayer_Tick(session->player, 0) || !LaboratoryInput_Init(&session->input) ||
        !SotnPresenter_Create("SOTN Native - Laboratory movement checkpoint (development)", 1024,
                              960, &session->presenter))
    {
        char detail[512] = {0};
        SDL_strlcpy(detail, SDL_GetError(), sizeof(detail));
        SotnLaboratory_Destroy(session);
        return SDL_SetError("Cannot initialize Laboratory movement checkpoint: %s", detail);
    }
    session->last_time = SDL_GetTicksNS();
    session->dirty = true;
    SDL_Log("Laboratory movement checkpoint: original movement/frames; "
            "combat, enemies, audio, saves, scripts, and original camera remain unintegrated.");
    *out = session;
    return true;
}

bool SotnLaboratory_Step(SotnLaboratory *session, Uint16 buttons)
{
    if (session == NULL)
        return SDL_InvalidParamError("laboratory");
    SotnPlayerState state = {0};
    if (!SotnPlayer_Tick(session->player, buttons) ||
        !SotnPlayer_GetState(session->player, &state) || !Transition(session, &state))
        return false;
    session->dirty = true;
    return true;
}

bool SotnLaboratory_Draw(SotnLaboratory *session)
{
    if (session == NULL)
        return SDL_InvalidParamError("laboratory");
    if (session->dirty)
    {
        SotnPlayerState state = {0};
        if (!SotnPlayer_GetState(session->player, &state))
            return false;
        const Sint32 x = Pixel(state.x), y = Pixel(state.y);
        const Sint32 max_x =
            (Sint32)session->room.foreground.collision.width * 16 - SOTN_FRAME_WIDTH;
        const Sint32 max_y =
            (Sint32)session->room.foreground.collision.height * 16 - SOTN_FRAME_HEIGHT;
        const Sint32 camera_x = SDL_clamp(x - 128, 0, SDL_max(0, max_x));
        const Sint32 camera_y = SDL_clamp(y - 128, 0, SDL_max(0, max_y));
        const SotnTileSources sources = {SotnResource_View(session->tiles),
                                         SotnResource_View(session->common)};
        SotnAlucardSprite sprite = {0};
        SDL_memset(session->frame, 0, sizeof(session->frame));
        if (!SotnTilemap_Draw(&session->room.background, sources, camera_x, camera_y,
                              session->frame, SOTN_FRAME_PIXELS) ||
            !SotnTilemap_Draw(&session->room.foreground, sources, camera_x, camera_y,
                              session->frame, SOTN_FRAME_PIXELS) ||
            !SotnAlucard_DecodeSprite(session->dra, SotnResource_View(session->arc),
                                      state.animation.sprite, &sprite) ||
            !SotnAlucard_DrawSprite(&sprite, sources.common, x - camera_x, y - camera_y,
                                    state.facing_left, session->frame, SOTN_FRAME_PIXELS) ||
            !SotnFrame_ToRgba(session->frame, SOTN_FRAME_PIXELS, session->rgba,
                              sizeof(session->rgba)) ||
            !SotnPresenter_SetImage(session->presenter, session->rgba, sizeof(session->rgba),
                                    SOTN_FRAME_WIDTH, SOTN_FRAME_HEIGHT))
            return false;
        session->dirty = false;
    }
    return SotnPresenter_Draw(session->presenter);
}

bool SotnLaboratory_Event(SotnLaboratory *session, const SDL_Event *event)
{
    if (session == NULL || event == NULL)
        return SDL_SetError("Laboratory event arguments are invalid");
    if (!LaboratoryInput_Event(&session->input, event))
        return false;
    switch (event->type)
    {
    case SDL_EVENT_WINDOW_FOCUS_LOST:
    case SDL_EVENT_WINDOW_MINIMIZED:
        session->paused = true;
        break;
    case SDL_EVENT_WINDOW_FOCUS_GAINED:
    case SDL_EVENT_WINDOW_RESTORED:
        session->paused = false;
        break;
    default:
        return true;
    }
    session->last_time = SDL_GetTicksNS();
    session->clock_units = 0;
    return true;
}

bool SotnLaboratory_Iterate(SotnLaboratory *session)
{
    if (session == NULL)
        return SDL_InvalidParamError("laboratory");
    const Uint64 now = SDL_GetTicksNS();
    const Uint64 elapsed = now - session->last_time;
    session->last_time = now;
    if (!session->paused && elapsed <= 250000000)
    {
        session->clock_units = SDL_min(session->clock_units + elapsed * 60, 8000000000ULL);
        unsigned int steps = 0;
        while (session->clock_units >= 1000000000 && steps < 8)
        {
            if (!SotnLaboratory_Step(session, LaboratoryInput_Buttons(&session->input)))
                return false;
            session->clock_units -= 1000000000;
            ++steps;
        }
    }
    else
        session->clock_units = 0;
    return SotnLaboratory_Draw(session);
}

void SotnLaboratory_Destroy(SotnLaboratory *session)
{
    if (session != NULL)
    {
        SotnPresenter_Destroy(session->presenter);
        LaboratoryInput_Quit(&session->input);
        SotnPlayer_Close(session->player);
        SotnResource_Close(session->arc);
        SotnResource_Close(session->common);
        SotnResource_Close(session->tiles);
        SotnStage_Close(session->stage);
        SDL_free(session);
    }
}
