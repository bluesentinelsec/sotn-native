// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/game_handlers.c:HandleTitle, 42398.c:MainGame, st/sel/34048.c and3410C.c,
// sotn-decomp7bc54d8157b6bed77e6bd15e5db9461a734fe1e5; private data stays external.
#include "input.h"
#include "startup_internal.h"
#include <sotn/actor_graphics.h>
#include <sotn/launch_renderer.h>
#include <sotn/playback.h>
#include <sotn/player.h>
#include <sotn/presenter.h>
#include <sotn/primitive_slots.h>
#include <sotn/resource.h>
#include <sotn/sound.h>
#include <sotn/startup.h>
enum
{
    STARTUP_PIXELS = 512 * 240
};
typedef struct StartupPrimitive
{
    Uint32 owner;
    SotnTitleSprite sprite;
} StartupPrimitive;
struct SotnStartup
{
    SotnResource *program, *first, *second;
    SotnResource *common;
    const SotnGameData *data;
    char *directory;
    SotnSaveStore *saves;
    SotnLaunchMenu menu;
    SotnLaunchSlot slots[15];
    Uint16 available;
    Uint8 requested_slot;
    char error[160];
    LaboratoryInput input;
    Uint64 last_time, clock_units;
    bool continue_enabled, paused, transferred;
    SotnDataView dra;
    SotnActorGraphics *graphics;
    SotnPresenter *presenter;
    SotnSound *sound;
    SotnPlayback *playback;
    SotnStartupState state;
    // The original namespace belongs to SEL while no gameplay session exists.
    // All title generations retire before a later menu/session can take ownership.
    StartupPrimitive primitives[SOTN_PRIMITIVE_SLOTS];
    Uint16 title_rows[SOTN_TITLE_SPRITES];
    Uint16 frame[STARTUP_PIXELS], next_frame[STARTUP_PIXELS];
    Uint8 rgba[STARTUP_PIXELS * 4];
    bool dirty;
};
void SotnStartup_Close(SotnStartup *s)
{
    if (!s)
        return;
    SotnPlayback_Destroy(s->playback);
    SotnSound_Destroy(s->sound);
    SotnPresenter_Destroy(s->presenter);
    SotnActorGraphics_Destroy(s->graphics);
    SotnResource_Close(s->second);
    SotnResource_Close(s->first);
    SotnResource_Close(s->program);
    SotnResource_Close(s->common);
    SotnSaveStore_Destroy(s->saves);
    LaboratoryInput_Quit(&s->input);
    SDL_free(s->directory);
    SDL_free(s);
}
bool SotnStartup_Open(const SotnGameData *data, const char *directory, SotnStartup **out)
{
    if (!out)
        return SDL_InvalidParamError("out");
    *out = NULL;
    SotnStartup *s = SDL_calloc(1, sizeof(*s));
    if (!s)
        return false;
    s->data = data;
    s->directory = directory ? SDL_strdup(directory) : NULL;
    SotnDataView executable = {0};
    if (!s->directory || !LaboratoryInput_Init(&s->input) ||
        !SotnGameData_GetFile(data, SOTN_GAME_DRA, &s->dra) ||
        !SotnGameData_GetFile(data, SOTN_GAME_MAIN, &executable) ||
        !SotnResource_Open(directory, SOTN_RESOURCE_SEL_CODE, &s->program) ||
        !SotnResource_Open(directory, SOTN_RESOURCE_TITLE_GRAPHICS_0, &s->first) ||
        !SotnResource_Open(directory, SOTN_RESOURCE_TITLE_GRAPHICS_1, &s->second) ||
        !SotnSound_CreateResident(s->dra, &s->sound) ||
        !SotnSound_ConfigureReverb(s->sound, executable) || !SotnPlayback_Create(&s->playback) ||
        !SotnPresenter_Create("SOTN Native", 960, 720, &s->presenter) ||
        !SotnPresenter_SetDisplaySize(s->presenter, 320, 240))
    {
        SotnStartup_Close(s);
        return false;
    }
    s->last_time = SDL_GetTicksNS();
    s->state.random = 1; // Existing native libc adapter's initial cursor, before any character.
    s->state.stage = 69;
    s->dirty = true;
    *out = s;
    return true;
}
static bool StartupRows(const SotnStartup *s, SotnPrimitiveSlots *slots)
{
    SDL_zero(*slots);
    Uint16 actual = 0;
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        if (s->primitives[i].owner)
        {
            slots->occupied[i / 64U] |= (Uint64)1 << (i % 64U);
            ++actual;
        }
    if (actual != s->state.primitive_count)
        return SDL_SetError("Startup lost its actual primitive lease count");
    SotnPrimitiveSlots referenced = {0};
    for (Uint16 i = 0; i < s->state.primitive_count; ++i)
    {
        const Uint16 row = i < 17 ? s->title_rows[i] : SOTN_PRIMITIVE_SLOTS;
        if (row >= SOTN_PRIMITIVE_SLOTS || s->primitives[row].owner != 1 ||
            (referenced.occupied[row / 64U] & ((Uint64)1 << (row % 64U))))
            return SDL_SetError("Startup lost a real title primitive generation");
        referenced.occupied[row / 64U] |= (Uint64)1 << (row % 64U);
    }
    if ((!s->state.title_active || !s->state.title.phase) && actual)
        return SDL_SetError("Startup has title rows before their actual callback birth");
    if (s->state.title_active && s->state.title.phase && !s->state.title.completed && actual != 17)
        return SDL_SetError("Startup lost its seventeen live title rows");
    return true;
}
static void StartupError(SotnStartup *s, const char *detail)
{
    SDL_strlcpy(s->error, detail, sizeof(s->error));
    for (size_t i = 0; s->error[i]; ++i)
        if ((Uint8)s->error[i] < 32 || (Uint8)s->error[i] > 126)
            s->error[i] = '?';
}
static bool StartupSlots(SotnStartup *s)
{
    s->available = 0;
    SDL_zero(s->slots);
    for (Uint16 i = 0; i < 15; ++i)
    {
        if (!SotnSaveStore_Exists(s->saves, i + 1, &s->slots[i].exists))
            return false;
        if (s->slots[i].exists)
        {
            SotnSaveState saved = {0};
            if (SotnSaveStore_Read(s->saves, i + 1, &saved))
            {
                s->slots[i].valid = true;
                s->slots[i].level = saved.character.level;
                s->slots[i].hp = saved.character.hp;
                s->slots[i].stage = saved.stage;
                s->slots[i].room = saved.room;
                s->available |= (Uint16)(1U << i);
            }
        }
    }
    return true;
}
bool SotnStartup_EnableContinue(SotnStartup *s)
{
    if (!s || s->state.menu_requested || s->transferred)
        return SDL_InvalidParamError("startup");
    if (s->continue_enabled)
        return true;
    SotnResource *common = NULL;
    SotnSaveStore *store = NULL;
    if (!SotnResource_Open(s->directory, SOTN_RESOURCE_COMMON_GRAPHICS, &common) ||
        !SotnSaveStore_Create(&store))
    {
        SotnResource_Close(common);
        SotnSaveStore_Destroy(store);
        return false;
    }
    s->common = common;
    s->saves = store;
    s->continue_enabled = true;
    return true;
}
static bool StartupMenu(SotnStartup *s, Uint16 buttons)
{
    if (!s->state.continue_active)
    {
        if (!StartupSlots(s) || !SotnLaunchMenu_Reset(&s->menu, s->state.previous_buttons, 1))
            return false;
        s->menu.phase = SOTN_LAUNCH_SLOTS;
        s->state.continue_active = true;
    }
    SotnLaunchAction action = SOTN_LAUNCH_NONE;
    const SotnLaunchPhase old = s->menu.phase;
    if (!SotnLaunchMenu_Tick(&s->menu, buttons, s->available, &action))
        return false;
    if (s->menu.phase == SOTN_LAUNCH_HOME)
    {
        const Sint32 retained = s->state.title.retained_word;
        s->state.title = (SotnTitleScreen){.retained_word = retained};
        s->state.stage = 69;
        s->state.menu_requested = false;
        s->state.continue_active = false;
        s->input.menu_open = false;
        s->error[0] = '\0';
    }
    else if (old == SOTN_LAUNCH_ERROR && s->menu.phase == SOTN_LAUNCH_SLOTS)
    {
        s->error[0] = '\0';
        if (!StartupSlots(s))
            return false;
    }
    if (action == SOTN_LAUNCH_LOAD)
        s->requested_slot = s->menu.slot;
    ++s->state.engine_clock;
    s->state.previous_buttons = buttons;
    s->state.continue_slot = s->menu.slot;
    s->input.menu_open = s->state.continue_active;
    SDL_memset(s->next_frame, 0, sizeof(s->next_frame));
    if (s->state.continue_active &&
        !SotnLaunchRenderer_Draw(SotnResource_View(s->common), s->menu, s->slots, s->error,
                                 s->next_frame, STARTUP_PIXELS))
        return false;
    Sint16 pcm[SOTN_SOUND_FRAMES * 2] = {0};
    if (!SotnSound_Tick(s->sound, pcm, SDL_arraysize(pcm)) ||
        !SotnPlayback_Queue(s->playback, pcm, SDL_arraysize(pcm)))
        return false;
    SDL_memcpy(s->frame, s->next_frame, sizeof(s->frame));
    s->dirty = true;
    return true;
}

bool SotnStartup_Step(SotnStartup *s, Uint16 buttons)
{
    if (!s || s->transferred)
        return SDL_InvalidParamError("startup");
    if (s->state.menu_requested)
    {
        if (!s->continue_enabled)
            return SDL_SetError(
                "Startup requires its attached next menu owner after title handoff");
        return StartupMenu(s, buttons);
    }
    SotnStartupState next = s->state;
    ++next.engine_clock;
    const Uint16 tapped = (Uint16)(buttons & ~next.previous_buttons);
    next.previous_buttons = buttons;
    SotnTitleScreenEvents events = {0};
    SotnPrimitiveSlots slots = {0};
    Uint16 rows[17] = {0};
    if (!StartupRows(s, &slots))
        return false;
    SDL_memcpy(rows, s->title_rows, sizeof(rows));
    if (!next.title_active)
    {
        switch (next.resident_step)
        {
        case 0:
            ++next.resident_step;
            break;
        case 1:
            if (!s->graphics &&
                !SotnActorGraphics_CreateTitle(s->dra, SotnResource_View(s->first),
                                               SotnResource_View(s->second), &s->graphics))
                return false;
            ++next.resident_step;
            break;
        case 2:
        case 3:
        case 4:
            ++next.resident_step;
            break;
        case 5:
            if (SotnResource_View(s->program).size != 355112)
                return SDL_SetError("Startup lost its actual selected SEL program");
            ++next.resident_step;
            break;
        case 6:
            if (!next.sel_step)
                ++next.sel_step;
            else
                next.title_active = true; // InitSel's explicit native VERSION_PC logo omission.
            break;
        default:
            return SDL_SetError("Startup resident dispatcher lost its original phase");
        }
    }
    else
    {
        if (!next.title.phase)
        {
            bool allocated = false;
            if (!SotnPrimitiveSlots_Reserve(&slots, 17, rows, &allocated))
                return false;
            if (!allocated)
                return SDL_SetError("Original title cannot start without its seventeen real SPRTs");
            next.primitive_count = 17;
        }
        if (!SotnTitleScreen_Tick(
                &next.title,
                (SotnTitleScreenInput){.engine_clock = next.engine_clock,
                                       .start_tapped = (tapped & SOTN_BUTTON_START) != 0},
                &next.random, &events))
            return false;
        if (events.memory_card)
            next.stage = 255;
        if (events.release_rows)
        {
            if (!SotnPrimitiveSlots_Release(&slots, 17, rows))
                return false;
            next.primitive_count = 0;
        }
        next.menu_requested = events.main_menu;
    }
    SDL_memset(s->next_frame, 0, sizeof(s->next_frame));
    if (next.title_active && !next.menu_requested)
        for (Uint16 priority = 0; priority < 3; ++priority)
            for (Uint16 i = 17; i > 0; --i)
                if (next.title.sprites[i - 1U].priority == priority &&
                    !SotnActorGraphics_TitleSprite(
                        s->graphics, &next.title.sprites[i - 1U],
                        (SotnActorSurface){s->next_frame, STARTUP_PIXELS, 512, 240}))
                    return false;
    Sint16 pcm[SOTN_SOUND_FRAMES * 2] = {0};
    if ((events.sound && !SotnSound_Queue(s->sound, events.sound)) ||
        !SotnSound_Tick(s->sound, pcm, SDL_arraysize(pcm)) ||
        !SotnPlayback_Queue(s->playback, pcm, SDL_arraysize(pcm)))
        return false;
    if (events.release_rows)
        for (Uint16 i = 0; i < 17; ++i)
            s->primitives[rows[i]] = (StartupPrimitive){0};
    else if (next.primitive_count)
        for (Uint16 i = 0; i < 17; ++i)
            s->primitives[rows[i]] = (StartupPrimitive){1, next.title.sprites[i]};
    SDL_memcpy(s->title_rows, rows, sizeof(rows));
    SDL_memcpy(s->frame, s->next_frame, sizeof(s->frame));
    s->state = next;
    s->dirty = true;
    return true;
}
bool SotnStartup_Draw(SotnStartup *s)
{
    if (!s)
        return SDL_InvalidParamError("startup");
    if (s->dirty)
    {
        const Uint32 pixels = (s->state.continue_active ? 256U : 512U) * 240U;
        for (Uint32 i = 0; i < pixels; ++i)
        {
            const Uint16 color = s->frame[i];
            for (Uint16 channel = 0; channel < 3; ++channel)
            {
                const Uint8 value = (Uint8)((color >> (channel * 5U)) & 31U);
                s->rgba[i * 4U + channel] = (Uint8)(value * 8U + value / 4U);
            }
            s->rgba[i * 4U + 3] = 255;
        }
        if (!SotnPresenter_SetImage(s->presenter, s->rgba, sizeof(s->rgba),
                                    s->state.continue_active ? 256 : 512, 240))
            return false;
        s->dirty = false;
    }
    return SotnPresenter_Draw(s->presenter);
}
bool SotnStartup_GetState(const SotnStartup *s, SotnStartupState *out)
{
    if (!s || !out)
        return SDL_InvalidParamError("startup state");
    *out = s->state;
    return true;
}
bool SotnStartup_ResetPlayback(SotnStartup *s)
{
    if (!s)
        return SDL_InvalidParamError("startup");
    return SotnPlayback_Reset(s->playback);
}

bool SotnStartup_TakeGame(SotnStartup *s, SotnLaboratory **out)
{
    if (!out)
        return SDL_InvalidParamError("out");
    *out = NULL;
    if (!s || s->transferred)
        return SDL_InvalidParamError("startup");
    if (!s->requested_slot)
        return true;
    SotnLaboratory *game = NULL;
    if (!Laboratory_ContinueStartup(s->data, s->directory, s->saves, s->requested_slot,
                                    s->state.engine_clock, s->state.previous_buttons, s->sound,
                                    &game))
    {
        StartupError(s, SDL_GetError());
        s->menu.phase = SOTN_LAUNCH_ERROR;
        s->requested_slot = 0;
        s->dirty = true;
        return true;
    }
    s->saves = NULL;
    s->requested_slot = 0;
    s->transferred = true;
    *out = game;
    return true;
}
bool SotnStartup_Event(SotnStartup *s, const SDL_Event *event)
{
    if (!s || !event)
        return SDL_InvalidParamError("startup event");
    if (!LaboratoryInput_Event(&s->input, event))
        return false;
    if (event->type >= SDL_EVENT_WINDOW_FIRST && event->type <= SDL_EVENT_WINDOW_LAST &&
        event->window.windowID != SotnPresenter_WindowID(s->presenter))
        return true;
    switch (event->type)
    {
    case SDL_EVENT_WINDOW_FOCUS_LOST:
    case SDL_EVENT_WINDOW_MINIMIZED:
        s->paused = true;
        break;
    case SDL_EVENT_WINDOW_FOCUS_GAINED:
    case SDL_EVENT_WINDOW_RESTORED:
        s->paused = false;
        break;
    default:
        return true;
    }
    s->last_time = SDL_GetTicksNS();
    s->clock_units = 0;
    return SotnPlayback_Reset(s->playback);
}
bool SotnStartup_Iterate(SotnStartup *s)
{
    if (!s)
        return SDL_InvalidParamError("startup");
    const Uint64 now = SDL_GetTicksNS(), elapsed = now - s->last_time;
    s->last_time = now;
    if (!s->paused && elapsed <= 250000000)
    {
        s->clock_units = SDL_min(s->clock_units + elapsed * 60, 8000000000ULL);
        for (Uint16 steps = 0; s->clock_units >= 1000000000 && steps < 8; ++steps)
        {
            if (!SotnStartup_Step(s, LaboratoryInput_Buttons(&s->input)))
                return false;
            s->clock_units -= 1000000000;
            if (s->requested_slot)
                break;
        }
    }
    else
    {
        s->clock_units = 0;
        if (!SotnPlayback_Reset(s->playback))
            return false;
    }
    return SotnStartup_Draw(s);
}
