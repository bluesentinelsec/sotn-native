#include "input.h"
#include <sotn/actor_graphics.h>
#include <sotn/actors.h>
#include <sotn/alucard_sprite.h>
#include <sotn/laboratory.h>
#include <sotn/player.h>
#include <sotn/presenter.h>
#include <sotn/resource.h>

struct SotnLaboratory
{
    SotnStage *stage;
    SotnStageRoom room;
    SotnDataView dra, nz0;
    SotnResource *tiles, *common, *arc;
    SotnPlayer *player;
    SotnActors *actors;
    SotnActorLayout *actor_layout;
    SotnActorGraphics *actor_graphics;
    SotnPresenter *presenter;
    LaboratoryInput input;
    Uint16 frame[SOTN_FRAME_PIXELS];
    Uint8 rgba[SOTN_FRAME_PIXELS * 4];
    Uint32 room_index;
    Uint64 last_time, clock_units;
    bool dirty, paused;
    bool warned_actors[0x4e];
};

static Sint32 Pixel(Sint32 fixed)
{
    return fixed >= 0 ? fixed / 65536 : (Sint32)(-((-(Sint64)fixed + 65535) / 65536));
}

static void Camera(const SotnLaboratory *session, SotnPlayerState player, Sint32 *x, Sint32 *y)
{
    const Sint32 max_x = (Sint32)session->room.foreground.collision.width * 16 - SOTN_FRAME_WIDTH;
    const Sint32 max_y = (Sint32)session->room.foreground.collision.height * 16 - SOTN_FRAME_HEIGHT;
    *x = SDL_clamp(Pixel(player.x) - 128, 0, SDL_max(0, max_x));
    *y = SDL_clamp(Pixel(player.y) - 128, 0, SDL_max(0, max_y));
}

static bool EnterActors(SotnLaboratory *session)
{
    SotnPlayerState player = {0};
    SotnActorLayout *layout = NULL;
    if (!SotnPlayer_GetState(session->player, &player) ||
        !SotnActorLayout_Open(session->nz0, session->room.object_layout_id, &layout))
        return false;
    Sint32 camera_x = 0, camera_y = 0;
    Camera(session, player, &camera_x, &camera_y);
    if ((session->room.object_graphics_id != 0 &&
         !SotnActorGraphics_LoadBank(session->actor_graphics,
                                     (Uint32)session->room.object_graphics_id - 1)) ||
        !SotnActors_Enter(session->actors, layout, session->room.foreground.collision, camera_x,
                          camera_y))
    {
        SotnActorLayout_Close(layout);
        return false;
    }
    SotnActorLayout_Close(session->actor_layout);
    session->actor_layout = layout;
    return true;
}

static bool TickActors(SotnLaboratory *session)
{
    SotnPlayerState player = {0};
    if (!SotnPlayer_GetState(session->player, &player))
        return false;
    Sint32 camera_x = 0, camera_y = 0;
    Camera(session, player, &camera_x, &camera_y);
    if (!SotnActors_Tick(session->actors, player, camera_x, camera_y))
        return false;
    for (Uint32 i = 0; i < SOTN_STAGE_ACTOR_SLOTS; ++i)
    {
        SotnActorState state = {0};
        if (!SotnActors_Get(session->actors, i, &state))
            return false;
        if (state.generation != 0 && state.unsupported)
        {
            if (state.id >= SDL_arraysize(session->warned_actors))
                return SDL_SetError("Laboratory actor has an invalid original entity ID");
            if (!session->warned_actors[state.id])
            {
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                            "Laboratory actor 0x%02x is not integrated; this development "
                            "checkpoint omits it",
                            (unsigned int)state.id);
                session->warned_actors[state.id] = true;
            }
        }
    }
    return true;
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
        if (!EnterActors(session))
            return false;
        SDL_Log("Laboratory checkpoint: room %u", (unsigned int)i);
        return true;
    }
    return SDL_SetError("Player reached an unsupported Laboratory room boundary");
}

bool SotnLaboratory_Create(const SotnGameData *data, const char *directory, Uint32 start_room,
                           SotnLaboratory **out)
{
    if (out == NULL)
        return SDL_InvalidParamError("out");
    *out = NULL;
    if (start_room != 1 && start_room != 3)
        return SDL_SetError("Laboratory development starts are currently room 1 or 3");
    SotnLaboratory *session = SDL_calloc(1, sizeof(*session));
    if (session == NULL)
        return false;
    session->room_index = start_room;
    if (!SotnGameData_GetFile(data, SOTN_GAME_DRA, &session->dra) ||
        !SotnGameData_GetFile(data, SOTN_GAME_NZ0, &session->nz0) ||
        !SotnStage_Open(session->nz0, &session->stage) ||
        !SotnStage_GetRoom(session->stage, session->room_index, &session->room) ||
        session->room.transition ||
        !SotnResource_Open(directory, SOTN_RESOURCE_LAB_GRAPHICS, &session->tiles) ||
        !SotnResource_Open(directory, SOTN_RESOURCE_COMMON_GRAPHICS, &session->common) ||
        !SotnResource_Open(directory, SOTN_RESOURCE_ALUCARD_GRAPHICS, &session->arc) ||
        !SotnPlayer_Create(session->dra, session->room.foreground.collision, 32, 135,
                           &session->player) ||
        !SotnPlayer_Tick(session->player, 0) ||
        !SotnActors_Create(session->dra, session->nz0, &session->actors) ||
        !SotnActorGraphics_Create(session->dra, session->nz0,
                                  (SotnTileSources){SotnResource_View(session->tiles),
                                                    SotnResource_View(session->common)},
                                  &session->actor_graphics) ||
        !EnterActors(session) || !TickActors(session) || !LaboratoryInput_Init(&session->input) ||
        !SotnPresenter_Create("SOTN Native - Laboratory actor checkpoint (development)", 1024, 960,
                              &session->presenter))
    {
        char detail[512] = {0};
        SDL_strlcpy(detail, SDL_GetError(), sizeof(detail));
        SotnLaboratory_Destroy(session);
        return SDL_SetError("Cannot initialize Laboratory actor checkpoint: %s", detail);
    }
    session->last_time = SDL_GetTicksNS();
    session->dirty = true;
    SDL_Log("Laboratory development start: room %u", (unsigned int)start_room);
    SDL_Log("Laboratory actor checkpoint: candles, busts, globe tables, and Bloody Zombie "
            "motion/animation; "
            "combat, drops, audio, other actors/effects, saves, scripts, and original camera "
            "remain incomplete.");
    *out = session;
    return true;
}

bool SotnLaboratory_Step(SotnLaboratory *session, Uint16 buttons)
{
    if (session == NULL)
        return SDL_InvalidParamError("laboratory");
    SotnPlayerState state = {0};
    if (!SotnPlayer_Tick(session->player, buttons) ||
        !SotnPlayer_GetState(session->player, &state) || !Transition(session, &state) ||
        !TickActors(session))
        return false;
    session->dirty = true;
    return true;
}

static bool DrawActorsAndPlayer(SotnLaboratory *session, SotnPlayerState player, Sint32 camera_x,
                                Sint32 camera_y)
{
    SotnActorState list[SOTN_STAGE_ACTOR_SLOTS] = {0};
    Uint32 count = 0;
    for (Uint32 slot = SOTN_STAGE_ACTOR_SLOTS; slot > 0; --slot)
    {
        SotnActorState state = {0};
        if (!SotnActors_Get(session->actors, slot - 1, &state))
            return false;
        if (state.generation == 0 || state.unsupported || state.frame == 0)
            continue;
        Uint32 position = count;
        while (position > 0 && list[position - 1].priority < state.priority)
        {
            list[position] = list[position - 1];
            --position;
        }
        list[position] = state;
        ++count;
    }
    SotnAlucardSprite alucard = {0};
    if (!SotnAlucard_DecodeSprite(session->dra, SotnResource_View(session->arc),
                                  player.animation.sprite, &alucard))
        return false;
    bool player_drawn = false;
    for (Uint32 i = 0; i <= count; ++i)
    {
        if (!player_drawn && (i == count || list[i].priority < 0x80))
        {
            if (!SotnAlucard_DrawSprite(&alucard, SotnResource_View(session->common),
                                        Pixel(player.x) - camera_x, Pixel(player.y) - camera_y,
                                        player.facing_left, session->frame, SOTN_FRAME_PIXELS))
                return false;
            player_drawn = true;
        }
        if (i == count)
            break;
        const SotnActorState actor = list[i];
        const SotnActorSprite sprite = {actor.animation_set,  actor.frame,      actor.palette,
                                        actor.texture_offset, actor.blend_mode, actor.facing_left};
        if (!SotnActorGraphics_Draw(session->actor_graphics, sprite, Pixel(actor.x) - camera_x,
                                    Pixel(actor.y) - camera_y, session->frame, SOTN_FRAME_PIXELS))
            return false;
    }
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
        Sint32 camera_x = 0, camera_y = 0;
        Camera(session, state, &camera_x, &camera_y);
        const SotnTileSources sources = {SotnResource_View(session->tiles),
                                         SotnResource_View(session->common)};
        SDL_memset(session->frame, 0, sizeof(session->frame));
        if (!SotnTilemap_Draw(&session->room.background, sources, camera_x, camera_y,
                              session->frame, SOTN_FRAME_PIXELS) ||
            !SotnTilemap_Draw(&session->room.foreground, sources, camera_x, camera_y,
                              session->frame, SOTN_FRAME_PIXELS) ||
            !DrawActorsAndPlayer(session, state, camera_x, camera_y) ||
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
        SotnActors_Destroy(session->actors);
        SotnActorLayout_Close(session->actor_layout);
        SotnActorGraphics_Destroy(session->actor_graphics);
        SotnPlayer_Close(session->player);
        SotnResource_Close(session->arc);
        SotnResource_Close(session->common);
        SotnResource_Close(session->tiles);
        SotnStage_Close(session->stage);
        SDL_free(session);
    }
}
