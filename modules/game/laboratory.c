#include "input.h"
#include <sotn/actor_graphics.h>
#include <sotn/actors.h>
#include <sotn/alucard_sprite.h>
#include <sotn/game_over.h>
#include <sotn/hud.h>
#include <sotn/inventory.h>
#include <sotn/laboratory.h>
#include <sotn/music.h>
#include <sotn/playback.h>
#include <sotn/player.h>
#include <sotn/presenter.h>
#include <sotn/resource.h>
#include <sotn/save.h>
#include <sotn/save_point_renderer.h>
#include <sotn/sound.h>
#include <sotn/subweapon_renderer.h>

struct SotnLaboratory
{
    SotnStage *stage;
    SotnStageRoom room;
    SotnWorldState world;
    SotnCamera camera;
    SotnDataView dra, nz0;
    SotnResource *tiles, *common, *arc, *weapons, *game_over_art;
    SotnPlayer *player;
    SotnActors *actors;
    SotnSubweapons *subweapons;
    SotnActorLayout *actor_layout;
    SotnActorGraphics *actor_graphics;
    SotnPresenter *presenter;
    SotnHud *hud;
    SotnCharacter *character;
    Uint8 player_common[0x42000];
    Uint16 cape;
    bool palette_ready;
    SotnInventoryMenu menu;
    SotnInventoryRenderer *inventory_renderer;
    Uint8 menu_rgba[SOTN_MENU_RGBA_BYTES];
    SotnGameOver *game_over;
    Uint16 scene_buttons, mp, hands[2];
    Uint64 simulation_ticks;
    SotnSaveStore *saves;
    SotnSavePoint save_point;
    SotnSavePointRenderer *save_renderer;
    bool save_room;
    LaboratoryInput input;
    Uint16 frame[SOTN_FRAME_PIXELS];
    Uint8 rgba[SOTN_FRAME_PIXELS * 4];
    Uint32 room_index;
    Uint64 last_time, clock_units;
    bool dirty, paused;
    bool warned_actors[0x4e];
    Uint16 displayed_hp, displayed_hearts, displayed_subweapon;
    Uint32 displayed_gold;
    SotnResource *sound_resources[5];
    SotnSound *sound;
    SotnPlayback *audio;
    SotnStream *music_stream;
    SotnMusic *music;
};

static bool OpenAudio(SotnLaboratory *session, const char *directory)
{
    const SotnResourceId ids[5] = {SOTN_RESOURCE_COMMON_AUDIO, SOTN_RESOURCE_ALUCARD_AUDIO_HEADER,
                                   SOTN_RESOURCE_ALUCARD_AUDIO_BODY, SOTN_RESOURCE_LAB_AUDIO_HEADER,
                                   SOTN_RESOURCE_LAB_AUDIO_BODY};
    for (Uint32 i = 0; i < 5; ++i)
        if (!SotnResource_Open(directory, ids[i], &session->sound_resources[i]))
            return false;
    const SotnSoundSources sources = {session->dra,
                                      SotnResource_View(session->sound_resources[0]),
                                      SotnResource_View(session->sound_resources[1]),
                                      SotnResource_View(session->sound_resources[2]),
                                      SotnResource_View(session->sound_resources[3]),
                                      SotnResource_View(session->sound_resources[4])};
    if (!SotnSound_Create(sources, &session->sound))
        return false;
    SDL_Log("Verifying private music container before streaming Laboratory music...");
    if (!SotnStream_OpenMusic(directory, &session->music_stream) ||
        !SotnMusic_Create(session->dra, session->music_stream, &session->music))
        return false;
    return SotnPlayback_Create(&session->audio);
}

static bool TickAudio(SotnLaboratory *session, const SotnActorCombat *combat, Uint16 player_sound)
{
    if (session->sound == NULL)
        return true;
    for (Uint32 i = 0; i < combat->sound_count; ++i)
        if (!SotnSound_Queue(session->sound, combat->sounds[i]))
            return false;
    if (player_sound != 0 && !SotnSound_Queue(session->sound, player_sound))
        return false;
    Uint16 sounds[64] = {0}, sound_count = 0;
    if (session->subweapons != NULL)
    {
        if (!SotnSubweapons_Sounds(session->subweapons, sounds, SDL_arraysize(sounds),
                                   &sound_count))
            return false;
        for (Uint16 i = 0; i < sound_count; ++i)
            if (!SotnSound_Queue(session->sound, sounds[i]))
                return false;
    }
    Sint16 pcm[SOTN_SOUND_FRAMES * 2] = {0};
    if (session->game_over == NULL && !SotnSound_Tick(session->sound, pcm, SDL_arraysize(pcm)))
        return false;
    if (session->music != NULL && (!session->save_room || session->game_over != NULL))
    {
        Sint16 music[SOTN_SOUND_FRAMES * 2] = {0};
        if (!SotnMusic_Tick(session->music, music, SDL_arraysize(music)))
            return false;
        for (Uint32 i = 0; i < SDL_arraysize(pcm); ++i)
            pcm[i] = (Sint16)SDL_clamp((Sint32)pcm[i] + music[i], -32768, 32767);
    }
    return SotnPlayback_Queue(session->audio, pcm, SDL_arraysize(pcm));
}

static Sint32 Pixel(Sint32 fixed)
{
    return fixed >= 0 ? fixed / 65536 : (Sint32)(-((-(Sint64)fixed + 65535) / 65536));
}

static void Camera(const SotnLaboratory *session, SotnPlayerState player, Sint32 *x, Sint32 *y)
{
    (void)player;
    *x = session->camera.x;
    *y = session->camera.y;
}

static bool EnterActors(SotnLaboratory *session)
{
    SotnPlayerState player = {0};
    SotnActorLayout *layout = NULL;
    if (!SotnPlayer_GetState(session->player, &player) ||
        !SotnActorLayout_Open(session->nz0, session->room.object_layout_id, &layout))
        return false;
    if (!SotnCamera_Enter(&session->camera, session->room.foreground.collision.width * 16,
                          session->room.foreground.collision.height * 16, Pixel(player.x),
                          Pixel(player.y)))
    {
        SotnActorLayout_Close(layout);
        return false;
    }
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
    if (!SotnActors_GetMap(session->actors, &session->room.foreground.collision) ||
        !SotnPlayer_ChangeRoom(session->player, session->room.foreground.collision, 0, 0) ||
        !SotnSubweapons_Enter(session->subweapons, session->room.foreground.collision))
        return false;
    return true;
}

static bool ApplyCharacter(SotnLaboratory *session)
{
    SotnCharacterState state = {0};
    SotnCharacterStats stats = {0};
    if (!SotnCharacter_Get(session->character, &state, &stats))
        return false;
    if (session->hands[0] != state.equipment[0] || session->hands[1] != state.equipment[1])
    {
        const SotnDataView weapons = SotnResource_View(session->weapons);
        if (!SotnPlayer_SetEquipment(session->player, weapons, state.equipment[0],
                                     state.equipment[1]))
            return false;
        for (Uint32 hand = 0; hand < 2; ++hand)
        {
            SotnEquipment equipment = {0};
            if (!SotnEquipment_Read(session->dra, state.equipment[hand], &equipment) ||
                !SotnActorGraphics_LoadWeapon(session->actor_graphics, weapons, hand, equipment))
                return false;
            session->hands[hand] = state.equipment[hand];
        }
    }
    if (!session->palette_ready || session->cape != state.equipment[4])
    {
        Uint16 colors[4] = {0};
        if (!SotnAlucard_CapePalette(session->dra, state.equipment[4], colors))
            return false;
        if (!session->palette_ready)
            SDL_memcpy(session->player_common, SotnResource_View(session->common).bytes,
                       sizeof(session->player_common));
        for (Uint32 i = 0; i < 4; ++i)
        {
            session->player_common[0x40000 + (6 + i) * 2] = (Uint8)colors[i];
            session->player_common[0x40000 + (6 + i) * 2 + 1] = (Uint8)(colors[i] >> 8);
        }
        session->cape = state.equipment[4];
        session->palette_ready = true;
    }
    session->mp = state.mp;
    return SotnPlayer_SetCharacter(session->player, &state, stats);
}

static bool TickActors(SotnLaboratory *session)
{
    SotnPlayerState player = {0};
    if (!SotnPlayer_GetState(session->player, &player))
        return false;
    if (!SotnCamera_Tick(&session->camera, Pixel(player.x), Pixel(player.y), false))
        return false;
    Sint32 camera_x = 0, camera_y = 0;
    Camera(session, player, &camera_x, &camera_y);
    SotnCharacterState profile = {0};
    if (!SotnCharacter_Get(session->character, &profile, NULL))
        return false;
    profile.hp = player.hp;
    profile.hp_max = player.hp_max;
    if (!SotnCharacter_Restore(session->character, &profile))
        return false;
    if (!SotnActors_SetButtons(session->actors, session->scene_buttons) ||
        !SotnSubweapons_Tick(session->subweapons, player, camera_x, camera_y) ||
        !SotnActors_Tick(session->actors, player, camera_x, camera_y))
        return false;
    SotnActorMotion motion = {0};
    if (!SotnActors_GetMotion(session->actors, &motion) ||
        !SotnPlayer_Displace(session->player, motion.x, motion.y, motion.contacts) ||
        (motion.protect && !SotnPlayer_StageGuard(session->player, motion.halt_vertical)))
        return false;
    SotnActorCombat combat = {0};
    SotnCharacterStats attributes = {0};
    if (!SotnActors_GetCombat(session->actors, &combat) ||
        !SotnCharacter_Get(session->character, &profile, &attributes))
        return false;
    if (player.step != SOTN_PLAYER_KILL && !ApplyCharacter(session))
        return false;
    const Uint16 damage =
        combat.contact_damage == 0
            ? 0
            : (Uint16)SDL_max(1, (Sint32)combat.contact_damage - attributes.defense);
    if (!SotnPlayer_Damage(session->player, damage, combat.contact_x, combat.contact_velocity))
        return false;
    if (!SotnPlayer_GetState(session->player, &player))
        return false;
    profile.hp = player.hp;
    if (!SotnCharacter_Restore(session->character, &profile))
        return false;
    if (player.hp != session->displayed_hp || combat.hearts != session->displayed_hearts ||
        combat.gold != session->displayed_gold || combat.subweapon != session->displayed_subweapon)
    {
        SDL_Log("Development inventory: HP %u/%u, hearts %u/%u, gold %u, subweapon %u", player.hp,
                player.hp_max, combat.hearts, combat.hearts_max, (unsigned int)combat.gold,
                combat.subweapon);
        session->displayed_hp = player.hp;
        session->displayed_hearts = combat.hearts;
        session->displayed_gold = combat.gold;
        session->displayed_subweapon = combat.subweapon;
    }
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

static bool EnterSaveRoom(SotnLaboratory *session)
{
    const bool was_save_room = session->save_room;
    session->save_room = (session->room.foreground.parameters & 0x20) != 0;
    session->input.menu_open = false;
    if (session->save_room)
    {
        Uint16 occupied = 0;
        if (session->saves == NULL && !SotnSaveStore_Create(&session->saves))
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot open native saves: %s",
                         SDL_GetError());
        if (session->saves != NULL)
            for (Uint32 slot = 1; slot <= SOTN_SAVE_SLOTS; ++slot)
            {
                bool exists = false;
                if (!SotnSaveStore_Exists(session->saves, slot, &exists))
                    return false;
                if (exists)
                    occupied |= (Uint16)(1U << (slot - 1));
            }
        if (!SotnSavePoint_Reset(&session->save_point, SOTN_SAVE_SLOTS,
                                 session->save_point.slot == 0 ? 1 : session->save_point.slot,
                                 occupied))
            return false;
    }
    if (session->audio != NULL && was_save_room != session->save_room)
    {
        if (!SotnPlayback_Reset(session->audio))
            return false;
        if (!session->save_room)
        {
            SotnMusic_Destroy(session->music);
            session->music = NULL;
            if (!SotnMusic_Create(session->dra, session->music_stream, &session->music))
                return false;
        }
    }
    return true;
}

static bool SaveState(SotnLaboratory *session, SotnSaveState *out)
{
    SotnPlayerState player = {0};
    SotnActorProgress progress = {0};
    if (!SotnPlayer_GetState(session->player, &player) ||
        !SotnActors_GetProgress(session->actors, &progress))
        return false;
    SotnCharacterState character = {0};
    if (!SotnCharacter_Get(session->character, &character, NULL))
        return false;
    character.hp = player.hp;
    *out = (SotnSaveState){.room = session->room_index,
                           .character = character,
                           .ticks = session->simulation_ticks,
                           .player_random = player.random,
                           .actor_random = progress.random,
                           .combat_random = progress.combat_random};
    return true;
}

static bool TickSaveRoom(SotnLaboratory *session, Uint16 buttons)
{
    if (!session->save_room)
        return true;
    SotnPlayerState player = {0};
    SotnSavePointAction action = {0};
    if (!SotnPlayer_GetState(session->player, &player) ||
        !SotnSavePoint_Tick(&session->save_point, player, buttons, &action))
        return false;
    if (action.heal)
    {
        SotnCharacterState profile = {0};
        if (!SotnCharacter_Get(session->character, &profile, NULL))
            return false;
        profile.hp = profile.hp_max;
        profile.mp = profile.mp_max;
        if (!SotnCharacter_Restore(session->character, &profile) || !ApplyCharacter(session))
            return false;
    }
    if (action.sound != 0 && !SotnSound_Queue(session->sound, action.sound))
        return false;
    if (action.write)
    {
        SotnSaveState state = {0};
        if (!SaveState(session, &state))
            return false;
        if (session->saves == NULL && !SotnSaveStore_Create(&session->saves))
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot open native saves: %s",
                         SDL_GetError());
        const bool saved = session->saves != NULL &&
                           SotnSaveStore_Write(session->saves, session->save_point.slot, state);
        if (!saved)
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Save slot %u failed: %s",
                         session->save_point.slot, SDL_GetError());
        else
            SDL_Log("Saved Laboratory room %u to native slot %u", session->room_index,
                    session->save_point.slot);
        if (!SotnSavePoint_Complete(&session->save_point, saved))
            return false;
    }
    session->input.menu_open = SotnSavePoint_Menu(&session->save_point);
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
        if (!SotnStage_GetRoomDefinitions(session->stage, i, 2048, &room))
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
        if (!SotnSubweapons_Enter(session->subweapons, room.foreground.collision))
            return false;
        session->room = room;
        session->room_index = i;
        if (!EnterActors(session) || !EnterSaveRoom(session))
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
    if (start_room != 1 && start_room != 3 && start_room != 4 && start_room != 5 &&
        start_room != 6 && start_room != 9 && start_room != 10 && start_room != 17 &&
        start_room != 18 && start_room != 20 && start_room != 22 && start_room != 23 &&
        start_room != 24 && start_room != 25)
        return SDL_SetError("Laboratory development starts are currently room 1, 3, 4, 5, 6, 9, "
                            "10, 17, 18, 20, 22, 23, 24, or 25");
    SotnLaboratory *session = SDL_calloc(1, sizeof(*session));
    if (session == NULL)
        return false;
    session->room_index = start_room;
    session->mp = 20;
    if (!SotnGameData_GetFile(data, SOTN_GAME_DRA, &session->dra) ||
        !SotnGameData_GetFile(data, SOTN_GAME_NZ0, &session->nz0) ||
        !SotnCharacter_Create(session->dra, &session->character) ||
        !SotnStage_Open(session->nz0, &session->stage) ||
        !SotnStage_GetRoomDefinitions(session->stage, session->room_index, 2048, &session->room) ||
        session->room.transition ||
        !SotnResource_Open(directory, SOTN_RESOURCE_LAB_GRAPHICS, &session->tiles) ||
        !SotnResource_Open(directory, SOTN_RESOURCE_COMMON_GRAPHICS, &session->common) ||
        !SotnResource_Open(directory, SOTN_RESOURCE_ALUCARD_GRAPHICS, &session->arc) ||
        !SotnResource_Open(directory, SOTN_RESOURCE_WEAPON_GRAPHICS, &session->weapons) ||
        !SotnResource_Open(directory, SOTN_RESOURCE_GAME_OVER, &session->game_over_art) ||
        !SotnPlayer_Create(
            session->dra, session->room.foreground.collision,
            start_room >= 22                                                              ? 128
            : (start_room == 5 || start_room == 6 || start_room == 9 || start_room == 10) ? 80
                                                                                          : 32,
            135, &session->player) ||
        !SotnSubweapons_Create(session->dra, session->character, session->room.foreground.collision,
                               &session->subweapons) ||
        !SotnPlayer_SetSubweapons(session->player, session->subweapons) ||
        !SotnPlayer_Tick(session->player, 0) ||
        !SotnActors_Create(session->dra, session->nz0, &session->actors) ||
        !SotnActors_SetCamera(session->actors, &session->camera) ||
        !SotnActors_SetWorld(session->actors, &session->world) ||
        !SotnActors_SetCharacter(session->actors, session->character) ||
        !SotnActors_SetSubweapons(session->actors, session->subweapons) ||
        !SotnInventoryRenderer_Create(session->dra, SotnResource_View(session->common),
                                      &session->inventory_renderer) ||
        !SotnActorGraphics_Create(session->dra, session->nz0,
                                  (SotnTileSources){SotnResource_View(session->tiles),
                                                    SotnResource_View(session->common)},
                                  &session->actor_graphics) ||
        !SotnLaboratory_SetEquipment(session, 86, 0) ||
        !SotnSavePointRenderer_Create(session->dra, SotnResource_View(session->common),
                                      &session->save_renderer) ||
        !SotnHud_Create(session->dra, SotnResource_View(session->common),
                        (SotnHudStatus){70, 70, 20, 20, 10, 0}, &session->hud) ||
        !EnterActors(session) || !TickActors(session) || !LaboratoryInput_Init(&session->input) ||
        !SotnPresenter_Create("SOTN Native - Laboratory combat checkpoint (development)", 1024, 960,
                              &session->presenter) ||
        !OpenAudio(session, directory) || !EnterSaveRoom(session))
    {
        char detail[512] = {0};
        SDL_strlcpy(detail, SDL_GetError(), sizeof(detail));
        SotnLaboratory_Destroy(session);
        return SDL_SetError("Cannot initialize Laboratory actor checkpoint: %s", detail);
    }
    session->last_time = SDL_GetTicksNS();
    session->dirty = true;
    SDL_Log("Laboratory development start: room %u", (unsigned int)start_room);
    SDL_Log(
        "Laboratory combat checkpoint: Basilard/Gladius/Mace and empty-hand attacks, Bloody Zombie "
        "damage/death, "
        "physical player damage, candles, and heart/gold/subweapon collection. "
        "Original sound effects, HUD, physical game over, and native save rooms are enabled; "
        "reverb, full "
        "effects, full progression, "
        "scripts, and original camera remain incomplete.");
    SDL_Log("Character checkpoint: inventory/equipment, EXP/levels, selected subweapon attacks, "
            "and version-three ordinary saves.");
    SDL_Log("Development profile: 70 HP, base stats 6, Gladius in left hand, Cube of Zoe enabled.");
    *out = session;
    return true;
}

bool SotnLaboratory_SetEquipment(SotnLaboratory *session, Uint16 left, Uint16 right)
{
    if (session == NULL)
        return SDL_InvalidParamError("laboratory");
    const Uint16 ids[2] = {left, right};
    SotnCharacterState state = {0};
    if (!SotnCharacter_Get(session->character, &state, NULL))
        return false;
    for (Uint32 i = 0; i < 2; ++i)
        if (state.equipment[i] != ids[i])
        {
            if (!SotnCharacter_Collect(session->character, false, ids[i]) ||
                !SotnCharacter_Equip(session->character, i, ids[i]))
                return false;
        }
    SotnEquipment equipment[2] = {0};
    const SotnDataView weapons = SotnResource_View(session->weapons);
    if (!SotnEquipment_Read(session->dra, left, &equipment[0]) ||
        !SotnEquipment_Read(session->dra, right, &equipment[1]) ||
        !SotnPlayer_SetEquipment(session->player, weapons, left, right))
        return false;
    for (Uint32 hand = 0; hand < 2; ++hand)
        if (!SotnActorGraphics_LoadWeapon(session->actor_graphics, weapons, hand, equipment[hand]))
            return false;
    session->hands[0] = left;
    session->hands[1] = right;
    if (!ApplyCharacter(session))
        return false;
    session->dirty = true;
    SDL_Log("Development hand equipment: left %u, right %u (0 empty, 18 Basilard, 86 Gladius, 128 "
            "Mace)",
            left, right);
    return true;
}

bool SotnLaboratory_Step(SotnLaboratory *session, Uint16 buttons)
{
    if (session == NULL)
        return SDL_InvalidParamError("laboratory");
    const bool start =
        (buttons & SOTN_BUTTON_START) && !(session->scene_buttons & SOTN_BUTTON_START);
    session->scene_buttons = buttons;
    if (session->game_over != NULL)
    {
        const SotnActorCombat combat = {0};
        if (!SotnGameOver_Tick(session->game_over, start) || !TickAudio(session, &combat, 0))
            return false;
        session->dirty = true;
        return true;
    }
    SotnPlayerState state = {0};
    if (!SotnPlayer_GetState(session->player, &state))
        return false;
    SotnActorControl control = {0};
    if (!SotnActors_GetControl(session->actors, &control))
        return false;
    const bool owns_save = session->save_room && session->save_point.phase != SOTN_SAVE_POINT_IDLE;
    if (session->menu.phase != SOTN_MENU_CLOSED ||
        (start && state.can_pause && !owns_save && control.timer == 0))
    {
        Uint16 sound = 0;
        bool changed = false;
        if (!SotnInventoryMenu_Tick(&session->menu, session->character, buttons, &sound,
                                    &changed) ||
            (changed && !ApplyCharacter(session)) ||
            (sound != 0 && !SotnSound_Queue(session->sound, sound)))
            return false;
        const SotnActorCombat silent = {0};
        if (!TickAudio(session, &silent, 0))
            return false;
        session->input.menu_open = session->menu.phase != SOTN_MENU_CLOSED;
        session->dirty = true;
        return true;
    }
    session->menu.previous = buttons;
    Uint16 player_buttons =
        session->save_room ? SotnSavePoint_Buttons(&session->save_point, state, buttons) : buttons;
    if (control.timer)
        player_buttons = control.buttons;
    player_buttons &= (Uint16) ~(SOTN_BUTTON_START | SOTN_BUTTON_CANCEL);
    session->simulation_ticks = SDL_min(session->simulation_ticks + 1, 99999ULL * 3600 * 60);
    if (!SotnPlayer_Tick(session->player, player_buttons) ||
        !SotnPlayer_GetState(session->player, &state) ||
        (state.step != SOTN_PLAYER_KILL && !Transition(session, &state)) || !TickActors(session) ||
        !SotnCharacter_TickResources(session->character, session->simulation_ticks) ||
        !TickSaveRoom(session, buttons))
        return false;
    SotnActorCombat combat = {0};
    SotnCharacterState progress = {0};
    if (!SotnCharacter_Get(session->character, &progress, NULL))
        return false;
    session->mp = progress.mp;
    if (!SotnPlayer_GetState(session->player, &state) ||
        !SotnActors_GetCombat(session->actors, &combat) ||
        !SotnHud_Tick(session->hud,
                      (SotnHudStatus){state.hp, state.hp_max, session->mp, progress.mp_max,
                                      combat.hearts, combat.subweapon}) ||
        !TickAudio(session, &combat, state.sound))
        return false;
    if (state.death.phase == 128)
    {
        session->dirty = true;
        if (!SotnLaboratory_Draw(session) ||
            !SotnGameOver_Create(SotnResource_View(session->game_over_art), session->frame,
                                 SOTN_FRAME_PIXELS, &session->game_over))
            return false;
        SotnMusic_Destroy(session->music);
        session->music = NULL;
        if (!SotnMusic_CreateGameOver(session->dra, session->music_stream, &session->music) ||
            !SotnPlayback_Reset(session->audio))
            return false;
        SDL_Log("Alucard died: original game-over presentation. Enter/gamepad Start advances after "
                "the reveal; title/menu flow remains unintegrated.");
    }
    session->dirty = true;
    return true;
}

static bool DrawActorPrimitives(SotnLaboratory *session, SotnActorState actor, Sint32 camera_x,
                                Sint32 camera_y)
{
    for (Uint32 i = 0; i < actor.primitive_count; ++i)
    {
        SotnActorPrimitive p = {0};
        if (!SotnActors_GetPrimitive(session->actors, actor.slot, i, &p))
            return false;
        if (!p.active)
            continue;
        const Sint32 x = Pixel(p.x) - camera_x, y = Pixel(p.y) - camera_y;
        const Sint32 left = p.textured ? x - p.width / 2 : x,
                     top = p.textured ? y - p.height / 2 : y;
        const Sint32 right = left + p.width, bottom = top + p.height;
        if (right <= 0 || left >= SOTN_FRAME_WIDTH || bottom <= 0 || top >= SOTN_FRAME_HEIGHT)
            continue;
        if (p.textured)
        {
            SotnTextureQuad q = {.x = {left, right, left, right},
                                 .y = {top, top, bottom, bottom},
                                 .page = p.page,
                                 .palette = p.palette,
                                 .blend = p.blend};
            for (Uint32 uv = 0; uv < 4; ++uv)
            {
                q.u[uv] = p.u[uv];
                q.v[uv] = p.v[uv];
            }
            if (!SotnActorGraphics_Quad(session->actor_graphics, q, session->frame,
                                        SOTN_FRAME_PIXELS))
                return false;
        }
        else
        {
            const Uint16 front =
                (Uint16)((p.red / 8) | ((p.green / 8) << 5) | ((p.blue / 8) << 10));
            for (Sint32 row = SDL_max(0, top); row < SDL_min(SOTN_FRAME_HEIGHT, bottom); ++row)
                for (Sint32 col = SDL_max(0, left); col < SDL_min(SOTN_FRAME_WIDTH, right); ++col)
                {
                    const size_t pixel = (size_t)row * SOTN_FRAME_WIDTH + (size_t)col;
                    const Uint16 back = session->frame[pixel];
                    session->frame[pixel] =
                        p.blend ? (Uint16)(((back & 0x7bde) >> 1) + ((front & 0x7bde) >> 1))
                                : front;
                }
        }
    }
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
        if (state.generation == 0 || state.unsupported ||
            (state.frame == 0 && state.id != 0xa && state.primitive_count == 0) ||
            (state.id == 0xa &&
             (state.step < 2 || (state.step == 4 && (session->simulation_ticks & 2)))))
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
    bool player_drawn = false, weapon_drawn = false;
    for (Uint32 i = 0; i <= count; ++i)
    {
        if (!weapon_drawn && (i == count || list[i].priority < 0x92))
        {
            if (player.attack.equipment != 0 && player.attack.sprite != 0)
            {
                const bool right = player.attack.right_hand;
                const SotnActorSprite weapon = {
                    .animation_set = right ? 18 : 16,
                    .frame = player.attack.sprite,
                    .palette = (Uint16)(right ? 0x128 : 0x110),
                    .texture_offset = (Uint16)(right ? 0x66 : 0x64),
                    .blend_mode = (Uint16)(player.attack.equipment == 86 ? 0x30 : 0),
                    .facing_left = player.facing_left};
                if (!SotnActorGraphics_Draw(session->actor_graphics, weapon,
                                            Pixel(player.x) - camera_x, Pixel(player.y) - camera_y,
                                            session->frame, SOTN_FRAME_PIXELS))
                    return false;
            }
            weapon_drawn = true;
        }
        if (!player_drawn && (i == count || list[i].priority < 0x94))
        {
            if (!SotnAlucard_DrawDeath(
                    &alucard,
                    (SotnDataView){session->player_common, sizeof(session->player_common)},
                    Pixel(player.x) - camera_x, Pixel(player.y) - camera_y, player.facing_left,
                    player.death, session->frame, SOTN_FRAME_PIXELS))
                return false;
            player_drawn = true;
        }
        if (i == count)
            break;
        const SotnActorState actor = list[i];
        if (actor.id == 0xa)
        {
            const Uint16 id = actor.parameters & 0x7fff;
            SotnItem item = {0};
            if (!SotnItem_Read(session->dra, id >= SOTN_HAND_ITEMS,
                               id >= SOTN_HAND_ITEMS ? (Uint16)(id - SOTN_HAND_ITEMS) : id,
                               &item) ||
                !SotnItemIcon_Draw(session->dra, item, Pixel(actor.x) - camera_x - 8,
                                   Pixel(actor.y) - camera_y - 8, session->frame,
                                   SOTN_FRAME_PIXELS))
                return false;
            continue;
        }
        const SotnActorSprite sprite = {actor.animation_set,  actor.frame,      actor.palette,
                                        actor.texture_offset, actor.blend_mode, actor.facing_left};
        if (!DrawActorPrimitives(session, actor, camera_x, camera_y))
            return false;
        if (!(actor.rotated
                  ? SotnActorGraphics_DrawRotated(session->actor_graphics, sprite,
                                                  Pixel(actor.x) - camera_x,
                                                  Pixel(actor.y) - camera_y, actor.rotation,
                                                  session->frame, SOTN_FRAME_PIXELS)
                  : SotnActorGraphics_Draw(session->actor_graphics, sprite,
                                           Pixel(actor.x) - camera_x, Pixel(actor.y) - camera_y,
                                           session->frame, SOTN_FRAME_PIXELS)))
            return false;
    }
    return true;
}

static bool DrawGameplay(SotnLaboratory *session)
{
    SotnPlayerState state = {0};
    if (!SotnPlayer_GetState(session->player, &state))
        return false;
    Sint32 camera_x = 0, camera_y = 0;
    Camera(session, state, &camera_x, &camera_y);
    const SotnTileSources sources = {SotnResource_View(session->tiles),
                                     SotnResource_View(session->common)};
    SDL_memset(session->frame, 0, sizeof(session->frame));
    return SotnTilemap_Draw(&session->room.background, sources, camera_x, camera_y, session->frame,
                            SOTN_FRAME_PIXELS) &&
           SotnTilemap_Draw(&session->room.foreground, sources, camera_x, camera_y, session->frame,
                            SOTN_FRAME_PIXELS) &&
           (!session->save_room ||
            SotnSavePointRenderer_Draw(session->save_renderer, session->save_point, false,
                                       session->frame, SOTN_FRAME_PIXELS)) &&
           DrawActorsAndPlayer(session, state, camera_x, camera_y) &&
           SotnSubweapons_Draw(session->subweapons, session->actor_graphics, session->dra,
                               session->simulation_ticks, camera_x, camera_y, session->frame,
                               SOTN_FRAME_PIXELS) &&
           (!session->save_room ||
            SotnSavePointRenderer_Draw(session->save_renderer, session->save_point, true,
                                       session->frame, SOTN_FRAME_PIXELS)) &&
           SotnHud_Draw(session->hud, session->frame, SOTN_FRAME_PIXELS) &&
           (!session->save_room ||
            SotnSavePointRenderer_Dialog(session->save_renderer, session->save_point,
                                         session->input.pad != NULL, session->frame,
                                         SOTN_FRAME_PIXELS));
}

bool SotnLaboratory_Draw(SotnLaboratory *session)
{
    if (session == NULL)
        return SDL_InvalidParamError("laboratory");
    if (session->dirty)
    {
        if (session->menu.phase != SOTN_MENU_CLOSED)
        {
            if (!SotnInventoryRenderer_Draw(session->inventory_renderer, session->menu,
                                            session->character, session->simulation_ticks,
                                            session->input.pad != NULL, session->menu_rgba,
                                            sizeof(session->menu_rgba)) ||
                !SotnPresenter_SetImage(session->presenter, session->menu_rgba,
                                        sizeof(session->menu_rgba), SOTN_MENU_WIDTH,
                                        SOTN_MENU_HEIGHT))
                return false;
            session->dirty = false;
            return SotnPresenter_Draw(session->presenter);
        }
        if (!(session->game_over != NULL
                  ? SotnGameOver_Draw(session->game_over, session->frame, SOTN_FRAME_PIXELS)
                  : DrawGameplay(session)) ||
            !SotnFrame_ToRgba(session->frame, SOTN_FRAME_PIXELS, session->rgba,
                              sizeof(session->rgba)) ||
            !SotnPresenter_SetImage(session->presenter, session->rgba, sizeof(session->rgba),
                                    SOTN_FRAME_WIDTH, SOTN_FRAME_HEIGHT))
            return false;
        session->dirty = false;
    }
    return SotnPresenter_Draw(session->presenter);
}

bool SotnLaboratory_Finished(const SotnLaboratory *session)
{
    return session != NULL && SotnGameOver_Finished(session->game_over);
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
    if (session->audio != NULL && !SotnPlayback_Reset(session->audio))
        return false;
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
    {
        session->clock_units = 0;
        if (session->audio != NULL && !SotnPlayback_Reset(session->audio))
            return false;
    }
    return SotnLaboratory_Draw(session);
}

void SotnLaboratory_Destroy(SotnLaboratory *session)
{
    if (session != NULL)
    {
        SotnGameOver_Destroy(session->game_over);
        SotnResource_Close(session->game_over_art);
        SotnInventoryRenderer_Destroy(session->inventory_renderer);
        SotnSaveStore_Destroy(session->saves);
        SotnSavePointRenderer_Destroy(session->save_renderer);
        SotnHud_Destroy(session->hud);
        SotnMusic_Destroy(session->music);
        SotnPlayback_Destroy(session->audio);
        SotnStream_Close(session->music_stream);
        SotnSound_Destroy(session->sound);
        for (Uint32 i = 0; i < 5; ++i)
            SotnResource_Close(session->sound_resources[i]);
        SotnPresenter_Destroy(session->presenter);
        LaboratoryInput_Quit(&session->input);
        SotnActors_Destroy(session->actors);
        SotnSubweapons_Destroy(session->subweapons);
        SotnCharacter_Destroy(session->character);
        SotnActorLayout_Close(session->actor_layout);
        SotnActorGraphics_Destroy(session->actor_graphics);
        SotnPlayer_Close(session->player);
        SotnResource_Close(session->weapons);
        SotnResource_Close(session->arc);
        SotnResource_Close(session->common);
        SotnResource_Close(session->tiles);
        SotnStage_Close(session->stage);
        SDL_free(session);
    }
}

static bool RestoreSession(SotnLaboratory *session, SotnSaveState state, Uint32 slot)
{
    const SotnCharacterState *c = &state.character;
    const SotnActorProgress progress = {c->hearts, c->subweapon,       c->gold,
                                        c->kills,  state.actor_random, state.combat_random};
    if (!SotnCharacter_Restore(session->character, c) || !ApplyCharacter(session) ||
        !SotnPlayer_Restore(session->player, c->hp, state.player_random) ||
        !SotnActors_Restore(session->actors, progress))
        return false;
    SotnHud_Destroy(session->hud);
    session->hud = NULL;
    if (!SotnHud_Create(
            session->dra, SotnResource_View(session->common),
            (SotnHudStatus){c->hp, c->hp_max, c->mp, c->mp_max, c->hearts, c->subweapon},
            &session->hud))
    {
        return false;
    }
    session->mp = c->mp;
    session->simulation_ticks = state.ticks;
    session->save_point.slot = (Uint8)slot;
    return true;
}

bool SotnLaboratory_Continue(const SotnGameData *data, const char *directory, Uint32 slot,
                             SotnLaboratory **out)
{
    if (out == NULL)
        return SDL_InvalidParamError("out");
    *out = NULL;
    SotnSaveStore *store = NULL;
    SotnSaveState state = {0};
    if (!SotnSaveStore_Create(&store) || !SotnSaveStore_Read(store, slot, &state))
    {
        SotnSaveStore_Destroy(store);
        return false;
    }
    SotnLaboratory *session = NULL;
    if (!SotnLaboratory_Create(data, directory, state.room, &session))
    {
        SotnSaveStore_Destroy(store);
        return false;
    }
    SotnSaveStore_Destroy(session->saves);
    session->saves = store;
    if (!RestoreSession(session, state, slot))
    {
        char detail[512] = {0};
        SDL_strlcpy(detail, SDL_GetError(), sizeof(detail));
        SotnLaboratory_Destroy(session);
        return SDL_SetError("Cannot restore native slot %u: %s", slot, detail);
    }
    SDL_Log("Loaded native slot %u in Laboratory room %u", slot, state.room);
    *out = session;
    return true;
}

bool SotnLaboratory_TestProfile(SotnLaboratory *session, const char *name)
{
    if (session == NULL || name == NULL)
        return SDL_SetError("A named fresh-session test profile is required");
    SotnCharacterState state = {0};
    if (!SotnCharacter_Get(session->character, &state, NULL))
        return false;
    if (SDL_strcmp(name, "inventory") == 0)
    {
        const Uint16 hands[] = {18, 128};
        const Uint16 body[] = {2, 27, 49, 77};
        for (Uint32 i = 0; i < SDL_arraysize(hands); ++i)
            if (!SotnCharacter_Collect(session->character, false, hands[i]))
                return false;
        for (Uint32 i = 0; i < SDL_arraysize(body); ++i)
            if (!SotnCharacter_Collect(session->character, true, body[i]))
                return false;
    }
    else if (SDL_strcmp(name, "progression") == 0)
    {
        Uint32 remaining = 0;
        if (!SotnCharacter_NextLevel(session->character, &remaining))
            return false;
        if (remaining > 0)
            state.experience += remaining - 1;
        if (!SotnCharacter_Restore(session->character, &state))
            return false;
    }
    else
    {
        const Uint16 subweapon = SDL_strcmp(name, "dagger") == 0       ? 1
                                 : SDL_strcmp(name, "axe") == 0        ? 2
                                 : SDL_strcmp(name, "holy-water") == 0 ? 3
                                 : SDL_strcmp(name, "stopwatch") == 0  ? 6
                                                                       : 0;
        if (subweapon == 0)
            return SDL_SetError("Unknown Laboratory test profile '%s'", name);
        state.subweapon = subweapon;
        state.hearts = state.hearts_max;
        if (!SotnCharacter_Restore(session->character, &state))
            return false;
    }
    if (!ApplyCharacter(session))
        return false;
    SDL_Log("Explicit development test profile: %s (granted checkpoint resources)", name);
    session->dirty = true;
    return true;
}
