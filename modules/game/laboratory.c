#include "input.h"
#include "startup_internal.h"
#include <sotn/actor_graphics.h>
#include <sotn/actor_order.h>
#include <sotn/actors.h>
#include <sotn/alucard_sprite.h>
#include <sotn/camera_shake.h>
#include <sotn/castle_map.h>
#include <sotn/cd_audio.h>
#include <sotn/combat.h>
#include <sotn/dialogue_renderer.h>
#include <sotn/fairy_runtime.h>
#include <sotn/frame_clip.h>
#include <sotn/game_over.h>
#include <sotn/hellfire_graphics.h>
#include <sotn/hud.h>
#include <sotn/hud_resources.h>
#include <sotn/inventory.h>
#include <sotn/laboratory.h>
#include <sotn/launch_renderer.h>
#include <sotn/main_loop.h>
#include <sotn/music.h>
#include <sotn/playback.h>
#include <sotn/player.h>
#include <sotn/presenter.h>
#include <sotn/relic.h>
#include <sotn/resource.h>
#include <sotn/save.h>
#include <sotn/save_point_renderer.h>
#include <sotn/scene_freeze.h>
#include <sotn/sound.h>
#include <sotn/spirit_capture.h>
#include <sotn/stage_assets.h>
#include <sotn/stage_profile.h>
#include <sotn/stage_route.h>
#include <sotn/subweapon_renderer.h>
#include <sotn/wolf_capture.h>

struct SotnLaboratory
{
    SotnStage *stage;
    SotnStageAssets *stage_assets;
    Uint16 stage_id;
    SotnStageRoute arrival;
    Uint8 teleport_mode;
    Uint16 engine_flash_frames;
    bool engine_flash;
    bool stage_pending, borrowed_music;
    SotnLaunchMenu launcher;
    SotnLaunchSlot launch_slots[15];
    SotnLaunchAction launch_action;
    Uint16 launch_available, blocked_buttons;
    bool launch_active;
    char launch_error[160];
    char *directory;
    SotnResource *map_art;
    SotnMapRenderer *map_renderer;
    bool map_open;
    Uint16 stage_script_mode;
    Uint8 map_opacity;
    Uint32 map_cursor, map_draw_cursor;
    SotnStageRoom room;
    SotnWorldState world;
    SotnCamera camera;
    SotnCameraShake shake;
    SotnDataView dra, nz0, executable;
    SotnResource *tiles, *common, *arc, *weapons, *game_over_art;
    SotnPlayer *player;
    SotnMistEffect mist_effect;
    bool throwing_random_used;
    bool subweapon_spent; /**< A shared-runtime subweapon spent hearts in this player update. */
    SotnGasCloud gas_cloud;
    SotnBatRuntime bat_runtime;
    SotnFairyRuntime fairy_runtime;
    SotnSwordRuntime sword_runtime;
    SotnSwordLease sword_leases[3];
    Uint16 familiar_overlay; /**< Identity whose overlay data the runtimes hold (3/5), or0. */
    SotnResource *fairy_resources[4];
    SotnCdAudio *fairy_cd;
    Uint16 fairy_scene;
    bool fairy_pause_disabled;
    SotnDeathImage death_image;
    Uint32 death_image_generation;
    SotnStatBuffs stat_buffs;
    SotnHudResources hud_resources;
    SotnSceneFreeze spell_freeze;
    bool spell_pause_pending;
    SotnWolfDrawPacket wolf_packets[SOTN_WOLF_FRAME_PACKETS];
    size_t wolf_packet_count;
    Uint16 wolf_previous_capture[63 * 63];
    Uint64 wolf_capture_tick;
    Uint32 player_effect_frame;
    bool wolf_capture_ready;
    Sint32 blink_camera_x, blink_camera_y, blink_rebase_x, blink_rebase_y;
    Uint32 mist_generation;
    Sint32 mist_camera_x, mist_camera_y, mist_rebase_x, mist_rebase_y;
    Uint16 mist_capture_sprite, mist_capture_bank;
    bool mist_present;
    SotnActors *actors;
    SotnSubweapons *subweapons;
    SotnActorLayout *actor_layout;
    SotnActorGraphics *actor_graphics;
    SotnPresenter *presenter;
    SotnHud *hud;
    SotnCharacter *character;
    Uint8 player_common[0x42000];
    Uint16 cape_glow_target;
    Uint8 cape_glow_delay;
    Uint16 cape;
    bool palette_ready;
    SotnInventoryMenu menu;
    SotnInventoryRenderer *inventory_renderer;
    Uint8 menu_rgba[SOTN_MENU_RGBA_BYTES];
    SotnGameOver *game_over;
    Uint8 game_over_audio_phase;
    Uint16 scene_buttons, mp, hands[2];
    Uint64 simulation_ticks;
    SotnPlaytime playtime;
    Uint32 system_timer, game_timer;
    SotnSaveStore *saves;
    bool borrowed_saves;
    SotnSavePoint save_point;
    SotnSavePointRenderer *save_renderer;
    bool save_room, clock_room, room_music_stopped;
    LaboratoryInput input;
    SotnActorDraw draw_items[SOTN_STAGE_ACTOR_SLOTS + SOTN_ACTOR_PRIMITIVES_MAX + 2200];
    Uint16 frame[SOTN_FRAME_PIXELS];
    Uint16 clip_backup[SOTN_FRAME_PIXELS];
    SotnActorTargetCursor draw_target;
    Uint8 rgba[SOTN_FRAME_PIXELS * 4];
    Uint32 room_index;
    Uint64 last_time, clock_units;
    bool dirty, paused, targets_pending;
    bool warned_actors[0x63];
    Uint16 displayed_hp, displayed_hearts, displayed_subweapon;
    Uint32 displayed_gold;
    SotnResource *sound_resources[5], *sequence_resource;
    SotnSound *sound;
    SotnPlayback *audio;
    SotnStream *music_stream;
    SotnMusic *music, *speech;
    Uint8 music_gain, master_gain;
    Uint16 xa_fade_counter, xa_fade_multiplier;
    Uint8 xa_command;
    bool music_fading, music_paused;
    SotnDialogueState dialogue;
    Uint32 portrait_offsets[2];
};

typedef struct SessionStart
{
    Uint16 stage;
    Uint32 room, slot;
    Uint32 system_timer, game_timer;
    Uint16 spirit_target_cursor;
    Uint16 water_breath_timer;
    Uint8 water_pain_timer;
    Uint8 teleport_mode;
    const SotnStatBuffs *stat_buffs;
    const SotnLaboratory *familiars; /**< Actual connected-stage source, borrowed until commit. */
    const SotnSaveState *saved;
    const SotnPlaytime *playtime;
    Uint32 map_cursor, map_draw_cursor;
    const SotnSound *startup_sound;
    SotnSaveStore *startup_store;
    Uint16 startup_buttons;
    const SotnStageRoute *arrival;
    SotnStream *music; /**< Borrowed during candidate construction; transferred only at commit. */
} SessionStart;

static bool CreateStageMusic(SotnLaboratory *s, SotnMusic **out)
{
    if (s->stage_id == 25 || s->stage_id == 26 || s->stage_id == 30)
    {
        // Original BO0/BO4/BO5 stage records select MU_NO_AUDIO until their encounter events.
        *out = NULL;
        return true;
    }
    if (s->stage_id == 0 || s->stage_id == 1 || s->stage_id == 2 || s->stage_id == 4 ||
        s->stage_id == 6)
        return SotnMusic_CreateStage(s->dra, s->music_stream, s->stage_id, out);
    if (s->stage_id == 12)
        return SotnMusic_Create(s->dra, s->music_stream, out);
    return SotnMusic_CreateEntrance(s->dra, s->music_stream,
                                    s->stage_id == 7 || s->world.flags[SOTN_ENTRANCE_ON] ||
                                        s->world.flags[SOTN_ENTRANCE_DEATH],
                                    out);
}

static bool StageMusicCommand(SotnLaboratory *s, Uint16 *command)
{
    SotnMusic *profile = NULL;
    SotnMusicState state = {0};
    if (!CreateStageMusic(s, &profile))
        return false;
    const bool read = !profile || SotnMusic_Get(profile, &state);
    *command = profile ? (Uint16)(state.id + 0x300U) : 0x2ff;
    SotnMusic_Destroy(profile);
    return read;
}

static SotnDataView StageGraphics(const SotnLaboratory *session)
{
    return session->stage_assets != NULL ? SotnStageAssets_Graphics(session->stage_assets)
                                         : SotnResource_View(session->tiles);
}

static bool StageOverlay(SotnLaboratory *session, const SotnGameData *data, const char *directory)
{
    if (session->stage_id == 12)
        return SotnGameData_GetFile(data, SOTN_GAME_NZ0, &session->nz0) &&
               SotnResource_Open(directory, SOTN_RESOURCE_LAB_GRAPHICS, &session->tiles);
    if (!SotnStageAssets_Open(directory, session->stage_id, &session->stage_assets))
        return false;
    session->nz0 = SotnStageAssets_Overlay(session->stage_assets);
    return true;
}

static bool OpenAudio(SotnLaboratory *session, const char *directory, const SotnSound *startup)
{
    static const struct
    {
        Uint16 stage;
        SotnResourceId header, body;
    } profiles[] = {
        {12, SOTN_RESOURCE_LAB_AUDIO_HEADER, SOTN_RESOURCE_LAB_AUDIO_BODY},
        {65, SOTN_RESOURCE_ENTRANCE_AUDIO_HEADER, SOTN_RESOURCE_ENTRANCE_AUDIO_BODY},
        {7, SOTN_RESOURCE_RETURN_AUDIO_HEADER, SOTN_RESOURCE_RETURN_AUDIO_BODY},
        {0, SOTN_RESOURCE_MARBLE_AUDIO_HEADER, SOTN_RESOURCE_MARBLE_AUDIO_BODY},
        {1, SOTN_RESOURCE_WALL_AUDIO_HEADER, SOTN_RESOURCE_WALL_AUDIO_BODY},
        {26, SOTN_RESOURCE_DOPPLE_AUDIO_HEADER, SOTN_RESOURCE_DOPPLE_AUDIO_BODY},
        {2, SOTN_RESOURCE_LIBRARY_AUDIO_HEADER, SOTN_RESOURCE_LIBRARY_AUDIO_BODY},
        {6, SOTN_RESOURCE_CHAPEL_AUDIO_HEADER, SOTN_RESOURCE_CHAPEL_AUDIO_BODY},
        {25, SOTN_RESOURCE_HIPPOGRYPH_AUDIO_HEADER, SOTN_RESOURCE_HIPPOGRYPH_AUDIO_BODY},
        {4, SOTN_RESOURCE_QUARTERS_AUDIO_HEADER, SOTN_RESOURCE_QUARTERS_AUDIO_BODY},
        {30, SOTN_RESOURCE_OLROX_AUDIO_HEADER, SOTN_RESOURCE_OLROX_AUDIO_BODY}};
    SotnResourceId header = SOTN_RESOURCE_COUNT, body = SOTN_RESOURCE_COUNT;
    for (Uint32 i = 0; i < SDL_arraysize(profiles); ++i)
        if (profiles[i].stage == session->stage_id)
        {
            header = profiles[i].header;
            body = profiles[i].body;
            break;
        }
    if (header == SOTN_RESOURCE_COUNT || body == SOTN_RESOURCE_COUNT)
        return SDL_SetError("Stage %u has no integrated sound-bank profile", session->stage_id);
    const SotnResourceId ids[5] = {SOTN_RESOURCE_COMMON_AUDIO, SOTN_RESOURCE_ALUCARD_AUDIO_HEADER,
                                   SOTN_RESOURCE_ALUCARD_AUDIO_BODY, header, body};
    for (Uint32 i = 0; i < 5; ++i)
        if (!SotnResource_Open(directory, ids[i], &session->sound_resources[i]))
            return false;
    const SotnSoundSources sources = {session->dra,
                                      SotnResource_View(session->sound_resources[0]),
                                      SotnResource_View(session->sound_resources[1]),
                                      SotnResource_View(session->sound_resources[2]),
                                      SotnResource_View(session->sound_resources[3]),
                                      SotnResource_View(session->sound_resources[4])};
    if (startup)
    {
        if (!SotnSound_PrepareStageBanks(startup, sources.player_header, sources.player_body,
                                         sources.stage_header, sources.stage_body, &session->sound))
            return false;
    }
    else if (!SotnSound_Create(sources, &session->sound) ||
             !SotnSound_ConfigureReverb(session->sound, session->executable))
        return false;
    if ((session->stage_id == 2 || session->stage_id == 6) &&
        (!SotnResource_Open(directory,
                            session->stage_id == 2 ? SOTN_RESOURCE_LIBRARY_SEQUENCE
                                                   : SOTN_RESOURCE_CHAPEL_SEQUENCE,
                            &session->sequence_resource) ||
         !SotnSound_SetStageSequence(session->sound, session->executable,
                                     SotnResource_View(session->sequence_resource),
                                     session->stage_id == 2 ? 0x202 : 0x204)))
        return false;
    if (session->music_stream == NULL)
    {
        SDL_Log("Verifying private music container for stage %u...", session->stage_id);
        if (!SotnStream_OpenMusic(directory, &session->music_stream))
            return false;
    }
    SotnMusic *empty = NULL;
    Uint16 command = 0;
    if (!SotnCdAudio_Create(session->dra, session->music_stream, &empty, session->system_timer,
                            &session->fairy_cd) ||
        !SotnCdAudio_BindSound(session->fairy_cd, session->sound) ||
        !StageMusicCommand(session, &command) ||
        (!(session->room.foreground.parameters & 0x20U) && command != 0x2ff &&
         !SotnCdAudio_Queue(session->fairy_cd, command)))
        return false;
    return SotnPlayback_Create(&session->audio);
}

static bool BindActorAudio(SotnLaboratory *s)
{
    bool playing = s->music != NULL;
    bool dialogue = SotnMusic_Playing(s->speech);
    bool busy = s->music_fading || s->xa_command != 0;
    if (s->fairy_cd)
    {
        SotnCdAudioState cd = {0};
        if (!SotnCdAudio_Get(s->fairy_cd, &cd))
            return false;
        playing = cd.xa_active;
        dialogue = cd.xa_active;
        busy = busy || !cd.fade_acknowledged || !cd.acknowledged;
    }
    return SotnActors_SetEngineServices(s->actors, s->executable, s->playtime, s->system_timer,
                                        playing) &&
           SotnActors_SetDialogueBusy(s->actors, busy) &&
           SotnActors_Dialogue(s->actors, dialogue, NULL);
}

static bool QueueOwnedStageCd(SotnLaboratory *s, const SotnActorCombat *combat)
{
    if (combat->restart_stage_music)
    {
        Uint16 command = 0;
        if (!StageMusicCommand(s, &command) ||
            !SotnCdAudio_Queue(s->fairy_cd, command == 0x2ff ? 0xa : command))
            return false;
    }
    if (combat->music_change &&
        !SotnCdAudio_Queue(s->fairy_cd, combat->music ? combat->music : 0xa))
        return false;
    return true;
}

static bool QueueNativeSound(SotnLaboratory *s, Uint16 id)
{
    if (s->fairy_cd && (id == 0xe || id == 0xf || (id >= 0xa0 && id <= 0xa8)))
        return SotnCdAudio_Queue(s->fairy_cd, id);
    return SotnSound_Queue(s->sound, id);
}

static bool QueueAudio(SotnLaboratory *session, const SotnActorCombat *combat, Uint16 player_sound,
                       Uint16 player_extra_sound, Uint16 player_third_sound, Uint16 guard_sound)
{
    if (session->sound == NULL)
        return true;
    if (session->fairy_cd && !QueueOwnedStageCd(session, combat))
        return false;
    if (combat->restart_stage_music && !session->fairy_cd)
    {
        SotnMusic *next = NULL;
        if (!CreateStageMusic(session, &next))
            return false;
        SotnMusic_Destroy(session->music);
        session->music = next;
        session->music_gain = 127;
        session->master_gain = 127;
        session->music_paused = session->music_fading = false;
        session->xa_command = 0;
    }
    if (combat->sequence_change)
    {
        if (session->fairy_cd)
        {
            if (!SotnCdAudio_Queue(session->fairy_cd, combat->sequence ? combat->sequence : 7))
                return false;
        }
        else if (combat->sequence == 0)
            SotnSound_StopSequence(session->sound);
        else if (!SotnSound_PlaySequence(session->sound, combat->sequence))
            return false;
    }
    if (combat->music_change && !session->fairy_cd)
    {
        SotnMusic *next = NULL;
        if (combat->music != 0)
        {
            if (combat->music == 0x31d)
            {
                if (!SotnMusic_CreateBoss(session->dra, session->music_stream, &next))
                    return false;
            }
            else if (combat->music == 0x309 && session->stage_id == 2)
            {
                if (!SotnMusic_CreateStage(session->dra, session->music_stream, 2, &next))
                    return false;
            }
            else if (combat->music == 0x313 && session->stage_id == 26)
            {
                if (!SotnMusic_CreateStage(session->dra, session->music_stream, 1, &next))
                    return false;
            }
            else if (combat->music == 0x334 && (session->stage_id == 25 || session->stage_id == 30))
            {
                if (!SotnMusic_CreateStage(session->dra, session->music_stream, session->stage_id,
                                           &next))
                    return false;
            }
            else if (combat->music == 0x317 && session->stage_id == 30)
            {
                if (!SotnMusic_CreateStage(session->dra, session->music_stream, 4, &next))
                    return false;
            }
            else if (combat->music == 0x30f)
            {
                if (!SotnMusic_CreateEntrance(session->dra, session->music_stream, true, &next))
                    return false;
            }
            else if (combat->music == 0x32c)
            {
                if (!SotnMusic_CreateEntranceQuiet(session->dra, session->music_stream, &next))
                    return false;
            }
            else if (combat->music == 0x32e)
            {
                if (!SotnMusic_Create(session->dra, session->music_stream, &next))
                    return false;
            }
            else
                return SDL_SetError("Unsupported event music request 0x%x in stage %u",
                                    combat->music, session->stage_id);
        }
        SotnMusic_Destroy(session->music);
        session->music = next;
        if (next)
        {
            session->music_gain = 127;
            session->music_paused = session->music_fading = false;
            session->xa_command = 0;
        }
    }
    if (combat->cd_command)
    {
        if (session->fairy_cd &&
            ((combat->cd_command >= 0x80 && combat->cd_command <= 0x84) ||
             (combat->cd_command >= 0x90 && combat->cd_command <= 0x94) ||
             combat->cd_command == 0x10 || combat->cd_command == 0x11 || combat->cd_command == 0xa))
        {
            if (!SotnCdAudio_Queue(session->fairy_cd, combat->cd_command))
                return false;
        }
        else if (combat->cd_command == 0x90 || combat->cd_command == 0x91 ||
                 combat->cd_command == 0x92 || combat->cd_command == 0x80)
        {
            session->xa_command = combat->cd_command == 0x80 ? 3 : 1;
            session->xa_fade_counter = 0;
            session->xa_fade_multiplier = combat->cd_command == 0x90   ? 128
                                          : combat->cd_command == 0x91 ? 8
                                          : combat->cd_command == 0x92 ? 4
                                                                       : 32;
        }
        else if (combat->cd_command != 0x10 && combat->cd_command != 0x11)
            return SDL_SetError("Dialogue issued an unsupported XA control command");
        if (!session->fairy_cd && (combat->cd_command == 0x10 || combat->cd_command == 0x11))
            session->music_fading = combat->cd_command == 0x10;
        if (!session->fairy_cd && combat->cd_command == 0x11)
        {
            session->music_paused = false;
            session->music_gain = 127;
        }
    }
    if (combat->voice_stop && session->fairy_cd)
    {
        if (!(combat->music_change && !combat->music) && !SotnCdAudio_Queue(session->fairy_cd, 0xa))
            return false;
    }
    else if (combat->voice_stop)
    {
        SotnMusic_Destroy(session->speech);
        session->speech = NULL;
    }
    if (combat->voice_change && session->fairy_cd)
    {
        if (!SotnCdAudio_Queue(session->fairy_cd, combat->voice))
            return false;
    }
    else if (combat->voice_change)
    {
        SotnMusic *next = NULL;
        if (!SotnMusic_CreateSpeech(session->dra, session->music_stream, combat->voice, &next))
            return false;
        SotnMusic_Destroy(session->speech);
        session->speech = next;
    }
    for (Uint32 i = 0; i < combat->sound_count; ++i)
        if (combat->volumes[i] == SDL_MAX_UINT16
                ? !QueueNativeSound(session, combat->sounds[i])
                : !SotnSound_QueueVolumePan(session->sound, combat->sounds[i], combat->volumes[i],
                                            combat->pans[i]))
            return false;
    if (combat->ambience_change &&
        (session->fairy_cd ? !SotnCdAudio_QueueAmbienceVolume(
                                 session->fairy_cd, combat->ambience_volume, combat->ambience_pan)
                           : !SotnSound_QueueAmbienceVolume(session->sound, combat->ambience_volume,
                                                            combat->ambience_pan)))
        return false;
    if (guard_sound != 0 && !QueueNativeSound(session, guard_sound))
        return false;
    if (player_sound != 0 && !QueueNativeSound(session, player_sound))
        return false;
    if (player_extra_sound != 0 && !QueueNativeSound(session, player_extra_sound))
        return false;
    if (player_third_sound != 0 && !QueueNativeSound(session, player_third_sound))
        return false;
    Uint16 sounds[64] = {0}, sound_count = 0;
    if (session->subweapons != NULL)
    {
        if (!SotnSubweapons_Sounds(session->subweapons, sounds, SDL_arraysize(sounds),
                                   &sound_count))
            return false;
        for (Uint16 i = 0; i < sound_count; ++i)
            if (!QueueNativeSound(session, sounds[i]))
                return false;
    }
    return true;
}

static bool TickAudio(SotnLaboratory *session, const SotnActorCombat *combat, Uint16 player_sound,
                      Uint16 player_extra_sound, Uint16 player_third_sound, Uint16 guard_sound)
{
    if (!QueueAudio(session, combat, player_sound, player_extra_sound, player_third_sound,
                    guard_sound))
        return false;
    if (session->fairy_cd && session->room_music_stopped && !session->save_room &&
        !session->clock_room && !session->game_over)
    {
        SotnCdAudioState cd = {0};
        // Engine_3 repeats StopMusic until actual XA activity clears before
        // requesting the retained stage profile; a cooldown is not readiness.
        if (!SotnCdAudio_Queue(session->fairy_cd, 0xa) || !SotnCdAudio_Get(session->fairy_cd, &cd))
            return false;
        if (!cd.xa_active)
        {
            if (!QueueOwnedStageCd(session, &(SotnActorCombat){.restart_stage_music = true}))
                return false;
            session->room_music_stopped = false;
        }
    }
    if (session->spell_pause_pending)
    {
        if (session->sound && !QueueNativeSound(session, 0x0e))
            return false;
        session->spell_pause_pending = false;
    }
    if (session->sound == NULL)
        return true;
    if (session->xa_command == 1 || session->xa_command == 3)
        ++session->xa_command; // Original CD setup tick, before volume stepping.
    else if (session->xa_command == 2 || session->xa_command == 4)
    {
        ++session->xa_fade_counter;
        const Uint32 decrease =
            127U * session->xa_fade_multiplier * session->xa_fade_counter / 512U;
        const Uint8 gain = (Uint8)(decrease >= 127 ? 0 : 127 - decrease);
        if (session->xa_command == 2)
            session->music_gain = gain;
        else
            session->master_gain = gain;
        if (!gain)
        {
            if (session->xa_command == 4)
                SotnSound_StopSequence(session->sound);
            // Native stream shutdown replaces the original subsequent CD stop command.
            SotnMusic_Destroy(session->music);
            session->music = NULL;
            session->xa_command = 0;
        }
    }
    if (session->music_fading)
    {
        session->music_gain = (Uint8)SDL_max(0, (Sint32)session->music_gain - 12);
        if (session->music_gain == 0)
        {
            session->music_fading = false;
            session->music_paused = true;
        }
    }
    Sint16 pcm[SOTN_SOUND_FRAMES * 2] = {0};
    Sint16 cd_pcm[SOTN_SOUND_FRAMES * 2] = {0};
    SotnSoundFrame sound_frame = {0};
    const bool run_sound = session->game_over == NULL || session->fairy_cd != NULL;
    if (run_sound && !SotnSound_UpdateCommands(session->sound, &sound_frame))
        return false;
    if (session->fairy_cd &&
        !SotnCdAudio_Tick(session->fairy_cd, session->system_timer, cd_pcm, SDL_arraysize(cd_pcm)))
        return false;
    if (session->fairy_cd)
    {
        SotnCdAudioState state = {0};
        if (!SotnCdAudio_Get(session->fairy_cd, &state))
            return false;
        session->master_gain = (Uint8)state.master_volume;
    }
    if (run_sound && !SotnSound_RenderFrame(session->sound, sound_frame, pcm, SDL_arraysize(pcm)))
        return false;
    if (session->music != NULL && !session->music_paused &&
        (!session->save_room || session->game_over != NULL))
    {
        Sint16 music[SOTN_SOUND_FRAMES * 2] = {0};
        if (!SotnMusic_Tick(session->music, music, SDL_arraysize(music)))
            return false;
        for (Uint32 i = 0; i < SDL_arraysize(pcm); ++i)
            pcm[i] = (Sint16)SDL_clamp(
                (Sint32)pcm[i] + (Sint32)music[i] * session->music_gain / 127, -32768, 32767);
    }
    if (session->fairy_cd)
    {
        for (Uint32 i = 0; i < SDL_arraysize(pcm); ++i)
            pcm[i] = (Sint16)SDL_clamp((Sint32)pcm[i] + cd_pcm[i], -32768, 32767);
    }
    if (SotnMusic_Playing(session->speech))
    {
        Sint16 speech[SOTN_SOUND_FRAMES * 2] = {0};
        if (!SotnMusic_Tick(session->speech, speech, SDL_arraysize(speech)))
            return false;
        for (Uint32 i = 0; i < SDL_arraysize(pcm); ++i)
            pcm[i] = (Sint16)SDL_clamp((Sint32)pcm[i] + speech[i], -32768, 32767);
    }
    if (session->master_gain != 127)
        for (Uint32 i = 0; i < SDL_arraysize(pcm); ++i)
            pcm[i] = (Sint16)((Sint32)pcm[i] * session->master_gain / 127);
    return SotnPlayback_Queue(session->audio, pcm, SDL_arraysize(pcm));
}

static bool TickGameOver(SotnLaboratory *s, bool start)
{
    if (!s->fairy_cd)
        return SotnGameOver_Tick(s->game_over, start);
    // Selected original Engine_10/Gameover_Init/2/3/4 audio boundaries. Readiness
    // belongs to the real queue/activity/transport, never the presentation timer.
    switch (s->game_over_audio_phase)
    {
    case 0:
        if (!SotnCdAudio_Queue(s->fairy_cd, 0x81))
            return false;
        s->game_over_audio_phase = 1;
        break;
    case 1:
        if (!SotnCdAudio_Queue(s->fairy_cd, 0x11))
            return false;
        s->game_over_audio_phase = 2;
        break;
    case 2: {
        SotnCdAudioState cd = {0};
        if (!SotnCdAudio_Get(s->fairy_cd, &cd))
            return false;
        if (cd.acknowledged && !cd.xa_active)
        {
            if (!SotnCdAudio_Queue(s->fairy_cd, 0x12) || !SotnCdAudio_Queue(s->fairy_cd, 0xb))
                return false;
            s->game_over_audio_phase = 3;
        }
        break;
    }
    case 3:
        if (!SotnCdAudio_Mute(s->fairy_cd))
            return false;
        s->game_over_audio_phase = 4;
        break;
    case 4:
        // F_GO is already verified/resident in the actual presentation owner.
        if (!SotnCdAudio_Queue(s->fairy_cd, 0x33e))
            return false;
        s->game_over_audio_phase = 5;
        break;
    case 5:
        return SotnGameOver_Tick(s->game_over, start);
    default:
        return SDL_SetError("Bound game-over audio lost its original lifecycle phase");
    }
    return true;
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
        !SotnActorLayout_OpenStage(session->nz0, session->stage_id, session->room.object_layout_id,
                                   &layout))
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
        (session->stage_id == 1 && session->room_index == 13 &&
         !SotnActorGraphics_PrepareOuterRope(session->actor_graphics)) ||
        !SotnActors_Enter(session->actors, layout, session->room.foreground.collision, camera_x,
                          camera_y) ||
        !SotnActors_SetBackground(session->actors, session->room.background.collision,
                                  &session->room.background.collision) ||
        !SotnActors_SetTileArt(session->actors, session->room.foreground.pages,
                               session->room.foreground.graphics,
                               session->room.foreground.palettes))
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

static bool LoadHandProgram(SotnLaboratory *s, SotnDataView weapons, Uint16 hand,
                            SotnEquipment equipment)
{
    Uint8 family = 255;
    bool bound = false;
    if (!SotnActorGraphics_WeaponBinding(s->actor_graphics, hand, &family, &bound))
        return false;
    SotnKarmaShared karma = {0};
    const bool reload_karma = equipment.id && equipment.family == 20 && (!bound || family != 20);
    if (reload_karma)
    {
        SotnDataView overlay = {0};
        if (!SotnWeapon_Overlay(weapons, equipment, &overlay) ||
            !SotnKarmaCoin_LoadProgram(&karma, overlay))
            return false;
    }
    if (!SotnActorGraphics_LoadWeapon(s->actor_graphics, weapons, hand, equipment))
        return false;
    if (reload_karma)
        s->bat_runtime.karma[hand] = karma;
    if (equipment.id && (!bound || family != equipment.family))
        s->bat_runtime.hand_program[hand] = (SotnHandWeaponProgram){0};
    // Original LoadWeaponPrg retains an already loaded family and skips FF. A genuine
    // weapon16 program replacement reloads its zero-initialized BSS, unlike a draw binding.
    if (equipment.id && equipment.family == 15 && (!bound || family != 15))
        s->bat_runtime.star_fire_counter[hand] = 0;
    if (equipment.id && equipment.family == 16 && (!bound || family != 16))
        s->bat_runtime.thrown_fire_counter[hand] = 0;
    return true;
}
static bool ApplyCharacter(SotnLaboratory *session)
{
    SotnCharacterState state = {0};
    SotnCharacterStats stats = {0};
    // The active bits (RELIC_FLAG_ACTIVE) of the Heart/Tooth/Rib/Ring/Eye of Vlad (relics25..29).
    Uint8 vlad = 0;
    for (Uint32 i = 0; i < 5; ++i)
        vlad |= (Uint8)((session->world.relics[25 + i] & 2U) ? 1U << i : 0U);
    if (!SotnCharacter_SetVladRelics(session->character, vlad) ||
        !SotnCharacter_SetRooms(session->character, session->world.rooms) ||
        !SotnCharacter_Get(session->character, &state, &stats))
        return false;
    SotnPlayerState current = {0};
    if (!SotnPlayer_GetState(session->player, &current))
        return false;
    if (state.equipment[SOTN_EQUIP_ARMOR] == 25 && !current.axe_armor.active)
    {
        // Validate/load actual weapon45 before binding the equipment-owned controller.
        // Restore the selected hand bank for the ordinary weapon and shared draw owners.
        SotnEquipment armor = {0}, hand = {0};
        const SotnDataView weapons = SotnResource_View(session->weapons);
        if (!SotnEquipment_ReadAxeArmor(session->dra, &armor) ||
            !SotnEquipment_Read(session->dra, state.equipment[0], &hand) ||
            !SotnActorGraphics_LoadWeapon(session->actor_graphics, weapons, 0, armor) ||
            !SotnActorGraphics_LoadWeapon(session->actor_graphics, weapons, 0, hand))
            return false;
    }
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
                (equipment.id && equipment.family != 255 &&
                 !LoadHandProgram(session, weapons, (Uint16)hand, equipment)))
                return false;
            session->hands[hand] = state.equipment[hand];
        }
    }
    if (!session->palette_ready || session->cape != state.equipment[4])
    {
        Uint16 colors[4] = {0};
        const Uint8 *base = session->palette_ready ? session->player_common
                                                   : SotnResource_View(session->common).bytes;
        for (Uint32 i = 0; i < 4; ++i)
            colors[i] = (Uint16)(base[0x4000c + i * 2] | (Uint16)base[0x4000d + i * 2] << 8);
        // The Crystal cloak52 queues a rotation job (0x410) whose first pass runs in TickCape.
        if (state.equipment[4] != 52 &&
            !SotnAlucard_CapePalette(session->dra, state.equipment[4], colors))
            return false;
        if (!session->palette_ready)
            SDL_memcpy(session->player_common, SotnResource_View(session->common).bytes,
                       sizeof(session->player_common));
        for (Uint32 i = 0; i < 4; ++i)
        {
            session->player_common[0x40000 + (6 + i) * 2] = (Uint8)colors[i];
            session->player_common[0x40000 + (6 + i) * 2 + 1] = (Uint8)(colors[i] >> 8);
        }
        session->cape_glow_target = 0;
        session->cape_glow_delay = 0;
        session->cape = state.equipment[4];
        session->palette_ready = true;
    }
    session->mp = state.mp;
    return SotnPlayer_SetCharacter(session->player, &state, stats) &&
           SotnPlayer_SetRelics(session->player, &session->world);
}

static bool SyncStatBuffs(SotnLaboratory *s)
{
    Uint16 mask = 0;
    return SotnStatBuffs_Mask(&s->stat_buffs, &mask) &&
           SotnCharacter_SetStatBuffMask(s->character, mask);
}
static bool BindStatBuffs(SotnLaboratory *s)
{
    SotnCharacterState progress = {0};
    SotnCharacterStats attributes = {0};
    SotnPlayerState player = {0};
    return SyncStatBuffs(s) && SotnCharacter_Get(s->character, &progress, &attributes) &&
           SotnPlayer_GetState(s->player, &player) &&
           (SotnPlayerStep_IsDead(player.step) ||
            SotnPlayer_SetCharacter(s->player, &progress, attributes)) &&
           SotnPlayer_SetStatBuffContext(s->player, &s->stat_buffs, attributes);
}
static bool TickHudResources(SotnLaboratory *s)
{
    SotnPlayerState player = {0};
    SotnCharacterState progress = {0};
    SotnHudResources next = s->hud_resources;
    SotnHudResourcesEvents events = {0};
    if (!SotnPlayer_GetState(s->player, &player) ||
        !SotnCharacter_Get(s->character, &progress, NULL) ||
        !SotnHudResources_Tick(
            &next,
            (SotnHudResourcesInput){.game_timer = s->game_timer,
                                    .player_status = player.status,
                                    .mp = progress.mp,
                                    .mp_max = progress.mp_max,
                                    .healing_mail = progress.equipment[SOTN_EQUIP_ARMOR] == 17,
                                    .mystic_pendant =
                                        progress.equipment[5] == 76 || progress.equipment[6] == 76,
                                    .mp_hold = s->bat_runtime.shield_mp_hold},
            &events))
        return false;
    if (events.mp != progress.mp)
    {
        progress.mp = events.mp;
        if (!SotnCharacter_Restore(s->character, &progress))
            return false;
    }
    if (events.heal && !SotnPlayer_QueueMailHealing(s->player))
        return false;
    s->hud_resources = next;
    return true;
}
static bool TickHudFrame(SotnLaboratory *s)
{
    SotnPlayerState player = {0};
    SotnCharacterState progress = {0};
    if (!TickHudResources(s) || !SotnPlayer_GetState(s->player, &player) ||
        !SotnCharacter_Get(s->character, &progress, NULL) ||
        !SotnHud_Tick(s->hud,
                      (SotnHudStatus){player.hp, player.hp_max, progress.mp, progress.mp_max,
                                      progress.hearts, progress.subweapon}) ||
        !SotnStatBuffs_Tick(&s->stat_buffs))
        return false;
    s->mp = progress.mp;
    return true;
}
static bool RestoreStatBuffs(SotnLaboratory *s, const SotnStatBuffs *retained)
{
    if (retained && !SotnStatBuffs_Mask(retained, NULL))
        return false;
    s->stat_buffs = retained ? *retained : (SotnStatBuffs){0};
    return SyncStatBuffs(s);
}
static bool TickCape(SotnLaboratory *session)
{
    if (session->cape != 56 && session->cape != 52)
        return true;
    Uint16 colors[4] = {0};
    for (Uint32 i = 0; i < 4; ++i)
        colors[i] = (Uint16)(session->player_common[0x4000c + i * 2] |
                             (Uint16)session->player_common[0x4000d + i * 2] << 8);
    if (!(session->cape == 52
              ? SotnAlucard_CrystalTick(session->dra, colors, &session->cape_glow_target,
                                        &session->cape_glow_delay)
              : SotnAlucard_TwilightTick(session->dra, colors, &session->cape_glow_target,
                                         &session->cape_glow_delay)))
        return false;
    for (Uint32 i = 0; i < 4; ++i)
    {
        session->player_common[0x4000c + i * 2] = (Uint8)colors[i];
        session->player_common[0x4000d + i * 2] = (Uint8)(colors[i] >> 8);
    }
    return true;
}

// dra/5087C.c:func_800F2014. Map drawing/secret-wall pixels remain a separate renderer.
static bool VisitMap(SotnLaboratory *session)
{
    SotnPlayerState player = {0};
    if (!SotnPlayer_GetState(session->player, &player))
        return false;
    const SotnStageLayer layer = session->room.foreground;
    const Sint32 x = Pixel(player.x), y = Pixel(player.y);
    if ((layer.parameters & 0x10) || x < 0 || y < 0 || x >= (Sint32)layer.collision.width * 16 ||
        y >= (Sint32)layer.collision.height * 16)
        return true; // Cross-room overshoot is resolved before revealing another room.
    return SotnWorld_Visit(&session->world, (Sint32)layer.left + x / 256,
                           (Sint32)layer.top + y / 256, false);
}

static bool DecodePlayerSprite(SotnLaboratory *s, SotnPlayerState player, SotnAlucardSprite *out)
{
    if (player.animation_bank == 15)
    {
        // Wolf -> Mist -> Wolf legitimately saves bank15. It uses the retained direct-color
        // image, not a normal/Bat ARC upload; Mist entry creates no Blink23 saved-image child.
        if (player.wolf.capture_bank == 15)
        {
            *out = (SotnAlucardSprite){0};
            return true;
        }
        player.animation_bank = player.wolf.capture_bank;
        player.animation.sprite = player.wolf.capture_sprite;
    }
    if (player.axe_armor.active && player.animation_bank == 16)
    {
        // Weapon45 owns its multipart body directly; original Blink/outline reject Axe status.
        *out = (SotnAlucardSprite){0};
        return true;
    }
    if (player.animation_bank == 13)
    {
        if (!SotnAlucard_DecodeBat(s->dra, SotnResource_View(s->arc), player.animation.sprite, out))
            return false;
        out->pivot_y += player.bat.settle_y;
        return true;
    }
    if (player.animation_bank != 1)
        return SDL_SetError("Player references an unintegrated private artwork bank");
    return SotnAlucard_DecodeSprite(s->dra, SotnResource_View(s->arc),
                                    (Uint16)(player.animation.sprite & 0x7fffU), out);
}
static Sint16 BlinkWord(Sint64 value)
{
    const Uint16 bits = (Uint16)(Uint64)value;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
static bool WolfCollision(void *context, Sint32 x, Sint32 y, SotnCollider *out)
{
    const SotnLaboratory *s = context;
    return SotnCollision_Query(&s->room.foreground.collision, x + s->camera.x, y + s->camera.y,
                               out);
}
static bool CaptureDeathImage(void *context, Uint32 generation)
{
    SotnLaboratory *s = context;
    if (!s || !generation || (s->death_image.captured && s->death_image_generation == generation))
        return SDL_SetError("Player-death capture requires its fresh actual texture owner");
    SotnDeathImage image = {0};
    if (!SotnActorGraphics_StoreDeathImage(s->actor_graphics, &image))
        return false;
    s->death_image = image;
    s->death_image_generation = generation;
    return true;
}
static bool DecayDeathImage(void *context, Uint32 generation, bool *finished)
{
    SotnLaboratory *s = context;
    if (!s || !generation || generation != s->death_image_generation || !finished)
        return SDL_SetError("Player-death upload lost its actual captured image generation");
    SotnDeathImage image = s->death_image;
    if (!SotnDeathImage_Tick(&image) ||
        !SotnActorGraphics_LoadDeathImage(s->actor_graphics, &image))
        return false;
    s->death_image = image;
    *finished = image.remaining == 0;
    return true;
}
static bool TickPlayer(SotnLaboratory *s, Uint16 buttons)
{
    // Player callbacks reserve only actual factory records. Restore those reservations if a
    // later player/animation/collision validation fails before its working copy commits.
    Uint8 factories[sizeof(s->bat_runtime.factories)] = {0};
    const SotnDeathImage death_image = s->death_image;
    const Uint32 death_generation = s->death_image_generation;
    SotnDissolveTextures *textures = NULL;
    SotnPlayerState before = {0};
    SotnCharacterState progress = {0};
    if (!SotnPlayer_GetState(s->player, &before) ||
        !SotnCharacter_Get(s->character, &progress, NULL))
        return false;
    s->throwing_random_used = false;
    s->subweapon_spent = false;
    if (before.death.native || before.water_death.active)
    {
        textures = SDL_malloc(sizeof(*textures));
        if (!textures || !SotnActorGraphics_SaveDissolve(s->actor_graphics, textures))
        {
            SDL_free(textures);
            return false;
        }
    }
    SDL_memcpy(factories, s->bat_runtime.factories, sizeof(factories));
    const SotnBatRuntimeChild *slot16 = &s->bat_runtime.children[0];
    Uint16 servant = 0;
    // CheckSwordBrothersInput gates on g_Servant == FAM_ACTIVE_SWORD.
    if (!SotnPlayer_SetSlot16Item(s->player, slot16->hand_weapon ? slot16->state.hand.item : 0) ||
        !SotnCharacter_GetFamiliars(s->character, NULL, &servant) ||
        !SotnPlayer_SetSwordBrothersContext(s->player, servant == 5))
        return false;
    if (SotnPlayer_Tick(s->player, buttons))
    {
        s->throwing_random_used = s->subweapon_spent = false;
        SDL_free(textures);
        return true;
    }
    SDL_memcpy(s->bat_runtime.factories, factories, sizeof(factories));
    char detail[512] = {0};
    SDL_strlcpy(detail, SDL_GetError(), sizeof(detail));
    bool random_restored = true;
    if (s->throwing_random_used || s->subweapon_spent)
    {
        SotnCharacterState current = {0};
        random_restored = SotnCharacter_Get(s->character, &current, NULL);
        if (random_restored)
        {
            current.random = progress.random;
            current.hearts = progress.hearts;
            random_restored = SotnCharacter_Restore(s->character, &current);
        }
        s->throwing_random_used = s->subweapon_spent = false;
    }
    s->death_image = death_image;
    s->death_image_generation = death_generation;
    const bool restored =
        !textures || SotnActorGraphics_RestoreDissolve(s->actor_graphics, textures);
    SDL_free(textures);
    if (!restored || !random_restored)
        return false;
    SDL_SetError("%s", detail);
    return false;
}
static bool HellfireFactory(void *context, SotnPlayerFactorySource source, Uint32 packed,
                            bool *allocated)
{
    SotnLaboratory *s = context;
    return SotnBatRuntime_RequestHellfire(&s->bat_runtime, source, packed, allocated);
}
static bool SwordFactory(void *context, SotnPlayerFactorySource source, Uint32 packed,
                         Sint16 argument, bool *allocated)
{
    SotnLaboratory *s = context;
    return SotnBatRuntime_RequestSword(&s->bat_runtime, source, packed, argument, allocated);
}
static bool HandFactory(void *context, SotnPlayerFactorySource source, Uint32 packed, Uint16 item,
                        bool *allocated)
{
    SotnLaboratory *s = context;
    return SotnBatRuntime_RequestHand(&s->bat_runtime, source, packed, item, allocated);
}
static bool TeleportFactory(void *context, SotnPlayerFactorySource source, Uint32 packed,
                            bool *allocated)
{
    SotnLaboratory *s = context;
    return SotnBatRuntime_RequestTeleport(&s->bat_runtime, source, packed, allocated);
}
static bool UseLibraryCard(void *context, Uint16 hand, Uint16 *equipped)
{
    SotnLaboratory *s = context;
    return SotnCharacter_UseLibraryCard(s->character, hand, equipped);
}
static bool MedicineFactory(void *context, SotnPlayerFactorySource source, Uint32 packed,
                            Uint16 item, bool *allocated)
{
    SotnLaboratory *s = context;
    return SotnBatRuntime_RequestMedicine(&s->bat_runtime, source, packed, item, allocated);
}
static bool UseMedicine(void *context, Uint16 hand, Uint16 item, Uint16 *equipped)
{
    SotnLaboratory *s = context;
    return SotnCharacter_UseMedicine(s->character, hand, item, equipped);
}
static bool FoodFactory(void *context, SotnPlayerFactorySource source, Uint32 packed, Uint16 item,
                        bool *allocated)
{
    SotnLaboratory *s = context;
    return SotnBatRuntime_RequestFood(&s->bat_runtime, source, packed, item, allocated);
}
static bool UseFood(void *context, Uint16 hand, Uint16 item, Uint16 *equipped)
{
    SotnLaboratory *s = context;
    return SotnCharacter_UseFood(s->character, hand, item, equipped);
}
static bool ThrowingFactory(void *context, SotnPlayerFactorySource source, Uint32 packed,
                            Uint16 item, bool *allocated)
{
    SotnLaboratory *s = context;
    return item == 24
               ? SotnBatRuntime_RequestKarma(&s->bat_runtime, source, packed, item, allocated)
           : item >= 75 && item <= 79
               ? SotnBatRuntime_RequestStar(&s->bat_runtime, source, packed, item, allocated)
           : item == 80 || item == 81 || item == 82 || item == 134 || item == 135 || item == 143 ||
                   item == 119 || item == 133 || item == 163 || item == 164 || item == 165 ||
                   item == 176 || item == 136 || item == 137 || item == 138 || item == 1 ||
                   item == 2 || item == 3 || item == 142
               ? SotnBatRuntime_RequestThrown(&s->bat_runtime, source, packed, item, allocated)
           : item == 25
               ? SotnBatRuntime_RequestMagicMissile(&s->bat_runtime, source, packed, allocated)
               : SotnBatRuntime_RequestThrowing(&s->bat_runtime, source, packed, item, allocated);
}
// func_8010EB5C for subweapons the shared runtime owns (dagger, axe, Rebound Stone, Vibhuti,
// Agunea, Bible, Cross):
// func_800FE3C4's heart check, CheckSubwpnChainLimit over slots32..47, heart spending, then the
// actual factory.
static bool SubweaponUse(void *context, SotnPlayerFactorySource source, bool *handled, bool *used,
                         Uint16 *animation)
{
    SotnLaboratory *s = context;
    SotnCharacterState progress = {0};
    if (!s || !handled || !used || !animation || !SotnCharacter_Get(s->character, &progress, NULL))
        return SDL_SetError("Shared subweapon use requires its session and outputs");
    *handled =
        progress.subweapon == SOTN_SUBWEAPON_DAGGER || progress.subweapon == SOTN_SUBWEAPON_AXE ||
        progress.subweapon == SOTN_SUBWEAPON_VIBHUTI ||
        progress.subweapon == SOTN_SUBWEAPON_REBOUND_STONE ||
        progress.subweapon == SOTN_SUBWEAPON_AGUNEA || progress.subweapon == SOTN_SUBWEAPON_BIBLE ||
        progress.subweapon == SOTN_SUBWEAPON_CROSS;
    *used = false;
    *animation = 0;
    if (!*handled)
        return true;
    SotnSubweaponDefinition definition = {0};
    if (!SotnSubweapon_Read(s->dra, progress.subweapon, &definition))
        return false;
    const Uint16 cost = SotnSubweapon_Cost(progress, definition.hearts);
    if (progress.hearts < cost)
        return true;
    Uint64 other = s->mist_present ? (Uint64)1 << 16 : 0;
    if (!SotnGasCloud_Occupied(&s->gas_cloud, &other))
        return false;
    const Uint64 occupied = SotnBatRuntime_Occupied(&s->bat_runtime) | other;
    if (SotnBatRuntime_SubweaponCount(&s->bat_runtime, progress.subweapon) >= definition.chain ||
        ((occupied >> 32) & 0xffffU) == 0xffffU)
        return true;
    progress.hearts = (Uint16)(progress.hearts - cost);
    if (!SotnCharacter_Restore(s->character, &progress))
        return false;
    s->subweapon_spent = true;
    bool allocated = false;
    if (!SotnBatRuntime_RequestSubweapon(&s->bat_runtime, source, definition.blueprint,
                                         (Sint16)(progress.subweapon << 9), &allocated))
        return false;
    *used = true;
    *animation = definition.animation;
    return true;
}
static bool UseThrowing(void *context, Uint16 hand, Uint16 item, Uint16 *equipped)
{
    SotnLaboratory *s = context;
    return SotnCharacter_UseThrowing(s->character, hand, item, equipped);
}
static bool ThrowingRandom(void *context, Uint16 *value)
{
    SotnLaboratory *s = context;
    SotnCharacterState progress = {0};
    if (!s || !value || !SotnCharacter_Get(s->character, &progress, NULL))
        return false;
    const Uint16 result = SotnCombat_Random(&progress.random);
    if (!SotnCharacter_Restore(s->character, &progress))
        return false;
    s->throwing_random_used = true;
    *value = result;
    return true;
}
static bool BindFood(SotnLaboratory *s)
{
    SotnCharacterState state = {0};
    if (!SotnCharacter_Get(s->character, &state, NULL))
        return false;
    Uint8 count[2] = {0}, throwing_count[2] = {0};
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
    {
        const SotnBatRuntimeChild *c = &s->bat_runtime.children[i];
        if (c->food && !c->state.food.inventory)
        {
            const Uint16 hand = (c->parameter & 0x8000U) != 0;
            count[hand] += c->state.food.item == state.equipment[hand];
        }
        // Factory birth already copies the actual item into ext.weapon.equipId (AE),
        // before the later SetWeaponProperties/attack activation callback.
        const Uint16 hand = (c->parameter & 0x8000U) != 0;
        const Uint16 item =
            c->karma                    ? c->state.karma.item
            : c->star                   ? c->state.star.motion.item
            : c->thrown                 ? c->state.thrown.item
            : c->magic                  ? c->state.magic.motion.item
            : c->neutron                ? 71
            : c->sire && !c->sire_child ? 72
            : c->pentagram              ? c->state.pentagram.item
            // Runesword's root carries143 from birth; its hitboxes only after
            // their first callback copies the raw parent's equip id. Heaven
            // sword's root carries119; its afterimages never take an id. Weapon12's
            // slashes and Chakram carry their own items from birth, as do weapon46's
            // Holbein root and Blue knuckles' arc (Holbein's slashes1/2 carry none).
            : c->weapon_secondary &&
                    (c->state.secondary.family == 14 || c->state.secondary.family == 13 ||
                     c->state.secondary.family == 12 ||
                     ((c->state.secondary.family == 53 || c->state.secondary.family == 46 ||
                       c->state.secondary.family == 47 || c->state.secondary.family == 40 ||
                       c->state.secondary.family == 41 || c->state.secondary.family == 42 ||
                       c->state.secondary.family == 44) &&
                      c->state.secondary.own_item))
                ? c->state.secondary.item
                : 0;
        // CheckChainLimit reads raw AE and params, not the callback's hand family.
        // Both original56 arrows retain params0, including right-hand F1 arrows.
        if (item && item == state.equipment[hand])
            ++throwing_count[hand];
    }
    // CheckChainLimit for each hand's QCF special counts children carrying its equip id, with
    // that hand's params bit (special_count) or the other hand's (special_cross, Shotel's punch).
    Uint8 special_count[2] = {0}, special_cross[2] = {0};
    for (Uint16 hand = 0; hand < 2; ++hand)
    {
        SotnEquipment held = {0};
        if (!state.equipment[hand] || !SotnEquipment_IsPlainWeapon(state.equipment[hand]))
            continue;
        if (!SotnEquipment_Read(s->dra, state.equipment[hand], &held))
            return false;
        for (Uint16 i = 0; held.special_move && i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        {
            const SotnBatRuntimeChild *c = &s->bat_runtime.children[i];
            const Uint16 item = c->hand_weapon        ? c->state.hand.item
                                : c->weapon_secondary ? c->state.secondary.item
                                                      : 0;
            const bool same = ((c->parameter & 0x8000U) != 0) == (hand != 0);
            special_count[hand] += item == held.special_move && same;
            special_cross[hand] += item == held.special_move && !same;
        }
    }
    // CheckChainLimit for the equipped combo spell: children carrying its item with its hand.
    const Uint32 combo = SotnPlayer_Combo(s->player);
    Uint8 combo_count = 0;
    for (Uint16 i = 0; combo && i < SOTN_BAT_RUNTIME_SLOTS; ++i)
    {
        const SotnBatRuntimeChild *c = &s->bat_runtime.children[i];
        combo_count += c->weapon_secondary && c->state.secondary.item == (combo & 0xffffU) &&
                       ((c->parameter & 0x8000U) != 0) == ((combo & 0x80000000U) != 0);
    }
    if (!SotnPlayer_SetComboChain(s->player, combo_count) ||
        !SotnPlayer_SetSpecialChain(s->player, special_count[0], special_count[1]) ||
        !SotnPlayer_SetSpecialCross(s->player, special_cross[0], special_cross[1]))
        return false;
    return SotnPlayer_SetFoodContext(s->player, FoodFactory, UseFood, s, count[0], count[1]) &&
           SotnPlayer_SetThrowingContext(s->player, ThrowingFactory, UseThrowing, ThrowingRandom, s,
                                         throwing_count[0], throwing_count[1]);
}
static bool ApplyMenuCharacter(SotnLaboratory *s)
{
    SotnCharacterState state = {0};
    SotnPlayerState player = {0};
    if (!SotnCharacter_Get(s->character, &state, NULL) || !SotnPlayer_GetState(s->player, &player))
        return false;
    const bool armor_program = player.axe_armor.active != (state.equipment[SOTN_EQUIP_ARMOR] == 25);
    const Uint8 hands = armor_program ? 3
                                      : (s->hands[0] != state.equipment[0]) |
                                            ((s->hands[1] != state.equipment[1]) << 1);
    SotnBatRuntime *next = SDL_malloc(sizeof(*next));
    if (!next)
        return SDL_OutOfMemory();
    *next = s->bat_runtime;
    const bool ok =
        SotnBatRuntime_PruneWeapons(next, hands) && SotnActors_CommitBatPrimitives(s->actors, next);
    if (ok)
        s->bat_runtime = *next;
    SDL_free(next);
    if (!ok || !ApplyCharacter(s))
        return false;
    if (armor_program)
    {
        SDL_zero(s->bat_runtime.hand_program);
        SDL_zero(s->bat_runtime.thrown_fire_counter);
        SDL_zero(s->bat_runtime.star_fire_counter);
        for (Uint16 hand = 0; hand < 2; ++hand)
            if (s->bat_runtime.karma[hand].loaded)
            {
                SotnDataView overlay = {0};
                if (!SotnWeapon_Overlay(SotnResource_View(s->weapons),
                                        (SotnEquipment){.id = 24, .family = 20}, &overlay) ||
                    !SotnKarmaCoin_LoadProgram(&s->bat_runtime.karma[hand], overlay))
                    return false;
            }
    }
    return true;
}
static bool CommitMedicine(SotnLaboratory *s)
{
    const SotnMedicineEffectEvents *e = &s->bat_runtime.medicine;
    if (!e->buff_mask && !e->protection && !e->cure_mask && !e->heal && !e->mp_full && !e->hammer)
        return true;
    SotnStatBuffs buffs = s->stat_buffs;
    SotnCharacterState character = {0};
    if (!SotnStatBuffs_Mask(&buffs, NULL) || !SotnCharacter_Get(s->character, &character, NULL))
        return false;
    for (Uint16 i = 0; i < SOTN_STAT_BUFFS; ++i)
        if ((e->buff_mask & (1U << i)) && !SotnStatBuffs_Give(&buffs, i))
            return false;
    if (e->mp_full)
    {
        character.mp = character.mp_max;
        if (!SotnCharacter_Restore(s->character, &character))
            return false;
    }
    if (!SotnPlayer_CommitMedicine(s->player, e, character.mp_max))
        return false;
    // The actual counters change now; derived attributes bind before the next player update.
    s->stat_buffs = buffs;
    return true;
}
static bool CaptureDissolveImage(void *context, SotnDeathImage *image)
{
    SotnLaboratory *s = context;
    return s && SotnActorGraphics_StoreDeathImage(s->actor_graphics, image);
}
static bool DissolvePresent(void *context, bool *present)
{
    SotnLaboratory *s = context;
    if (!s || !present)
        return SDL_SetError("Stone death lost its actual shared runtime query");
    bool found = false;
    // Original PlayerStepKill searches actual entity slots16..63, including step zero.
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
    {
        const SotnBatRuntimeChild *child = &s->bat_runtime.children[i];
        if (!SotnBatRuntime_DissolveValid(child))
            return SDL_SetError("Stone death found an invalid actual dissolve generation");
        found |= child->kind == 32;
    }
    *present = found;
    return true;
}
static bool UploadDissolveImage(void *context, const SotnDeathImage *image, bool secondary)
{
    SotnLaboratory *s = context;
    return s && SotnActorGraphics_LoadDissolve(s->actor_graphics, image, secondary);
}
static bool HoldDissolveUpload(const SotnBatRuntime *r, SotnPlayerState p)
{
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if (r->children[i].kind == 32)
        {
            const Uint16 phase = r->children[i].state.dissolve.phase;
            if (phase == 1 ||
                ((phase == 2 || phase == 3) && p.animation.id >= 0x38 && p.animation.id <= 0x3a))
                return true;
        }
    return false;
}
static bool TickBlinksWork(SotnLaboratory *s, SotnPlayerState player, bool entering)
{
    const Sint64 dx = (Sint64)s->blink_rebase_x + s->blink_camera_x - s->camera.x,
                 dy = (Sint64)s->blink_rebase_y + s->blink_camera_y - s->camera.y;
    if (dx < SDL_MIN_SINT16 || dx > SDL_MAX_SINT16 || dy < SDL_MIN_SINT16 || dy > SDL_MAX_SINT16 ||
        s->blink_rebase_x < SDL_MIN_SINT16 || s->blink_rebase_x > SDL_MAX_SINT16 ||
        s->blink_rebase_y < SDL_MIN_SINT16 || s->blink_rebase_y > SDL_MAX_SINT16)
        return SDL_SetError("Bat room/camera translation exceeds its original word");
    SotnAlucardSprite sprite = {0};
    SotnAlucardSprite talisman = {0};
    bool need_talisman = player.dodge.applied;
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        need_talisman = need_talisman || s->bat_runtime.children[i].kind == 46;
    if (need_talisman &&
        !SotnAlucard_DecodeSprite(s->dra, SotnResource_View(s->arc), 0x9f, &talisman))
        return false;
    Uint64 other = 0;
    SotnPrimitiveSlots primitive_slots = {0};
    SotnSpiritTargetSnapshot targets = {0};
    SotnFoodTextPlan food_text = {0};
    Uint8 weapon_family[2] = {0};
    bool weapon_bound[2] = {false};
    if (!SotnActors_SpiritTargets(s->actors, &targets) || !DecodePlayerSprite(s, player, &sprite) ||
        !SotnGasCloud_Occupied(&s->gas_cloud, &other) ||
        !SotnActors_PrimitiveSlots(s->actors, &primitive_slots) ||
        !SotnActors_GetFoodTextPlan(s->actors, &food_text) ||
        !SotnActorGraphics_WeaponBinding(s->actor_graphics, 0, &weapon_family[0],
                                         &weapon_bound[0]) ||
        !SotnActorGraphics_WeaponBinding(s->actor_graphics, 1, &weapon_family[1],
                                         &weapon_bound[1]) ||
        (sprite.width && !player.water_death.captured && !player.death.captured &&
         !(player.animation.sprite & 0x8000U) && !HoldDissolveUpload(&s->bat_runtime, player) &&
         !SotnActorGraphics_UploadPlayer(s->actor_graphics, &sprite)) ||
        !SotnBatRuntime_Tick(
            &s->bat_runtime, s->character,
            (SotnBatRuntimeInput){
                .dra = s->dra,
                .food_text = &food_text,
                .food_collision = s->room.foreground.collision,
                .executable = s->executable,
                .weapons = SotnResource_View(s->weapons),
                .weapon_family = {weapon_family[0], weapon_family[1]},
                .weapon_bound = {weapon_bound[0], weapon_bound[1]},
                .player = player,
                .image = {sprite.width, sprite.height, (Sint16)sprite.pivot_x,
                          (Sint16)sprite.pivot_y},
                .talisman_image = {talisman.width, talisman.height, (Sint16)talisman.pivot_x,
                                   (Sint16)talisman.pivot_y},
                .other_occupied = other | (s->mist_present ? (Uint64)1 << 16 : 0),
                .wolf_owned = true,
                .wolf_query = WolfCollision,
                .wolf_context = s,
                .primitive_slots = primitive_slots,
                .freeze = s->spell_freeze.request,
                .game_timer = s->game_timer,
                .engine_timer = s->system_timer,
                .dissolve_image = &s->death_image,
                .dissolve_capture = CaptureDissolveImage,
                .dissolve_upload = UploadDissolveImage,
                .dissolve_context = s,
                .hellfire_up = (player.buttons & SOTN_BUTTON_UP) != 0,
                .spirit_targets = &targets,
                .force_echo_enabled = (s->world.relics[SOTN_RELIC_FORCE_ECHO] & 3U) == 3,
                .holy_symbol_enabled = (s->world.relics[14] & 3U) == 3,
                .camera_x = s->camera.x,
                .camera_y = s->camera.y,
                .camera_left = s->camera.left,
                .camera_right = s->camera.right,
                .room_shift_x = (Sint16)s->blink_rebase_x,
                .room_shift_y = (Sint16)s->blink_rebase_y,
                .screen_shift_x = (Sint16)dx,
                .screen_shift_y = (Sint16)dy,
                .entering = entering}) ||
        !SotnActors_CommitBatPrimitives(s->actors, &s->bat_runtime))
        return false;
    // Weapon51's glow and weapon26's green cycle upload their g_Clut words once the pass has
    // committed (weapon51's mask0 replaces the whole word).
    // Dynamite's LoadImage patches its hand's weapon texture once the pass has committed.
    for (Uint32 hand = 0; hand < 2; ++hand)
        if ((s->bat_runtime.weapon_image >> hand) & 1U &&
            !SotnActorGraphics_LoadDynamiteImage(s->actor_graphics, hand))
            return false;
    for (Uint16 i = 0; i < s->bat_runtime.clut_count; ++i)
        if (!SotnActorGraphics_ModifyWeaponClut(
                s->actor_graphics, s->bat_runtime.clut[i].palette, s->bat_runtime.clut[i].color,
                s->bat_runtime.clut[i].mask, s->bat_runtime.clut[i].value))
            return false;
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if (s->bat_runtime.children[i].kind == 46 && s->bat_runtime.children[i].initialized)
        {
            if (!SotnActorGraphics_UploadTalisman(s->actor_graphics, &talisman))
                return false;
            break;
        }
    // Marks are one-update writes; later row commits must not replay stale target identities.
    s->bat_runtime.food_text.changed = false;
    s->bat_runtime.target_marks[0] = s->bat_runtime.target_marks[1] = 0;
    if (!entering)
    {
        if (!SotnPlayer_CommitTeleport(s->player, s->bat_runtime.teleport_palette,
                                       s->bat_runtime.teleport_acknowledged) ||
            !SotnPlayer_CommitHellfire(s->player, s->bat_runtime.hellfire_signal,
                                       s->bat_runtime.hellfire_palette) ||
            (s->bat_runtime.hellfire_upload &&
             !SotnHellfireGraphics_Upload(s->actor_graphics, s->dra)) ||
            (s->bat_runtime.cross_upload &&
             !SotnHellfireGraphics_UploadCross(s->actor_graphics, s->dra)))
            return false;
        s->bat_runtime.hellfire_upload = s->bat_runtime.cross_upload = false;
        if (s->bat_runtime.freeze_changed &&
            !SotnSceneFreeze_Request(&s->spell_freeze, s->bat_runtime.freeze))
            return false;
        for (Uint16 i = 0; i < s->bat_runtime.capture_count; ++i)
        {
            const SotnSpiritCapture capture = s->bat_runtime.captures[i];
            if (capture.slot < 16 || capture.slot >= 64 || !capture.generation ||
                s->bat_runtime.children[capture.slot - 16U].kind != 63 ||
                s->bat_runtime.children[capture.slot - 16U].generation != capture.generation ||
                s->bat_runtime.children[capture.slot - 16U].primitive_count != 9)
                return SDL_SetError("Spirit capture lost its actual nine-row portal generation");
            if (!SotnSpiritCapture_Frame(s->actor_graphics, s->frame, SOTN_FRAME_PIXELS, capture.x,
                                         capture.y))
                return false;
        }
        s->bat_runtime.capture_count = 0;
        if (s->bat_runtime.afterimage_reset && !SotnPlayer_ResetAfterimages(s->player, 1))
            return false;
        if (player.step == SOTN_PLAYER_WOLF && s->bat_runtime.wolf.generation &&
            !SotnPlayer_CommitWolfBody(s->player, &s->bat_runtime.wolf, &s->bat_runtime.wolf_frame,
                                       s->camera.x, s->camera.y))
            return false;
        if (s->bat_runtime.use_ticks &&
            !SotnPlayer_SetUseTimer(s->player, s->bat_runtime.use_ticks))
            return false;
        if (s->bat_runtime.startup_ticks &&
            !SotnPlayer_SetWeaponStartup(s->player, s->bat_runtime.startup_ticks))
            return false;
        if (s->bat_runtime.spell_use_ticks &&
            !SotnPlayer_SetSpellUseTimer(s->player, s->bat_runtime.spell_use_ticks))
            return false;
        if (s->bat_runtime.shield_heal &&
            !SotnPlayer_CommitShieldHit(s->player, s->bat_runtime.shield_protection))
            return false;
        // Dark shield's spell: g_Player.unk20 = 8 (transform lockout) every update.
        if (s->bat_runtime.transform_lockout &&
            !SotnPlayer_SetTransformLockout(s->player, s->bat_runtime.transform_lockout))
            return false;
        // Iron shield's guard: func_8010E168(0,0x60) alone (no healing).
        if (!s->bat_runtime.shield_heal && s->bat_runtime.shield_protection &&
            !SotnPlayer_CommitProtection(s->player, s->bat_runtime.shield_protection))
            return false;
        if (s->bat_runtime.dark_minimum &&
            !SotnPlayer_RaiseDarkTimer(s->player, s->bat_runtime.dark_minimum))
            return false;
        for (Uint16 i = 0; i < s->bat_runtime.sound_count; ++i)
            if (!(s->bat_runtime.sound_volume_mask & ((Uint64)1 << i)
                      ? SotnSound_QueueVolumePan(s->sound, s->bat_runtime.sounds[i],
                                                 s->bat_runtime.sound_volumes[i], 0)
                      : SotnSound_Queue(s->sound, s->bat_runtime.sounds[i])))
                return false;
        if (player.gravity.ceiling_impact && !SotnCameraShake_Request(&s->shake, s->dra, 3))
            return false;
        if (player.hurt.shake_requested &&
            !SotnCameraShake_Request(&s->shake, s->dra, player.hurt.shake))
            return false;
        for (Uint16 i = 0; i < s->bat_runtime.wolf_shake_count; ++i)
            if (!SotnCameraShake_Request(&s->shake, s->dra, s->bat_runtime.wolf_shakes[i]))
                return false;
        if (s->bat_runtime.dissolve_frame_load && !SotnPlayer_CommitDissolveFrame(s->player, true))
            return false;
        if (!CommitMedicine(s) || !SotnPlayer_CommitFood(s->player, s->bat_runtime.food_events,
                                                         s->bat_runtime.food_event_count))
            return false;
        // The original conditional DestroyEntityWeapon(true) belongs to the live shield.
        // Commit only after every fallible shared effect/resource operation succeeded.
        if (s->bat_runtime.magic_clear_shield && !SotnPlayer_ClearShield(s->player))
            return false;
        s->bat_runtime.magic_clear_shield = false;
        if (player.hand_owned)
        {
            const SotnBatRuntimeChild *hand = &s->bat_runtime.children[0];
            const Uint16 phase =
                s->bat_runtime.hand_shield && hand->hand_weapon ? hand->state.hand.motion.phase : 0;
            if (!SotnPlayer_CommitHandShield(s->player, phase, hand->kind >= 0xf0))
                return false;
        }
    }
    s->blink_rebase_x = s->blink_rebase_y = 0;
    s->blink_camera_x = s->camera.x;
    s->blink_camera_y = s->camera.y;
    return true;
}

static bool TickBlinks(SotnLaboratory *s, SotnPlayerState player, bool entering)
{
    bool transaction = player.medicine.used || player.food.used || player.throwing.used;
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        transaction |= s->bat_runtime.children[i].kind == 32 ||
                       s->bat_runtime.children[i].kind == 38 ||
                       s->bat_runtime.children[i].kind == 39 || s->bat_runtime.children[i].food ||
                       s->bat_runtime.children[i].neutron || s->bat_runtime.children[i].sire ||
                       s->bat_runtime.children[i].pentagram || s->bat_runtime.children[i].magic ||
                       s->bat_runtime.children[i].thrown || s->bat_runtime.children[i].star ||
                       s->bat_runtime.children[i].karma || s->bat_runtime.children[i].hand_weapon ||
                       s->bat_runtime.children[i].thrown_particle_parent ||
                       (s->bat_runtime.children[i].kind == 5 &&
                        (s->bat_runtime.children[i].parameter >> 8) == 12);
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_FACTORIES; ++i)
        transaction |= s->bat_runtime.factories[i].cursor.entity_id &&
                       (s->bat_runtime.factories[i].cursor.blueprint == 39 ||
                        s->bat_runtime.factories[i].cursor.blueprint == 53 ||
                        s->bat_runtime.factories[i].cursor.blueprint == 55 ||
                        s->bat_runtime.factories[i].cursor.blueprint == 50 ||
                        s->bat_runtime.factories[i].cursor.blueprint == 48 ||
                        s->bat_runtime.factories[i].cursor.blueprint == 42 ||
                        s->bat_runtime.factories[i].cursor.blueprint == 43 ||
                        s->bat_runtime.factories[i].cursor.blueprint == 56 ||
                        s->bat_runtime.factories[i].cursor.blueprint == 62 ||
                        s->bat_runtime.factories[i].cursor.blueprint == 63 ||
                        s->bat_runtime.factories[i].cursor.blueprint == 68 ||
                        s->bat_runtime.factories[i].star_factory ||
                        s->bat_runtime.factories[i].karma_factory ||
                        s->bat_runtime.factories[i].thrown_factory ||
                        s->bat_runtime.factories[i].thrown_particles);
    for (Uint16 i = 0; i < player.hurt.count; ++i)
        transaction |= (player.hurt.factories[i].blueprint & 0xfffU) == 39;
    if (!transaction)
        return TickBlinksWork(s, player, entering);
    SotnDissolveTextures *textures = SDL_malloc(sizeof(*textures));
    SotnBatRuntime *runtime = SDL_malloc(sizeof(*runtime));
    SotnCharacterState character = {0};
    SotnFoodTextPlan food_text = {0};
    const SotnDeathImage image = s->death_image;
    const Uint32 generation = s->death_image_generation;
    if (!textures || !runtime || !SotnActorGraphics_SaveDissolve(s->actor_graphics, textures) ||
        !SotnCharacter_Get(s->character, &character, NULL) ||
        !SotnActors_GetFoodTextPlan(s->actors, &food_text))
    {
        SDL_free(textures);
        SDL_free(runtime);
        return false;
    }
    *runtime = s->bat_runtime;
    const bool ok = TickBlinksWork(s, player, entering);
    if (!ok)
    {
        char detail[512] = {0};
        SDL_strlcpy(detail, SDL_GetError(), sizeof(detail));
        s->bat_runtime = *runtime;
        s->death_image = image;
        s->death_image_generation = generation;
        // Restore the departing UI lease with the departing player rows in one commit.
        SotnFoodTextPlan current = {0};
        const bool food_snapshot = SotnActors_GetFoodTextPlan(s->actors, &current);
        runtime->food_text = food_text;
        runtime->food_text.previous = current.lease;
        runtime->food_text.previous_serial = current.serial;
        runtime->food_text.changed = true;
        const bool restored = food_snapshot &&
                              SotnActorGraphics_RestoreDissolve(s->actor_graphics, textures) &&
                              SotnCharacter_Restore(s->character, &character) &&
                              SotnActors_CommitBatPrimitives(s->actors, runtime) &&
                              SotnPlayer_RollbackDissolve(s->player, &player);
        s->bat_runtime.food_text.changed = false;
        SDL_free(textures);
        SDL_free(runtime);
        if (!restored)
            return false;
        return SDL_SetError("%s", detail);
    }
    SDL_free(textures);
    SDL_free(runtime);
    return true;
}

static bool SyncDarkPalette(SotnLaboratory *s)
{
    SotnPlayerState p = {0};
    SotnPlayerGlow glow = {0};
    if (!SotnPlayer_GetState(s->player, &p) ||
        !SotnActorGraphics_SetAbsorption(s->actor_graphics, p.dark.ticks != 0) ||
        !SotnActorGraphics_GetPlayerGlow(s->actor_graphics, &glow))
        return false;
    s->player_common[0x40014] = (Uint8)glow.color;
    s->player_common[0x40015] = (Uint8)(glow.color >> 8);
    return true;
}

static bool TickMist(SotnLaboratory *s, SotnPlayerState player, bool entering)
{
    const Sint64 dx = (Sint64)s->mist_rebase_x + s->mist_camera_x - s->camera.x,
                 dy = (Sint64)s->mist_rebase_y + s->mist_camera_y - s->camera.y;
    if (dx < SDL_MIN_SINT16 || dx > SDL_MAX_SINT16 || dy < SDL_MIN_SINT16 || dy > SDL_MAX_SINT16 ||
        !SotnMistEffect_Translate(&s->mist_effect, (Sint32)dx, (Sint32)dy, s->mist_present))
        return SDL_SetError("Mist camera/room translation exceeds its original coordinate word");
    s->mist_rebase_x = s->mist_rebase_y = 0;
    s->mist_camera_x = s->camera.x;
    s->mist_camera_y = s->camera.y;
    if (!SyncDarkPalette(s))
        return false;
    if (entering)
        return SotnGasCloud_Tick(&s->gas_cloud, player, s->mist_present ? &s->mist_effect : NULL,
                                 s->mist_generation, s->camera.x, s->camera.y,
                                 (s->world.relics[SOTN_RELIC_GAS_CLOUD] & 2U) != 0);
    Uint16 colors[16] = {0};
    for (Uint16 i = 0; i < 16; ++i)
        colors[i] = (Uint16)(s->player_common[0x40000 + i * 2U] |
                             (Uint16)s->player_common[0x40001 + i * 2U] << 8);
    if (!SotnActorGraphics_SetPlayerPalette(s->actor_graphics, colors))
        return false;
    if (player.mist.retire && s->mist_present)
    {
        if (s->mist_generation != player.mist.generation || s->mist_effect.phase != 5)
            return SDL_SetError("Mist retirement lost its ready generation-owned visual child");
        s->mist_present = false;
        s->mist_generation = 0;
    }
    if (player.mist.interrupted && s->mist_present)
    {
        if (s->mist_generation != player.mist.generation || player.mist.active)
            return SDL_SetError("Script reset lost its actual live Mist owner");
        s->mist_present = false;
        s->mist_generation = 0;
    }
    if (player.mist.started && (!s->mist_present || s->mist_generation != player.mist.generation))
    {
        SotnMistEffect fresh = {.x = (Sint32)((Sint64)player.x - (Sint64)s->camera.x * 65536),
                                .y = (Sint32)((Sint64)player.y - (Sint64)s->camera.y * 65536)};
        // These globals outlive the original entity: initialization writes only point highs.
        SDL_memcpy(fresh.points, s->mist_effect.points, sizeof(fresh.points));
        SDL_memcpy(fresh.color_phase, s->mist_effect.color_phase, sizeof(fresh.color_phase));
        SDL_memcpy(fresh.color_base, s->mist_effect.color_base, sizeof(fresh.color_base));
        s->mist_effect = fresh;
        s->mist_generation = player.mist.generation;
        s->mist_present = true;
        s->mist_capture_sprite = player.mist.capture_sprite;
        s->mist_capture_bank = player.mist.capture_bank;
        if ((s->world.relics[SOTN_RELIC_GAS_CLOUD] & 2U) &&
            !SotnGasCloud_Begin(&s->gas_cloud, s->dra, s->character, player.mist.generation,
                                (Uint16) ~(SotnBatRuntime_Occupied(&s->bat_runtime) >> 32)))
            return false;
        bool accepted = false;
        if (!SotnActorGraphics_RequestMistPalette(s->actor_graphics, 27, &accepted) ||
            !SotnActorGraphics_CancelMistPalette(s->actor_graphics) ||
            !SotnActorGraphics_RequestMistPalette(
                s->actor_graphics, (s->world.relics[SOTN_RELIC_GAS_CLOUD] & 2U) ? 0x11f : 0x11c,
                &accepted))
            return false;
    }
    if (s->mist_present)
    {
        if (!player.mist.active || player.mist.generation != s->mist_generation)
            return SDL_SetError("Mist visual child lost its active player generation");
        if (player.mist.request_restore && s->mist_effect.phase < 3)
        {
            s->mist_effect.phase = 3;
            s->mist_capture_sprite = player.animation.sprite;
            s->mist_capture_bank = player.animation_bank;
        }
        SotnMistWedge capture[8] = {0};
        const Sint32 x = Pixel(player.x) - s->camera.x, y = Pixel(player.y) - s->camera.y;
        if (x < SDL_MIN_SINT16 || x > SDL_MAX_SINT16 || y < SDL_MIN_SINT16 || y > SDL_MAX_SINT16)
            return SDL_SetError("Mist player anchor exceeds its original screen coordinate word");
        SotnMistEffectInput input = {.dra = s->dra,
                                     .executable = s->executable,
                                     .x = (Sint16)x,
                                     .y = (Sint16)y,
                                     .vx = player.velocity_x,
                                     .vy = player.velocity_y,
                                     .game_timer = s->game_timer,
                                     .priority = player.priority,
                                     .alternate_sheet = player.animation_bank == 15,
                                     .facing_left = player.facing_left};
        if (s->mist_effect.phase == 0 || s->mist_effect.phase == 3)
        {
            if (s->mist_capture_bank == 15)
            {
                if (!SotnWolfCapture_Snapshot(s->dra, s->bat_runtime.wolf.body.capture, x, y,
                                              player.facing_left, capture))
                    return false;
            }
            else
            {
                SotnAlucardSprite sprite = {0};
                SotnPlayerState captured = player;
                captured.animation.sprite = s->mist_capture_sprite;
                captured.animation_bank = s->mist_capture_bank;
                if (!DecodePlayerSprite(s, captured, &sprite) ||
                    !SotnActorGraphics_CapturePlayer(s->actor_graphics, &sprite, x, y,
                                                     player.facing_left, capture))
                    return false;
            }
            input.capture = capture;
        }
        SotnCharacterState progress = {0};
        const bool initialize = s->mist_effect.phase == 0;
        if (initialize)
        {
            if (!SotnCharacter_Get(s->character, &progress, NULL))
                return false;
            // EntityMist uses libc rand(), separate from the stage's Random() cursor.
            for (Uint16 i = 0; i < 4; ++i)
                input.random[i] = SotnCombat_Random(&progress.random);
        }
        SotnMistEffectEvents events = {0};
        if (!SotnMistEffect_Tick(&s->mist_effect, input, &events) ||
            (initialize &&
             (events.random_count != 4 || !SotnCharacter_Restore(s->character, &progress))))
            return false;
        bool accepted = false;
        if ((events.cancel_palette && !SotnActorGraphics_CancelMistPalette(s->actor_graphics)) ||
            (events.restore_palette &&
             !SotnActorGraphics_RequestMistPalette(s->actor_graphics, 0x11d, &accepted)))
            return false;
    }
    return SotnGasCloud_Tick(&s->gas_cloud, player, s->mist_present ? &s->mist_effect : NULL,
                             s->mist_generation, s->camera.x, s->camera.y,
                             (s->world.relics[SOTN_RELIC_GAS_CLOUD] & 2U) != 0) &&
           SotnActorGraphics_TickMistPalette(s->actor_graphics);
}

static Sint32 FairyFixed(Uint32 value)
{
    return value <= SDL_MAX_SINT32 ? (Sint32)value : (Sint32)((Sint64)value - 4294967296LL);
}
static SotnFairyPlayer FairyPlayer(SotnPlayerState p, Sint32 camera_x, Sint32 camera_y)
{
    return (SotnFairyPlayer){.x = FairyFixed((Uint32)p.x - (Uint32)camera_x * 65536U),
                             .y = FairyFixed((Uint32)p.y - (Uint32)camera_y * 65536U),
                             .velocity_x = p.velocity_x,
                             .velocity_y = p.velocity_y,
                             .status = p.status,
                             .damage_effects = p.received_damage_effects,
                             .step = p.step,
                             .substep = p.substep,
                             .previous_step = p.control_step,
                             .previous_substep = p.control_substep,
                             .collision = (Uint16)p.contacts,
                             .priority = p.priority,
                             .facing_left = p.facing_left};
}
// Original card relics, g_Status.statsFamiliars indices, servant files and g_Events tables.
typedef struct FamiliarProfile
{
    Uint16 identity, card, record;
    SotnResourceId code;
    Uint32 events;
} FamiliarProfile;
static const FamiliarProfile FAMILIAR_FAIRY = {3, 20, 2, SOTN_RESOURCE_FAIRY_CODE, 0x80172d2cU};
static const FamiliarProfile FAMILIAR_SWORD = {5, 22, 4, SOTN_RESOURCE_SWORD_CODE, 0x801719e4U};
static const FamiliarProfile *Familiar(Uint16 identity)
{
    return identity == 3 ? &FAMILIAR_FAIRY : identity == 5 ? &FAMILIAR_SWORD : NULL;
}
static bool FamiliarInitialized(const SotnLaboratory *s, Uint16 identity)
{
    return identity == 3   ? s->fairy_runtime.initialized
           : identity == 5 ? s->sword_runtime.initialized
                           : false;
}
// CheckAllEntitiesValid over stage slots: the Sword's target search finds one exactly then.
static bool FamiliarEnemies(SotnLaboratory *s, Sint32 camera_x, Sint32 camera_y, bool *out)
{
    bool enemies = false;
    for (Uint16 i = 0; i < 128; ++i)
    {
        SotnActorState actor = {0};
        if (!SotnActors_Get(s->actors, i, &actor))
            return false;
        const Sint32 x = Pixel(actor.x) - camera_x, y = Pixel(actor.y) - camera_y;
        if (actor.id && actor.hitbox_state && !(actor.flags & 0x200000U) && x >= -16 && x <= 272 &&
            y >= 0 && y <= 240 && actor.hit_points < 0x7000)
            enemies = true;
    }
    *out = enemies;
    return true;
}
// Screen-space stage slots64..191 for the Sword's target search; empty slots read zero, as
// DestroyEntity leaves them.
static bool SwordTargets(SotnLaboratory *s, Sint32 camera_x, Sint32 camera_y,
                         SotnSwordTarget out[SOTN_SWORD_TARGETS], Uint32 generations[])
{
    for (Uint16 i = 0; i < SOTN_SWORD_TARGETS; ++i)
    {
        SotnActorState actor = {0};
        if (!SotnActors_Get(s->actors, i, &actor))
            return false;
        generations[i] = actor.id ? actor.generation : 0;
        out[i] = !actor.id ? (SotnSwordTarget){0}
                           : (SotnSwordTarget){.id = actor.id,
                                               .hitbox_state = actor.hitbox_state,
                                               .flags = actor.flags,
                                               .x = (Sint16)(Pixel(actor.x) - camera_x),
                                               .y = (Sint16)(Pixel(actor.y) - camera_y),
                                               .hit_points = actor.hit_points};
    }
    return true;
}
// Every live Sword part holds exactly its actual committed row lease.
static bool ValidateSword(const SotnLaboratory *s)
{
    SotnSwordLease wanted[3] = {0};
    if (!SotnSwordRuntime_Leases(&s->sword_runtime, wanted))
        return false;
    for (Uint16 i = 0; i < 3; ++i)
    {
        const SotnSwordLease *l = &s->sword_leases[i];
        if (wanted[i].count != l->count ||
            (l->count && (wanted[i].generation != l->generation || wanted[i].slot != l->slot)))
            return SDL_SetError("Sword familiar part %u lost its actual row lease", i);
        for (Uint16 row = 0; row < l->count; ++row)
            if (!SotnActors_SwordRow(s->actors, l, row))
                return false;
    }
    return true;
}
static bool CommitSwordRows(SotnLaboratory *s, const SotnSwordRuntime *runtime)
{
    SotnSwordLease wanted[3] = {0};
    return SotnSwordRuntime_Leases(runtime, wanted) &&
           SotnActors_CommitSwordRows(s->actors, s->sword_leases, wanted);
}
static SotnSwordInput SwordInput(SotnLaboratory *s, SotnDataView overlay, SotnPlayerState p,
                                 SotnFamiliarStats stats, Sint32 camera_x, Sint32 camera_y)
{
    // Original 4CE2C.c clears FgLayer flags unless the layer's20/40 special bits apply.
    const Uint16 flags =
        (s->room.foreground.parameters & 0x60U) ? s->room.foreground.parameters : 0;
    return (SotnSwordInput){.overlay = overlay,
                            .executable = s->executable,
                            .player = FairyPlayer(p, camera_x, camera_y),
                            .stats = stats,
                            .room_flags = flags,
                            .stage = s->stage_id,
                            .castle_flag = s->world.flags[464],
                            .global_flash = SotnSubweapons_Frozen(s->subweapons),
                            .event_source = {overlay, FAMILIAR_SWORD.events, 49},
                            .room = {.world = &s->world,
                                     .room_x = s->room.foreground.left,
                                     .room_y = s->room.foreground.top,
                                     .camera_x = (Sint16)camera_x,
                                     .camera_y = (Sint16)camera_y,
                                     .stage = s->stage_id,
                                     .servant = FAMILIAR_SWORD.identity},
                            .sound = {.room_flags = flags}};
}
static bool CommitSwordEvents(SotnLaboratory *s, const SotnSwordEvents *e)
{
    if (e->castle_flag_write)
    {
        s->world.flags[464] = e->castle_flag;
        s->dirty = true;
    }
    if (e->pause_write)
        s->fairy_pause_disabled = !e->pause_allowed;
    return !e->cd_command || SotnCdAudio_Queue(s->fairy_cd, e->cd_command);
}
static bool FairyInput(SotnLaboratory *s, Uint16 gate, Sint32 camera_x, Sint32 camera_y,
                       SotnFairyRuntimeInput *out)
{
    SotnPlayerState p = {0};
    SotnCharacterState c = {0};
    SotnFamiliarProgress progress = {0};
    SotnCdAudioState cd = {0};
    SotnActorControl control = {0};
    Uint16 selected = 0;
    if (!SotnPlayer_GetState(s->player, &p) || !SotnCharacter_Get(s->character, &c, NULL) ||
        !SotnCharacter_GetFamiliars(s->character, &progress, &selected) || selected != 3 ||
        (s->world.relics[FAMILIAR_FAIRY.card] & 3U) != 3 || !SotnCdAudio_Get(s->fairy_cd, &cd) ||
        !SotnActors_GetControl(s->actors, &control))
        return SDL_SetError("Fairy input lost actual owned card, progression, player or audio");
    bool enemies = false;
    if (!FamiliarEnemies(s, camera_x, camera_y, &enemies))
        return false;
    const SotnDataView overlay = SotnResource_View(s->fairy_resources[0]);
    // Original 4CE2C.c clears FgLayer flags unless the layer's20/40 special bits apply.
    const Uint16 flags =
        (s->room.foreground.parameters & 0x60U) ? s->room.foreground.parameters : 0;
    SotnFairyRuntimeInput in = {
        .gate_scene = gate,
        .fairy = {.overlay = overlay,
                  .executable = s->executable,
                  .player = FairyPlayer(p, camera_x, camera_y),
                  .hp = p.hp,
                  .hp_max = p.hp_max,
                  .level = (Uint16)progress.records[2].level,
                  .load_count = progress.records[2].loads,
                  .room_flags = flags,
                  .scene = s->fairy_scene,
                  .tilemap_x = s->room.foreground.left * 256 + camera_x,
                  .tilemap_y = s->room.foreground.top * 256 + camera_y,
                  .cutscene = control.pause_disabled,
                  .enemies = enemies,
                  .inverted = s->stage_id >= 32,
                  .global_flash = SotnSubweapons_Frozen(s->subweapons),
                  .medicine_active = SotnBatRuntime_MedicinePresent(&s->bat_runtime)},
        .event_source = {overlay, FAMILIAR_FAIRY.events, 49},
        .room = {.world = &s->world,
                 .room_x = s->room.foreground.left,
                 .room_y = s->room.foreground.top,
                 .camera_x = (Sint16)camera_x,
                 .camera_y = (Sint16)camera_y,
                 .stage = s->stage_id,
                 .servant = selected},
        .sound = {.room_flags = flags,
                  .cutscene = control.pause_disabled,
                  .cd_queue_empty = cd.queue_empty,
                  .command_acknowledged = cd.acknowledged,
                  .xa_playing = cd.xa_active}};
    SDL_memcpy(in.fairy.hand_count, c.hand_count, sizeof(in.fairy.hand_count));
    SDL_memcpy(in.fairy.buffs, s->stat_buffs.timers, sizeof(in.fairy.buffs));
    *out = in;
    return true;
}
static bool LoadFamiliar(SotnLaboratory *s, Uint16 selected, bool engine, Uint16 retained)
{
    SotnPlayerState player = {0};
    SotnFamiliarProgress progress = {0};
    Uint16 previous = 0;
    // The overlay this load needs resident: the selection, or an engine handoff's cached one.
    const Uint16 want = selected ? selected : retained;
    const FamiliarProfile *profile = Familiar(want);
    if (!s || (selected && !Familiar(selected)) || (retained && !Familiar(retained)) ||
        (selected && retained && selected != retained) || (!engine && retained) ||
        !SotnPlayer_GetState(s->player, &player) ||
        !SotnCharacter_GetFamiliars(s->character, &progress, &previous) ||
        !SotnFamiliarProgress_Validate(&progress) || (previous && !Familiar(previous)) ||
        (engine && previous != selected) ||
        !SotnFairyRuntime_Validate(&s->fairy_runtime, s->actors) || !ValidateSword(s))
        return SDL_SetError("Familiar loading requires actual player/progression/lifecycle owners");
    SotnWorldState world = s->world;
    Uint16 selected_cards = 0;
    for (Uint16 id = 0; id < SOTN_RELICS; ++id)
    {
        SotnItem relic = {0};
        if (!SotnRelic_Read(s->dra, id, &relic))
            return false;
        if (relic.type)
        {
            if (world.relics[id] > 3 || world.relics[id] == 2)
                return SDL_SetError("Familiar selection lost its actual found/active card bits");
            world.relics[id] &= 1U;
            if (selected && relic.type == selected)
            {
                if (id != Familiar(selected)->card || ++selected_cards != 1 ||
                    !(world.relics[id] & 1U))
                    return SDL_SetError("Familiar selection requires its actually owned card");
                world.relics[id] |= 2;
            }
        }
    }
    if (selected && selected_cards != 1)
        return SDL_SetError("Familiar selection lost its actual US card definition");
    const bool loaded = s->fairy_resources[0] != NULL;
    for (Uint16 i = 0; i < 4; ++i)
        if ((s->fairy_resources[i] != NULL) != loaded)
            return SDL_SetError("Familiar cache contains incomplete actual resources");
    if ((loaded && !Familiar(s->familiar_overlay)) ||
        (!engine && previous &&
         (!loaded || s->familiar_overlay != previous || !s->fairy_cd ||
          !FamiliarInitialized(s, previous))) ||
        (loaded && (!s->fairy_cd || !FamiliarInitialized(s, s->familiar_overlay) || s->music)) ||
        (s->fairy_cd && s->music))
        return SDL_SetError("Familiar selection lost its actual cached binding or CD owner");
    if (!selected && !retained)
    {
        SotnSwordRuntime *sword = SDL_malloc(sizeof(*sword));
        if (!sword)
            return false;
        *sword = s->sword_runtime;
        const bool ok = SotnSwordRuntime_Clear(sword) && CommitSwordRows(s, sword) &&
                        SotnFairyRuntime_Clear(&s->fairy_runtime, s->actors) &&
                        (engine || SotnCharacter_SelectFamiliar(s->character, 0, false));
        if (ok)
        {
            s->sword_runtime = *sword;
            s->world = world;
            s->dirty = true;
        }
        SDL_free(sword);
        return ok;
    }
    const bool cached = loaded && s->familiar_overlay == want;
    SotnResource *opened[4] = {0};
    SotnActorGraphics *graphics = NULL;
    SotnSound *sound = NULL;
    SotnCdAudio *cd = NULL;
    Uint16 clut[SOTN_SWORD_CLUT_WORDS] = {0};
    SotnFairyRuntime *runtime = SDL_malloc(sizeof(*runtime));
    SotnSwordRuntime *sword = SDL_malloc(sizeof(*sword));
    if (!runtime || !sword)
        goto fail;
    if (!cached)
    {
        for (Uint16 i = 0; i < 4; ++i)
            if (!SotnResource_Open(s->directory, (SotnResourceId)(profile->code + i), &opened[i]))
                goto fail;
        SotnMusic *empty = NULL;
        if ((want == FAMILIAR_FAIRY.identity
                 ? !SotnActorGraphics_PrepareFamiliar(
                       s->actor_graphics, SotnResource_View(opened[0]),
                       SotnResource_View(opened[1]), 0x80170040, 0x801722e8, &graphics)
                 : (!SotnSword_Clut(SotnResource_View(opened[0]), clut) ||
                    !SotnActorGraphics_PrepareFamiliarModel(s->actor_graphics,
                                                            SotnResource_View(opened[1]), clut,
                                                            SOTN_SWORD_CLUT_WORDS, &graphics))) ||
            !SotnSound_PrepareFamiliarBank(s->sound, SotnResource_View(opened[2]),
                                           SotnResource_View(opened[3]), &sound))
            goto fail;
        if (s->fairy_cd
                ? !SotnCdAudio_CheckSoundBinding(s->fairy_cd, sound)
                : (!SotnCdAudio_Create(s->dra, s->music_stream, &empty, s->system_timer, &cd) ||
                   !SotnCdAudio_BindSound(cd, sound)))
            goto fail;
        SotnMusicState music = {0};
        if (s->music && !SotnMusic_Get(s->music, &music))
            goto fail;
    }
    const SotnDataView overlay = SotnResource_View(cached ? s->fairy_resources[0] : opened[0]);
    if (cached && want == FAMILIAR_SWORD.identity && !SotnSword_Clut(overlay, clut))
        goto fail;
    const Uint16 flags =
        (s->room.foreground.parameters & 0x60U) ? s->room.foreground.parameters : 0;
    const SotnFairyRuntimeInput input = {
        .fairy = {.overlay = overlay,
                  .executable = s->executable,
                  .player = FairyPlayer(player, s->camera.x, s->camera.y),
                  .hp = player.hp,
                  .hp_max = player.hp_max,
                  .level = (Uint16)progress.records[FAMILIAR_FAIRY.record].level,
                  .load_count = progress.records[FAMILIAR_FAIRY.record].loads,
                  .room_flags = flags,
                  .inverted = s->stage_id >= 32},
        .event_source = {overlay, FAMILIAR_FAIRY.events, 49}};
    *runtime = s->fairy_runtime;
    *sword = s->sword_runtime;
    // Another familiar's overlay replaces the RAM image: the wanted one's data starts fresh.
    if (s->familiar_overlay != want)
    {
        if (want == FAMILIAR_FAIRY.identity)
            SDL_zerop(runtime);
        else
            SDL_zerop(sword);
    }
    if (previous != selected)
    {
        SDL_zero(runtime->slots);
        if (s->familiar_overlay == want && !SotnSwordRuntime_Clear(sword))
            goto fail;
    }
    const Uint16 mode = engine ? 0 : previous == selected ? 3 : 1;
    SotnSwordEvents sword_events = {0};
    if ((selected == FAMILIAR_FAIRY.identity &&
         !SotnFairyRuntime_Init(runtime, s->actors, input, mode)) ||
        (selected == FAMILIAR_SWORD.identity &&
         !SotnSwordRuntime_Init(sword,
                                SwordInput(s, overlay, player,
                                           progress.records[FAMILIAR_SWORD.record], s->camera.x,
                                           s->camera.y),
                                mode, &sword_events)))
        goto fail;
    if (sword_events.castle_flag_write)
        world.flags[464] = sword_events.castle_flag;
    // A departing Sword's slots retire with their rows; another overlay keeps no Sword runtime.
    if (want != FAMILIAR_SWORD.identity && !SotnSwordRuntime_Clear(sword))
        goto fail;
    // Everything that can allocate, read private files or validate metadata finishes first.
    // These final operations touch only the already validated live owners on this main thread.
    const bool rows = CommitSwordRows(s, sword);
    if (!rows || (previous != selected && !SotnFairyRuntime_Clear(&s->fairy_runtime, s->actors)) ||
        (cd && !SotnCdAudio_AdoptMusic(cd, &s->music, s->system_timer)) ||
        (!engine && !SotnCharacter_SelectFamiliar(s->character, selected, !cached)))
        goto fail;
    if (graphics)
    {
        // The compatibility check above and all fallible preparation finished
        // before committing this dependency. Keep the actual CD transport/rings.
        if (s->fairy_cd && !SotnCdAudio_BindSound(s->fairy_cd, sound))
            goto fail;
        SotnActorGraphics_Destroy(s->actor_graphics);
        s->actor_graphics = graphics;
        graphics = NULL;
        SotnSound_Destroy(s->sound);
        s->sound = sound;
        sound = NULL;
        if (cd)
        {
            s->fairy_cd = cd;
            cd = NULL;
        }
        for (Uint16 i = 0; i < 4; ++i)
        {
            SotnResource_Close(s->fairy_resources[i]);
            s->fairy_resources[i] = opened[i];
            opened[i] = NULL;
        }
    }
    else if (sword_events.clut_upload &&
             !SotnActorGraphics_LoadFamiliarClut(s->actor_graphics, clut, SOTN_SWORD_CLUT_WORDS))
        goto fail;
    if (want == FAMILIAR_SWORD.identity)
    {
        s->sword_runtime = *sword;
        if (s->familiar_overlay != want)
            SDL_zero(s->fairy_runtime);
    }
    else
    {
        s->fairy_runtime = *runtime;
        SDL_zero(s->sword_runtime);
    }
    s->familiar_overlay = want;
    s->world = world;
    s->dirty = true;
    SDL_free(runtime);
    SDL_free(sword);
    return true;
fail:
    SotnActorGraphics_Destroy(graphics);
    SotnCdAudio_Destroy(cd);
    SotnSound_Destroy(sound);
    for (Uint16 i = 0; i < 4; ++i)
        SotnResource_Close(opened[i]);
    SDL_free(runtime);
    SDL_free(sword);
    return false;
}

bool SotnLaboratory_SelectFamiliar(SotnLaboratory *s, Uint16 selected)
{
    SotnPlayerState player = {0};
    SotnActorControl control = {0};
    if (!s || s->game_over || s->launch_active || s->stage_pending || s->map_open ||
        s->menu.phase != SOTN_MENU_CLOSED || s->spell_freeze.active || s->spell_freeze.request ||
        s->fairy_scene || s->fairy_pause_disabled || !SotnPlayer_GetState(s->player, &player) ||
        !player.can_pause || !SotnActors_GetControl(s->actors, &control) ||
        control.pause_disabled || control.timer || control.hold)
        return SDL_SetError("Familiar selection requires actual idle player/menu/lifecycle owners");
    return LoadFamiliar(s, selected, false, 0);
}
static bool InitializeFamiliar(SotnLaboratory *s, const SotnLaboratory *source)
{
    Uint16 selected = 0;
    for (Uint16 id = 0; id < SOTN_RELICS; ++id)
    {
        SotnItem relic = {0};
        if (!SotnRelic_Read(s->dra, id, &relic))
            return false;
        if (relic.type && (s->world.relics[id] & 2U))
        {
            if (!Familiar(relic.type) || Familiar(relic.type)->card != id || selected ||
                s->world.relics[id] != 3)
                return SDL_SetError(
                    "Engine familiar initialization requires one actual supported card");
            selected = relic.type;
        }
    }
    SotnFamiliarProgress progress = {0};
    bool cached = false;
    if (source)
    {
        Uint16 previous = 0;
        if (!SotnCharacter_GetFamiliars(source->character, &progress, &previous) ||
            previous != selected ||
            !SotnFairyRuntime_Validate(&source->fairy_runtime, source->actors) ||
            !ValidateSword(source))
            return SDL_SetError(
                "Connected familiar handoff lost actual selection or namespace ownership");
        cached = source->fairy_resources[0] != NULL;
        for (Uint16 i = 0; i < 4; ++i)
            if ((source->fairy_resources[i] != NULL) != cached)
                return SDL_SetError("Connected familiar handoff lost its complete resource cache");
        if ((cached && (!Familiar(source->familiar_overlay) || !source->fairy_cd ||
                        !FamiliarInitialized(source, source->familiar_overlay) || source->music)) ||
            (!cached && (selected || source->fairy_runtime.initialized ||
                         source->sword_runtime.initialized)) ||
            (source->fairy_cd && source->music))
            return SDL_SetError(
                "Connected familiar handoff lost actual loaded overlay/audio ownership");
        if (cached)
        {
            // Engine_Init destroys entities/rows, not overlay data; this pool has no rows yet.
            s->fairy_runtime = source->fairy_runtime;
            SDL_zero(s->fairy_runtime.slots);
            s->sword_runtime = source->sword_runtime;
            if (!SotnSwordRuntime_Clear(&s->sword_runtime))
                return false;
            s->familiar_overlay = source->familiar_overlay;
        }
    }
    else if (!SotnCharacter_GetFamiliars(s->character, &progress, NULL))
        return false;
    if (!SotnCharacter_RestoreFamiliarState(s->character, &progress, selected))
        return false;
    if (!selected && !cached)
        return true;
    // Engine_Init omits the new stage XA request in an actual flag20 room.
    // This candidate decoder has not ticked or been adopted by another owner.
    if (s->room.foreground.parameters & 0x20U)
    {
        SotnMusic_Destroy(s->music);
        s->music = NULL;
    }
    return LoadFamiliar(s, selected, true, cached ? s->familiar_overlay : 0);
}

static bool TickFairy(SotnLaboratory *s, Uint16 gate, Sint32 camera_x, Sint32 camera_y)
{
    if (!s->fairy_runtime.initialized || !SotnFairyRuntime_Occupied(&s->fairy_runtime))
        return true;
    for (Uint16 slot = 4; slot < 8; ++slot)
    {
        SotnFairyRuntimeInput in = {0};
        SotnFairyRuntimeEvents events = {0};
        SotnCharacterState c = {0};
        if (!FairyInput(s, gate, camera_x, camera_y, &in) ||
            !SotnCharacter_Get(s->character, &c, NULL))
            return false;
        Uint32 random = c.random;
        if (!SotnFairyRuntime_UpdateSlot(&s->fairy_runtime, s->actors, slot, in, &random, &events))
            return false;
        c.random = random;
        if (!SotnCharacter_Restore(s->character, &c))
            return false;
        const SotnFairyEvents *e = &events.root;
        if (e->factory)
        {
            SotnPlayerFactorySource source = s->fairy_runtime.slots[slot - 4U].source;
            source.x = FairyFixed((Uint32)source.x + (Uint32)camera_x * 65536U);
            source.y = FairyFixed((Uint32)source.y + (Uint32)camera_y * 65536U);
            bool allocated = false;
            if (!SotnBatRuntime_RequestFamiliarMedicine(&s->bat_runtime, source, e->factory,
                                                        &allocated))
                return false;
        }
        if (e->consume && !SotnCharacter_UseFamiliarItem(s->character, e->consume))
            return false;
        if (e->clear_shared)
        {
            if (!SotnActors_ClearPlayerRuntime(s->actors, &s->bat_runtime) ||
                !SotnSubweapons_ClearPlayerEntities(s->subweapons))
                return false;
            s->mist_present = false;
            s->mist_generation = 0;
            SDL_zero(s->gas_cloud);
        }
        if (e->restore_resources)
        {
            if (!e->clear_shared || !SotnCharacter_Get(s->character, &c, NULL))
                return SDL_SetError("Fairy resource restoration lost its actual cleanup boundary");
            c.hp = c.hp_max;
            c.mp = c.mp_max;
            if (!SotnCharacter_Restore(s->character, &c) ||
                !SotnPlayer_CommitFairyResources(s->player, c.hp_max, c.mp_max))
                return false;
            s->mp = c.mp;
        }
        if (e->scene_write)
        {
            if (!SotnPlayer_SetFairyScene(s->player, e->scene))
                return false;
            s->fairy_scene = e->scene;
        }
        if (e->pause_write)
            s->fairy_pause_disabled = !e->pause_allowed;
        if (e->release_song)
            return SDL_SetError("US Fairy emitted an unsupported alternate-familiar song release");
        if (e->shake && !SotnCameraShake_Request(&s->shake, s->dra, 4))
            return false;
        for (Uint16 i = 0; i < e->sound_count; ++i)
            if (!SotnSound_Queue(s->sound, e->sounds[i]))
                return false;
        if (events.cd_command && !SotnCdAudio_Queue(s->fairy_cd, events.cd_command))
            return false;
    }
    return true;
}

// Actual slot4..7 Sword callbacks: real row leases before each callback, requests after it.
static bool TickSword(SotnLaboratory *s, Uint16 gate, Sint32 camera_x, Sint32 camera_y)
{
    if (s->familiar_overlay != FAMILIAR_SWORD.identity || !s->sword_runtime.initialized)
        return true;
    bool occupied = false;
    for (Uint16 i = 0; i < 4; ++i)
        occupied = occupied || s->sword_runtime.slots[i].identity;
    if (!occupied)
        return true;
    if (gate != 0 && gate != 3)
        return SDL_SetError("Sword familiar callbacks require an ordinary or frozen pass");
    SotnSwordRuntime *next = SDL_malloc(sizeof(*next));
    SotnSwordTarget *targets = SDL_malloc(sizeof(*targets) * SOTN_SWORD_TARGETS);
    Uint32 generations[SOTN_SWORD_TARGETS] = {0};
    bool ok = next && targets;
    for (Uint16 slot = 4; ok && slot < 8; ++slot)
    {
        const SotnSwordSlot *old = &s->sword_runtime.slots[slot - 4U];
        if (!old->identity || (gate == 3 && !(old->flags & 0x10000U)))
            continue;
        SotnPlayerState p = {0};
        SotnCharacterState c = {0};
        SotnFamiliarProgress progress = {0};
        Uint16 selected = 0;
        SotnCdAudioState cd = {0};
        SotnActorControl control = {0};
        bool enemies = false, allocated = false;
        const Uint16 request = SotnSwordRuntime_RowRequest(&s->sword_runtime, slot);
        ok = SotnPlayer_GetState(s->player, &p) && SotnCharacter_Get(s->character, &c, NULL) &&
             SotnCharacter_GetFamiliars(s->character, &progress, &selected) &&
             SotnActors_GetControl(s->actors, &control) && s->fairy_cd &&
             SotnCdAudio_Get(s->fairy_cd, &cd) &&
             FamiliarEnemies(s, camera_x, camera_y, &enemies) &&
             SwordTargets(s, camera_x, camera_y, targets, generations) &&
             (!request || SotnActors_FreeRows(s->actors, request, &allocated));
        if (ok && (selected != FAMILIAR_SWORD.identity ||
                   (s->world.relics[FAMILIAR_SWORD.card] & 3U) != 3))
            ok = SDL_SetError("Sword familiar lost its actual owned active card");
        if (!ok)
            break;
        SotnSwordInput in = SwordInput(s, SotnResource_View(s->fairy_resources[0]), p,
                                       progress.records[FAMILIAR_SWORD.record], camera_x, camera_y);
        in.brothers = p.sword_brothers.pulse; // g_Player.unk70 from this update's player
        in.cutscene = in.sound.cutscene = control.pause_disabled;
        in.enemies = enemies;
        in.targets = targets;
        in.tilemap_x = (Uint32)(s->room.foreground.left * 256 + camera_x);
        in.tilemap_y = (Uint32)(s->room.foreground.top * 256 + camera_y);
        in.random = c.random;
        in.allocated = allocated;
        in.sound.cd_queue_empty = cd.queue_empty;
        in.sound.command_acknowledged = cd.acknowledged;
        in.sound.xa_playing = cd.xa_active;
        SotnSwordEvents events = {0};
        *next = s->sword_runtime;
        ok = SotnSwordRuntime_UpdateSlot(next, slot, in, &events);
        ok = ok && CommitSwordRows(s, next);
        if (!ok)
            break;
        // The callback's original side effects, in order: the search's flag write, its voices
        // and func_us_801724E8's DE entities in the shared slots17..46.
        Uint64 other = s->mist_present ? (Uint64)1 << 16 : 0;
        if ((events.mark && !SotnActors_MarkFamiliarTarget(s->actors, (Uint16)(events.mark - 1U),
                                                           generations[events.mark - 1U],
                                                           targets[events.mark - 1U].id)) ||
            !SotnGasCloud_Occupied(&s->gas_cloud, &other))
        {
            ok = false;
            break;
        }
        for (Uint16 i = 0; ok && i < events.sound_count; ++i)
            ok = SotnSound_Queue(s->sound, events.sounds[i]);
        // Sword Brothers' DF: GetFreeEntity(0x11, 0x2F) at the sword, spell7 (global attack).
        if (ok && events.brothers_hitbox)
        {
            const SotnSwordSlot *self = &next->slots[slot - 4U];
            bool spawned = false;
            ok = SotnBatRuntime_SpawnFamiliarHitbox(
                &s->bat_runtime, other,
                (SotnPlayerFactorySource){
                    .x = FairyFixed((Uint32)self->x + (Uint32)camera_x * 65536U),
                    .y = FairyFixed((Uint32)self->y + (Uint32)camera_y * 65536U),
                    .priority = self->priority,
                    .facing_left = self->facing_left},
                0, 7, (Uint16)progress.records[FAMILIAR_SWORD.record].level, &spawned);
        }
        for (Uint16 i = 0; ok && i < events.hitbox_count; ++i)
        {
            const SotnSwordHitbox *h = &events.hitboxes[i];
            bool spawned = false;
            ok = SotnBatRuntime_SpawnFamiliarHitbox(
                &s->bat_runtime, other,
                (SotnPlayerFactorySource){.x = FairyFixed((Uint32)(h->x + camera_x) << 16),
                                          .y = FairyFixed((Uint32)(h->y + camera_y) << 16),
                                          .priority = h->priority,
                                          .facing_left = h->facing_left},
                h->parameter, h->spell, (Uint16)progress.records[FAMILIAR_SWORD.record].level,
                &spawned);
        }
        if (ok)
        {
            s->sword_runtime = *next;
            c.random = events.random;
            ok =
                SotnCharacter_Restore(s->character, &c) && CommitSwordEvents(s, &events) &&
                (!events.gift_item || SotnCharacter_Collect(s->character, false, events.gift_item));
        }
        // The farewell turned the card off (found only) and cleared g_Servant; the overlay stays.
        if (ok && events.dismiss)
        {
            if (s->world.relics[FAMILIAR_SWORD.card] == 3)
                s->world.relics[FAMILIAR_SWORD.card] = 1;
            ok = SotnCharacter_SelectFamiliar(s->character, 0, false);
            s->dirty = true;
        }
    }
    SDL_free(next);
    SDL_free(targets);
    return ok;
}
// dra/menu.c MENU_STEP_EXIT_10/11: on leaving the menu the active card's familiar switches
// (loads+1, menu-switch init) or, if unchanged, reinitializes as MENU_SAME_SERVANT.
static bool CloseMenuFamiliar(SotnLaboratory *s)
{
    Uint16 wanted = 0, previous = 0;
    if (!SotnCharacter_GetFamiliars(s->character, NULL, &previous))
        return false;
    for (Uint16 id = 0; id < SOTN_RELICS; ++id)
    {
        SotnItem relic = {0};
        if (!SotnRelic_Read(s->dra, id, &relic))
            return false;
        if (relic.type && (s->world.relics[id] & 2U))
            wanted = relic.type;
    }
    if (!wanted && !previous)
        return true;
    return LoadFamiliar(s, wanted, false, 0);
}
static bool CommitHandRetirement(SotnLaboratory *s, SotnPlayerState p)
{
    if (!p.hand_retire)
        return true;
    if (p.hand_retire > 2)
        return SDL_SetError("Ordinary hand retirement lost its actual player request");
    SotnBatRuntime *next = SDL_malloc(sizeof(*next));
    if (!next)
        return SDL_OutOfMemory();
    *next = s->bat_runtime;
    const bool ok = SotnBatRuntime_RetireHand(next, p.hand_retire == 1) &&
                    SotnActors_CommitBatPrimitives(s->actors, next);
    if (ok)
        s->bat_runtime = *next;
    SDL_free(next);
    return ok;
}
static bool UpdateActors(SotnLaboratory *session, bool entering)
{
    // Connected-room translation already used the actual old/new player screen positions.
    // Only subsequent camera movement belongs to this destination initialization pass.
    const Sint32 fairy_camera_x = entering ? session->camera.x : session->blink_camera_x;
    const Sint32 fairy_camera_y = entering ? session->camera.y : session->blink_camera_y;
    SotnPlayerState player = {0};
    if (!SotnPlayer_GetState(session->player, &player))
        return false;
    if ((!entering && !CommitHandRetirement(session, player)) || !VisitMap(session) ||
        !SotnCamera_Tick(&session->camera, Pixel(player.x), Pixel(player.y), false))
        return false;
    Sint32 camera_x = 0, camera_y = 0;
    Camera(session, player, &camera_x, &camera_y);
    const Uint16 fairy_scene = session->fairy_scene;
    if ((!entering &&
         (!TickFairy(session, session->spell_freeze.request, fairy_camera_x, fairy_camera_y) ||
          !TickSword(session, session->spell_freeze.request, fairy_camera_x, fairy_camera_y))) ||
        !TickMist(session, player, entering) || !TickBlinks(session, player, entering) ||
        !SotnPlayer_GetState(session->player, &player))
        return false;
    // Snapshot the original pass gate before slot4 can request FFF. The remaining8..63
    // callbacks and ordinary stage still finish this birth frame before engine-five entry.
    if ((!entering && fairy_scene != session->fairy_scene &&
         !SotnSceneFreeze_Request(&session->spell_freeze, session->fairy_scene)) ||
        !SotnFairyRuntime_Translate(&session->fairy_runtime, session->actors,
                                    (Sint16)(fairy_camera_x - camera_x),
                                    (Sint16)(fairy_camera_y - camera_y), 0, 0, false) ||
        !SotnSwordRuntime_Translate(&session->sword_runtime, (Sint16)(fairy_camera_x - camera_x),
                                    (Sint16)(fairy_camera_y - camera_y), 0, 0, false))
        return false;
    SotnCharacterState profile = {0};
    if (!SotnCharacter_Get(session->character, &profile, NULL))
        return false;
    profile.hp = player.hp;
    profile.hp_max = player.hp_max;
    if (!SotnCharacter_Restore(session->character, &profile))
        return false;
    // Initializing a newly entered room must not advance projectiles or replay the
    // hand-attack sound already consumed by the departing room in this frame.
    if (entering)
        player.attack.sound = false;
    if (!BindActorAudio(session) ||
        !SotnActors_SetButtons(session->actors, session->scene_buttons) ||
        (!entering && !SotnSubweapons_Tick(session->subweapons, player, camera_x, camera_y)) ||
        !SotnActors_Tick(session->actors, player, camera_x, camera_y))
        return false;
    if (!SotnActors_GetDialogue(session->actors, &session->dialogue))
        return false;
    SotnPaletteBlend palette = {0};
    SotnPaletteRedShift red_shift = {0};
    if (!SotnActors_GetPaletteBlend(session->actors, &palette) ||
        !SotnActorGraphics_BlendPalette(session->actor_graphics, palette) ||
        !SotnActors_GetPaletteRedShift(session->actors, &red_shift) ||
        !SotnActorGraphics_ReddenPalette(session->actor_graphics, red_shift))
        return false;
    if (session->dialogue.background_enabled)
        session->room.background.flags |= 1;
    SotnActorMotion motion = {0};
    if (!SotnActors_GetMotion(session->actors, &motion) ||
        (motion.align_horizontal && !SotnPlayer_StageAlignX(session->player, motion.alignment_x)) ||
        (motion.align_vertical && !SotnPlayer_StageAlignY(session->player, motion.alignment_y)) ||
        (motion.grab_changed && !SotnPlayer_RequestGrab(session->player, motion.grab)) ||
        !SotnPlayer_Displace(session->player, motion.x, motion.y, motion.contacts) ||
        (!entering && !SotnBatRuntime_PlayerShove(&session->bat_runtime, motion.x, motion.y)) ||
        (motion.stop_velocity && !SotnPlayer_StageStop(session->player)) ||
        (motion.stop_horizontal && !SotnPlayer_StageStopHorizontal(session->player)) ||
        (motion.protect && !SotnPlayer_StageGuard(session->player, motion.halt_vertical)) ||
        !SotnPlayer_BellInside(session->player, motion.bell_inside))
        return false;
    if (!SotnPlayer_SetChairKind(session->player, motion.chair_kind))
        return false;
    SotnActorCombat combat = {0};
    if (!SotnActors_GetCombat(session->actors, &combat) ||
        !SotnCharacter_Get(session->character, &profile, NULL))
        return false;
    if (combat.blood_contacts &&
        !SotnPlayer_QueueBloodHealing(session->player, combat.blood_heal, combat.blood_contacts))
        return false;
    if (combat.kick_hit &&
        !SotnPlayer_KickHit(session->player, combat.kick_generation, combat.kick_hit))
        return false;
    if (!SotnPlayerStep_IsDead(player.step) && !ApplyCharacter(session))
        return false;
    if (!SotnPlayer_QueueContact(session->player,
                                 (SotnPlayerContact){.attack = combat.contact_damage,
                                                     .parameters = combat.contact_element,
                                                     .source_x = combat.contact_x,
                                                     .source_velocity = combat.contact_velocity}))
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
                            "Stage %u actor 0x%02x is not integrated; this development "
                            "checkpoint omits it",
                            (unsigned int)session->stage_id, (unsigned int)state.id);
                session->warned_actors[state.id] = true;
            }
        }
    }
    return true;
}

static bool TickActors(SotnLaboratory *session)
{
    return UpdateActors(session, false);
}

static bool EnterSaveRoom(SotnLaboratory *session)
{
    const bool was_save_room = session->save_room;
    const bool was_clock_room = session->clock_room;
    session->clock_room = session->stage_id == 0 && session->room_index == 9;
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
    if (session->audio != NULL &&
        (was_save_room != session->save_room || was_clock_room != session->clock_room))
    {
        if (!SotnPlayback_Reset(session->audio))
            return false;
        if (session->fairy_cd)
        {
            if (session->save_room)
            {
                if (!SotnCdAudio_Queue(session->fairy_cd, 0xa))
                    return false;
                session->room_music_stopped = true;
            }
        }
        else if (!session->save_room && !session->clock_room)
        {
            SotnMusic_Destroy(session->music);
            session->music = NULL;
            if (!CreateStageMusic(session, &session->music))
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
    SotnFamiliarProgress familiars = {0};
    if (!SotnCharacter_Get(session->character, &character, NULL) ||
        !SotnCharacter_GetFamiliars(session->character, &familiars, NULL))
        return false;
    Uint64 playtime_ticks = 0;
    if (!SotnPlaytime_Ticks(session->playtime, &playtime_ticks))
        return false;
    character.hp = player.hp;
    *out = (SotnSaveState){.stage = session->stage_id,
                           .room = session->room_index,
                           .character = character,
                           .world = session->world,
                           .ticks = playtime_ticks,
                           .player_random = player.random,
                           .actor_random = progress.random,
                           .combat_random = progress.combat_random,
                           .familiars = familiars};
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
        if (!SotnCharacter_Restore(session->character, &profile) || !ApplyCharacter(session) ||
            !SotnHud_SaveHp(session->hud))
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
            SDL_Log("Saved stage %u room %u to native slot %u", session->stage_id,
                    session->room_index, session->save_point.slot);
        if (!SotnSavePoint_Complete(&session->save_point, saved))
            return false;
    }
    session->input.menu_open = SotnSavePoint_Menu(&session->save_point);
    return true;
}

static bool QueueStageArrival(SotnLaboratory *session, SotnStageRoute route)
{
    const SotnStageProfile *destination = SotnStageProfile_Get(route.stage);
    if (destination == NULL || !destination->connected)
        return SDL_SetError("Stage %u is outside the integrated native castle checkpoint",
                            route.stage);
    session->arrival = route;
    session->stage_pending = true;
    return true;
}

static bool RetireSharedRoom(SotnLaboratory *s)
{
    bool present = false;
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        present |= s->bat_runtime.children[i].kind == 32 || s->bat_runtime.children[i].kind == 33 ||
                   s->bat_runtime.children[i].kind == 65 || s->bat_runtime.children[i].kind == 38 ||
                   s->bat_runtime.children[i].kind == 39 || s->bat_runtime.children[i].food ||
                   s->bat_runtime.children[i].neutron || s->bat_runtime.children[i].sire ||
                   s->bat_runtime.children[i].pentagram || s->bat_runtime.children[i].magic ||
                   s->bat_runtime.children[i].thrown || s->bat_runtime.children[i].star ||
                   s->bat_runtime.children[i].karma || s->bat_runtime.children[i].subweapon ||
                   s->bat_runtime.children[i].thrown_particle_parent ||
                   (s->bat_runtime.children[i].kind == 5 &&
                    (s->bat_runtime.children[i].parameter >> 8) == 12);
    for (Uint16 i = 0; i < SDL_arraysize(s->bat_runtime.factories); ++i)
        present |= s->bat_runtime.factories[i].cursor.entity_id != 0;
    if (!present)
        return true;
    SotnBatRuntime *next = SDL_malloc(sizeof(*next));
    if (!next)
        return SDL_OutOfMemory();
    *next = s->bat_runtime;
    const bool committed =
        SotnBatRuntime_PruneDissolve(next) && SotnBatRuntime_PruneIce(next) &&
        SotnBatRuntime_PruneTeleport(next) && SotnBatRuntime_PruneMedicines(next) &&
        SotnBatRuntime_PruneFood(next) && SotnBatRuntime_PruneNeutron(next) &&
        SotnBatRuntime_PruneSire(next) && SotnBatRuntime_PrunePentagram(next) &&
        SotnBatRuntime_PruneMagicMissile(next) && SotnBatRuntime_PruneThrown(next) &&
        SotnBatRuntime_PruneStar(next) && SotnBatRuntime_PruneKarma(next) &&
        SotnBatRuntime_PruneSubweapons(next) && SotnBatRuntime_PruneRoomFactories(next) &&
        SotnActors_CommitBatPrimitives(s->actors, next);
    if (committed)
        s->bat_runtime = *next;
    SDL_free(next);
    return committed;
}
static bool Transition(SotnLaboratory *session, const SotnPlayerState *state)
{
    const Sint32 local_x = Pixel(state->x), local_y = Pixel(state->y);
    const Sint32 width = (Sint32)session->room.foreground.collision.width * 16;
    const Sint32 height = (Sint32)session->room.foreground.collision.height * 16;
    SotnRoomCrossing crossing = {0};
    if (session->stage_id == 6)
    {
        SotnRoomWindow window = {0};
        if (!SotnActors_GetRoomWindow(session->actors, &window) ||
            !SotnStageRoute_WindowCrossing(window, local_x, local_y, (Uint32)width, (Uint32)height,
                                           (state->contacts & 1) != 0 || (state->status & 3U) != 0,
                                           &crossing))
            return false;
    }
    else if (!SotnStageRoute_Crossing(local_x, local_y, (Uint32)width, (Uint32)height,
                                      (state->contacts & 1) != 0 || (state->status & 3U) != 0,
                                      &crossing))
        return false;
    if (!crossing.active)
        return true;
    const Sint32 world_x = (Sint32)session->room.left * 256 + crossing.x;
    const Sint32 world_y = (Sint32)session->room.top * 256 + crossing.y;
    if (session->stage_id == 1 || session->stage_id == 4 || session->stage_id == 6)
    {
        SotnStageRoute boss = {0};
        bool found = false;
        const Sint32 chunk_x = world_x >= 0 ? world_x / 256 : -((255 - world_x) / 256);
        const Sint32 chunk_y = world_y >= 0 ? world_y / 256 : -((255 - world_y) / 256);
        if (!SotnStageRoute_FindBoss(session->dra, session->stage_id, chunk_x, chunk_y,
                                     &session->world, &found, &boss))
            return false;
        if (found)
            return QueueStageArrival(session, boss);
    }
    for (Uint32 i = 0; i < SotnStage_RoomCount(session->stage); ++i)
    {
        SotnStageRoom room = {0};
        if (!SotnStage_GetRoomDefinitions(session->stage, i, 2048, &room))
            return false;
        if (world_x < (Sint32)room.left * 256 || world_x >= ((Sint32)room.right + 1) * 256 ||
            world_y < (Sint32)room.top * 256 || world_y >= ((Sint32)room.bottom + 1) * 256)
            continue;
        if (room.transition)
        {
            SotnStageRoute route = {0};
            if (!SotnStageRoute_Read(session->dra, session->stage_id, room.layout_id, &route))
                return false;
            return QueueStageArrival(session, route);
        }
        const Sint32 offset_x = ((Sint32)session->room.left - room.left) * 256;
        const Sint32 offset_y = ((Sint32)session->room.top - room.top) * 256 + crossing.arrival_y;
        const SotnFairyPlayer departing = FairyPlayer(*state, session->camera.x, session->camera.y);
        if (!SotnFairyRuntime_Validate(&session->fairy_runtime, session->actors) ||
            !ValidateSword(session))
            return false;
        if (!SotnPlayer_ChangeRoom(session->player, room.foreground.collision, offset_x, offset_y))
            return false;
        if (!SotnSubweapons_Enter(session->subweapons, room.foreground.collision))
            return false;
        session->mist_rebase_x += offset_x;
        session->mist_rebase_y += offset_y;
        session->blink_rebase_x += offset_x;
        session->blink_rebase_y += offset_y;
        session->room = room;
        session->room_index = i;
        SotnPlayerState arrived = {0};
        if (!RetireSharedRoom(session) || !EnterActors(session) ||
            !SotnPlayer_GetState(session->player, &arrived))
            return false;
        const SotnFairyPlayer destination =
            FairyPlayer(arrived, session->camera.x, session->camera.y);
        // Original Engine_3 retains flag20000 entities and flag100 packets by the actual
        // player displacement. It does not call ServantInit or advance familiar callbacks.
        if (!SotnFairyRuntime_Translate(&session->fairy_runtime, session->actors, 0, 0,
                                        FairyFixed((Uint32)departing.x - (Uint32)destination.x),
                                        FairyFixed((Uint32)departing.y - (Uint32)destination.y),
                                        true) ||
            !SotnSwordRuntime_Translate(&session->sword_runtime, 0, 0,
                                        FairyFixed((Uint32)departing.x - (Uint32)destination.x),
                                        FairyFixed((Uint32)departing.y - (Uint32)destination.y),
                                        true) ||
            !CommitSwordRows(session, &session->sword_runtime) || !EnterSaveRoom(session) ||
            !SotnPlayer_ClearRoomContact(session->player))
            return false;
        SDL_Log("Native checkpoint: stage %u room %u", session->stage_id, (unsigned int)i);
        return true;
    }
    // An early vertical probe can still be inside a sealed room. Apply no rebase
    // when no neighbor exists; ordinary collision retains the current owner.
    if ((session->stage_id != 6 || crossing.arrival_y != 0) && local_x >= 0 && local_y >= 0 &&
        local_x < width && local_y < height)
        return true;
    return SDL_SetError("Player reached an unsupported boundary in stage %u room %u",
                        session->stage_id, session->room_index);
}

static bool PrimeAudio(SotnLaboratory *session, bool render)
{
    SotnActorCombat combat = {0};
    return SotnActors_GetCombat(session->actors, &combat) &&
           (render ? TickAudio(session, &combat, 0, 0, 0, 0)
                   : QueueAudio(session, &combat, 0, 0, 0, 0));
}

static bool RestoreCharacter(SotnLaboratory *session, SotnSaveState state);
static bool RestoreCharacterAt(SotnLaboratory *, SotnSaveState, const SotnPlaytime *);
static bool RestoreSessionAt(SotnLaboratory *, SotnSaveState, Uint32, const SotnPlaytime *);
static bool RestoreSession(SotnLaboratory *session, SotnSaveState state, Uint32 slot);

static bool InitializeEquipment(SotnLaboratory *session, const SotnSaveState *saved)
{
    if (saved != NULL || session->stage_id != 65)
        return SotnLaboratory_SetEquipment(session, 86, 0);
    if (!SotnCharacter_OpeningGear(session->character))
        return false;
    if (SotnStageProfile_EntranceAfterDeath(session->room_index))
    {
        if (!SotnCharacter_DeathTheft(session->character))
            return false;
        session->world.flags[SOTN_ENTRANCE_DEATH] = 1;
        session->world.flags[SOTN_ENTRANCE_ON] = 1;
    }
    return ApplyCharacter(session);
}

static bool ValidateStart(const SotnLaboratory *session, SessionStart start)
{
    if (session->room.transition)
        return SDL_SetError("Stage %u room %u is a connector, not a playable arrival",
                            session->stage_id, session->room_index);
    if (start.saved != NULL &&
        (start.saved->stage != start.stage || start.saved->room != start.room))
        return SDL_SetError("Saved stage and room do not match the requested native session");
    if (start.arrival != NULL &&
        (start.arrival->stage != start.stage || start.arrival->room != start.room ||
         start.arrival->x >= session->room.foreground.collision.width * 16 ||
         start.arrival->y >= session->room.foreground.collision.height * 16))
        return SDL_SetError("Stage %u room %u arrival is outside its playable bounds", start.stage,
                            start.room);
    return true;
}

static bool InitializeCommonPlayer(SotnLaboratory *session)
{
    SotnPrimitiveSlots slots = {0};
    return SotnActors_PrimitiveSlots(session->actors, &slots) &&
           SotnBatRuntime_InitCommon(&session->bat_runtime, slots) &&
           SotnActors_CommitBatPrimitives(session->actors, &session->bat_runtime) &&
           SotnPlayer_SetAfterimageContext(session->player, session->bat_runtime.common_generation,
                                           session->system_timer);
}
static bool InitializeParticleVectors(SotnLaboratory *session)
{
    SotnCharacterState progress = {0};
    SotnPlayerParticleVectors vectors = {0};
    if (!SotnCharacter_Get(session->character, &progress, NULL) ||
        !SotnPlayerParticleVectors_Init(&vectors, session->executable, &progress.random) ||
        !SotnCharacter_Restore(session->character, &progress))
        return false;
    session->bat_runtime.particle_vectors = vectors;
    return true;
}
static bool InitializeTeleportArrival(SotnLaboratory *s, Uint8 mode)
{
    if (!mode)
        return true;
    if (mode < 4 || mode > 6)
        return SDL_SetError("Native teleport arrival lost its original engine mode");
    const Uint16 substep = mode == 6 ? 1 : mode == 4 ? 3 : 5;
    // RunMainEngine requests two white global GPU tiles on actual mode4/5/6 arrival.
    // Prepare the first committed output now; repeated draws never spend another tile.
    s->engine_flash = true;
    s->engine_flash_frames = 1;
    SotnPlayerState p = {0};
    bool allocated = false;
    // AluInit creates the factory after the64 death-vector RNG calls. No child tick here.
    return SotnPlayer_SetTeleportContext(s->player, TeleportFactory, UseLibraryCard, s,
                                         s->room.left * 256) &&
           (mode != 6 || SotnPlayer_SetArrivalFacing(s->player, true)) &&
           SotnPlayer_BeginTeleportArrival(s->player, substep) &&
           SotnPlayer_GetState(s->player, &p) &&
           SotnBatRuntime_RequestTeleport(
               &s->bat_runtime,
               (SotnPlayerFactorySource){
                   .x = p.x, .y = p.y, .priority = p.priority, .facing_left = p.facing_left},
               121U | (Uint32)substep << 16, &allocated);
}
static bool CreateSession(const SotnGameData *data, const char *directory, SessionStart start,
                          SotnLaboratory **out)
{
    const Uint16 stage_id = start.stage;
    const Uint32 start_room = start.room, slot = start.slot;
    const SotnSaveState *saved = start.saved;

    if (out == NULL)
        return SDL_InvalidParamError("out");
    *out = NULL;
    if (SotnStageProfile_Get(stage_id) == NULL)
        return SDL_SetError("Native session does not support stage %u", stage_id);
    if (stage_id == 12 && start_room != 1 && start_room != 3 && start_room != 4 &&
        start_room != 5 && start_room != 6 && start_room != 9 && start_room != 10 &&
        start_room != 14 && start_room != 16 && start_room != 17 && start_room != 18 &&
        start_room != 20 && start_room != 22 && start_room != 23 && start_room != 24 &&
        start_room != 25)
        return SDL_SetError("Laboratory development starts are currently room 1, 3, 4, 5, 6, 9, "
                            "10, 14, 16, 17, 18, 20, 22, 23, 24, or 25");
    SotnLaboratory *session = SDL_calloc(1, sizeof(*session));
    if (session == NULL)
        return false;
    if (!SotnPlaytime_FromTicks(saved ? saved->ticks : 0, &session->playtime))
    {
        SDL_free(session);
        return false;
    }
    if (start.playtime)
        session->playtime = *start.playtime;
    session->saves = start.startup_store;
    session->borrowed_saves = start.startup_store != NULL;
    session->system_timer = start.system_timer;
    session->map_cursor = start.map_cursor;
    session->map_draw_cursor = start.map_draw_cursor;
    session->game_timer = start.game_timer;
    if (start.spirit_target_cursor >= SOTN_SPELL_TARGETS)
    {
        SDL_free(session);
        return SDL_SetError("Native start lost its original spirit target cursor");
    }
    session->bat_runtime.target_cursor = start.spirit_target_cursor;
    if (saved != NULL)
    {
        session->world = saved->world;
        session->simulation_ticks = saved->ticks;
    }
    if (saved == NULL && stage_id != 65)
        session->world.relics[SOTN_RELIC_CUBE] = 3;
    session->directory = directory != NULL ? SDL_strdup(directory) : NULL;
    session->stage_id = stage_id;
    session->music_gain = 127;
    session->master_gain = 127;
    session->music_stream = start.music;
    session->borrowed_music = start.music != NULL;
    session->room_index = start_room;
    session->mp = 20;
    if (session->directory == NULL ||
        !SotnGameData_GetFile(data, SOTN_GAME_MAIN, &session->executable) ||
        !SotnGameData_GetFile(data, SOTN_GAME_DRA, &session->dra) ||
        !StageOverlay(session, data, directory) ||
        !SotnCharacter_Create(session->dra, &session->character) ||
        !SotnCharacter_SetPlaytime(session->character, session->playtime) ||
        !SotnCharacter_BindExecutable(session->character, session->executable) ||
        !SotnStage_Open(session->nz0, &session->stage) ||
        !SotnStage_GetRoomDefinitions(session->stage, session->room_index, 2048, &session->room) ||
        !ValidateStart(session, start) ||
        !SotnResource_Open(directory, SOTN_RESOURCE_COMMON_GRAPHICS, &session->common) ||
        !SotnResource_Open(directory, SOTN_RESOURCE_ALUCARD_GRAPHICS, &session->arc) ||
        !SotnResource_Open(directory, SOTN_RESOURCE_WEAPON_GRAPHICS, &session->weapons) ||
        !SotnResource_Open(directory, SOTN_RESOURCE_GAME_OVER, &session->game_over_art) ||
        !SotnPlayer_Create(
            session->dra, session->room.foreground.collision,
            start.arrival                                  ? start.arrival->x
            : (session->room.foreground.parameters & 0x20) ? 128
            : stage_id == 12 && start_room == 1            ? 240
            : stage_id == 65                               ? (start_room == 11 ? 16 : 128)
            : start_room >= 22                             ? 128
            : (start_room == 5 || start_room == 6 || start_room == 9 || start_room == 10) ? 80
                                                                                          : 32,
            start.arrival                        ? start.arrival->y
            : stage_id == 65 && start_room == 11 ? 388
                                                 : 135,
            &session->player) ||
        !SotnPlayer_SetHandContext(session->player, HandFactory, ThrowingRandom, session) ||
        !SotnPlayer_SetScriptContext(session->player, session->executable) ||
        !SotnPlayer_SetStageContext(session->player, stage_id) ||
        !SotnSubweapons_Create(session->dra, session->character, session->room.foreground.collision,
                               &session->subweapons) ||
        !SotnPlayer_SetSubweapons(session->player, session->subweapons) ||
        !SotnPlayer_SetSubweaponContext(session->player, SubweaponUse, session) ||
        (start.arrival != NULL &&
         !SotnPlayer_SetArrivalFacing(session->player, (start.arrival->x & 255) >= 128)) ||
        (start.arrival == NULL && !SotnPlayer_Tick(session->player, 0)) ||
        !SotnActors_CreateStage(session->dra, session->nz0, stage_id, &session->actors) ||
        !InitializeCommonPlayer(session) ||
        !SotnActors_SetCamera(session->actors, &session->camera) ||
        !SotnActors_SetWorld(session->actors, &session->world) ||
        !SotnActors_SetCharacter(session->actors, session->character) ||
        !SotnActors_SetSubweapons(session->actors, session->subweapons) ||
        !SotnActors_SetGasCloud(session->actors, &session->gas_cloud) ||
        !SotnActors_SetBatRuntime(session->actors, &session->bat_runtime) ||
        !SotnInventoryRenderer_Create(session->dra, SotnResource_View(session->common),
                                      &session->inventory_renderer) ||
        !SotnActorGraphics_CreateStage(
            session->dra, session->nz0,
            (SotnTileSources){StageGraphics(session), SotnResource_View(session->common)}, stage_id,
            &session->actor_graphics) ||
        !SotnActorGraphics_InitPlayerPalettes(session->actor_graphics) ||
        !InitializeEquipment(session, saved) ||
        !SotnActorGraphics_InitPlayerGlow(session->actor_graphics) ||
        !SotnPlayer_SetDarkContext(session->player, true) ||
        !SotnPlayer_SetSpiritContext(session->player, true) ||
        !SotnPlayer_SetSoulContext(session->player, true) ||
        !SotnSavePointRenderer_Create(session->dra, SotnResource_View(session->common),
                                      &session->save_renderer) ||
        !SotnHud_Create(session->dra, SotnResource_View(session->common),
                        (SotnHudStatus){70, 70, 20, 20, 10, 0}, &session->hud) ||
        (saved != NULL && !RestoreCharacterAt(session, *saved, start.playtime)) ||
        !SotnPlayer_SetAxeContext(session->player, SotnResource_View(session->weapons), true) ||
        !EnterActors(session) ||
        // Teleports may begin inside a closed red-door column. Its actor must open
        // the owned tiles before the first player collision, as in original room entry.
        !TickActors(session) || (start.arrival != NULL && !SotnPlayer_Tick(session->player, 0)) ||
        (saved != NULL && !(start.playtime ? RestoreSessionAt(session, *saved, slot, start.playtime)
                                           : RestoreSession(session, *saved, slot))) ||
        !SotnPlayer_SetWaterBreathTimer(session->player, start.water_breath_timer) ||
        !SotnPlayer_SetWaterPainTimer(session->player, start.water_pain_timer) ||
        !InitializeParticleVectors(session) ||
        !InitializeTeleportArrival(session, start.teleport_mode) ||
        !SotnPlayer_SetDeathCapture(session->player, CaptureDeathImage, DecayDeathImage, session) ||
        !RestoreStatBuffs(session, start.stat_buffs) || !LaboratoryInput_Init(&session->input) ||
        !SotnPresenter_Create("SOTN Native - Castle checkpoint (development)", 1024, 960,
                              &session->presenter) ||
        !OpenAudio(session, directory, start.startup_sound) || !EnterSaveRoom(session) ||
        !InitializeFamiliar(session, start.familiars) ||
        !PrimeAudio(session, start.startup_sound == NULL))
    {
        char detail[512] = {0};
        SDL_strlcpy(detail, SDL_GetError(), sizeof(detail));
        SotnLaboratory_Destroy(session);
        return SDL_SetError("Cannot initialize native stage %u room %u: %s", stage_id, start_room,
                            detail);
    }
    if (start.startup_sound)
    {
        if (!SotnPlayer_SeedInput(session->player, start.startup_buttons))
        {
            SotnLaboratory_Destroy(session);
            return false;
        }
        session->scene_buttons = start.startup_buttons;
    }
    session->last_time = SDL_GetTicksNS();
    session->dirty = true;
    SDL_Log("Native development start: stage %u room %u", stage_id, (unsigned int)start_room);
    if (stage_id == 30)
        SDL_Log(
            "Olrox arena internal foundation: private room/artwork/sound resources, "
            "common pickups, decorations, paired doors, humanoid combat, orbs, portals, bats and "
            "fireballs/skull columns, real transformation/monster combat and captured "
            "defeat/reward, breakable furnishings and sitting. Full effects/audio/parity and "
            "further destinations remain incomplete. Original NO2 travel is enabled.");
    else if (stage_id == 26)
        SDL_Log("Doppleganger checkpoint: placed opening, selected combat, physical defeat, "
                "seals, reward and connected Outer Wall progression. Full effects/parity remain "
                "incomplete.");
    else if (stage_id == 25)
        SDL_Log("Hippogryph checkpoint: selected body/attacks, physical damage, "
                "defeat effects, arena doors/music, Life Max Up, clouds/sky and Maria's dialogue, "
                "speech, flashbacks and player/camera control, with connected Chapel travel. "
                "Full encounter/presentation parity remains incomplete.");
    else if (stage_id == 2)
        SDL_Log("Long Library development adapters: common pickups, selected breakables, "
                "Skeleton/Flea, Flea Armor, Dhuron, plants, Ectoplasm and book combat, music and "
                "saves; placed Lesser Demon entry, selected combat, seals and reward. "
                "Selected Librarian introduction, buy/sell, speech and exit run. Full shop panels, "
                "optional services and remaining scripts are incomplete. Selected Outer Wall "
                "travel is enabled; this is not a complete Library or authentic game release.");
    else if (stage_id == 4)
        SDL_Log(
            "Olrox's Quarters internal checkpoint: selected prisoners, spikes, common pickups, "
            "red doors, breakables/debris, decorations, camera, Valhalla/Hammer/Blade Knights, "
            "Spectral Sword, Skelerang and Bloody Zombie adapters. Selected Marble/Chapel travel "
            "and original Olrox entry/return now connect. Further destinations and full "
            "effects/audio/route parity remain incomplete.");
    else if (stage_id == 6)
        SDL_Log(
            "Royal Chapel checkpoint: private resources, selected shared actors, "
            "lamps/debris, furnishings, shortcut, selected scenery/gargoyles and Winged "
            "Guard/Bat/Crow/Raven/Skelerang/Halberd/Corner Guard/Bone Pillar/Hunting Girl/"
            "Spectral Sword combat, Spike Breaker corridor tiles/damage/debris, bells "
            "and selected internal tower exits, clouds, sky land, stained glass, chairs "
            "and confessional ghosts/blades/chime, plus Maria's selected script, speech "
            "and camera. Selected Hippogryph, Laboratory and Olrox's Quarters travel now connect; "
            "further destinations and full area/encounter parity remain incomplete.");
    else if (stage_id == 0 || stage_id == 1)
        SDL_Log("Early castle development checkpoint: shared combat, selected actors, relics, "
                "ordinary saves, music and connectors. Area-specific enemies, mechanisms and "
                "boss progression remain incomplete.");
    else if (stage_id != 12)
        SDL_Log("Entrance checkpoint: selected enemies, Death opening, common combat/pickups, "
                "and connection to Laboratory. Complete actors/effects and other castle routes "
                "remain incomplete.");
    else
        SDL_Log(
            "Laboratory checkpoint: selected enemies, mechanisms, paired bosses, original "
            "camera follow/locks, HUD, combat/audio, and ordinary saves. Full effects, reverb, "
            "complete castle scripts, other stages, and full-game progression remain incomplete.");
    SDL_Log(
        "Character checkpoint: inventory/equipment, EXP/levels, selected subweapon attacks, "
        "and version-nine saves with stage, exploration, castle events and cutscene completion.");
    if (saved == NULL)
        SDL_Log("Development entry: original Entrance equipment when applicable; "
                "70 HP, base stats 6. Prologue-derived initialization remains separate.");
    *out = session;
    return true;
}

bool SotnLaboratory_Create(const SotnGameData *data, const char *directory, Uint32 start_room,
                           SotnLaboratory **out)
{
    return CreateSession(data, directory, (SessionStart){.stage = 12, .room = start_room}, out);
}

bool SotnLaboratory_CreateEntrance(const SotnGameData *data, const char *directory, Uint32 room,
                                   SotnLaboratory **out)
{
    if (out == NULL)
        return SDL_InvalidParamError("out");
    *out = NULL;
    SotnStageRoute arrival = {0};
    if (room == 0)
    {
        SotnDataView dra = {0};
        if (!SotnGameData_GetFile(data, SOTN_GAME_DRA, &dra) ||
            !SotnStageRoute_Read(dra, 65, 17, &arrival))
            return false;
        if (arrival.stage != 65 || arrival.room != 0)
            return SDL_SetError("Entrance development entry does not match its original arrival");
    }
    return CreateSession(
        data, directory,
        (SessionStart){.stage = 65, .room = room, .arrival = room == 0 ? &arrival : NULL}, out);
}

bool SotnLaboratory_CreateStage(const SotnGameData *data, const char *directory, Uint16 stage,
                                Uint32 room, SotnLaboratory **out)
{
    if (stage == 65)
        return SotnLaboratory_CreateEntrance(data, directory, room, out);
    return CreateSession(data, directory, (SessionStart){.stage = stage, .room = room}, out);
}

bool SotnLaboratory_SetEquipment(SotnLaboratory *session, Uint16 left, Uint16 right)
{
    if (session == NULL)
        return SDL_InvalidParamError("laboratory");
    SotnPlayerState current = {0};
    if (!SotnPlayer_GetState(session->player, &current) || current.attack_mode)
        return SDL_SetError("Development equipment requires an idle ordinary hand owner");
    if ((SotnEquipment_IsTwoHand(left) || SotnEquipment_IsTwoHand(right)) && left != right)
        return SDL_SetError("Development two-hand equipment requires both actual hand slots");
    const Uint16 ids[2] = {left, right};
    SotnCharacterState state = {0};
    if (!SotnCharacter_Get(session->character, &state, NULL))
        return false;
    for (Uint32 i = 0; i < 2; ++i)
    {
        if (!SotnCharacter_Get(session->character, &state, NULL))
            return false;
        if (state.equipment[i] != ids[i])
        {
            if (!SotnCharacter_Collect(session->character, false, ids[i]) ||
                !SotnCharacter_Equip(session->character, i, ids[i]))
                return false;
        }
    }
    if (!ApplyMenuCharacter(session))
        return false;
    session->dirty = true;
    SDL_Log("Development hand equipment: left %u, right %u (0 empty, 18 Basilard, 86 Gladius, 128 "
            "Mace)",
            left, right);
    return true;
}

static bool DrawGameplay(SotnLaboratory *session);

static bool EnterSpellFreeze(SotnLaboratory *s)
{
    bool entered = false;
    if (!SotnSceneFreeze_EndFrame(&s->spell_freeze, &entered))
        return false;
    if (entered)
    {
        if (!SotnPlayer_ResetAfterimages(s->player, 1))
            return false;
        s->spell_pause_pending = true;
    }
    return true;
}

static bool TickSpellFreeze(SotnLaboratory *s, Uint16 buttons, SotnSceneFreezeFrame mode)
{
    const bool release = mode == SOTN_SCENE_RELEASE;
    SotnPlayerState player = {0};
    SotnActorCombat combat = {0};
    if (!SotnPlayer_SetAfterimageContext(s->player, s->bat_runtime.common_generation,
                                         s->system_timer) ||
        !SotnPlayer_TickSpellFreeze(s->player, buttons, !release) ||
        !SotnPlayer_GetState(s->player, &player))
        return false;
    if (release)
    {
        // Original engine five thaws on a separate frame, without any child/stage tick.
        if (!SotnPlayer_EndSpellFreeze(s->player) || !QueueNativeSound(s, 0x0f))
            return false;
    }
    else
    {
        ++s->player_effect_frame;
        if (!TickFairy(s, 3, s->camera.x, s->camera.y) ||
            !TickSword(s, 3, s->camera.x, s->camera.y) || !TickBlinks(s, player, false) ||
            !BindActorAudio(s) || !SotnActors_SetButtons(s->actors, buttons) ||
            !SotnActors_TickFrozen(s->actors, player, s->camera.x, s->camera.y) ||
            !SotnActors_GetCombat(s->actors, &combat) ||
            !SotnActors_GetDialogue(s->actors, &s->dialogue))
            return false;
        SotnPaletteBlend blend = {0};
        SotnPaletteRedShift red = {0};
        if (!SotnActors_GetPaletteBlend(s->actors, &blend) ||
            !SotnActorGraphics_BlendPalette(s->actor_graphics, blend) ||
            !SotnActors_GetPaletteRedShift(s->actors, &red) ||
            !SotnActorGraphics_ReddenPalette(s->actor_graphics, red) ||
            !SotnActorGraphics_TickMistPalette(s->actor_graphics) ||
            !SotnCameraShake_Tick(&s->shake, s->dra))
            return false;
        // Mist/Gas and the selected subweapons have no initialized flag10000 update.
        // Their controllers and pending contacts remain held, not reset or consumed.
    }
    if (!TickHudFrame(s) || !TickAudio(s, &combat, 0, 0, 0, 0))
        return false;
    s->dirty = s->targets_pending = true;
    return true;
}

static bool TickFairyFreeze(SotnLaboratory *s, Uint16 buttons)
{
    SotnFairyRuntimeInput input = {0};
    SotnPlayerState player = {0};
    SotnActorCombat combat = {0};
    if (!s->fairy_runtime.initialized || s->fairy_scene != 0xfff ||
        !FairyInput(s, 0xfff, s->camera.x, s->camera.y, &input))
        return SDL_SetError("FFF engine requires actual Fairy/card/resource ownership");
    if (!SotnPlayer_SetAfterimageContext(s->player, s->bat_runtime.common_generation,
                                         s->system_timer) ||
        !BindStatBuffs(s) || !SotnPlayer_SetDamageContext(s->player, s->game_timer) ||
        !TickPlayer(s, buttons) || !SotnPlayer_GetState(s->player, &player) ||
        !TickFairy(s, 0xfff, s->camera.x, s->camera.y) ||
        !SotnPlayer_GetState(s->player, &player) ||
        (s->fairy_scene != 0xfff && !SotnSceneFreeze_Request(&s->spell_freeze, s->fairy_scene)) ||
        !BindActorAudio(s) || !SotnActors_SetButtons(s->actors, buttons) ||
        !SotnActors_TickFrozen(s->actors, player, s->camera.x, s->camera.y) ||
        !SotnActors_GetCombat(s->actors, &combat) || !TickHudFrame(s) ||
        !TickAudio(s, &combat, player.sound, player.extra_sound, player.third_sound, 0))
        return false;
    // FFF omits shared8..63, group recount and camera/shake. Main-loop playtime follows later.
    // A callback clearing FFF still finishes this full frozen frame; release is next frame.
    s->dirty = s->targets_pending = true;
    return true;
}

static bool StepFrame(SotnLaboratory *session, Uint16 buttons)
{
    if (session == NULL)
        return SDL_InvalidParamError("laboratory");
    if (session->stage_pending)
        return true; // The application commits an arrival once, after its catch-up ticks.
    // 4DA70.c consumes the global tile once per prepared engine frame, including pause.
    session->engine_flash = session->engine_flash_frames != 0;
    if (session->engine_flash_frames)
        --session->engine_flash_frames;
    // Original render-target writes are simulation-frame side effects. Preserve a skipped
    // presentation's captures before the next actor update can replace their one-tick commands.
    if (session->targets_pending &&
        (SotnActors_HasTargets(session->actors) ||
         session->bat_runtime.wolf.environment_reserved) &&
        !DrawGameplay(session))
        return false;
    session->targets_pending = false;
    // Original palette jobs run after the previous frame, including pause. Apply
    // that update before this frame can change timers or capture current artwork.
    if (!SyncStatBuffs(session) || !SotnActorGraphics_TickPlayerGlow(session->actor_graphics) ||
        !SyncDarkPalette(session))
        return false;
    ++session->system_timer; // Original g_Timer advances through paused engine frames.
    session->blocked_buttons &= buttons;
    buttons &= (Uint16)~session->blocked_buttons;
    const bool select =
        (buttons & SOTN_BUTTON_SELECT) && !(session->scene_buttons & SOTN_BUTTON_SELECT);
    const bool start =
        (buttons & SOTN_BUTTON_START) && !(session->scene_buttons & SOTN_BUTTON_START);
    session->scene_buttons = buttons;
    if (session->launch_active)
    {
        // A render frame may run several ticks before the application consumes this request.
        if (session->launch_action == SOTN_LAUNCH_NONE &&
            !SotnLaunchMenu_Tick(&session->launcher, buttons, session->launch_available,
                                 &session->launch_action))
            return false;
        session->dirty = true;
        const SotnActorCombat silent = {0};
        return TickAudio(session, &silent, 0, 0, 0, 0);
    }

    if (session->game_over != NULL)
    {
        const SotnActorCombat combat = {0};
        if (!TickGameOver(session, start) || !TickAudio(session, &combat, 0, 0, 0, 0))
            return false;
        session->dirty = true;
        return true;
    }
    SotnPlayerState state = {0};
    if (!SotnPlayer_GetState(session->player, &state))
        return false;
    SotnSceneFreezeFrame spell_frame = SOTN_SCENE_NORMAL;
    if (!SotnSceneFreeze_BeginFrame(&session->spell_freeze, &spell_frame))
        return false;
    if (spell_frame == SOTN_SCENE_FAIRY)
        return TickFairyFreeze(session, buttons);
    if (spell_frame != SOTN_SCENE_NORMAL)
        return TickSpellFreeze(session, buttons, spell_frame);
    SotnActorControl control = {0};
    if (!SotnActors_GetControl(session->actors, &control))
        return false;
    const bool owns_save = session->save_room && session->save_point.phase != SOTN_SAVE_POINT_IDLE;
    if (session->map_open || (select && state.can_pause && !owns_save && !control.pause_disabled &&
                              !session->fairy_pause_disabled && control.timer == 0 &&
                              session->menu.phase == SOTN_MENU_CLOSED))
    {
        if (!session->map_open)
        {
            if (session->map_renderer == NULL &&
                (!SotnResource_Open(session->directory, SOTN_RESOURCE_CASTLE_MAP,
                                    &session->map_art) ||
                 !SotnMapRenderer_Create(SotnResource_View(session->map_art),
                                         SotnResource_View(session->common), session->dra,
                                         &session->map_renderer)))
                return false;
            session->map_open = true;
        }
        else if (start || select)
            session->map_open = false;
        session->map_opacity = (Uint8)(session->map_open ? SDL_min(64, session->map_opacity + 8)
                                                         : SDL_max(0, session->map_opacity - 8));
        const SotnActorCombat silent = {0};
        session->dirty = true;
        return TickAudio(session, &silent, 0, 0, 0, 0);
    }
    if (session->map_opacity)
        session->map_opacity = (Uint8)SDL_max(0, session->map_opacity - 8);

    if (session->menu.phase != SOTN_MENU_CLOSED ||
        (start && state.can_pause && !owns_save && !control.pause_disabled &&
         !session->fairy_pause_disabled && control.timer == 0))
    {
        Uint16 sound = 0;
        bool changed = false;
        const bool was_open = session->menu.phase != SOTN_MENU_CLOSED;
        if (!SotnInventoryMenu_Tick(&session->menu, session->character, buttons, &sound, &changed,
                                    &session->world, session->dra) ||
            (changed && !ApplyMenuCharacter(session)) ||
            (sound != 0 && !SotnSound_Queue(session->sound, sound)))
            return false;
        const bool is_open = session->menu.phase != SOTN_MENU_CLOSED;
        if (was_open && !is_open && !CloseMenuFamiliar(session))
            return false;
        if (session->fairy_cd && was_open != is_open)
        {
            const Uint16 enter[] = {0xa7, 0xa3, 0xe}, leave[] = {0xf, 0xa4, 0xa8};
            const Uint16 *commands = is_open ? enter : leave;
            for (Uint16 i = 0; i < SDL_arraysize(enter); ++i)
                if (!SotnCdAudio_Queue(session->fairy_cd, commands[i]))
                    return false;
        }
        const SotnActorCombat silent = {0};
        if (!TickAudio(session, &silent, 0, 0, 0, 0))
            return false;
        session->input.menu_open = session->menu.phase != SOTN_MENU_CLOSED;
        if (!session->input.menu_open)
            session->blocked_buttons |= buttons & SOTN_BUTTON_CANCEL;
        session->dirty = true;
        return true;
    }
    session->menu.previous = buttons;
    Uint16 player_buttons =
        session->save_room ? SotnSavePoint_Buttons(&session->save_point, state, buttons) : buttons;
    player_buttons &= (Uint16) ~(SOTN_BUTTON_START | SOTN_BUTTON_SELECT);
    session->simulation_ticks = SDL_min(session->simulation_ticks + 1, 99999ULL * 3600 * 60);
    ++session->player_effect_frame;
    ++session->game_timer;
    const SotnPlayerBlink *bat_effect = NULL;
    const SotnPlayerFactory *bat_factory = NULL;
    const SotnPlayerBlink *wolf_effect = NULL;
    const SotnPlayerFactory *wolf_factory = NULL;
    SotnWaterVolume water = {0};
    if (!SotnActors_GetWater(session->actors, &water) ||
        !SotnPlayer_SetWaterContext(session->player,
                                    (SotnPlayerWater){water.feet, water.waist, water.head}) ||
        !SotnBatRuntime_Context(&session->bat_runtime, state.bat.generation, &bat_effect,
                                &bat_factory) ||
        !SotnBatRuntime_Context(&session->bat_runtime, state.wolf.generation, &wolf_effect,
                                &wolf_factory))
        return false;
    const Uint16 script_mode = control.hold ? 2 : control.mode;
    if (!SotnPlayerStep_IsDead(state.step) &&
        (control.timer || script_mode || session->stage_script_mode) &&
        !SotnPlayer_SetSimulatedPad(
            session->player, (SotnPlayerSimulatedPad){.buttons = control.buttons,
                                                      .mode = script_mode,
                                                      .remaining = control.timer  ? control.timer
                                                                   : control.hold ? 1
                                                                                  : 0}))
        return false;
    session->stage_script_mode = script_mode;
    if (!SotnSubweapons_SetPaused(session->subweapons, control.block_stopwatch) ||
        !SotnPlayer_SetMistContext(
            session->player, session->mist_present ? &session->mist_effect : NULL,
            session->mist_generation, session->game_timer, water.waist != 0) ||
        !SotnPlayer_SetBatRuntimeContext(session->player, bat_effect, bat_factory,
                                         bat_effect || bat_factory ? state.bat.generation : 0,
                                         SotnBatRuntime_Available(&session->bat_runtime)) ||
        !SotnPlayer_SetWolfContext(session->player, &session->bat_runtime.wolf, wolf_effect,
                                   wolf_factory,
                                   wolf_effect || wolf_factory ? state.wolf.generation : 0,
                                   SotnBatRuntime_Available(&session->bat_runtime),
                                   session->camera.x, session->camera.y) ||
        !SotnPlayer_SetAfterimageContext(session->player, session->bat_runtime.common_generation,
                                         session->system_timer) ||
        !SotnPlayer_SetHellfireContext(session->player, HellfireFactory, session, session->camera.x,
                                       session->bat_runtime.hellfire_signal,
                                       session->bat_runtime.hellfire_palette) ||
        !SotnPlayer_SetSwordContext(session->player, SwordFactory, session,
                                    session->bat_runtime.children[0].kind != 0 ||
                                        session->mist_present) ||
        !SotnPlayer_SetHandContext(session->player, HandFactory, ThrowingRandom, session) ||
        !SotnPlayer_SetMedicineContext(session->player, MedicineFactory, UseMedicine, session,
                                       SotnBatRuntime_MedicinePresent(&session->bat_runtime)) ||
        !SotnPlayer_SetSubweaponContext(session->player, SubweaponUse, session) ||
        !BindFood(session) ||
        !SotnPlayer_SetTeleportContext(session->player, TeleportFactory, UseLibraryCard, session,
                                       session->room.left * 256) ||
        !SotnPlayer_SetStoneContext(session->player, DissolvePresent, session) ||
        !SotnPlayer_SetDeathCapture(session->player, CaptureDeathImage, DecayDeathImage, session) ||
        !BindStatBuffs(session) ||
        !SotnPlayer_SetDamageContext(session->player, session->game_timer) ||
        !TickPlayer(session, player_buttons) || !SotnPlayer_GetState(session->player, &state) ||
        (state.hud_hp_hold && !SotnHud_NotifyHp(session->hud)) ||
        (state.heart_gain.requested &&
         !SotnCharacter_AddHearts(session->character, state.heart_gain.amount)) ||
        (state.defense_buff_minimum &&
         (!SotnStatBuffs_Minimum(&session->stat_buffs, SOTN_BUFF_DEFENSE,
                                 state.defense_buff_minimum) ||
          !SyncStatBuffs(session))) ||
        (state.mp_spent && !SotnCharacter_SpendMP(session->character, state.mp_spent)) ||
        (state.muramasa_blood && !SotnCharacter_AddMuramasaBlood(session->character)))
        return false;
    // Player's casting grunt precedes the later factory/child callbacks and their sounds.
    // Keep an actual earlier guard voice ahead of it; the final audio pass must not replay it.
    if ((state.throwing.used || state.hand_used) &&
        ((state.guard.applied && state.guard.sound &&
          !QueueNativeSound(session, state.guard.sound)) ||
         (state.throwing_grunt && !QueueNativeSound(session, state.throwing_grunt)) ||
         (state.hand_grunt && !QueueNativeSound(session, state.hand_grunt))))
        return false;
    if (state.dark.started || state.spirit.started || state.hellfire_started || state.soul.started)
    {
        SotnCharacterState learned = {0};
        if (!SotnCharacter_Get(session->character, &learned, NULL) ||
            !SotnCharacter_LearnSpell(&learned, state.spirit.started     ? state.spirit.spell
                                                : state.hellfire_started ? 2
                                                : state.soul.started     ? 5
                                                                         : 0) ||
            !SotnCharacter_Restore(session->character, &learned))
            return false;
    }
    if (state.bat_commands.learn_wing_smash)
    {
        SotnCharacterState learned = {0};
        if (!SotnCharacter_Get(session->character, &learned, NULL) ||
            !SotnCharacter_LearnSpell(&learned, 6) ||
            !SotnCharacter_Restore(session->character, &learned))
            return false;
    }
    if (state.wolf.commands.learn_charge)
    {
        SotnCharacterState learned = {0};
        if (!SotnCharacter_Get(session->character, &learned, NULL) ||
            !SotnCharacter_LearnSpell(&learned, 4) ||
            !SotnCharacter_Restore(session->character, &learned))
            return false;
    }
    if (session->stage_pending)
    {
        session->dirty = true;
        return true;
    }
    if (!TickActors(session) || !TickCape(session) || !TickSaveRoom(session, buttons) ||
        !EnterSpellFreeze(session))
        return false;
    SotnActorCombat combat = {0};
    if (!SotnPlayer_GetState(session->player, &state) ||
        !SotnActors_GetCombat(session->actors, &combat) || !TickHudFrame(session) ||
        !TickAudio(session, &combat, state.sound, state.extra_sound, state.third_sound,
                   state.guard.applied && !state.throwing.used && !state.hand_used
                       ? state.guard.sound
                       : 0))
        return false;
    if (!SotnPlayerStep_IsDead(state.step) && !session->spell_freeze.active &&
        !session->bat_runtime.teleport_warp_mode)
    {
        // RunMainEngine performs player/DRA/stage callbacks before room lookup.
        // Freeze and actual teleport requests preempt it. Preserve departing audio;
        // entering initializes the destination without a second shared/player update.
        const Uint32 previous_room = session->room_index;
        if (!Transition(session, &state))
            return false;
        if (!session->stage_pending && session->room_index != previous_room)
        {
            SotnActorCombat entered = {0};
            if (!UpdateActors(session, true) || !SotnActors_GetCombat(session->actors, &entered) ||
                !QueueAudio(session, &entered, 0, 0, 0, 0))
                return false;
        }
    }
    if (!SotnCameraShake_Tick(&session->shake, session->dra))
        return false;
    if (session->bat_runtime.teleport_warp_mode)
    {
        SotnStageRoute route = {0};
        if (!SotnStageRoute_Teleport(session->dra, session->bat_runtime.teleport_warp_mode,
                                     &route) ||
            !QueueStageArrival(session, route))
            return false;
        session->teleport_mode = session->bat_runtime.teleport_warp_mode;
        // The departing engine requests FFFF white frames before resource preparation.
        session->engine_flash = true;
        session->engine_flash_frames = 65534;
    }
    if (state.death.phase == 128)
    {
        session->dirty = true;
        if (!SotnLaboratory_Draw(session) ||
            !SotnGameOver_Create(SotnResource_View(session->game_over_art), session->frame,
                                 SOTN_FRAME_PIXELS, &session->game_over))
            return false;
        if (session->fairy_cd)
        {
            if (session->music || session->speech)
                return SDL_SetError("Bound game-over audio lost its single CD decoder owner");
            session->game_over_audio_phase = 0;
        }
        else
        {
            SotnMusic_Destroy(session->music);
            session->music = NULL;
            if (!SotnMusic_CreateGameOver(session->dra, session->music_stream, &session->music))
                return false;
        }
        if (!SotnPlayback_Reset(session->audio))
            return false;
        SDL_Log("Alucard died: original game-over presentation. Enter/gamepad Start advances after "
                "the reveal; Continue returns through the native menu. Original title/card menus "
                "remain unintegrated.");
    }
    session->dirty = true;
    session->targets_pending = true;
    return true;
}

bool SotnLaboratory_Step(SotnLaboratory *session, Uint16 buttons)
{
    if (!session)
        return SDL_InvalidParamError("laboratory");
    if (session->stage_pending)
        return true;
    if (!StepFrame(session, buttons))
        return false;
    // Each committed native fixed60 engine frame supplies one SDK frame quantum.
    // The original MainGame clock follows UpdateGame/render preparation, including
    // frozen/release/FFF/dead frames; post-update menu/map flags hold it.
    SotnCharacterState profile = {0};
    if (!SotnCharacter_Get(session->character, &profile, NULL))
        return false;
    SotnMainLoopTail next = {session->playtime, session->map_cursor, profile.random};
    if (!SotnMainLoop_Advance(&next, 256, session->menu.phase != SOTN_MENU_CLOSED,
                              session->map_open) ||
        !SotnCharacter_CommitMainLoop(session->character, next.playtime, next.random))
        return false;
    // The original map packet samples before the tail increments its global cursor.
    session->map_draw_cursor = session->map_cursor;
    session->map_cursor = next.map_cursor;
    session->playtime = next.playtime;
    return true;
}

static bool DrawGravityBeams(SotnLaboratory *s, SotnPlayerState p, Sint32 camera_x, Sint32 camera_y)
{
    for (Uint32 i = 0; i < SOTN_PLAYER_GRAVITY_BEAMS; ++i)
    {
        const SotnPlayerGravityBeam b = p.gravity.beams[i];
        if (!b.active)
            continue;
        for (Uint32 part = 0; part < 4; ++part)
        {
            const Sint32 half = SDL_max(0, (Sint32)(b.width / 256) - (Sint32)part);
            const Sint32 start_x = Pixel(b.x) - camera_x, start_y = Pixel(b.y) - camera_y;
            const Sint32 end_x = Pixel(p.x) - camera_x, end_y = Pixel(p.y) - camera_y + 12;
            const SotnTextureQuad q = {
                .x = {start_x - half, start_x + half, end_x - half, end_x + half},
                .y = {start_y, start_y, end_y, end_y},
                .page = 0x20,
                .blend = true,
                .gouraud = true,
                .untextured = true,
                .vertex_red = {0, 0, 64, 64},
                .vertex_green = {0, 0, 64, 64},
                .vertex_blue = {192, 192, 64, 64}};
            if (!SotnActorGraphics_Quad(s->actor_graphics, q, s->frame, SOTN_FRAME_PIXELS))
                return false;
        }
    }
    return true;
}

static SotnActorSurface MainSurface(SotnLaboratory *session)
{
    return (SotnActorSurface){session->frame, SOTN_FRAME_PIXELS, SOTN_FRAME_WIDTH,
                              SOTN_FRAME_HEIGHT};
}
static bool SwitchTarget(SotnLaboratory *s, SotnActorPrimitive p)
{
    return SotnActorGraphics_ApplyTarget(s->actor_graphics, MainSurface(s), p, &s->draw_target);
}

static bool DrawPrimitiveRange(SotnLaboratory *session, SotnActorState actor, Sint32 camera_x,
                               Sint32 camera_y, Uint32 begin, Uint32 limit)
{
    const SotnActorSurface surface = session->draw_target.surface;
    if (begin > limit || limit > actor.primitive_count)
        return SDL_SetError("Actor draw range exceeds its owned primitives");
    for (Uint32 i = begin; i < limit; ++i)
    {
        SotnActorPrimitive p = {0};
        if (!SotnActors_GetPrimitive(session->actors, actor.slot, i, &p))
            return false;
        if (!p.active)
            continue;
        if (p.clip || p.target)
            return SDL_SetError("Actor draw-area changes require ordered stage presentation");
        const Sint32 shake_x = p.camera_fixed ? 0 : session->shake.x,
                     shake_y = p.camera_fixed ? 0 : session->shake.y;
        const Sint32 x = Pixel(p.x) - (p.screen_space ? 0 : camera_x) + shake_x +
                         session->draw_target.offset_x,
                     y = Pixel(p.y) - (p.screen_space ? 0 : camera_y) + shake_y +
                         session->draw_target.offset_y;
        if (p.line)
        {
            if (p.textured)
                return SDL_SetError("Actor line must use endpoint colors rather than a texture");
            const SotnColorLine line = {.x = {x + p.offset_x[0], x + p.offset_x[1]},
                                        .y = {y + p.offset_y[0], y + p.offset_y[1]},
                                        .red = {p.red, p.line_red},
                                        .green = {p.green, p.line_green},
                                        .blue = {p.blue, p.line_blue},
                                        .blend = p.blend,
                                        .blend_mode = p.line_blend_mode};
            if (!SotnActorGraphics_LineSurface(line, surface))
                return false;
            continue;
        }
        Sint32 left = p.textured ? x - p.width / 2 : x, top = p.textured ? y - p.height / 2 : y;
        Sint32 right = left + p.width, bottom = top + p.height;
        if (p.quad)
        {
            left = right = x + p.offset_x[0];
            top = bottom = y + p.offset_y[0];
            for (Uint32 vertex = 1; vertex < 4; ++vertex)
            {
                left = SDL_min(left, x + p.offset_x[vertex]);
                right = SDL_max(right, x + p.offset_x[vertex]);
                top = SDL_min(top, y + p.offset_y[vertex]);
                bottom = SDL_max(bottom, y + p.offset_y[vertex]);
            }
        }
        if (right <= 0 || left >= surface.width || bottom <= 0 || top >= surface.height)
            continue;
        if (p.textured || p.quad)
        {
            SotnTextureQuad q = {.x = {left, right, left, right},
                                 .y = {top, top, bottom, bottom},
                                 .page = p.page,
                                 .palette = p.palette,
                                 .blend = p.blend,
                                 .tint = p.modulate,
                                 .red = p.red,
                                 .green = p.green,
                                 .blue = p.blue,
                                 .gouraud = p.gouraud,
                                 .framebuffer = p.framebuffer,
                                 .untextured = !p.textured};
            for (Uint32 uv = 0; uv < 4; ++uv)
            {
                if (p.quad)
                {
                    q.x[uv] = x + p.offset_x[uv];
                    q.y[uv] = y + p.offset_y[uv];
                }
                q.u[uv] = p.u[uv];
                q.v[uv] = p.v[uv];
                q.vertex_red[uv] = p.vertex_red[uv];
                q.vertex_green[uv] = p.vertex_green[uv];
                q.vertex_blue[uv] = p.vertex_blue[uv];
            }
            if (p.framebuffer && !SotnActorGraphics_CaptureFrame(session->actor_graphics,
                                                                 session->frame, SOTN_FRAME_PIXELS))
                return false;
            if (!SotnActorGraphics_QuadSurface(session->actor_graphics, q, surface))
                return false;
        }
        else
        {
            const Uint16 front =
                (Uint16)((p.red / 8) | ((p.green / 8) << 5) | ((p.blue / 8) << 10));
            const Sint32 start = SDL_max(0, left), end = SDL_min(surface.width, right);
            for (Sint32 row = SDL_max(0, top); row < SDL_min(surface.height, bottom); ++row)
            {
                if (p.blend && start < end)
                {
                    const SotnColorLine tile = {.x = {start, end - 1},
                                                .y = {row, row},
                                                .red = {p.red, p.red},
                                                .green = {p.green, p.green},
                                                .blue = {p.blue, p.blue},
                                                .blend = true,
                                                .blend_mode = (Uint8)((p.page >> 5) & 3U)};
                    if (!SotnActorGraphics_LineSurface(tile, surface))
                        return false;
                }
                else
                    for (Sint32 col = start; col < end; ++col)
                        surface.pixels[(size_t)row * surface.width + (size_t)col] = front;
            }
        }
    }
    return true;
}

static bool DrawActorPrimitives(SotnLaboratory *session, SotnActorState actor, Sint32 camera_x,
                                Sint32 camera_y)
{
    return DrawPrimitiveRange(session, actor, camera_x, camera_y, 0, actor.primitive_count);
}

static bool ActorDrawError(SotnActorState actor)
{
    char detail[256] = {0};
    SDL_strlcpy(detail, SDL_GetError(), sizeof(detail));
    return SDL_SetError("Actor 0x%x slot %u bank 0x%x frame 0x%x palette 0x%x: %s", actor.id,
                        actor.slot, actor.animation_set, actor.frame, actor.palette, detail);
}

// Selected LIB parallax precedes foreground tiles; Chapel/BO5 sky precedes both tile layers.
// Original later-submitted primitive packets precede tile packets at the tied bucket.
static bool DrawPlayerEffectPolygon(SotnLaboratory *s, const SotnPlayerEffectQuad *p, bool triangle)
{
    const SotnActorSurface surface =
        s->draw_target.mode
            ? s->draw_target.surface
            : (SotnActorSurface){s->frame, SOTN_FRAME_PIXELS, SOTN_FRAME_WIDTH, SOTN_FRAME_HEIGHT};
    const bool fixed = (p->draw_mode & 0x2080U) != 0;
    return (triangle ? SotnActorGraphics_PlayerTriangle : SotnActorGraphics_PlayerEffect)(
        s->actor_graphics, p, s->draw_target.offset_x + (fixed ? 0 : s->shake.x),
        s->draw_target.offset_y + (fixed ? 0 : s->shake.y), surface);
}
static bool DrawPlayerEffectQuad(SotnLaboratory *s, const SotnPlayerEffectQuad *p)
{
    return DrawPlayerEffectPolygon(s, p, false);
}
// Weapon secondaries build vertices from original private rows, some past their tables; those
// follow the GPU's vertex wrap and span rule instead of the native fail-fast bound.
static bool DrawOriginalEffectQuad(SotnLaboratory *s, const SotnPlayerEffectQuad *p)
{
    const SotnActorSurface surface =
        s->draw_target.mode
            ? s->draw_target.surface
            : (SotnActorSurface){s->frame, SOTN_FRAME_PIXELS, SOTN_FRAME_WIDTH, SOTN_FRAME_HEIGHT};
    const bool fixed = (p->draw_mode & 0x2080U) != 0;
    return SotnActorGraphics_PlayerEffectGpu(
        s->actor_graphics, p, s->draw_target.offset_x + (fixed ? 0 : s->shake.x),
        s->draw_target.offset_y + (fixed ? 0 : s->shake.y), surface);
}
// The cross's rows keep original screen words (no DRAW_UNK02 or FLAG_POS_CAMERA_LOCKED).
// Beam: the uploaded gradient as a 15-bit direct page, additive, grey once it spreads. Giant
// cross faces: page 0x1C, private UVs/palettes, opaque. Sparks: page 0x1A, palette 0x1B0.
static bool DrawCrossRow(SotnLaboratory *s, const SotnSubweaponEntity *m, Uint16 row)
{
    SotnPlayerEffectQuad q = {0};
    if (m->id == SOTN_SUBWEAPON_CROSS)
    {
        const SotnCrossBeam *b = &m->beam;
        if (row)
            return SDL_SetError("Cross beam draw names a row it does not own");
        q = (SotnPlayerEffectQuad){.x = {b->x0, b->x1, b->x0, b->x1},
                                   .y = {b->y0, b->y0, b->y2, b->y2},
                                   .u = {1, 0x30, 1, 0x30},
                                   .v = {0xf8, 0xf8, 0xf8, 0xf8},
                                   .red = {b->grey, b->grey, b->grey, b->grey},
                                   .green = {b->grey, b->grey, b->grey, b->grey},
                                   .blue = {b->grey, b->grey, b->grey, b->grey},
                                   .page = 0x11c,
                                   .priority = m->priority,
                                   .draw_mode = b->draw_mode};
    }
    else if (m->id == SOTN_GIANT_CROSS)
    {
        if (row >= SOTN_GIANT_CROSS_ROWS || !((m->cross.visible >> row) & 1U) ||
            !SotnSubweaponEntity_CrossFace(s->dra, row, q.u, q.v, &q.palette))
            return SDL_SetError("Giant cross draw lost its visible face %u", row);
        for (Uint16 v = 0; v < 4; ++v)
        {
            q.x[v] = m->cross.x[row][v];
            q.y[v] = m->cross.y[row][v];
        }
        q.page = 0x1c;
        q.priority = m->cross.priority;
    }
    else
    {
        const SotnCrossParticle *p = row < SOTN_CROSS_PARTICLE_ROWS ? &m->sparks.rows[row] : NULL;
        if (!p || !p->visible || !SotnSubweaponEntity_SparkFace(s->dra, p->shown, q.u, q.v))
            return SDL_SetError("Cross particle draw lost its shown spark %u", row);
        const Sint16 size = p->shown >= 3 ? 4 : 6;
        for (Uint16 v = 0; v < 4; ++v)
        {
            q.x[v] = (Sint16)(p->x + ((v & 1U) ? size : -size));
            q.y[v] = (Sint16)(p->placed + ((v & 2U) ? size : -size));
        }
        q.page = 0x1a;
        q.palette = 0x1b0;
        q.priority = p->priority;
    }
    return DrawOriginalEffectQuad(s, &q);
}
static bool DrawIceQuad(SotnLaboratory *s, const SotnPlayerEffectQuad *p)
{
    const bool fixed = (p->draw_mode & 0x2080U) != 0;
    const Sint32 x = s->draw_target.offset_x + (fixed ? 0 : s->shake.x),
                 y = s->draw_target.offset_y + (fixed ? 0 : s->shake.y);
    SotnTextureQuad q = {.page = (Uint16)(p->draw_mode & 0x60U),
                         .blend = (p->draw_mode & 1U) != 0,
                         .gouraud = true,
                         .untextured = true};
    for (Uint16 v = 0; v < 4; ++v)
    {
        q.x[v] = p->x[v] + x;
        q.y[v] = p->y[v] + y;
        q.vertex_red[v] = p->red[v];
        q.vertex_green[v] = p->green[v];
        q.vertex_blue[v] = p->blue[v];
    }
    return (p->draw_mode & 8U) || SotnActorGraphics_QuadSurface(NULL, q, s->draw_target.surface);
}
static bool DrawTeleportRow(SotnLaboratory *s, const SotnTeleportEffect *effect, Uint16 row)
{
    const SotnPlayerEffectQuad *q = &effect->quads[row];
    if (row >= 4)
        return DrawPlayerEffectQuad(s, q);
    if (row >= 2)
        return DrawIceQuad(s, q);
    SotnPlayerEffectQuad tile = *q;
    for (Uint16 v = 0; v < 4; ++v)
    {
        tile.x[v] = BlinkWord(q->x[0] + (v & 1U ? q->u[0] : 0));
        tile.y[v] = BlinkWord(q->y[0] + (v & 2U ? q->v[0] : 0));
        tile.red[v] = q->red[0];
        tile.green[v] = q->green[0];
        tile.blue[v] = q->blue[0];
    }
    return DrawIceQuad(s, &tile);
}
static bool DrawAxeArmorSprite(SotnLaboratory *s, SotnActorSprite sprite, Sint32 x, Sint32 y,
                               Sint16 rotation, Sint16 pivot_y, Uint8 opacity, bool rotated)
{
    const SotnDataView weapons = SotnResource_View(s->weapons);
    SotnEquipment armor = {0}, hand = {0};
    if (!SotnEquipment_ReadAxeArmor(s->dra, &armor) ||
        !SotnEquipment_Read(s->dra, s->hands[0], &hand) ||
        !SotnActorGraphics_LoadWeapon(s->actor_graphics, weapons, 0, armor))
        return false;
    const bool drawn = SotnActorGraphics_DrawSurface(
        s->actor_graphics, sprite, x + s->draw_target.offset_x + s->shake.x,
        y + s->draw_target.offset_y + s->shake.y,
        (SotnSpriteTransform){.rotation = rotation,
                              .scale_x = 256,
                              .scale_y = 256,
                              .opacity = opacity,
                              .executable = s->executable,
                              .pivot_y = pivot_y,
                              .sdk_rotation = rotated,
                              .original_uv = true},
        s->draw_target.surface);
    if (!drawn)
        return false;
    return SotnActorGraphics_LoadWeapon(s->actor_graphics, weapons, 0, hand);
}
static bool DrawAxeArmorPlayer(SotnLaboratory *s, SotnPlayerState p)
{
    if (!p.axe_armor.active || p.animation_bank != 16)
        return SDL_SetError("Axe Armor body lost its actual equipped artwork owner");
    return DrawAxeArmorSprite(s,
                              (SotnActorSprite){.animation_set = 16,
                                                .frame = p.animation.sprite,
                                                .palette = p.palette,
                                                .texture_offset = 0x64,
                                                .facing_left = p.facing_left},
                              Pixel(p.x) - s->camera.x, Pixel(p.y) - s->camera.y,
                              (p.axe_armor.draw_flags & 4U) ? p.axe_armor.rotation : 0,
                              p.draw_pivot_y,
                              (p.axe_armor.draw_flags & 8U) ? p.axe_armor.opacity : 128,
                              (p.axe_armor.draw_flags & 4U) != 0);
}

static bool PrepareWolfPresentation(SotnLaboratory *s, SotnPlayerState p, Sint32 camera_x,
                                    Sint32 camera_y)
{
    s->wolf_packet_count = 0;
    const SotnWolfRuntime *wolf = &s->bat_runtime.wolf;
    if (!wolf->environment_reserved)
        return true;
    // Repeated/paused draws must sample the same previous simulation frame, even
    // after this frame's ordered environment has written the next capture.
    SotnActorSurface target = {0};
    if (!SotnActorGraphics_BeginTarget(
            s->actor_graphics, (SotnActorTarget){.x = 512, .y = 448, .width = 63, .height = 63},
            &target))
        return false;
    const bool snapshot = !s->wolf_capture_ready || s->wolf_capture_tick != s->player_effect_frame;
    if (snapshot)
        SDL_memcpy(s->wolf_previous_capture, target.pixels, sizeof(s->wolf_previous_capture));
    else
        SDL_memcpy(target.pixels, s->wolf_previous_capture, sizeof(s->wolf_previous_capture));
    if (!SotnActorGraphics_EndTarget(s->actor_graphics, !snapshot))
        return false;
    s->wolf_capture_ready = true;
    s->wolf_capture_tick = s->player_effect_frame;
    if (!wolf->generation && p.animation_bank != 15)
        return true;
    for (Uint16 i = 0; i < 15; ++i)
    {
        const Uint16 slot = i ? (Uint16)(i + 16U) : 0;
        if (i && !wolf->generation)
            continue;
        if (!i && p.animation_bank != 15)
            continue;
        if (i && (!wolf->children[i - 1U].generation ||
                  wolf->children[i - 1U].lineage != wolf->generation ||
                  wolf->children[i - 1U].slot != slot))
            return SDL_SetError("Wolf presentation lost its actual body lineage");
        SotnWolfPart part = i ? wolf->body.parts[i - 1U]
                              : (SotnWolfPart){.x = p.x - camera_x * 65536,
                                               .y = p.y - camera_y * 65536,
                                               .bank = 15,
                                               .sprite = p.animation.sprite,
                                               .texture_offset = 126,
                                               .palette = p.wolf.palette,
                                               .priority = p.priority,
                                               .rotation = p.wolf.rotation,
                                               .draw_flags = p.wolf.draw_flags,
                                               .blend = p.wolf.blend,
                                               .opacity = p.wolf.opacity,
                                               .facing_left = p.facing_left};
        // RenderEntities adds the shared offset before both normal and capture packets.
        const Uint32 shifted_x = (Uint32)((Sint64)part.x + (Sint64)s->shake.x * 65536),
                     shifted_y = (Uint32)((Sint64)part.y + (Sint64)s->shake.y * 65536);
        part.x = (Sint32)(shifted_x <= SDL_MAX_SINT32 ? (Sint64)shifted_x
                                                      : (Sint64)shifted_x - 4294967296LL);
        part.y = (Sint32)(shifted_y <= SDL_MAX_SINT32 ? (Sint64)shifted_y
                                                      : (Sint64)shifted_y - 4294967296LL);
        size_t count = 0;
        if (!SotnWolfPresentation_Packets(
                s->dra, s->executable,
                (SotnWolfDrawInput){.part = part,
                                    .player_x = BlinkWord(Pixel(p.x) - camera_x),
                                    .player_y = BlinkWord(Pixel(p.y) - camera_y),
                                    .player_priority = p.priority,
                                    .slot = slot,
                                    .available = 768,
                                    .game_timer = s->system_timer},
                s->wolf_packets + s->wolf_packet_count,
                SDL_arraysize(s->wolf_packets) - s->wolf_packet_count, &count))
            return false;
        s->wolf_packet_count += count;
    }
    return true;
}
static bool DrawRotatedPlayer(SotnLaboratory *s, SotnPlayerState player,
                              const SotnAlucardSprite *sprite, Uint16 blend)
{
    if (!sprite->width)
        return true;
    if (!player.water_death.captured && !player.death.captured &&
        !(player.animation.sprite & 0x8000U) &&
        !SotnActorGraphics_UploadPlayer(s->actor_graphics, sprite))
        return false;
    const Sint32 x = Pixel(player.x) - s->camera.x, y = Pixel(player.y) - s->camera.y,
                 left = player.facing_left ? x - sprite->pivot_x - sprite->width + 1
                                           : x + sprite->pivot_x;
    SotnPlayerEffectQuad q = {.page = 0x18,
                              .palette = (Uint16)(player.palette & 0x7fffU),
                              .draw_mode = (Uint16)((blend & 0x60U) | (blend & 0x10U ? 1U : 0U))};
    for (Uint16 v = 0; v < 4; ++v)
    {
        q.x[v] = BlinkWord(left + ((v & 1U) ? sprite->width : 0));
        q.y[v] = BlinkWord(y + sprite->pivot_y + ((v & 2U) ? sprite->height : 0));
        q.u[v] = (Uint8)(player.facing_left ? ((v & 1U) ? 3 : 3 + sprite->width)
                                            : ((v & 1U) ? 4 + sprite->width : 4));
        q.v[v] = (Uint8)(1 + ((v & 2U) ? sprite->height : 0));
        if (player.gravity.upside_down)
        {
            const Sint32 local_x = sprite->pivot_x + ((v & 1U) ? sprite->width : 0);
            q.x[v] = BlinkWord(x + (player.facing_left ? local_x : 1 - local_x));
            q.y[v] = BlinkWord(y + 5 - sprite->pivot_y - ((v & 2U) ? sprite->height : 0));
            q.u[v] = (Uint8)((v & 1U) ? 4 + sprite->width : 4);
        }
    }
    if (player.water_death.active)
    {
        const SotnWaterDeath d = player.water_death.motion;
        q.draw_mode =
            (Uint16)((d.blend & 0x60U) | (d.blend & 0x10U ? 1U : 0U) | (d.color_blend ? 4U : 0U));
        for (Uint16 v = 0; v < 4; ++v)
        {
            q.red[v] = q.green[v] = d.red;
            q.blue[v] = d.blue;
        }
        if (player.rotated &&
            !SotnPlayerEffect_Rotate(
                s->executable,
                BlinkWord(x + (player.facing_left ? -player.draw_pivot_x : player.draw_pivot_x)),
                BlinkWord(y + player.draw_pivot_y), BlinkWord(d.rotation), player.facing_left, &q))
            return false;
    }
    else if (player.death.native)
    {
        const SotnPlayerDeath d = player.death;
        q.draw_mode =
            (Uint16)((d.blend & 0x60U) | (d.blend & 0x10U ? 1U : 0U) | (d.color_blend ? 4U : 0U));
        for (Uint16 v = 0; v < 4; ++v)
        {
            q.red[v] = d.red;
            q.green[v] = d.variant == 3 ? d.red : d.blue;
            q.blue[v] = d.blue;
        }
    }
    else if (player.body_color_blend)
    {
        q.draw_mode |= 4;
        for (Uint16 v = 0; v < 4; ++v)
            q.red[v] = q.green[v] = q.blue[v] = player.body_grey[v];
    }
    if (!player.water_death.active && player.rotated &&
        !SotnPlayerEffect_Rotate(
            s->executable,
            BlinkWord(x + (player.facing_left ? -player.draw_pivot_x : player.draw_pivot_x)),
            BlinkWord(y + player.draw_pivot_y),
            player.fairy_rescue.recovered ? player.fairy_rescue.rotation
            : player.death.native         ? player.death.rotation
            : player.stone.active         ? player.stone.rotation
                                          : player.bat.rotation,
            player.facing_left, &q))
        return false;
    return DrawPlayerEffectQuad(s, &q);
}

static bool DrawMistQuad(SotnLaboratory *s, Uint16 index)
{
    if (!s->mist_present || index >= 80)
        return SDL_SetError("Mist draw command lost its owned visual primitive");
    const SotnMistQuad *p = &s->mist_effect.quads[index];
    if (p->hidden)
        return true;
    SotnTextureQuad q = {.page = (Uint16)(p->page + 0x20U),
                         .palette = p->palette,
                         .blend = true,
                         .modulate = true,
                         .gouraud = true};
    for (Uint16 i = 0; i < 4; ++i)
    {
        q.x[i] = p->x[i] + s->draw_target.offset_x + s->shake.x;
        q.y[i] = p->y[i] + s->draw_target.offset_y + s->shake.y;
        q.u[i] = p->u[i];
        q.v[i] = p->v[i];
        q.vertex_red[i] = q.vertex_green[i] = q.vertex_blue[i] = p->grey[i];
    }
    const SotnActorSurface surface =
        s->draw_target.mode
            ? s->draw_target.surface
            : (SotnActorSurface){s->frame, SOTN_FRAME_PIXELS, SOTN_FRAME_WIDTH, SOTN_FRAME_HEIGHT};
    return SotnActorGraphics_QuadSurface(s->actor_graphics, q, surface);
}

static bool DrawAfterimages(SotnLaboratory *s, SotnPlayerState p, Sint32 camera_x, Sint32 camera_y)
{
    if (!p.afterimage_generation || p.afterimage_generation != s->bat_runtime.common_generation)
        return SDL_SetError("Afterimage displays lost their actual common stage lease");
    // Original addPrim reverses equal-priority entity slots: 3,2,1 after later stage/weapon slots.
    for (Uint16 i = 3; i > 0; --i)
    {
        const SotnAfterimageDisplay d = p.afterimage.displays[i - 1U];
        if (!(d.sprite & 0x7fffU))
            continue;
        SotnAlucardSprite image = {0};
        SotnPlayerEffectQuad q = {0};
        if (!SotnAlucard_DecodeSprite(s->dra, SotnResource_View(s->arc), d.sprite & 0x7fffU,
                                      &image) ||
            !SotnActorGraphics_Afterimage(s->actor_graphics, &image, d,
                                          Pixel(d.x) - camera_x + s->shake.x,
                                          Pixel(d.y) - camera_y + s->shake.y, &q) ||
            !SotnActorGraphics_PlayerEffect(s->actor_graphics, &q, s->draw_target.offset_x,
                                            s->draw_target.offset_y, s->draw_target.surface))
            return false;
    }
    return true;
}
static bool DrawActorsAndPlayer(SotnLaboratory *session, SotnPlayerState player, Sint32 camera_x,
                                Sint32 camera_y, bool background)
{
    const Sint32 view_x = camera_x - session->shake.x, view_y = camera_y - session->shake.y;
    if (session->stage_id == 26)
    {
        SotnActorState body = {0};
        if (!SotnActors_Get(session->actors, 0, &body))
            return false;
        if (body.generation && body.id == 0x16 && !body.unsupported && (body.frame & 0x7fffU))
        {
            const SotnActorSprite sprite = {body.animation_set,  body.frame,      body.palette,
                                            body.texture_offset, body.blend_mode, body.facing_left};
            if (body.animation_set == 0x8002 &&
                !SotnActorGraphics_UpdateDoppleBat(session->actor_graphics, body.frame,
                                                   body.bat_offset))
                return false;
            SotnDataView erased = {0};
            if (!SotnActors_DoppleDeathMask(session->actors, &erased))
                return false;
            if (erased.size)
            {
                if (!SotnActorGraphics_DoppleDissolve(session->actor_graphics, sprite, erased))
                    return false;
            }
            else
            {
                SotnActorSprite displayed = sprite;
                if (body.step == 17)
                    displayed.frame &= 0x7fffU;
                if (!SotnActorGraphics_LoadDopplePose(session->actor_graphics, displayed))
                    return false;
            }
        }
    }
    SotnActorState list[SOTN_STAGE_ACTOR_SLOTS] = {0};
    Uint32 count = 0;
    for (Uint32 slot = SOTN_STAGE_ACTOR_SLOTS; slot > 0; --slot)
    {
        SotnActorState state = {0};
        if (!SotnActors_Get(session->actors, slot - 1, &state))
            return false;
        // BO5's dialogue packets (509..511) must cover the original HUD (494..496).
        if (session->stage_id == 25 && state.id == 0x2a)
            continue;
        const bool background_actor =
            (session->stage_id == 2 && state.id >= 0x16 && state.id <= 0x19) ||
            ((session->stage_id == 6 || session->stage_id == 25) &&
             (state.id == 0x1f || state.id == 0x20));
        if (background_actor != background)
            continue;
        if (state.generation == 0 || state.unsupported ||
            (state.frame == 0 && state.id != 0xa &&
             !(session->stage_id == 65 && state.id == 0x59) && state.primitive_count == 0) ||
            (state.id == 0xa &&
             (state.step < 2 || (state.step == 4 && (session->simulation_ticks & 2)))))
            continue;
        Uint32 position = count;
        while (position > 0 && list[position - 1].priority > state.priority)
        {
            list[position] = list[position - 1];
            --position;
        }
        list[position] = state;
        ++count;
    }
    size_t draw_count = 0;
    for (Uint16 i = 0; i < count; ++i)
    {
        bool split = false;
        for (Uint16 part = 0; part < list[i].primitive_count; ++part)
        {
            SotnActorPrimitive primitive = {0};
            if (!SotnActors_GetPrimitive(session->actors, list[i].slot, part, &primitive))
                return false;
            split = split || primitive.priority_set;
        }
        if (split)
            for (Uint16 part = 0; part < list[i].primitive_count; ++part)
            {
                SotnActorPrimitive primitive = {0};
                if (!SotnActors_GetPrimitive(session->actors, list[i].slot, part, &primitive))
                    return false;
                if (primitive.active &&
                    !SotnActorOrder_Add(
                        session->draw_items, SDL_arraysize(session->draw_items), &draw_count,
                        (SotnActorDraw){.priority = primitive.priority_set ? primitive.priority
                                                                           : list[i].priority,
                                        .actor = i,
                                        .primitive = part,
                                        .kind = SOTN_DRAW_PRIMITIVE}))
                    return false;
            }
        if (!SotnActorOrder_Add(
                session->draw_items, SDL_arraysize(session->draw_items), &draw_count,
                (SotnActorDraw){.priority = list[i].priority,
                                .actor = i,
                                .kind = split ? SOTN_DRAW_SPRITE : SOTN_DRAW_ACTOR}))
            return false;
    }
    if (!background)
        for (Uint16 i = SotnActors_FoodTextRows(session->actors); i > 0; --i)
        {
            SotnActorPrimitive row = {0};
            if (!SotnActors_GetFoodText(session->actors, (Uint16)(i - 1), &row) ||
                !SotnActorOrder_Add(session->draw_items, SDL_arraysize(session->draw_items),
                                    &draw_count,
                                    (SotnActorDraw){.priority = row.priority,
                                                    .primitive = (Uint16)(i - 1),
                                                    .kind = SOTN_DRAW_FOOD_TEXT}))
                return false;
        }
    if (!background && session->fairy_runtime.initialized)
        for (Uint16 index = 0; index < 4; ++index)
        {
            const SotnFairyRuntimeSlot *c = &session->fairy_runtime.slots[index];
            if (!c->identity)
                continue;
            if (!c->generation)
                return SDL_SetError("Fairy draw lost its actual slot generation");
            if (c->identity >= SOTN_FAIRY_FOLLOW && c->identity <= SOTN_FAIRY_HINT)
            {
                if (c->cursor.fairy.animation.sprite &&
                    !SotnActorOrder_Add(session->draw_items, SDL_arraysize(session->draw_items),
                                        &draw_count,
                                        (SotnActorDraw){.priority = c->source.priority,
                                                        .actor = index,
                                                        .kind = SOTN_DRAW_FAIRY_SPRITE}))
                    return false;
            }
            else if (c->lease.count)
            {
                if (c->lease.generation != c->generation || c->lease.identity != c->identity ||
                    c->lease.slot != index + 4U || (c->identity != 0xdd && c->identity != 0xdf))
                    return SDL_SetError("Fairy draw lost its actual DD/DF lease identity");
                if (c->identity == 0xdd
                        ? (!c->cursor.apple.phase || c->cursor.apple.phase > 5 ||
                           c->cursor.apple.removed || c->lease.count != SOTN_FAIRY_APPLE_ROWS)
                        : (!c->cursor.item.phase || c->cursor.item.phase > 2 ||
                           c->cursor.item.removed || !c->cursor.item.count ||
                           c->cursor.item.count >= SOTN_FAIRY_ITEM_ROWS ||
                           c->lease.count != c->cursor.item.count + 1U))
                    return SDL_SetError("Fairy draw lost its actual live effect cursor");
                for (Uint16 row = 0; row < c->lease.count; ++row)
                    if (!SotnActors_FairyEffectRow(session->actors, &c->lease, row))
                        return false;
                const Uint16 fairy_rows =
                    c->identity == 0xdd ? SOTN_FAIRY_APPLE_ROWS : c->cursor.item.count;
                for (Uint16 row = 0; row < fairy_rows; ++row)
                {
                    const Uint16 mode = c->identity == 0xdd ? c->cursor.apple.quads[row].draw_mode
                                                            : c->cursor.item.tiles[row].draw_mode;
                    const Uint16 priority = c->identity == 0xdd
                                                ? c->cursor.apple.quads[row].priority
                                                : c->cursor.item.priority;
                    if (!(mode & 8U) &&
                        !SotnActorOrder_Add(
                            session->draw_items, SDL_arraysize(session->draw_items), &draw_count,
                            (SotnActorDraw){.priority = priority,
                                            .actor = index,
                                            .primitive = row,
                                            .kind = c->identity == 0xdd ? SOTN_DRAW_FAIRY_LIGHT
                                                                        : SOTN_DRAW_FAIRY_TILE}))
                        return false;
                }
            }
        }
    if (!background && session->sword_runtime.initialized)
    {
        if (!ValidateSword(session))
            return false;
        for (Uint16 part = 0; part < 3; ++part)
        {
            const SotnSwordLease *lease = &session->sword_leases[part];
            const SotnPlayerEffectQuad *rows = part == 0   ? session->sword_runtime.model
                                               : part == 1 ? session->sword_runtime.trail
                                                           : session->sword_runtime.halo;
            for (Uint16 row = 0; row < lease->count; ++row)
                if (!(rows[row].draw_mode & 8U) &&
                    !SotnActorOrder_Add(session->draw_items, SDL_arraysize(session->draw_items),
                                        &draw_count,
                                        (SotnActorDraw){.priority = rows[row].priority,
                                                        .actor = part,
                                                        .primitive = row,
                                                        .kind = SOTN_DRAW_SWORD_FAMILIAR}))
                    return false;
        }
    }
    if (!background && session->mist_present)
        for (Uint16 i = 0; i < 80; ++i)
            if (!session->mist_effect.quads[i].hidden &&
                !SotnActorOrder_Add(
                    session->draw_items, SDL_arraysize(session->draw_items), &draw_count,
                    (SotnActorDraw){.priority = session->mist_effect.quads[i].priority,
                                    .primitive = i,
                                    .kind = SOTN_DRAW_MIST}))
                return false;
    if (!background && session->bat_runtime.echo_generation)
        for (Uint16 i = SOTN_BAT_ECHO_DRIVERS; i < SOTN_BAT_ECHO_PRIMITIVES; ++i)
        {
            SotnActorPrimitive p = {0};
            const Uint16 id = session->bat_runtime.echo_primitives[i];
            if (!SotnActors_GetBatEchoPrimitive(session->actors,
                                                session->bat_runtime.echo_generation, id, &p) ||
                (p.active && !SotnActorOrder_Add(session->draw_items,
                                                 SDL_arraysize(session->draw_items), &draw_count,
                                                 (SotnActorDraw){.priority = p.priority,
                                                                 .primitive = id,
                                                                 .kind = SOTN_DRAW_BAT_ECHO})))
                return false;
        }
    if (!background)
        for (Uint16 owner = 0; owner < SOTN_BAT_RUNTIME_SLOTS; ++owner)
        {
            const SotnBatRuntimeChild *child = &session->bat_runtime.children[owner];
            if (child->kind == 38 && child->initialized)
                for (Uint16 row = SOTN_PLAYER_CIRCLE_ROWS; row > 0; --row)
                {
                    const SotnPlayerEffectQuad *q = &child->state.circle.quads[row - 1U];
                    if (!(q->draw_mode & 8U) &&
                        !SotnActorOrder_Add(
                            session->draw_items, SDL_arraysize(session->draw_items), &draw_count,
                            (SotnActorDraw){
                                .priority = q->priority,
                                .primitive = (Uint16)(owner * SOTN_PLAYER_CIRCLE_ROWS + row - 1U),
                                .kind = SOTN_DRAW_PLAYER_CIRCLE}))
                        return false;
                }
            if (child->kind == 65 && child->initialized)
                for (Uint16 row = SOTN_TELEPORT_ROWS; row > 0; --row)
                {
                    const SotnPlayerEffectQuad *q = &child->state.teleport.quads[row - 1U];
                    if (!(q->draw_mode & 8U) &&
                        !SotnActorOrder_Add(
                            session->draw_items, SDL_arraysize(session->draw_items), &draw_count,
                            (SotnActorDraw){.priority = q->priority,
                                            .primitive =
                                                (Uint16)(owner * SOTN_TELEPORT_ROWS + row - 1U),
                                            .kind = SOTN_DRAW_TELEPORT}))
                        return false;
                }
            if (child->kind == 32 && child->initialized)
                for (Uint16 row = SOTN_STONE_DISSOLVE_ROWS; row > 0; --row)
                {
                    const SotnPlayerEffectQuad *q = &child->state.dissolve.quads[row - 1U];
                    if (!(q->draw_mode & 8U) &&
                        !SotnActorOrder_Add(
                            session->draw_items, SDL_arraysize(session->draw_items), &draw_count,
                            (SotnActorDraw){
                                .priority = q->priority,
                                .primitive = (Uint16)(owner * SOTN_STONE_DISSOLVE_ROWS + row - 1U),
                                .kind = SOTN_DRAW_STONE_DISSOLVE}))
                        return false;
                }
            if (child->kind == 33 && child->initialized)
                for (Uint16 row = SOTN_ICE_HIT_ROWS; row > 0; --row)
                {
                    const SotnPlayerEffectQuad *q = &child->state.ice.quads[row - 1U];
                    if (!(q->draw_mode & 8U) &&
                        !SotnActorOrder_Add(
                            session->draw_items, SDL_arraysize(session->draw_items), &draw_count,
                            (SotnActorDraw){.priority = q->priority,
                                            .primitive =
                                                (Uint16)(owner * SOTN_ICE_HIT_ROWS + row - 1U),
                                            .kind = SOTN_DRAW_ICE_HIT}))
                        return false;
                }
            if (child->kind == 30 && child->initialized)
                for (Uint16 row = 6; row > 0; --row)
                {
                    const SotnPlayerEffectQuad *q = &child->state.thunder.quads[row - 1U];
                    if (!(q->draw_mode & 8U) &&
                        !SotnActorOrder_Add(
                            session->draw_items, SDL_arraysize(session->draw_items), &draw_count,
                            (SotnActorDraw){.priority = q->priority,
                                            .primitive = (Uint16)(owner * 6U + row - 1U),
                                            .kind = SOTN_DRAW_THUNDER_HIT}))
                        return false;
                }
            if (child->kind == 47 && child->initialized && child->primitive_count == 16)
                for (Uint16 row = 16; row > 0; --row)
                {
                    const Uint16 id = row - 1;
                    const SotnPlayerEffectQuad *q = &child->state.holy.quads[id];
                    if (!(q->draw_mode & 8U) &&
                        !SotnActorOrder_Add(session->draw_items, SDL_arraysize(session->draw_items),
                                            &draw_count,
                                            (SotnActorDraw){.priority = q->priority,
                                                            .primitive = (Uint16)(owner * 16U + id),
                                                            .kind = SOTN_DRAW_HOLY_HIT}))
                        return false;
                }
            if (child->pentagram && child->initialized)
                for (Uint16 row = SOTN_PENTAGRAM_ROWS; row > 0; --row)
                {
                    const SotnPlayerEffectQuad *q =
                        &child->state.pentagram.motion.polygons[row - 1U];
                    if (!(q->draw_mode & 8U) &&
                        !SotnActorOrder_Add(
                            session->draw_items, SDL_arraysize(session->draw_items), &draw_count,
                            (SotnActorDraw){.priority = q->priority,
                                            .primitive =
                                                (Uint16)(owner * SOTN_PENTAGRAM_ROWS + row - 1U),
                                            .kind = SOTN_DRAW_PENTAGRAM}))
                        return false;
                }
            if (child->sire && child->initialized && child->state.sire.animation.frame &&
                !SotnActorOrder_Add(session->draw_items, SDL_arraysize(session->draw_items),
                                    &draw_count,
                                    (SotnActorDraw){.priority = child->source.priority,
                                                    .primitive = owner,
                                                    .kind = SOTN_DRAW_SIRE}))
                return false;
            if (child->neutron && child->initialized && child->state.neutron.animation.frame &&
                !SotnActorOrder_Add(session->draw_items, SDL_arraysize(session->draw_items),
                                    &draw_count,
                                    (SotnActorDraw){.priority = child->source.priority,
                                                    .primitive = owner,
                                                    .kind = SOTN_DRAW_NEUTRON}))
                return false;
            if (child->magic && child->initialized && child->state.magic.animation.frame &&
                child->state.magic.motion.bank &&
                !((child->state.magic.motion.draw_flags & 0x80U) &&
                  (((owner + 16U) ^ session->system_timer) & 1U)) &&
                !SotnActorOrder_Add(session->draw_items, SDL_arraysize(session->draw_items),
                                    &draw_count,
                                    (SotnActorDraw){.priority = child->source.priority,
                                                    .primitive = owner,
                                                    .kind = SOTN_DRAW_MAGIC_MISSILE}))
                return false;
            if (child->hand_weapon && child->initialized)
            {
                const SotnHandWeapon *w = &child->state.hand.motion;
                if (w->frame && w->bank &&
                    !((w->draw_flags & 0x80U) && (((owner + 16U) ^ session->system_timer) & 1U)) &&
                    !SotnActorOrder_Add(session->draw_items, SDL_arraysize(session->draw_items),
                                        &draw_count,
                                        (SotnActorDraw){.priority = w->priority,
                                                        .primitive = owner,
                                                        .kind = SOTN_DRAW_HAND_WEAPON}))
                    return false;
                for (Uint16 row = child->primitive_count; row > 0; --row)
                    if (!(w->tiles[row - 1U].draw_mode & 8U) &&
                        !SotnActorOrder_Add(session->draw_items, SDL_arraysize(session->draw_items),
                                            &draw_count,
                                            (SotnActorDraw){.priority = w->tiles[row - 1U].priority,
                                                            .primitive = (Uint16)(row - 1U),
                                                            .kind = SOTN_DRAW_HAND_TILE}))
                        return false;
            }
            if (child->weapon_secondary && child->initialized)
            {
                const SotnWeaponSecondary *w = &child->state.secondary.motion;
                if (w->frame && w->bank &&
                    !((w->draw_flags & 0x80U) && (((owner + 16U) ^ session->system_timer) & 1U)) &&
                    !SotnActorOrder_Add(session->draw_items, SDL_arraysize(session->draw_items),
                                        &draw_count,
                                        (SotnActorDraw){.priority = w->priority,
                                                        .primitive = owner,
                                                        .kind = SOTN_DRAW_WEAPON_SECONDARY}))
                    return false;
                // Weapon53's slash and weapon51's fan blade own one entity-relative GT4;
                // weapon48's ring and weapon1's punch rush own sixteen screen-space GT4s.
                const Uint16 family = child->state.secondary.family, role = child->kind & 15U;
                const bool slash = family == 53 || family == 56 || family == 51 ||
                                   (family == 12 && role == 0),
                           // Weapon30's soldiers draw their own dawn quads (and hide some);
                           // weapon13's combo sweep owns one late LINE_G2 row.
                    ring = (role == 2 && family != 30 && family != 13) || family == 1 ||
                           (family == 50 && role == 4),
                           sparkle = family == 50 && role == 1, rune = family == 14 && role == 0,
                           jewel = family == 37 && role == 1, knuckle = family == 46,
                           dynamite = family == 47 && role == 0, refresh = family == 44;
                // Weapon47's fuse TILEs or explosion GT4s, world pixels.
                for (Uint16 q = dynamite ? child->primitive_count : 0; q > 0; --q)
                    if (!(w->dyn_quads[q - 1U].draw_mode & 8U) &&
                        !SotnActorOrder_Add(
                            session->draw_items, SDL_arraysize(session->draw_items), &draw_count,
                            (SotnActorDraw){.priority = w->dyn_quads[q - 1U].priority,
                                            .primitive = (Uint16)(owner * 64U + q - 1U),
                                            .kind = SOTN_DRAW_WEAPON_SECONDARY_QUAD}))
                        return false;
                // Weapon46: Holbein dagger's slash GT4 or Blue knuckles' thirty-two arc G4s,
                // screen-space.
                for (Uint16 q = knuckle ? child->primitive_count : 0; q > 0; --q)
                    if (!(w->knuckle_quads[q - 1U].draw_mode & 8U) &&
                        !SotnActorOrder_Add(
                            session->draw_items, SDL_arraysize(session->draw_items), &draw_count,
                            (SotnActorDraw){.priority = w->knuckle_quads[q - 1U].priority,
                                            .primitive = (Uint16)(owner * 64U + q - 1U),
                                            .kind = SOTN_DRAW_WEAPON_SECONDARY_QUAD}))
                        return false;
                // Weapon44: Heart Refresh's sixteen bands and heart, screen-space GT4s.
                for (Uint16 q = refresh ? child->primitive_count : 0; q > 0; --q)
                    if (!(w->refresh_quads[q - 1U].draw_mode & 8U) &&
                        !SotnActorOrder_Add(
                            session->draw_items, SDL_arraysize(session->draw_items), &draw_count,
                            (SotnActorDraw){.priority = w->refresh_quads[q - 1U].priority,
                                            .primitive = (Uint16)(owner * 64U + q - 1U),
                                            .kind = SOTN_DRAW_WEAPON_SECONDARY_QUAD}))
                        return false;
                // Weapon37's eight sparkles (of sixteen rows) are screen-space GT4s.
                for (Uint16 q = jewel ? child->primitive_count : 0; q > 0; --q)
                    if (!(w->jewel_quads[q - 1U].draw_mode & 8U) &&
                        !SotnActorOrder_Add(
                            session->draw_items, SDL_arraysize(session->draw_items), &draw_count,
                            (SotnActorDraw){.priority = w->jewel_quads[q - 1U].priority,
                                            .primitive = (Uint16)(owner * 64U + q - 1U),
                                            .kind = SOTN_DRAW_WEAPON_SECONDARY_QUAD}))
                        return false;
                // Shield spells (E8) own eleven screen-space GT4s.
                for (Uint16 q = SotnWeaponSecondary_ShieldSpell(family) ? child->primitive_count
                                                                        : 0;
                     q > 0; --q)
                    if (!(w->spell_quads[q - 1U].draw_mode & 8U) &&
                        !SotnActorOrder_Add(
                            session->draw_items, SDL_arraysize(session->draw_items), &draw_count,
                            (SotnActorDraw){.priority = w->spell_quads[q - 1U].priority,
                                            .primitive = (Uint16)(owner * 64U + q - 1U),
                                            .kind = SOTN_DRAW_WEAPON_SECONDARY_QUAD}))
                        return false;
                // Weapon30's circle (eleven) and soldiers (fourteen) own entity-relative GT4s.
                for (Uint16 q = family == 30 ? child->primitive_count : 0; q > 0; --q)
                    if (!(w->dawn_quads[q - 1U].draw_mode & 8U) &&
                        !SotnActorOrder_Add(
                            session->draw_items, SDL_arraysize(session->draw_items), &draw_count,
                            (SotnActorDraw){.priority = w->dawn_quads[q - 1U].priority,
                                            .primitive = (Uint16)(owner * 64U + q - 1U),
                                            .kind = SOTN_DRAW_WEAPON_SECONDARY_QUAD}))
                        return false;
                // Weapon14's forty-eight trail GT4s share the compact-row command.
                for (Uint16 q = rune ? child->primitive_count : 0; q > 0; --q)
                    if (!w->rune_hidden[q - 1U] &&
                        !SotnActorOrder_Add(
                            session->draw_items, SDL_arraysize(session->draw_items), &draw_count,
                            (SotnActorDraw){.priority = w->row_priority,
                                            .primitive = (Uint16)(owner * 80U + q - 1U),
                                            .kind = SOTN_DRAW_WEAPON_SPARKLE}))
                        return false;
                // Weapon50's eighty sparkles are compact rows rebuilt at draw time.
                for (Uint16 q = sparkle ? 80 : 0; q > 0; --q)
                    if (w->ice_size[q - 1U] &&
                        !SotnActorOrder_Add(
                            session->draw_items, SDL_arraysize(session->draw_items), &draw_count,
                            (SotnActorDraw){.priority = w->priority,
                                            .primitive = (Uint16)(owner * 80U + q - 1U),
                                            .kind = SOTN_DRAW_WEAPON_SPARKLE}))
                        return false;
                for (Uint16 q = slash ? 1 : ring ? child->primitive_count : 0; q > 0; --q)
                    if (!SotnActorOrder_Add(
                            session->draw_items, SDL_arraysize(session->draw_items), &draw_count,
                            (SotnActorDraw){.priority = slash ? w->quad.priority
                                                              : w->quads[q - 1U].priority,
                                            .primitive = (Uint16)(owner * 64U + q - 1U),
                                            .kind = SOTN_DRAW_WEAPON_SECONDARY_QUAD}))
                        return false;
                for (Uint16 row = slash || ring || sparkle || rune || jewel || knuckle ||
                                          dynamite || refresh || family == 30 ||
                                          SotnWeaponSecondary_ShieldSpell(family)
                                      ? 0
                                      : child->primitive_count;
                     row > 0; --row)
                    if (!(family == 13 ? w->heaven_line.hidden : w->rows[row - 1U].hidden) &&
                        !SotnActorOrder_Add(
                            session->draw_items, SDL_arraysize(session->draw_items), &draw_count,
                            (SotnActorDraw){.priority = w->row_priority,
                                            .primitive = (Uint16)(owner * 64U + row - 1U),
                                            .kind = SOTN_DRAW_WEAPON_SECONDARY_LINE}))
                        return false;
            }
            if (child->karma && child->initialized)
            {
                const SotnKarmaCoin *m = &child->state.karma.motion;
                if (m->frame && m->bank &&
                    !((m->draw_flags & 0x80U) && (((owner + 16U) ^ session->system_timer) & 1U)) &&
                    !SotnActorOrder_Add(
                        session->draw_items, SDL_arraysize(session->draw_items), &draw_count,
                        (SotnActorDraw){
                            .priority = m->priority, .primitive = owner, .kind = SOTN_DRAW_KARMA}))
                    return false;
                for (Uint16 row = m->phase >= 5 ? child->primitive_count : 0; row > 0; --row)
                    if (!(m->quads[row - 1U].draw_mode & 8U) &&
                        !SotnActorOrder_Add(
                            session->draw_items, SDL_arraysize(session->draw_items), &draw_count,
                            (SotnActorDraw){.priority = m->quads[row - 1U].priority,
                                            .primitive =
                                                (Uint16)(owner * SOTN_KARMA_ROWS + row - 1U),
                                            .kind = SOTN_DRAW_KARMA_QUAD}))
                        return false;
            }
            if (child->star && child->initialized)
            {
                const SotnThrownWeapon *m = &child->state.star.motion.motion;
                if (m->frame && m->bank &&
                    !((m->draw_flags & 0x80U) && (((owner + 16U) ^ session->system_timer) & 1U)) &&
                    !SotnActorOrder_Add(
                        session->draw_items, SDL_arraysize(session->draw_items), &draw_count,
                        (SotnActorDraw){
                            .priority = m->priority, .primitive = owner, .kind = SOTN_DRAW_STAR}))
                    return false;
                if (child->primitive_count)
                    for (Uint16 j = 32; j > 0; --j)
                        if (!(child->state.star.motion.sparks[j - 1U].draw_mode & 8U) &&
                            !SotnActorOrder_Add(
                                session->draw_items, SDL_arraysize(session->draw_items),
                                &draw_count,
                                (SotnActorDraw){.priority = (Uint16)(m->priority - 2U),
                                                .primitive = (Uint16)(owner * 32U + j - 1U),
                                                .kind = SOTN_DRAW_STAR_SPARK}))
                            return false;
            }
            // Rebound Stone's sixteen additive LINE_G2 trail rows (world pixels).
            for (Uint16 j = child->subweapon && child->initialized &&
                                    child->state.subweapon.motion.id == SOTN_SUBWEAPON_REBOUND_STONE
                                ? SOTN_REBOUND_STONE_LINES
                                : 0;
                 j > 0; --j)
                if (!(child->state.subweapon.motion.stone.lines[j - 1U].draw_mode & 8U) &&
                    !SotnActorOrder_Add(
                        session->draw_items, SDL_arraysize(session->draw_items), &draw_count,
                        (SotnActorDraw){.priority = child->state.subweapon.motion.priority,
                                        .primitive = (Uint16)(owner * 16U + j - 1U),
                                        .kind = SOTN_DRAW_SUBWEAPON_LINE}))
                    return false;
            // Agunea's additive LINE_G2 from its throw point to the bolt.
            if (child->subweapon && child->initialized &&
                child->state.subweapon.motion.id == SOTN_SUBWEAPON_AGUNEA &&
                !(child->state.subweapon.motion.agunea.draw_mode & 8U) &&
                !SotnActorOrder_Add(
                    session->draw_items, SDL_arraysize(session->draw_items), &draw_count,
                    (SotnActorDraw){.priority = child->state.subweapon.motion.agunea.priority,
                                    .primitive = (Uint16)(owner * 16U),
                                    .kind = SOTN_DRAW_SUBWEAPON_LINE}))
                return false;
            // The Bible's book (once shown) and each trail's single GT4.
            if (child->subweapon && child->initialized &&
                ((child->state.subweapon.motion.id == SOTN_SUBWEAPON_BIBLE &&
                  child->state.subweapon.motion.bible.mode) ||
                 child->state.subweapon.motion.id == SOTN_BIBLE_TRAIL) &&
                !SotnActorOrder_Add(
                    session->draw_items, SDL_arraysize(session->draw_items), &draw_count,
                    (SotnActorDraw){.priority = child->state.subweapon.motion.priority,
                                    .primitive = (Uint16)(owner * 64U),
                                    .kind = SOTN_DRAW_SUBWEAPON_QUAD}))
                return false;
            // The dagger's blade GT4 and additive trail LINE_G2 (same priority, blade first).
            if (child->subweapon && child->initialized &&
                child->state.subweapon.motion.id == SOTN_SUBWEAPON_DAGGER)
            {
                const SotnDagger *d = &child->state.subweapon.motion.dagger;
                if ((!(d->quad_mode & 8U) &&
                     !SotnActorOrder_Add(session->draw_items, SDL_arraysize(session->draw_items),
                                         &draw_count,
                                         (SotnActorDraw){.priority = d->row_priority,
                                                         .primitive = (Uint16)(owner * 64U),
                                                         .kind = SOTN_DRAW_SUBWEAPON_QUAD})) ||
                    (!(d->line_mode & 8U) &&
                     !SotnActorOrder_Add(session->draw_items, SDL_arraysize(session->draw_items),
                                         &draw_count,
                                         (SotnActorDraw){.priority = d->row_priority,
                                                         .primitive = (Uint16)(owner * 16U),
                                                         .kind = SOTN_DRAW_SUBWEAPON_LINE})))
                    return false;
            }
            // The axe and its afterimages (screen words, all at one priority).
            for (Uint16 j = child->subweapon && child->initialized &&
                                    child->state.subweapon.motion.id == SOTN_SUBWEAPON_AXE
                                ? SOTN_AXE_ROWS
                                : 0;
                 j > 0; --j)
                if (((child->state.subweapon.motion.axe.visible >> (j - 1U)) & 1U) &&
                    !SotnActorOrder_Add(
                        session->draw_items, SDL_arraysize(session->draw_items), &draw_count,
                        (SotnActorDraw){.priority = child->state.subweapon.motion.axe.row_priority,
                                        .primitive = (Uint16)(owner * 64U + j - 1U),
                                        .kind = SOTN_DRAW_SUBWEAPON_QUAD}))
                    return false;
            // The cross beam's GT4, a giant cross's visible faces and the particles' shown sparks.
            if (child->subweapon && child->initialized &&
                child->state.subweapon.motion.id == SOTN_SUBWEAPON_CROSS &&
                !SotnActorOrder_Add(
                    session->draw_items, SDL_arraysize(session->draw_items), &draw_count,
                    (SotnActorDraw){.priority = child->state.subweapon.motion.priority,
                                    .primitive = (Uint16)(owner * 64U),
                                    .kind = SOTN_DRAW_SUBWEAPON_QUAD}))
                return false;
            for (Uint16 j = child->subweapon && child->initialized &&
                                    child->state.subweapon.motion.id == SOTN_GIANT_CROSS
                                ? SOTN_GIANT_CROSS_ROWS
                                : 0;
                 j > 0; --j)
                if (((child->state.subweapon.motion.cross.visible >> (j - 1U)) & 1U) &&
                    !SotnActorOrder_Add(
                        session->draw_items, SDL_arraysize(session->draw_items), &draw_count,
                        (SotnActorDraw){.priority = child->state.subweapon.motion.cross.priority,
                                        .primitive = (Uint16)(owner * 64U + j - 1U),
                                        .kind = SOTN_DRAW_SUBWEAPON_QUAD}))
                    return false;
            for (Uint16 j = child->subweapon && child->initialized &&
                                    child->state.subweapon.motion.id == SOTN_CROSS_PARTICLES
                                ? SOTN_CROSS_PARTICLE_ROWS
                                : 0;
                 j > 0; --j)
                if (child->state.subweapon.motion.sparks.rows[j - 1U].visible &&
                    !SotnActorOrder_Add(
                        session->draw_items, SDL_arraysize(session->draw_items), &draw_count,
                        (SotnActorDraw){
                            .priority = child->state.subweapon.motion.sparks.rows[j - 1U].priority,
                            .primitive = (Uint16)(owner * 64U + j - 1U),
                            .kind = SOTN_DRAW_SUBWEAPON_QUAD}))
                    return false;
            // The lightning's drawn GT4 rows 1..end (row0 stays hidden).
            for (Uint16 j = child->subweapon && child->initialized &&
                                    child->state.subweapon.motion.id == SOTN_AGUNEA_BOLT
                                ? child->state.subweapon.motion.bolt.end
                                : 0;
                 j > 0; --j)
                if (!SotnActorOrder_Add(
                        session->draw_items, SDL_arraysize(session->draw_items), &draw_count,
                        (SotnActorDraw){.priority = child->state.subweapon.motion.priority,
                                        .primitive = (Uint16)(owner * 64U + j),
                                        .kind = SOTN_DRAW_SUBWEAPON_QUAD}))
                    return false;
            // Vibhuti's twelve opaque ash TILEs (world pixels); DRAW_HIDE rows are skipped.
            for (Uint16 j = child->subweapon && child->initialized &&
                                    child->state.subweapon.motion.id == SOTN_SUBWEAPON_VIBHUTI
                                ? SOTN_VIBHUTI_PARTICLES
                                : 0;
                 j > 0; --j)
                if (!(child->state.subweapon.motion.particles[j - 1U].draw_mode & 8U) &&
                    !SotnActorOrder_Add(
                        session->draw_items, SDL_arraysize(session->draw_items), &draw_count,
                        (SotnActorDraw){.priority = child->state.subweapon.motion.priority,
                                        .primitive = (Uint16)(owner * 16U + j - 1U),
                                        .kind = SOTN_DRAW_SUBWEAPON_TILE}))
                    return false;
            if (child->thrown && child->initialized && child->state.thrown.motion.frame &&
                child->state.thrown.motion.bank &&
                !((child->state.thrown.motion.draw_flags & 0x80U) &&
                  (((owner + 16U) ^ session->system_timer) & 1U)) &&
                !SotnActorOrder_Add(session->draw_items, SDL_arraysize(session->draw_items),
                                    &draw_count,
                                    (SotnActorDraw){.priority = child->source.priority,
                                                    .primitive = owner,
                                                    .kind = SOTN_DRAW_THROWN}))
                return false;
            if (child->food && child->initialized && child->state.food.frame &&
                !((child->state.food.draw_flags & 0x80U) &&
                  (((owner + 16U) ^ session->system_timer) & 1U)) &&
                !SotnActorOrder_Add(session->draw_items, SDL_arraysize(session->draw_items),
                                    &draw_count,
                                    (SotnActorDraw){.priority = child->source.priority,
                                                    .primitive = owner,
                                                    .kind = SOTN_DRAW_FOOD}))
                return false;
            if (!child->food && !child->sire && !child->magic && !child->thrown && !child->star &&
                !child->karma && !child->weapon_secondary && child->initialized &&
                (child->kind == 0xe1 || child->kind == 0xf1 || child->kind == 0xe2 ||
                 child->kind == 0xf2) &&
                !SotnActorOrder_Add(session->draw_items, SDL_arraysize(session->draw_items),
                                    &draw_count,
                                    (SotnActorDraw){.priority = child->source.priority,
                                                    .primitive = owner,
                                                    .kind = SOTN_DRAW_SWORD}))
                return false;
            if (child->kind == 37 && child->initialized && child->primitive_count)
                for (Uint16 row = 18; row > 0; --row)
                {
                    const Uint16 id = row - 1;
                    const SotnImmunityText *n = &child->state.immunity_text;
                    const bool hidden = (id == 1 && (n->suffix.draw_mode & 8U)) ||
                                        (id >= 2 && (n->tiles[id - 2U].draw_mode & 8U));
                    if (!hidden &&
                        !SotnActorOrder_Add(session->draw_items, SDL_arraysize(session->draw_items),
                                            &draw_count,
                                            (SotnActorDraw){.priority = 0x1b8,
                                                            .primitive = (Uint16)(owner * 18U + id),
                                                            .kind = SOTN_DRAW_IMMUNITY_TEXT}))
                        return false;
                }
            if (child->kind == 19 && child->initialized)
            {
                if (owner < 40 || !child->generation || child->primitive_count != 20 ||
                    !child->state.healing_number.phase || child->state.healing_number.removed)
                    return SDL_SetError("Healing number draw lost its actual child and rows");
                for (Uint16 row = 20; row > 0; --row)
                {
                    const Uint16 id = row - 1;
                    const SotnHealingNumber *n = &child->state.healing_number;
                    const bool hidden =
                        id < 4 ? (n->digits[id].draw_mode & 8U) != 0 : n->tiles[id - 4U].hidden;
                    if (!hidden &&
                        !SotnActorOrder_Add(session->draw_items, SDL_arraysize(session->draw_items),
                                            &draw_count,
                                            (SotnActorDraw){.priority = 0x1b8,
                                                            .primitive = (Uint16)(owner * 20U + id),
                                                            .kind = SOTN_DRAW_HEALING_NUMBER}))
                        return false;
                }
            }
            if (child->kind == 49 && child->initialized &&
                !SotnActorOrder_Add(session->draw_items, SDL_arraysize(session->draw_items),
                                    &draw_count,
                                    (SotnActorDraw){.priority = child->source.priority,
                                                    .primitive = owner,
                                                    .kind = SOTN_DRAW_AXE_SHADOW}))
                return false;
            if (child->kind == 51 && child->initialized &&
                !(child->state.heart.blink && (((owner + 16U) ^ session->system_timer) & 1U)) &&
                !SotnActorOrder_Add(session->draw_items, SDL_arraysize(session->draw_items),
                                    &draw_count,
                                    (SotnActorDraw){.priority = child->state.heart.priority,
                                                    .primitive = owner,
                                                    .kind = SOTN_DRAW_RISING_HEART}))
                return false;
            if (child->kind == 2 && child->initialized && child->state.smoke.sprite &&
                !SotnActorOrder_Add(session->draw_items, SDL_arraysize(session->draw_items),
                                    &draw_count,
                                    (SotnActorDraw){.priority = child->state.smoke.priority,
                                                    .primitive = owner,
                                                    .kind = SOTN_DRAW_PLAYER_SMOKE}))
                return false;
            if (child->kind == 5 && child->initialized)
            {
                if (!child->generation ||
                    child->state.particles.count > SOTN_PLAYER_PARTICLE_LIMIT ||
                    !child->state.particles.count || child->state.particles.phase != 1 ||
                    child->state.particles.removed ||
                    child->primitive_count != child->state.particles.count + 1U)
                    return SDL_SetError(
                        "Particle presentation lost its actual child/row ownership");
                for (Uint16 tile = child->state.particles.count; tile > 0; --tile)
                    if (!(child->state.particles.tiles[tile - 1U].draw_mode & 8U) &&
                        !SotnActorOrder_Add(
                            session->draw_items, SDL_arraysize(session->draw_items), &draw_count,
                            (SotnActorDraw){
                                .priority = child->state.particles.priority,
                                .primitive =
                                    (Uint16)(owner * SOTN_PLAYER_PARTICLE_LIMIT + tile - 1U),
                                .kind = SOTN_DRAW_PLAYER_PARTICLE}))
                        return false;
            }
            if (child->kind == 16 && child->initialized && child->state.fire_hit.frame &&
                !SotnActorOrder_Add(session->draw_items, SDL_arraysize(session->draw_items),
                                    &draw_count,
                                    (SotnActorDraw){.priority = child->state.fire_hit.priority,
                                                    .primitive = owner,
                                                    .kind = SOTN_DRAW_FIRE_HIT}))
                return false;
            if (child->kind == 54 && child->initialized && child->state.dark.frame &&
                !SotnActorOrder_Add(session->draw_items, SDL_arraysize(session->draw_items),
                                    &draw_count,
                                    (SotnActorDraw){.priority = child->state.dark.priority,
                                                    .primitive = owner,
                                                    .kind = SOTN_DRAW_DARK_HIT}))
                return false;
            if (child->kind == 45 && child->initialized && child->state.fire.frame &&
                !SotnActorOrder_Add(session->draw_items, SDL_arraysize(session->draw_items),
                                    &draw_count,
                                    (SotnActorDraw){.priority = child->state.fire.motion.priority,
                                                    .primitive = owner,
                                                    .kind = SOTN_DRAW_BAT_FIREBALL}))
                return false;
            if (child->initialized &&
                ((child->kind == 24 && child->state.hellfire_trail.frame) ||
                 ((child->kind == 26 || child->kind == 27) && child->state.hellfire_fire.frame)) &&
                !SotnActorOrder_Add(session->draw_items, SDL_arraysize(session->draw_items),
                                    &draw_count,
                                    (SotnActorDraw){.priority = child->source.priority,
                                                    .primitive = owner,
                                                    .kind = SOTN_DRAW_HELLFIRE_SPRITE}))
                return false;
            if (child->kind == 61 && child->initialized && child->state.spirit.frame &&
                !SotnActorOrder_Add(
                    session->draw_items, SDL_arraysize(session->draw_items), &draw_count,
                    (SotnActorDraw){
                        .priority = 0x1c3, .primitive = owner, .kind = SOTN_DRAW_SPIRIT}))
                return false;
            if (child->kind == 63 && child->initialized && child->primitive_count == 9 &&
                !SotnActorOrder_Add(
                    session->draw_items, SDL_arraysize(session->draw_items), &draw_count,
                    (SotnActorDraw){
                        .priority = 0x1c0, .primitive = owner, .kind = SOTN_DRAW_SPIRIT_PORTAL}))
                return false;
            const SotnPlayerEffectQuad *quads = NULL;
            Uint16 packet_count = 0;
            if (!SotnBatRuntime_Visual(&session->bat_runtime, owner, &quads, &packet_count))
                return false;
            for (Uint16 q = 0; q < packet_count; ++q)
                if (!SotnActorOrder_Add(session->draw_items, SDL_arraysize(session->draw_items),
                                        &draw_count,
                                        (SotnActorDraw){.priority = quads[q].priority,
                                                        .primitive = (Uint16)(owner * 8U + q),
                                                        .kind = SOTN_DRAW_PLAYER_BLINK}))
                    return false;
        }
    if (!background && session->bat_runtime.wolf.environment_reserved)
    {
        if (session->bat_runtime.wolf.root_reserved)
            for (Uint16 i = 6; i > 0; --i)
                if (!(session->bat_runtime.wolf.effect.quads[i - 1U].draw_mode & 8U) &&
                    !SotnActorOrder_Add(
                        session->draw_items, SDL_arraysize(session->draw_items), &draw_count,
                        (SotnActorDraw){.priority =
                                            session->bat_runtime.wolf.effect.quads[i - 1U].priority,
                                        .primitive = (Uint16)(i - 1U),
                                        .kind = SOTN_DRAW_WOLF_IMAGE}))
                    return false;
        // Original RenderPrimitives follows RenderEntities and inserts at the OT head:
        // retained primitive images draw before entity packets at equal priority.
        for (size_t i = session->wolf_packet_count; i > 0; --i)
            if (!SotnActorOrder_Add(
                    session->draw_items, SDL_arraysize(session->draw_items), &draw_count,
                    (SotnActorDraw){.priority = session->wolf_packets[i - 1U].quad.priority,
                                    .primitive = (Uint16)(i - 1U),
                                    .kind = SOTN_DRAW_WOLF_PACKET}))
                return false;
        for (Uint16 i = 0; i < 2; ++i)
            if (!SotnActorOrder_Add(session->draw_items, SDL_arraysize(session->draw_items),
                                    &draw_count,
                                    (SotnActorDraw){.priority = i ? 0x1b0 : 0x190,
                                                    .primitive = i,
                                                    .kind = SOTN_DRAW_WOLF_TARGET}))
                return false;
    }
    if (background)
    {
        for (size_t i = 0; i < draw_count; ++i)
        {
            const SotnActorDraw command = session->draw_items[i];
            const SotnActorState actor = list[command.actor];
            if (command.kind == SOTN_DRAW_PRIMITIVE)
            {
                if (!DrawPrimitiveRange(session, actor, camera_x, camera_y, command.primitive,
                                        command.primitive + 1U))
                    return ActorDrawError(actor);
            }
            else if (command.kind == SOTN_DRAW_ACTOR &&
                     !DrawActorPrimitives(session, actor, camera_x, camera_y))
                return ActorDrawError(actor);
        }
        return true;
    }
    SotnAlucardSprite alucard = {0};
    if (!DecodePlayerSprite(session, player, &alucard))
        return false;
    bool player_drawn = false, weapon_drawn = false, beam_drawn = false, afterimages_drawn = false;
    SotnFrameClip clip = {0, 0, SOTN_FRAME_WIDTH, SOTN_FRAME_HEIGHT};
    for (size_t i = 0; i <= draw_count; ++i)
    {
        if (!SotnFrameClip_CopyOutside(clip, session->frame, session->clip_backup,
                                       SOTN_FRAME_PIXELS))
            return false;
        if (!beam_drawn && (i == draw_count || session->draw_items[i].priority > 0x92))
        {
            if (!DrawGravityBeams(session, player, view_x, view_y))
                return false;
            beam_drawn = true;
        }
        if (!weapon_drawn && (i == draw_count || session->draw_items[i].priority > 0x92))
        {
            const SotnPlayerAttack hands[] = {player.attack, player.shield};
            for (Uint32 hand = 0; hand < SDL_arraysize(hands); ++hand)
            {
                const SotnPlayerAttack item = hands[hand];
                if (item.equipment == 0 || item.sprite == 0)
                    continue;
                const bool right = item.right_hand;
                const SotnActorSprite weapon = {
                    .animation_set = right ? 18 : 16,
                    .frame = item.sprite,
                    .palette = (Uint16)(right ? 0x128 : 0x110),
                    .texture_offset = (Uint16)(right ? 0x66 : 0x64),
                    .blend_mode = (Uint16)(item.equipment == 86 || item.equipment == 123 ||
                                                   item.equipment == 168 || item.equipment == 126
                                               ? 0x30
                                               : 0),
                    .facing_left = player.facing_left};
                if (!SotnActorGraphics_Draw(session->actor_graphics, weapon,
                                            Pixel(player.x) - view_x, Pixel(player.y) - view_y,
                                            session->frame, SOTN_FRAME_PIXELS))
                    return false;
            }
            weapon_drawn = true;
        }
        if (!afterimages_drawn && (i == draw_count || session->draw_items[i].priority > 0x92))
        {
            if (!DrawAfterimages(session, player, camera_x, camera_y))
                return false;
            afterimages_drawn = true;
        }
        if (!player_drawn && (i == draw_count || session->draw_items[i].priority > 0x94))
        {
            const SotnDataView common = {session->player_common, sizeof(session->player_common)};
            const bool drawn =
                player.mist.active || player.bat.hidden || player.hellfire_hidden ||
                player.animation_bank == 15 ||
                (player.axe_armor.active
                     ? DrawAxeArmorPlayer(session, player)
                     : ((player.animation.sprite & 0x8000U) || player.stone.active ||
                                player.death.native || player.water_death.active ||
                                player.fairy_rescue.blend || player.animation_bank == 13 ||
                                player.rotated ||
                                (!SotnPlayerStep_IsDead(player.step) && !player.stone.active &&
                                 (player.palette != 0x8100 || player.body_color_blend))
                            ? DrawRotatedPlayer(session, player, &alucard,
                                                player.fairy_rescue.blend)
                        : player.stone.active ? SotnAlucard_DrawPetrified(
                                                    &alucard, common, Pixel(player.x) - view_x,
                                                    Pixel(player.y) - view_y, player.facing_left,
                                                    player.stone, session->frame, SOTN_FRAME_PIXELS)
                        : player.gravity.upside_down
                            ? SotnAlucard_DrawGravity(&alucard, common, Pixel(player.x) - view_x,
                                                      Pixel(player.y) - view_y, player.facing_left,
                                                      session->frame, SOTN_FRAME_PIXELS)
                            : SotnAlucard_DrawDeath(&alucard, common, Pixel(player.x) - view_x,
                                                    Pixel(player.y) - view_y, player.facing_left,
                                                    player.death, session->frame,
                                                    SOTN_FRAME_PIXELS)));
            if (!drawn)
                return false;
            if (player.sleep.active && player.sleep.frame &&
                !SotnActorGraphics_Draw(session->actor_graphics,
                                        (SotnActorSprite){.animation_set = 17,
                                                          .frame = player.sleep.frame,
                                                          .palette = 0x819f,
                                                          .texture_offset = 0x50},
                                        Pixel(player.sleep.x) - view_x,
                                        Pixel(player.sleep.y) - view_y, session->frame,
                                        SOTN_FRAME_PIXELS))
                return false;
            player_drawn = true;
        }
        if (i == draw_count)
        {
            if (!SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return false;
            break;
        }
        const SotnActorDraw command = session->draw_items[i];
        if (command.kind == SOTN_DRAW_SWORD_FAMILIAR)
        {
            if (command.actor > 2 ||
                !SotnActors_SwordRow(session->actors, &session->sword_leases[command.actor],
                                     command.primitive))
                return SDL_SetError("Sword familiar draw lost its actual leased row");
            const SotnPlayerEffectQuad *rows = command.actor == 0   ? session->sword_runtime.model
                                               : command.actor == 1 ? session->sword_runtime.trail
                                                                    : session->sword_runtime.halo;
            if (!SotnActorGraphics_PlayerEffectGpu(
                    session->actor_graphics, &rows[command.primitive],
                    session->draw_target.offset_x + session->shake.x,
                    session->draw_target.offset_y + session->shake.y, session->draw_target.surface))
                return false;
            continue;
        }
        if (command.kind == SOTN_DRAW_FAIRY_SPRITE || command.kind == SOTN_DRAW_FAIRY_TILE ||
            command.kind == SOTN_DRAW_FAIRY_LIGHT)
        {
            if (!session->fairy_runtime.initialized || command.actor >= 4)
                return SDL_SetError("Fairy draw requires its actual four-slot owner");
            const SotnFairyRuntimeSlot *c = &session->fairy_runtime.slots[command.actor];
            if (!c->generation)
                return SDL_SetError("Fairy draw has a retired slot generation");
            const Sint32 offset_x = session->draw_target.offset_x + session->shake.x,
                         offset_y = session->draw_target.offset_y + session->shake.y;
            if (command.kind == SOTN_DRAW_FAIRY_SPRITE)
            {
                if (c->identity < SOTN_FAIRY_FOLLOW || c->identity > SOTN_FAIRY_HINT ||
                    c->cursor.fairy.mode != c->identity ||
                    !SotnActorGraphics_DrawSurface(
                        session->actor_graphics,
                        (SotnActorSprite){.animation_set = 0x8014,
                                          .frame = c->cursor.fairy.animation.sprite,
                                          .palette = 0x140,
                                          .texture_offset = 0x6c,
                                          .facing_left = c->source.facing_left},
                        Pixel(c->source.x) + offset_x, Pixel(c->source.y) + offset_y,
                        (SotnSpriteTransform){.scale_x = 256, .scale_y = 256, .opacity = 128},
                        session->draw_target.surface))
                    return false;
            }
            else if (!SotnActors_FairyEffectRow(session->actors, &c->lease, command.primitive))
                return false;
            else if (command.kind == SOTN_DRAW_FAIRY_LIGHT)
            {
                if (c->identity != 0xdd || command.primitive >= SOTN_FAIRY_APPLE_ROWS ||
                    session->draw_target.surface.width != SOTN_FRAME_WIDTH ||
                    session->draw_target.surface.height != SOTN_FRAME_HEIGHT ||
                    !SotnActorGraphics_PlayerColorEffect(&c->cursor.apple.quads[command.primitive],
                                                         offset_x, offset_y,
                                                         session->draw_target.surface))
                    return SDL_SetError(
                        "Fairy light requires its actual row and normal raster target");
            }
            else
            {
                if (c->identity != 0xdf || command.primitive >= c->cursor.item.count)
                    return SDL_SetError("Fairy item draw exceeds its actual particle chain");
                const SotnPlayerParticle *t = &c->cursor.item.tiles[command.primitive];
                const Sint32 x = t->screen_x + offset_x, y = t->screen_y + offset_y;
                if (!(t->draw_mode & 8U) && t->width && t->height &&
                    !SotnActorGraphics_QuadSurface(
                        session->actor_graphics,
                        (SotnTextureQuad){.x = {x, x + t->width, x, x + t->width},
                                          .y = {y, y, y + t->height, y + t->height},
                                          .untextured = true,
                                          .red = t->red,
                                          .green = t->green,
                                          .blue = t->blue,
                                          .blend = (t->draw_mode & 1U) != 0,
                                          .page = (Uint16)(t->draw_mode & 0x60U)},
                        session->draw_target.surface))
                    return false;
            }
            if (!SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return false;
            continue;
        }
        if (command.kind == SOTN_DRAW_WOLF_TARGET)
        {
            if (!session->bat_runtime.wolf.environment_reserved ||
                !SwitchTarget(session, (SotnActorPrimitive){.target = command.primitive ? 4 : 3,
                                                            .clip_left = 512,
                                                            .clip_top = 448,
                                                            .clip_right = 575,
                                                            .clip_bottom = 511,
                                                            .target_offset_x = 512,
                                                            .target_offset_y = 448,
                                                            .target_clear =
                                                                (player.status & 0x4000000U) != 0}))
                return false;
            clip = (SotnFrameClip){0, 0, SOTN_FRAME_WIDTH, SOTN_FRAME_HEIGHT};
            continue;
        }
        if (command.kind == SOTN_DRAW_WOLF_PACKET || command.kind == SOTN_DRAW_WOLF_IMAGE)
        {
            const SotnWolfRuntime *wolf = &session->bat_runtime.wolf;
            if ((command.kind == SOTN_DRAW_WOLF_PACKET &&
                 command.primitive >= session->wolf_packet_count) ||
                (command.kind == SOTN_DRAW_WOLF_IMAGE &&
                 (!wolf->root_reserved || command.primitive >= 6)))
                return SDL_SetError("Wolf draw lost its actual body packet/root owner");
            const SotnPlayerEffectQuad *q = command.kind == SOTN_DRAW_WOLF_PACKET
                                                ? &session->wolf_packets[command.primitive].quad
                                                : &wolf->effect.quads[command.primitive];
            const bool drawn = command.kind == SOTN_DRAW_WOLF_PACKET
                                   ? SotnActorGraphics_PlayerEffect(session->actor_graphics, q,
                                                                    session->draw_target.offset_x,
                                                                    session->draw_target.offset_y,
                                                                    session->draw_target.surface)
                                   : DrawPlayerEffectQuad(session, q);
            if (!drawn || !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                                     SOTN_FRAME_PIXELS))
                return false;
            continue;
        }
        if (command.kind == SOTN_DRAW_PLAYER_CIRCLE)
        {
            const Uint16 owner = command.primitive / SOTN_PLAYER_CIRCLE_ROWS,
                         row = command.primitive % SOTN_PLAYER_CIRCLE_ROWS;
            if (owner >= SOTN_BAT_RUNTIME_SLOTS)
                return SDL_SetError("Player circle packet exceeds its actual child slots");
            const SotnBatRuntimeChild *child = &session->bat_runtime.children[owner];
            if (child->kind != 38 || !SotnBatRuntime_CircleValid(child) || !child->initialized ||
                !DrawPlayerEffectQuad(session, &child->state.circle.quads[row]) ||
                !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return SDL_SetError("Player circle draw lost its actual thirty-two-row owner");
            continue;
        }
        if (command.kind == SOTN_DRAW_TELEPORT)
        {
            const Uint16 owner = command.primitive / SOTN_TELEPORT_ROWS,
                         row = command.primitive % SOTN_TELEPORT_ROWS;
            if (owner >= SOTN_BAT_RUNTIME_SLOTS)
                return SDL_SetError("Teleport packet exceeds its actual child slots");
            const SotnBatRuntimeChild *child = &session->bat_runtime.children[owner];
            if (child->kind != 65 || !SotnBatRuntime_TeleportValid(child) || !child->initialized ||
                !DrawTeleportRow(session, &child->state.teleport, row) ||
                !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return SDL_SetError("Teleport draw lost its actual36-row owner");
            continue;
        }
        if (command.kind == SOTN_DRAW_STONE_DISSOLVE)
        {
            const Uint16 owner = command.primitive / SOTN_STONE_DISSOLVE_ROWS,
                         row = command.primitive % SOTN_STONE_DISSOLVE_ROWS;
            if (owner >= SOTN_BAT_RUNTIME_SLOTS)
                return SDL_SetError("Dissolve packet exceeds its actual child slots");
            const SotnBatRuntimeChild *child = &session->bat_runtime.children[owner];
            if (child->kind != 32 || !SotnBatRuntime_DissolveValid(child) || !child->initialized ||
                !DrawPlayerEffectQuad(session, &child->state.dissolve.quads[row]) ||
                !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return SDL_SetError("Dissolve draw lost its actual thirty-six-row child owner");
            continue;
        }
        if (command.kind == SOTN_DRAW_ICE_HIT)
        {
            const Uint16 owner = command.primitive / SOTN_ICE_HIT_ROWS,
                         row = command.primitive % SOTN_ICE_HIT_ROWS;
            if (owner >= SOTN_BAT_RUNTIME_SLOTS)
                return SDL_SetError("Ice packet exceeds its actual child slots");
            const SotnBatRuntimeChild *child = &session->bat_runtime.children[owner];
            if (child->kind != 33 || !child->generation || !child->initialized ||
                child->primitive_count != SOTN_ICE_HIT_ROWS || child->state.ice.removed ||
                child->state.ice.phase < 1 || child->state.ice.phase > 2 ||
                !DrawIceQuad(session, &child->state.ice.quads[row]) ||
                !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return SDL_SetError("Ice draw lost its actual twenty-four-row child owner");
            continue;
        }
        if (command.kind == SOTN_DRAW_THUNDER_HIT)
        {
            const Uint16 owner = command.primitive / 6U, row = command.primitive % 6U;
            if (owner >= SOTN_BAT_RUNTIME_SLOTS)
                return SDL_SetError("Thunder packet exceeds its actual child slots");
            const SotnBatRuntimeChild *child = &session->bat_runtime.children[owner];
            if (child->kind != 30 || !child->generation || !child->initialized ||
                child->primitive_count != 6 || child->state.thunder.removed ||
                child->state.thunder.phase < 1 || child->state.thunder.phase > 2 ||
                !DrawPlayerEffectQuad(session, &child->state.thunder.quads[row]) ||
                !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return SDL_SetError("Thunder draw lost its actual six-row child/packet owner");
            continue;
        }
        if (command.kind == SOTN_DRAW_HOLY_HIT)
        {
            const Uint16 owner = command.primitive / 16U, row = command.primitive % 16U;
            if (owner >= SOTN_BAT_RUNTIME_SLOTS)
                return SDL_SetError("Holy packet lost its actual child index");
            const SotnBatRuntimeChild *child = &session->bat_runtime.children[owner];
            if (child->kind != 47 || !child->initialized || !child->generation ||
                child->primitive_count != 16 || child->state.holy.phase != 1 ||
                child->state.holy.removed ||
                !DrawPlayerEffectQuad(session, &child->state.holy.quads[row]) ||
                !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return SDL_SetError("Holy draw lost its actual sixteen-row child/packet owner");
            continue;
        }
        if (command.kind == SOTN_DRAW_IMMUNITY_TEXT)
        {
            const Uint16 owner = command.primitive / 18U, row = command.primitive % 18U;
            if (owner >= SOTN_BAT_RUNTIME_SLOTS)
                return SDL_SetError("Text draw lost its actual packet index");
            const SotnBatRuntimeChild *child = &session->bat_runtime.children[owner];
            if (child->kind != 37 || !child->initialized || !child->generation ||
                child->primitive_count != 18 || child->state.immunity_text.phase < 3 ||
                child->state.immunity_text.removed)
                return SDL_SetError("Immunity text draw lost its actual child and eighteen rows");
            const SotnImmunityText *n = &child->state.immunity_text;
            if (row < 2)
            {
                if (!DrawPlayerEffectQuad(session, row ? &n->suffix : &n->quad))
                    return false;
            }
            else
            {
                const Sint32 x = n->tiles[row - 2U].x + session->draw_target.offset_x +
                                 session->shake.x,
                             y = n->tiles[row - 2U].y + session->draw_target.offset_y +
                                 session->shake.y;
                const SotnTextureQuad q = {.x = {x, x + 1, x, x + 1},
                                           .y = {y, y, y + 1, y + 1},
                                           .untextured = true,
                                           .red = 128,
                                           .green = 128,
                                           .blue = 128};
                if (!SotnActorGraphics_QuadSurface(session->actor_graphics, q,
                                                   session->draw_target.surface))
                    return false;
            }
            if (!SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return false;
            continue;
        }
        if (command.kind == SOTN_DRAW_HEALING_NUMBER)
        {
            const Uint16 owner = command.primitive / 20U, row = command.primitive % 20U;
            const SotnBatRuntimeChild *child = &session->bat_runtime.children[owner];
            if (child->kind != 19 || !child->initialized || !child->generation ||
                child->primitive_count != 20 || child->state.healing_number.removed)
                return SDL_SetError("Healing number packet lost its actual child/row owner");
            const SotnHealingNumber *n = &child->state.healing_number;
            if (row < 4)
            {
                if (!DrawPlayerEffectQuad(session, &n->digits[row]))
                    return false;
            }
            else
            {
                const Sint32 x = n->tiles[row - 4U].x + session->draw_target.offset_x +
                                 session->shake.x,
                             y = n->tiles[row - 4U].y + session->draw_target.offset_y +
                                 session->shake.y;
                const SotnTextureQuad q = {.x = {x, x + 1, x, x + 1},
                                           .y = {y, y, y + 1, y + 1},
                                           .untextured = true,
                                           .red = n->damage && n->number ? 255 : 64,
                                           .green = n->damage ? 64 : 255,
                                           .blue = n->damage && !n->number ? 255 : 64};
                if (!SotnActorGraphics_QuadSurface(session->actor_graphics, q,
                                                   session->draw_target.surface))
                    return false;
            }
            if (!SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return false;
            continue;
        }
        if (command.kind == SOTN_DRAW_AXE_SHADOW)
        {
            const SotnBatRuntimeChild *c = &session->bat_runtime.children[command.primitive];
            if (c->kind != 49 || !c->generation || !c->initialized || c->primitive_count ||
                !DrawAxeArmorSprite(
                    session,
                    (SotnActorSprite){.animation_set = c->state.axe_shadow.bank,
                                      .frame = c->state.axe_shadow.frame,
                                      .palette = c->state.axe_shadow.palette,
                                      .texture_offset = c->state.axe_shadow.texture,
                                      .facing_left = c->source.facing_left},
                    Pixel(c->source.x) - camera_x, Pixel(c->source.y) - camera_y, 0, 0,
                    (c->state.axe_shadow.draw_flags & 8U) ? c->state.axe_shadow.opacity : 128,
                    false) ||
                !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return false;
            continue;
        }
        if (command.kind == SOTN_DRAW_RISING_HEART)
        {
            const SotnBatRuntimeChild *child = &session->bat_runtime.children[command.primitive];
            if (child->kind != 51 || !child->generation || !child->initialized ||
                !child->state.heart.initialized || child->state.heart.removed)
                return SDL_SetError("Rising heart draw lost its actual child owner");
            if (!SotnActorGraphics_DrawSurface(
                    session->actor_graphics,
                    (SotnActorSprite){
                        .animation_set = 3, .frame = 14, .facing_left = child->source.facing_left},
                    Pixel(child->state.heart.x) + session->draw_target.offset_x + session->shake.x,
                    Pixel(child->state.heart.y) + session->draw_target.offset_y + session->shake.y,
                    (SotnSpriteTransform){.scale_x = 256, .scale_y = 256, .opacity = 128},
                    session->draw_target.surface) ||
                !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return false;
            continue;
        }
        if (command.kind == SOTN_DRAW_PLAYER_SMOKE || command.kind == SOTN_DRAW_PLAYER_PARTICLE)
        {
            const Uint16 index = command.kind == SOTN_DRAW_PLAYER_SMOKE
                                     ? command.primitive
                                     : command.primitive / SOTN_PLAYER_PARTICLE_LIMIT;
            const SotnBatRuntimeChild *child = &session->bat_runtime.children[index];
            if (!child->generation || !child->initialized ||
                child->kind != (command.kind == SOTN_DRAW_PLAYER_SMOKE ? 2 : 5))
                return SDL_SetError("Player secondary effect draw lost its actual child");
            if (child->kind == 2)
            {
                const SotnPlayerSmoke puff = child->state.smoke;
                if (!SotnActorGraphics_DrawScaled(
                        session->actor_graphics,
                        (SotnActorSprite){.animation_set = 5,
                                          .frame = puff.sprite,
                                          .palette = 0x8195,
                                          .blend_mode = 0x10,
                                          .facing_left = puff.facing_left},
                        Pixel(puff.x) + session->draw_target.offset_x + session->shake.x,
                        Pixel(puff.y) + session->draw_target.offset_y + session->shake.y,
                        puff.scale, puff.scale, session->draw_target.surface))
                    return false;
            }
            else
            {
                const Uint16 tile = command.primitive % SOTN_PLAYER_PARTICLE_LIMIT;
                if (tile >= child->state.particles.count)
                    return SDL_SetError("Particle draw exceeds its actual owned tile list");
                const SotnPlayerParticle t = child->state.particles.tiles[tile];
                const bool fixed = (t.draw_mode & 0x2080U) != 0;
                const Sint32 x = t.screen_x + session->draw_target.offset_x +
                                 (fixed ? 0 : session->shake.x),
                             y = t.screen_y + session->draw_target.offset_y +
                                 (fixed ? 0 : session->shake.y);
                if (x > -(Sint32)t.width && y > -(Sint32)t.height &&
                    x < session->draw_target.surface.width &&
                    y < session->draw_target.surface.height)
                {
                    const SotnTextureQuad q = {.x = {x, x + t.width, x, x + t.width},
                                               .y = {y, y, y + t.height, y + t.height},
                                               .untextured = true,
                                               .red = t.red,
                                               .green = t.green,
                                               .blue = t.blue,
                                               .blend = (t.draw_mode & 1U) != 0,
                                               .page = (Uint16)(t.draw_mode & 0x60U)};
                    if (!SotnActorGraphics_QuadSurface(session->actor_graphics, q,
                                                       session->draw_target.surface))
                        return false;
                }
            }
            if (!SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return false;
            continue;
        }
        if (command.kind == SOTN_DRAW_BAT_ECHO)
        {
            SotnActorPrimitive p = {0};
            if (!SotnActors_GetBatEchoPrimitive(
                    session->actors, session->bat_runtime.echo_generation, command.primitive, &p))
                return false;
            SotnTextureQuad q = {
                .page = p.page, .blend = true, .gouraud = true, .untextured = true};
            for (Uint16 v = 0; v < 4; ++v)
            {
                q.x[v] = p.offset_x[v] + session->draw_target.offset_x +
                         (p.camera_fixed ? 0 : session->shake.x);
                q.y[v] = p.offset_y[v] + session->draw_target.offset_y +
                         (p.camera_fixed ? 0 : session->shake.y);
                q.vertex_red[v] = p.vertex_red[v];
                q.vertex_green[v] = p.vertex_green[v];
                q.vertex_blue[v] = p.vertex_blue[v];
            }
            if (!SotnActorGraphics_QuadSurface(session->actor_graphics, q,
                                               session->draw_target.surface) ||
                !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return false;
            continue;
        }
        if (command.kind == SOTN_DRAW_SPIRIT || command.kind == SOTN_DRAW_SPIRIT_PORTAL)
        {
            if (command.primitive >= SOTN_BAT_RUNTIME_SLOTS)
                return SDL_SetError("Spirit draw exceeds its actual child slots");
            const SotnBatRuntimeChild *child = &session->bat_runtime.children[command.primitive];
            if (!child->generation || !child->initialized ||
                child->kind != (command.kind == SOTN_DRAW_SPIRIT ? 61 : 63) ||
                child->primitive_count != (command.kind == SOTN_DRAW_SPIRIT ? 16 : 9))
                return SDL_SetError("Spirit draw lost its actual child/row owner");
            if (child->kind == 61)
            {
                const SotnSpirit motion = child->state.spirit.motion;
                if (!SotnActorGraphics_DrawSurface(
                        session->actor_graphics,
                        (SotnActorSprite){.animation_set = 9,
                                          .frame = child->state.spirit.frame,
                                          .palette = 0x819f,
                                          .blend_mode = 0x70,
                                          .facing_left = motion.facing_left},
                        Pixel(motion.x) + session->shake.x + session->draw_target.offset_x,
                        Pixel(motion.y) + session->shake.y + session->draw_target.offset_y,
                        (SotnSpriteTransform){.rotation = motion.rotation,
                                              .scale_x = 256,
                                              .scale_y = 256,
                                              .opacity = 128},
                        session->draw_target.surface))
                    return false;
            }
            else
            {
                const SotnSpiritPortal portal = child->state.portal;
                const Sint32 x = Pixel(portal.x), y = Pixel(portal.y),
                             ox = session->shake.x + session->draw_target.offset_x,
                             oy = session->shake.y + session->draw_target.offset_y;
                const SotnTextureQuad q = {.x = {BlinkWord(x - 32) + ox, BlinkWord(x + 31) + ox,
                                                 BlinkWord(x - 32) + ox, BlinkWord(x + 31) + ox},
                                           .y = {BlinkWord(y - 32) + oy, BlinkWord(y - 32) + oy,
                                                 BlinkWord(y + 31) + oy, BlinkWord(y + 31) + oy},
                                           .untextured = true};
                if (!SotnActorGraphics_QuadSurface(session->actor_graphics, q,
                                                   session->draw_target.surface))
                    return false;
            }
            if (!SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return false;
            continue;
        }
        if (command.kind == SOTN_DRAW_FOOD_TEXT)
        {
            SotnActorPrimitive p = {0};
            if (!SotnActors_GetFoodText(session->actors, command.primitive, &p))
                return false;
            SotnTextureQuad q = {.page = p.page,
                                 .palette = p.palette,
                                 .blend = p.blend,
                                 .gouraud = p.gouraud,
                                 .untextured = !p.textured};
            for (Uint16 v = 0; v < 4; ++v)
            {
                q.x[v] = Pixel(p.x) + p.offset_x[v] + session->draw_target.offset_x +
                         (p.camera_fixed ? 0 : session->shake.x);
                q.y[v] = Pixel(p.y) + p.offset_y[v] + session->draw_target.offset_y +
                         (p.camera_fixed ? 0 : session->shake.y);
                q.u[v] = p.u[v];
                q.v[v] = p.v[v];
                q.vertex_red[v] = p.vertex_red[v];
                q.vertex_green[v] = p.vertex_green[v];
                q.vertex_blue[v] = p.vertex_blue[v];
            }
            if (!SotnActorGraphics_QuadSurface(session->actor_graphics, q,
                                               session->draw_target.surface) ||
                !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return false;
            continue;
        }
        if (command.kind == SOTN_DRAW_PENTAGRAM)
        {
            const Uint16 owner = command.primitive / SOTN_PENTAGRAM_ROWS,
                         row = command.primitive % SOTN_PENTAGRAM_ROWS;
            if (owner >= SOTN_BAT_RUNTIME_SLOTS)
                return SDL_SetError("Pentagram packet exceeds its actual child slots");
            const SotnBatRuntimeChild *c = &session->bat_runtime.children[owner];
            if (!c->pentagram || !c->initialized || !SotnBatRuntime_PentagramValid(c, owner) ||
                !DrawPlayerEffectPolygon(session, &c->state.pentagram.motion.polygons[row],
                                         row != 0) ||
                !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return SDL_SetError("Pentagram draw lost its actual twenty-five-row owner");
            continue;
        }
        if (command.kind == SOTN_DRAW_SIRE)
        {
            const SotnBatRuntimeChild *c = &session->bat_runtime.children[command.primitive];
            if (!c->sire || !c->initialized || !SotnBatRuntime_SireValid(c, command.primitive))
                return SDL_SetError("Sire draw lost its actual weapon18 child");
            const SotnSireParent *p = &c->state.sire.parent;
            const SotnSireChild *b = &c->state.sire.child;
            const bool burst = c->sire_child;
            const Uint8 flags = burst ? b->draw_flags : p->draw_flags;
            if (!SotnActorGraphics_DrawSurface(
                    session->actor_graphics,
                    (SotnActorSprite){.animation_set = burst ? b->bank : p->bank,
                                      .frame = c->state.sire.animation.frame,
                                      .palette = burst ? b->palette : p->palette,
                                      .texture_offset = burst ? b->texture : p->texture,
                                      .blend_mode = burst ? b->blend : p->blend,
                                      .facing_left = false},
                    Pixel(c->source.x) - view_x + session->draw_target.offset_x + session->shake.x,
                    Pixel(c->source.y) - view_y + session->draw_target.offset_y + session->shake.y,
                    (SotnSpriteTransform){
                        .scale_x = (flags & 1U) ? (burst ? b->scale_x : p->scale_x) : 256,
                        .scale_y = (flags & 2U) ? (burst ? b->scale_y : p->scale_y) : 256,
                        .pivot_x = burst ? 0 : p->pivot_x,
                        .sdk_scale = true,
                        .original_uv = true,
                        .opacity = 128},
                    session->draw_target.surface) ||
                !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return false;
            continue;
        }
        if (command.kind == SOTN_DRAW_NEUTRON)
        {
            const SotnBatRuntimeChild *c = &session->bat_runtime.children[command.primitive];
            const bool right = c->kind == 0xf0;
            const SotnNeutron *n = &c->state.neutron.motion;
            if (!c->neutron || !c->initialized ||
                !SotnBatRuntime_NeutronValid(c, command.primitive))
                return SDL_SetError("Neutron draw lost its actual weapon19 child");
            if (!SotnActorGraphics_DrawSurface(
                    session->actor_graphics,
                    (SotnActorSprite){.animation_set = right ? 18 : 16,
                                      .frame = c->state.neutron.animation.frame,
                                      .palette = right ? 0x128 : 0x110,
                                      .texture_offset = right ? 0x66 : 0x64,
                                      .blend_mode = (Uint8)n->blend,
                                      .facing_left = n->facing_left},
                    Pixel(c->source.x) - view_x + session->draw_target.offset_x + session->shake.x,
                    Pixel(c->source.y) - view_y + session->draw_target.offset_y + session->shake.y,
                    (SotnSpriteTransform){.rotation = n->rotation,
                                          .scale_x = n->scale_x,
                                          .scale_y = n->scale_y,
                                          .executable = session->executable,
                                          .sdk_rotation = true,
                                          .opacity = (n->draw_flags & 8U) ? n->opacity : 128},
                    session->draw_target.surface) ||
                !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return false;
            continue;
        }
        if (command.kind == SOTN_DRAW_SUBWEAPON_QUAD)
        {
            const Uint16 index = command.primitive / 64U, row = command.primitive % 64U;
            const SotnBatRuntimeChild *c = &session->bat_runtime.children[index];
            const SotnSubweaponEntity *m = &c->state.subweapon.motion;
            if (index < SOTN_BAT_RUNTIME_SLOTS && c->subweapon && c->initialized && !row &&
                (m->id == SOTN_SUBWEAPON_BIBLE || m->id == SOTN_BIBLE_TRAIL) &&
                SotnBatRuntime_SubweaponValid(c, index))
            {
                // The book: page 0x1E/palette 0x17F, raw texture (drawMode 0x100), 16x24.
                // A trail: page 0x1C/palette 0x19D, ABR0 half-transparent grey (0x115), 16x16.
                const bool book = m->id == SOTN_SUBWEAPON_BIBLE;
                const Sint64 cx = book ? Pixel(m->x) : m->trail.x,
                             cy = book ? Pixel(m->y) : m->trail.y;
                const Sint32 w = 8, h = book ? 12 : 8;
                const Uint8 grey = book ? 0x80 : (Uint8)m->trail.grey;
                SotnPlayerEffectQuad q = {
                    .u = {book ? 0x98 : 0x20, book ? 0xa8 : 0x30, book ? 0x98 : 0x20,
                          book ? 0xa8 : 0x30},
                    .v = {book ? 0xd8 : 0, book ? 0xd8 : 0, book ? 0xf0 : 0x10, book ? 0xf0 : 0x10},
                    .red = {grey, grey, grey, grey},
                    .green = {grey, grey, grey, grey},
                    .blue = {grey, grey, grey, grey},
                    .page = book ? 0x1e : 0x1c,
                    .palette = book ? 0x17f : 0x19d,
                    .priority = m->priority,
                    .draw_mode = book ? 0x100 : 0x115};
                for (Uint16 v = 0; v < 4; ++v)
                {
                    q.x[v] = BlinkWord(cx + ((v & 1U) ? w : -w) - view_x);
                    q.y[v] = BlinkWord(cy + ((v & 2U) ? h : -h) - view_y);
                }
                if (!DrawOriginalEffectQuad(session, &q) ||
                    !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                               SOTN_FRAME_PIXELS))
                    return false;
                continue;
            }
            if (index < SOTN_BAT_RUNTIME_SLOTS && c->subweapon && c->initialized &&
                row < SOTN_AXE_ROWS && m->id == SOTN_SUBWEAPON_AXE &&
                ((m->axe.visible >> row) & 1U) && SotnBatRuntime_SubweaponValid(c, index))
            {
                // Row 0 opaque (0x100); afterimages additive, tinted from D_800B0628 (0x135).
                const SotnAxe *a = &m->axe;
                Uint8 rgb[3] = {0};
                if (row && !SotnSubweaponEntity_AxeTint(session->dra, a->tint[row - 1U], rgb))
                    return false;
                SotnPlayerEffectQuad q = {.u = {0, 0x18, 0, 0x18},
                                          .v = {0, 0, 0x28, 0x28},
                                          .red = {rgb[0], rgb[0], rgb[0], rgb[0]},
                                          .green = {rgb[1], rgb[1], rgb[1], rgb[1]},
                                          .blue = {rgb[2], rgb[2], rgb[2], rgb[2]},
                                          .page = 0x1c,
                                          .palette = a->palette,
                                          .priority = a->row_priority,
                                          .draw_mode = row ? 0x135 : 0x100};
                for (Uint16 v = 0; v < 4; ++v)
                {
                    q.x[v] = a->x[row][v];
                    q.y[v] = a->y[row][v];
                }
                if (!DrawOriginalEffectQuad(session, &q) ||
                    !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                               SOTN_FRAME_PIXELS))
                    return false;
                continue;
            }
            if (index < SOTN_BAT_RUNTIME_SLOTS && c->subweapon && c->initialized && !row &&
                m->id == SOTN_SUBWEAPON_DAGGER && SotnBatRuntime_SubweaponValid(c, index))
            {
                // Page 0x1C/palette 0x1AB with its rotated UVs; DRAW_COLORS greys the fade.
                const SotnDagger *d = &m->dagger;
                SotnPlayerEffectQuad q = {.u = {0x18, 0x18, 0x20, 0x20},
                                          .v = {0x18, 0, 0x18, 0},
                                          .red = {d->grey, d->grey, d->grey, d->grey},
                                          .green = {d->grey, d->grey, d->grey, d->grey},
                                          .blue = {d->grey, d->grey, d->grey, d->grey},
                                          .page = 0x1c,
                                          .palette = 0x1ab,
                                          .priority = d->row_priority,
                                          .draw_mode = d->quad_mode};
                for (Uint16 v = 0; v < 4; ++v)
                {
                    q.x[v] = BlinkWord(d->x[v] - view_x);
                    q.y[v] = BlinkWord(d->y[v] - view_y);
                }
                if (!DrawOriginalEffectQuad(session, &q) ||
                    !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                               SOTN_FRAME_PIXELS))
                    return false;
                continue;
            }
            if (index < SOTN_BAT_RUNTIME_SLOTS && c->subweapon && c->initialized &&
                (m->id == SOTN_SUBWEAPON_CROSS || m->id == SOTN_GIANT_CROSS ||
                 m->id == SOTN_CROSS_PARTICLES) &&
                SotnBatRuntime_SubweaponValid(c, index))
            {
                if (!DrawCrossRow(session, m, row) ||
                    !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                               SOTN_FRAME_PIXELS))
                    return false;
                continue;
            }
            if (index >= SOTN_BAT_RUNTIME_SLOTS || !c->subweapon || !c->initialized ||
                c->state.subweapon.motion.id != SOTN_AGUNEA_BOLT ||
                !SotnBatRuntime_SubweaponValid(c, index) || !row ||
                row > c->state.subweapon.motion.bolt.end)
                return SDL_SetError("Agunea lightning draw lost its actual row ownership");
            const SotnAguneaBolt *b = &c->state.subweapon.motion.bolt;
            // Page 0x1A columns 0x90.. cycle every six rows; DRAW_COLORS modulates one grey.
            const Uint8 u = (Uint8)((row % 6U) * 16U + 0x90U);
            SotnPlayerEffectQuad q = {.u = {u, u, (Uint8)(u + 16U), (Uint8)(u + 16U)},
                                      .v = {0xd0, 0xc0, 0xd0, 0xc0},
                                      .red = {b->grey, b->grey, b->grey, b->grey},
                                      .green = {b->grey, b->grey, b->grey, b->grey},
                                      .blue = {b->grey, b->grey, b->grey, b->grey},
                                      .page = 0x1a,
                                      .palette = b->white ? 0x15f : 0x194,
                                      .priority = c->state.subweapon.motion.priority,
                                      .draw_mode = 6};
            for (Uint16 v = 0; v < 4; ++v)
            {
                q.x[v] = BlinkWord((Sint64)b->x[row][v] - view_x);
                q.y[v] = BlinkWord((Sint64)b->y[row][v] - view_y);
            }
            if (!DrawOriginalEffectQuad(session, &q) ||
                !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return false;
            continue;
        }
        if (command.kind == SOTN_DRAW_SUBWEAPON_LINE)
        {
            const Uint16 index = command.primitive / 16U, row = command.primitive % 16U;
            const SotnBatRuntimeChild *c = &session->bat_runtime.children[index];
            const bool agunea = c->state.subweapon.motion.id == SOTN_SUBWEAPON_AGUNEA,
                       dagger = c->state.subweapon.motion.id == SOTN_SUBWEAPON_DAGGER;
            if (index >= SOTN_BAT_RUNTIME_SLOTS || !c->subweapon || !c->initialized ||
                (c->state.subweapon.motion.id != SOTN_SUBWEAPON_REBOUND_STONE &&
                 !((agunea || dagger) && !row)) ||
                !SotnBatRuntime_SubweaponValid(c, index))
                return SDL_SetError("Subweapon LINE_G2 draw lost its actual row ownership");
            if (dagger)
            {
                const SotnDagger *d = &c->state.subweapon.motion.dagger;
                const Sint32 ox = session->draw_target.offset_x + session->shake.x - view_x,
                             oy = session->draw_target.offset_y + session->shake.y - view_y;
                // drawMode0x33: additive (ABR1), orange at the blade fading to black.
                const SotnColorLine line = {.x = {d->line_x[0] + ox, d->line_x[1] + ox},
                                            .y = {d->line_y[0] + oy, d->line_y[1] + oy},
                                            .red = {0x7f, 0},
                                            .green = {0x3f, 0},
                                            .blue = {0, 0},
                                            .blend = true,
                                            .blend_mode = (Uint8)((d->line_mode >> 5) & 3U)};
                if ((!(d->line_mode & 8U) &&
                     !SotnActorGraphics_LineSurface(line, session->draw_target.surface)) ||
                    !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                               SOTN_FRAME_PIXELS))
                    return false;
                continue;
            }
            if (agunea)
            {
                const SotnAgunea *a = &c->state.subweapon.motion.agunea;
                const Sint32 ox = session->draw_target.offset_x + session->shake.x - view_x,
                             oy = session->draw_target.offset_y + session->shake.y - view_y;
                // drawMode0x331: additive (ABR1), black at the throw point.
                const SotnColorLine line = {.x = {a->start_x + ox, a->end_x + ox},
                                            .y = {a->start_y + oy, a->end_y + oy},
                                            .red = {0, a->red},
                                            .green = {0, a->green},
                                            .blue = {0, a->blue},
                                            .blend = true,
                                            .blend_mode = (Uint8)((a->draw_mode >> 5) & 3U)};
                if (!(a->draw_mode & 8U) &&
                    !SotnActorGraphics_LineSurface(line, session->draw_target.surface))
                    return false;
                if (!SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                               SOTN_FRAME_PIXELS))
                    return false;
                continue;
            }
            const SotnReboundLine *l = &c->state.subweapon.motion.stone.lines[row];
            const Sint32 ox = session->draw_target.offset_x + session->shake.x - view_x,
                         oy = session->draw_target.offset_y + session->shake.y - view_y;
            // drawMode0x33: semi-transparent with ABR1 (additive); both ends share one grey.
            const SotnColorLine line = {.x = {l->x0 + ox, l->x1 + ox},
                                        .y = {l->y0 + oy, l->y1 + oy},
                                        .red = {l->grey, l->grey},
                                        .green = {l->grey, l->grey},
                                        .blue = {l->grey, l->grey},
                                        .blend = (l->draw_mode & 1U) != 0,
                                        .blend_mode = (Uint8)((l->draw_mode >> 5) & 3U)};
            if (!(l->draw_mode & 8U) &&
                !SotnActorGraphics_LineSurface(line, session->draw_target.surface))
                return false;
            if (!SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return false;
            continue;
        }
        if (command.kind == SOTN_DRAW_SUBWEAPON_TILE)
        {
            const Uint16 index = command.primitive / 16U, tile = command.primitive % 16U;
            if (index >= SOTN_BAT_RUNTIME_SLOTS || tile >= SOTN_VIBHUTI_PARTICLES)
                return SDL_SetError("Subweapon TILE draw exceeds its actual rows");
            const SotnBatRuntimeChild *c = &session->bat_runtime.children[index];
            if (!c->subweapon || !c->initialized || !SotnBatRuntime_SubweaponValid(c, index) ||
                c->state.subweapon.motion.id != SOTN_SUBWEAPON_VIBHUTI)
                return SDL_SetError("Subweapon TILE draw lost its actual row ownership");
            const SotnVibhutiParticle *t = &c->state.subweapon.motion.particles[tile];
            const Sint32 x = Pixel(t->x) - view_x + session->draw_target.offset_x +
                             session->shake.x,
                         y = Pixel(t->y) - view_y + session->draw_target.offset_y +
                             session->shake.y;
            if (!(t->draw_mode & 8U) &&
                !SotnActorGraphics_QuadSurface(
                    session->actor_graphics,
                    (SotnTextureQuad){.x = {x, x + t->width, x, x + t->width},
                                      .y = {y, y, y + t->height, y + t->height},
                                      .untextured = true,
                                      .red = 255,
                                      .green = 255,
                                      .blue = 255},
                    session->draw_target.surface))
                return false;
            if (!SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return false;
            continue;
        }
        if (command.kind == SOTN_DRAW_STAR_SPARK)
        {
            const Uint16 index = command.primitive / 32U, tile = command.primitive % 32U;
            const SotnBatRuntimeChild *c = &session->bat_runtime.children[index];
            if (!c->star || !c->initialized || !SotnBatRuntime_StarValid(c, index) ||
                c->primitive_count != 32 || (c->kind & 15U) != 3)
                return SDL_SetError("Weapon15 TILE draw lost actual row ownership");
            const SotnStarSpark *t = &c->state.star.motion.sparks[tile];
            const Sint32 x = t->x - view_x + session->draw_target.offset_x + session->shake.x,
                         y = t->y - view_y + session->draw_target.offset_y + session->shake.y;
            if (!(t->draw_mode & 8U) &&
                !SotnActorGraphics_QuadSurface(session->actor_graphics,
                                               (SotnTextureQuad){.x = {x, x + 1, x, x + 1},
                                                                 .y = {y, y, y + 1, y + 1},
                                                                 .untextured = true,
                                                                 .red = 128,
                                                                 .green = 64},
                                               session->draw_target.surface))
                return false;
            if (!SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return false;
            continue;
        }
        if (command.kind == SOTN_DRAW_HAND_WEAPON || command.kind == SOTN_DRAW_HAND_TILE)
        {
            const SotnBatRuntimeChild *c = &session->bat_runtime.children[0];
            const SotnHandWeapon *w = &c->state.hand.motion;
            if (!c->hand_weapon || !c->initialized || !SotnBatRuntime_HandValid(c, 0))
                return SDL_SetError("Ordinary hand draw lost its actual shared slot16 owner");
            if (command.kind == SOTN_DRAW_HAND_TILE &&
                (c->state.hand.family == 54 || c->state.hand.family == 56))
            {
                // Weapon54 trails are opaque screen-space LINE_G2 rows (drawMode0x300).
                if (c->primitive_count != SOTN_HAND_WEAPON_GURTHANG_ROWS ||
                    command.primitive >= SOTN_HAND_WEAPON_GURTHANG_ROWS)
                    return SDL_SetError("Ordinary Gurthang trail lost its actual optional row");
                const SotnPlayerEffectQuad *q = &w->tiles[command.primitive];
                const Sint32 ox = session->draw_target.offset_x + session->shake.x,
                             oy = session->draw_target.offset_y + session->shake.y;
                const SotnColorLine line = {.x = {q->x[0] + ox, q->x[1] + ox},
                                            .y = {q->y[0] + oy, q->y[1] + oy},
                                            .red = {q->red[0], q->red[1]},
                                            .green = {q->green[0], q->green[1]},
                                            .blue = {q->blue[0], q->blue[1]}};
                if (!SotnActorGraphics_LineSurface(line, session->draw_target.surface))
                    return false;
            }
            else if (command.kind == SOTN_DRAW_HAND_TILE && c->state.hand.family == 1)
            {
                // Weapon1 punch trails are textured screen-space GT4s.
                if (c->primitive_count != SOTN_HAND_WEAPON_FIST_ROWS ||
                    command.primitive >= SOTN_HAND_WEAPON_FIST_ROWS)
                    return SDL_SetError("Ordinary fist trail lost its actual mandatory row");
                if (!DrawPlayerEffectQuad(session, &w->tiles[command.primitive]))
                    return false;
            }
            else if (command.kind == SOTN_DRAW_HAND_TILE)
            {
                if (c->primitive_count != 16 || command.primitive >= 16)
                    return SDL_SetError("Ordinary sword TILE lost its actual optional row");
                const SotnPlayerEffectQuad *p = &w->tiles[command.primitive];
                const Sint32 x = p->x[0] + session->draw_target.offset_x + session->shake.x,
                             y = p->y[0] + session->draw_target.offset_y + session->shake.y;
                if (!SotnActorGraphics_QuadSurface(
                        session->actor_graphics,
                        (SotnTextureQuad){.x = {x, x + p->u[0], x, x + p->u[0]},
                                          .y = {y, y, y + p->v[0], y + p->v[0]},
                                          .page = (Uint16)(p->draw_mode & 0x60U),
                                          .untextured = true,
                                          .blend = (p->draw_mode & 1U) != 0,
                                          .red = p->red[0],
                                          .green = p->green[0],
                                          .blue = p->blue[0]},
                        session->draw_target.surface))
                    return false;
            }
            else if (!SotnActorGraphics_DrawSurface(
                         session->actor_graphics,
                         (SotnActorSprite){.animation_set = (Uint16)(w->bank & 0x7fffU),
                                           .frame = w->frame,
                                           .palette = w->palette,
                                           .texture_offset = w->texture,
                                           .blend_mode = w->blend,
                                           .facing_left = w->facing_left},
                         Pixel(w->x) - view_x + session->draw_target.offset_x + session->shake.x,
                         Pixel(w->y) - view_y + session->draw_target.offset_y + session->shake.y,
                         (SotnSpriteTransform){.scale_x = 256,
                                               .scale_y = (w->draw_flags & 2U) ? w->scale_y : 256,
                                               .rotation = (w->draw_flags & 4U) ? w->rotation : 0,
                                               .pivot_y = w->pivot_y,
                                               .opacity = 128,
                                               .executable = session->executable,
                                               .sdk_rotation = true},
                         session->draw_target.surface))
                return false;
            if (!SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return false;
            continue;
        }
        if (command.kind == SOTN_DRAW_WEAPON_SPARKLE)
        {
            const Uint16 owner = (Uint16)(command.primitive / 80U),
                         index = (Uint16)(command.primitive % 80U);
            const SotnBatRuntimeChild *c = &session->bat_runtime.children[owner];
            const SotnWeaponSecondary *w = &c->state.secondary.motion;
            if (c->weapon_secondary && c->state.secondary.family == 14)
            {
                if (!c->initialized || (c->kind & 15U) || index >= c->primitive_count ||
                    w->rune_hidden[index] || !SotnBatRuntime_SecondaryValid(c, owner))
                    return SDL_SetError("Runesword trail draw lost its actual root/row lease");
                // Row i spans the blade at callbacks i and i+1: its x0/x2 repeat row i-1's
                // x1/x3 (row0 keeps its birth corners). drawMode0x173, tpage0x19.
                const bool right = (c->parameter & 0x8000U) != 0;
                const Sint32 x0 = index ? w->rune_x1[index - 1U] : w->rune_x0,
                             y0 = index ? w->rune_y1[index - 1U] : w->rune_y0,
                             x2 = index ? w->rune_x3[index - 1U] : w->rune_x2,
                             y2 = index ? w->rune_y3[index - 1U] : w->rune_y2;
                const Uint8 v0 = right ? 0xe0 : 0x60, v2 = right ? 0xff : 0x7f;
                const SotnPlayerEffectQuad q = {
                    .x = {BlinkWord(x0 - view_x), BlinkWord(w->rune_x1[index] - view_x),
                          BlinkWord(x2 - view_x), BlinkWord(w->rune_x3[index] - view_x)},
                    .y = {BlinkWord(y0 - view_y), BlinkWord(w->rune_y1[index] - view_y),
                          BlinkWord(y2 - view_y), BlinkWord(w->rune_y3[index] - view_y)},
                    .u = {w->rune_u0[index], w->rune_u1[index], w->rune_u0[index],
                          w->rune_u1[index]},
                    .v = {v0, v0, v2, v2},
                    .green = {0x10},
                    .red = {1},
                    .palette = right ? 0x135 : 0x11d,
                    .page = 0x19,
                    .priority = w->row_priority,
                    .draw_mode = w->draw_mode};
                if (!DrawOriginalEffectQuad(session, &q) ||
                    !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                               SOTN_FRAME_PIXELS))
                    return false;
                continue;
            }
            if (!c->weapon_secondary || !c->initialized || c->state.secondary.family != 50 ||
                (c->kind & 15U) != 1 || index >= c->primitive_count || !w->ice_size[index] ||
                !SotnBatRuntime_SecondaryValid(c, owner))
                return SDL_SetError("Weapon50 sparkle draw lost its actual child/row lease");
            // drawMode0x331 (TPAGE|TPAGE2|UNK_100|UNK_200, transparent) with palette0x1B0.
            const Sint16 z = w->ice_size[index], x = w->ice_cx[index], y = w->ice_cy[index];
            SotnPlayerEffectQuad q = {
                .x = {(Sint16)(x - z), (Sint16)(x + z), (Sint16)(x - z), (Sint16)(x + z)},
                .y = {(Sint16)(y - z), (Sint16)(y - z), (Sint16)(y + z), (Sint16)(y + z)},
                .palette = 0x1b0,
                .page = 0x1a,
                .priority = w->priority,
                .draw_mode = w->draw_mode};
            for (Uint16 k = 0; k < 4; ++k)
            {
                q.u[k] = w->ice_uv[index][k * 2U];
                q.v[k] = w->ice_uv[index][k * 2U + 1U];
            }
            if (!DrawOriginalEffectQuad(session, &q) ||
                !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return false;
            continue;
        }
        if (command.kind == SOTN_DRAW_WEAPON_SECONDARY_QUAD)
        {
            // Commands pack owner*64+row: Iron shield's spell owns thirty-three rows.
            const Uint16 owner = (Uint16)(command.primitive / 64U),
                         index = (Uint16)(command.primitive % 64U);
            const SotnBatRuntimeChild *c = &session->bat_runtime.children[owner];
            const bool slash = c->state.secondary.family == 53 || c->state.secondary.family == 56 ||
                               c->state.secondary.family == 51 ||
                               (c->state.secondary.family == 12 && !(c->kind & 15U));
            const bool dawn = c->state.secondary.family == 30,
                       spell = SotnWeaponSecondary_ShieldSpell(c->state.secondary.family),
                       jewel = c->state.secondary.family == 37 && (c->kind & 15U) == 1,
                       knuckle = c->state.secondary.family == 46,
                       dynamite = c->state.secondary.family == 47 && !(c->kind & 15U),
                       refresh = c->state.secondary.family == 44;
            if (!c->weapon_secondary || !c->initialized || index >= c->primitive_count ||
                (!slash && !dawn && !spell && !jewel && !knuckle && !dynamite && !refresh &&
                 (c->kind & 15U) != 2 && c->state.secondary.family != 1 &&
                 !(c->state.secondary.family == 50 && (c->kind & 15U) == 4)) ||
                (slash && index) || !SotnBatRuntime_SecondaryValid(c, owner))
                return SDL_SetError("Weapon secondary GT4 draw lost its actual child/row lease");
            const SotnWeaponSecondary *w = &c->state.secondary.motion;
            SotnPlayerEffectQuad q = slash      ? w->quad
                                     : dawn     ? w->dawn_quads[index]
                                     : spell    ? w->spell_quads[index]
                                     : jewel    ? w->jewel_quads[index]
                                     : knuckle  ? w->knuckle_quads[index]
                                     : dynamite ? w->dyn_quads[index]
                                     : refresh  ? w->refresh_quads[index]
                                                : w->quads[index];
            // Dynamite's rows are world pixels: 1x1 fuse TILEs (u0/v0 extents, flat colour) until
            // the explosion converts them to GT4 rays.
            if (dynamite)
            {
                SotnPlayerEffectQuad screen = q;
                for (Uint16 v = 0; v < 4; ++v)
                {
                    const Sint32 x = w->dyn_gt4 ? q.x[v] : q.x[0] + (v & 1U ? q.u[0] : 0),
                                 y = w->dyn_gt4 ? q.y[v] : q.y[0] + (v & 2U ? q.v[0] : 0);
                    screen.x[v] = BlinkWord(x - view_x);
                    screen.y[v] = BlinkWord(y - view_y);
                    if (!w->dyn_gt4)
                    {
                        screen.red[v] = q.red[0];
                        screen.green[v] = q.green[0];
                        screen.blue[v] = q.blue[0];
                    }
                }
                if (!(w->dyn_gt4 ? DrawOriginalEffectQuad(session, &screen)
                                 : DrawIceQuad(session, &screen)) ||
                    !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                               SOTN_FRAME_PIXELS))
                    return false;
                continue;
            }
            // Blue knuckles' arc rows are untextured G4s (prim->type = PRIM_G4).
            if (knuckle && (c->kind & 15U) == 1)
            {
                if (!DrawIceQuad(session, &q) ||
                    !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                               SOTN_FRAME_PIXELS))
                    return false;
                continue;
            }
            if (dawn && (c->kind & 15U) == 1 && index == 10 && w->dawn_line)
            {
                // The circle's row10 tether is a LINE_G2 (ABR from drawMode bits5..6).
                const Sint32 ox = session->draw_target.offset_x + session->shake.x + Pixel(w->x) -
                                  view_x,
                             oy = session->draw_target.offset_y + session->shake.y + Pixel(w->y) -
                                  view_y;
                const SotnColorLine line = {.x = {q.x[0] + ox, q.x[1] + ox},
                                            .y = {q.y[0] + oy, q.y[1] + oy},
                                            .red = {q.red[0], q.red[1]},
                                            .green = {q.green[0], q.green[1]},
                                            .blue = {q.blue[0], q.blue[1]},
                                            .blend = (q.draw_mode & 1U) != 0,
                                            .blend_mode = (Uint8)((q.draw_mode >> 5) & 3U)};
                if (!SotnActorGraphics_LineSurface(line, session->draw_target.surface) ||
                    !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                               SOTN_FRAME_PIXELS))
                    return false;
                continue;
            }
            // Weapon24's spell rows0/1 are screen-fixed TILEs (x0/y0, u0/v0 extents, flat
            // colour); drawMode0x51 selects subtractive blending, darkening the screen.
            if (spell && c->state.secondary.family == 24 && (c->kind & 15U) == 8 && index < 2)
            {
                SotnPlayerEffectQuad tile = q;
                for (Uint16 v = 0; v < 4; ++v)
                {
                    tile.x[v] = BlinkWord(q.x[0] + (v & 1U ? q.u[0] : 0));
                    tile.y[v] = BlinkWord(q.y[0] + (v & 2U ? q.v[0] : 0));
                    tile.red[v] = q.red[0];
                    tile.green[v] = q.green[0];
                    tile.blue[v] = q.blue[0];
                }
                if (!DrawIceQuad(session, &tile) ||
                    !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                               SOTN_FRAME_PIXELS))
                    return false;
                continue;
            }
            // Weapon52's spell rows12..22 are untextured G4 ribbons (prim->type = PRIM_G4).
            if (spell && c->state.secondary.family == 52 && (c->kind & 15U) == 8 && index >= 12)
            {
                if (!DrawIceQuad(session, &q) ||
                    !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                               SOTN_FRAME_PIXELS))
                    return false;
                continue;
            }
            // Weapon53/51 and weapon30's circle offsets follow the entity (DRAW_UNK02); weapon30's
            // soldier rows are world pixels; ring and rush are screen-space.
            const bool world = dawn && (c->kind & 15U) == 2;
            for (Uint16 v = 0; (slash || dawn) && v < 4; ++v)
            {
                q.x[v] = BlinkWord(q.x[v] + (world ? 0 : Pixel(w->x)) - view_x);
                q.y[v] = BlinkWord(q.y[v] + (world ? 0 : Pixel(w->y)) - view_y);
            }
            if (!DrawOriginalEffectQuad(session, &q) ||
                !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return false;
            continue;
        }
        if (command.kind == SOTN_DRAW_WEAPON_SECONDARY ||
            command.kind == SOTN_DRAW_WEAPON_SECONDARY_LINE)
        {
            const Uint16 owner = command.kind == SOTN_DRAW_WEAPON_SECONDARY
                                     ? command.primitive
                                     : (Uint16)(command.primitive / 64U),
                         row = (Uint16)(command.primitive % 64U);
            const SotnBatRuntimeChild *c = &session->bat_runtime.children[owner];
            const SotnWeaponSecondary *w = &c->state.secondary.motion;
            if (!c->weapon_secondary || !c->initialized ||
                !SotnBatRuntime_SecondaryValid(c, owner) ||
                (command.kind == SOTN_DRAW_WEAPON_SECONDARY_LINE && row >= c->primitive_count))
                return SDL_SetError("Weapon secondary draw lost its actual child/row lease");
            if (command.kind == SOTN_DRAW_WEAPON_SECONDARY_LINE)
            {
                // Original drawMode0x331/0x339: semi-transparent LINE_G2 whose DR_MODE selects
                // ABR from bits5..6 (additive for all selected secondaries).
                // Weapon13's combo sweep keeps its one row beside its motion words.
                const SotnWeaponSecondaryRow *q =
                    c->state.secondary.family == 13 ? &w->heaven_line : &w->rows[row];
                const Sint32 ox = session->draw_target.offset_x + session->shake.x,
                             oy = session->draw_target.offset_y + session->shake.y;
                const SotnColorLine line = {.x = {q->x[0] + ox, q->x[1] + ox},
                                            .y = {q->y[0] + oy, q->y[1] + oy},
                                            .red = {q->red[0], q->red[1]},
                                            .green = {q->green[0], q->green[1]},
                                            .blue = {q->blue[0], q->blue[1]},
                                            .blend = (w->draw_mode & 1U) != 0,
                                            .blend_mode = (Uint8)((w->draw_mode >> 5) & 3U)};
                if (!SotnActorGraphics_LineSurface(line, session->draw_target.surface))
                    return false;
            }
            else if (c->state.secondary.family == 49 || c->state.secondary.family == 6 ||
                     c->state.secondary.family == 7 || c->state.secondary.family == 14 ||
                     c->state.secondary.family == 13 || c->state.secondary.family == 12 ||
                     c->state.secondary.family == 34 || c->state.secondary.family == 30 ||
                     c->state.secondary.family == 37 || c->state.secondary.family == 47 ||
                     c->state.secondary.family == 40 || c->state.secondary.family == 41 ||
                     c->state.secondary.family == 42 ||
                     SotnWeaponSecondary_ShieldSpell(c->state.secondary.family))
            {
                // w_037 coins draw DRA bank3 (the gold coin) with their parts' palettes.
                // w_049 sparks draw DRA bank9 rotated (ENTITY_ROTATE); puffs and pillars use
                // its SetSpriteBank2 frames with SDK X/Y scale; w_006 crescents use bank1.
                const Sint32 x =
                    Pixel(w->x) - view_x + session->draw_target.offset_x + session->shake.x;
                const Sint32 y =
                    Pixel(w->y) - view_y + session->draw_target.offset_y + session->shake.y;
                const SotnActorSprite sprite = {.animation_set = (Uint16)(w->bank & 0x7fffU),
                                                .frame = w->frame,
                                                .palette = w->palette,
                                                .texture_offset = w->texture,
                                                .blend_mode = w->blend,
                                                .facing_left = w->facing_left};
                const SotnSpriteTransform t = {
                    .rotation = (w->draw_flags & 4U) ? w->rotate : 0,
                    .scale_x = (w->draw_flags & 1U) ? w->scale_x : 256,
                    .scale_y = (w->draw_flags & 2U) ? w->scale_y : 256,
                    // Weapon40/42's monsters scale about rotPivotY.
                    .pivot_y = c->state.secondary.family == 40 || c->state.secondary.family == 42
                                   ? w->pivot_y
                                   : 0,
                    // Weapon13 afterimages shade by entity opacity;
                    // ENTITY_MASK_R/G/B hold their channel at 128
                    // (weapon29's afterimages fade to red).
                    .opacity = (w->draw_flags & 8U) ? w->opacity : 128,
                    .tint = (w->draw_flags & 8U) && (w->draw_flags & 0x70U),
                    .red = (w->draw_flags & 0x10U) ? 128 : w->opacity,
                    .green = (w->draw_flags & 0x20U) ? 128 : w->opacity,
                    .blue = (w->draw_flags & 0x40U) ? 128 : w->opacity,
                    .executable = session->executable,
                    .sdk_rotation = true};
                const Uint16 bank = (Uint16)(w->bank & 0x7fffU);
                // A zero ENTITY_SCALEX/Y collapses the sprite to nothing (shield spells' birth).
                const bool collapsed =
                    ((w->draw_flags & 1U) && !w->scale_x) || ((w->draw_flags & 2U) && !w->scale_y);
                if (!collapsed &&
                    !(bank == 17 || bank == 19 ? SotnActorGraphics_DrawSecondaryWeaponSurface
                                               : SotnActorGraphics_DrawSurface)(
                        session->actor_graphics, sprite, x, y, t, session->draw_target.surface))
                    return false;
            }
            else if (c->state.secondary.family == 1)
            {
                // w_001's fireball draws DRA animSet9 with its own SDK X/Y scales.
                if (!SotnActorGraphics_DrawScaled(
                        session->actor_graphics,
                        (SotnActorSprite){.animation_set = w->bank,
                                          .frame = w->frame,
                                          .palette = w->palette,
                                          .blend_mode = w->blend,
                                          .facing_left = w->facing_left},
                        Pixel(w->x) - view_x + session->draw_target.offset_x + session->shake.x,
                        Pixel(w->y) - view_y + session->draw_target.offset_y + session->shake.y,
                        (Uint16)w->scale_x, (Uint16)w->scale_y, session->draw_target.surface))
                    return false;
            }
            else if (!SotnActorGraphics_DrawSecondaryWeaponSurface(
                         session->actor_graphics,
                         (SotnActorSprite){.animation_set = (Uint16)(w->bank & 0x7fffU),
                                           .frame = w->frame,
                                           .palette = w->palette,
                                           .texture_offset = w->texture,
                                           .blend_mode = w->blend,
                                           .facing_left = w->facing_left},
                         Pixel(w->x) - view_x + session->draw_target.offset_x + session->shake.x,
                         Pixel(w->y) - view_y + session->draw_target.offset_y + session->shake.y,
                         (SotnSpriteTransform){.scale_x = 256,
                                               .scale_y = (w->draw_flags & 2U) ? w->scale_y : 256,
                                               .pivot_y = w->pivot_y,
                                               .opacity = 128,
                                               .executable = session->executable,
                                               .sdk_rotation = true},
                         session->draw_target.surface))
                return false;
            if (!SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return false;
            continue;
        }
        if (command.kind == SOTN_DRAW_KARMA_QUAD)
        {
            const Uint16 owner = command.primitive / SOTN_KARMA_ROWS,
                         row = command.primitive % SOTN_KARMA_ROWS;
            const SotnBatRuntimeChild *c = &session->bat_runtime.children[owner];
            if (!c->karma || !c->initialized || !SotnBatRuntime_KarmaValid(c, owner) ||
                row >= c->primitive_count)
                return SDL_SetError("Karma draw lost its actual26-row lease");
            const SotnPlayerEffectQuad *q = &c->state.karma.motion.quads[row];
            if (!(row == 0 ? SotnActorGraphics_PlayerColorEffect(
                                 q, session->draw_target.offset_x + session->shake.x,
                                 session->draw_target.offset_y + session->shake.y,
                                 session->draw_target.surface)
                           : DrawPlayerEffectQuad(session, q)))
                return false;
            continue;
        }
        if (command.kind == SOTN_DRAW_KARMA)
        {
            const SotnBatRuntimeChild *c = &session->bat_runtime.children[command.primitive];
            const SotnKarmaCoin *m = &c->state.karma.motion;
            if (!c->karma || !c->initialized || !m->bank ||
                !SotnBatRuntime_KarmaValid(c, command.primitive))
                return SDL_SetError("Karma sprite lost actual callback/raw-bank ownership");
            const Uint16 bank =
                m->bank >= 0x8010 && m->bank <= 0x8013 ? (Uint16)(m->bank & 0x7fffU) : m->bank;
            if (!(m->bank == 0x8011 || m->bank == 0x8013
                      ? SotnActorGraphics_DrawSecondaryWeaponSurface
                      : SotnActorGraphics_DrawSurface)(
                    session->actor_graphics,
                    (SotnActorSprite){.animation_set = bank,
                                      .frame = m->frame,
                                      .palette = m->palette,
                                      .texture_offset = m->texture,
                                      .blend_mode = m->blend,
                                      .facing_left = m->facing_left},
                    Pixel(c->source.x) - view_x + session->draw_target.offset_x + session->shake.x,
                    Pixel(c->source.y) - view_y + session->draw_target.offset_y + session->shake.y,
                    (SotnSpriteTransform){.scale_x = 256,
                                          .scale_y = 256,
                                          .opacity = m->draw_flags & 8U ? m->opacity : 128,
                                          .executable = session->executable,
                                          .sdk_rotation = true},
                    session->draw_target.surface) ||
                !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return false;
            continue;
        }
        if (command.kind == SOTN_DRAW_STAR)
        {
            const SotnBatRuntimeChild *c = &session->bat_runtime.children[command.primitive];
            const SotnThrownWeapon *m = &c->state.star.motion.motion;
            if (!c->star || !c->initialized || !m->bank ||
                !SotnBatRuntime_StarValid(c, command.primitive))
                return SDL_SetError("Weapon15 draw lost its actual callback/row/parent ownership");
            const Uint16 bank =
                m->bank >= 0x8010 && m->bank <= 0x8013 ? (Uint16)(m->bank & 0x7fffU) : m->bank;
            if (!(m->bank == 0x8011 || m->bank == 0x8013
                      ? SotnActorGraphics_DrawSecondaryWeaponSurface
                      : SotnActorGraphics_DrawSurface)(
                    session->actor_graphics,
                    (SotnActorSprite){.animation_set = bank,
                                      .frame = m->frame,
                                      .palette = m->palette,
                                      .texture_offset = m->texture,
                                      .blend_mode = m->blend,
                                      .facing_left = m->facing_left},
                    Pixel(c->source.x) - view_x + session->draw_target.offset_x + session->shake.x,
                    Pixel(c->source.y) - view_y + session->draw_target.offset_y + session->shake.y,
                    (SotnSpriteTransform){
                        .scale_x = m->draw_flags & 1U ? c->state.star.motion.scale_x : 256,
                        .scale_y = m->draw_flags & 2U ? c->state.star.motion.scale_y : 256,
                        .rotation = m->draw_flags & 4U ? m->rotation : 0,
                        .opacity = m->draw_flags & 8U ? m->opacity : 128,
                        .executable = session->executable,
                        .sdk_rotation = true},
                    session->draw_target.surface) ||
                !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return false;
            continue;
        }
        if (command.kind == SOTN_DRAW_THROWN)
        {
            const SotnBatRuntimeChild *c = &session->bat_runtime.children[command.primitive];
            const SotnThrownWeapon *m = &c->state.thrown.motion;
            if (!c->thrown || !c->initialized || !m->bank ||
                !SotnBatRuntime_ThrownValid(c, command.primitive))
                return SDL_SetError("Weapon16 draw lost its actual callback/row/parent ownership");
            const Uint16 bank =
                m->bank >= 0x8010 && m->bank <= 0x8013 ? (Uint16)(m->bank & 0x7fffU) : m->bank;
            if (!(m->bank == 0x8011 || m->bank == 0x8013
                      ? SotnActorGraphics_DrawSecondaryWeaponSurface
                      : SotnActorGraphics_DrawSurface)(
                    session->actor_graphics,
                    (SotnActorSprite){.animation_set = bank,
                                      .frame = m->frame,
                                      .palette = m->palette,
                                      .texture_offset = m->texture,
                                      .blend_mode = m->blend,
                                      .facing_left = m->facing_left},
                    Pixel(c->source.x) - view_x + session->draw_target.offset_x + session->shake.x,
                    Pixel(c->source.y) - view_y + session->draw_target.offset_y + session->shake.y,
                    (SotnSpriteTransform){.scale_x = 256,
                                          .scale_y = 256,
                                          .rotation = m->draw_flags & 4U ? m->rotation : 0,
                                          .opacity = m->draw_flags & 8U ? m->opacity : 128,
                                          .executable = session->executable,
                                          .sdk_rotation = true},
                    session->draw_target.surface) ||
                !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return false;
            continue;
        }
        if (command.kind == SOTN_DRAW_MAGIC_MISSILE)
        {
            const SotnBatRuntimeChild *c = &session->bat_runtime.children[command.primitive];
            const SotnMagicMissile *m = &c->state.magic.motion;
            if (!c->magic || !c->initialized || !m->bank ||
                !SotnBatRuntime_MagicMissileValid(c, command.primitive))
                return SDL_SetError("Magic Missile draw lost its actual weapon22 child");
            // Native weapon16/18 bind the actual hand overlays. Other raw banks retain
            // the original parent identity, including a replaced Wolf parent bank15.
            const Uint16 bank =
                m->bank >= 0x8010 && m->bank <= 0x8013 ? (Uint16)(m->bank & 0x7fffU) : m->bank;
            if (!(m->bank == 0x8011 || m->bank == 0x8013
                      ? SotnActorGraphics_DrawSecondaryWeaponSurface
                      : SotnActorGraphics_DrawSurface)(
                    session->actor_graphics,
                    (SotnActorSprite){.animation_set = bank,
                                      .frame = c->state.magic.animation.frame,
                                      .palette = m->palette,
                                      .texture_offset = m->texture,
                                      .facing_left = m->facing_left},
                    Pixel(c->source.x) - view_x + session->draw_target.offset_x + session->shake.x,
                    Pixel(c->source.y) - view_y + session->draw_target.offset_y + session->shake.y,
                    (SotnSpriteTransform){.scale_x = 256, .scale_y = 256, .opacity = 128},
                    session->draw_target.surface) ||
                !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return false;
            continue;
        }
        if (command.kind == SOTN_DRAW_FOOD)
        {
            const SotnBatRuntimeChild *c = &session->bat_runtime.children[command.primitive];
            const bool right = c->kind >= 0xf0;
            if (!c->food || !c->initialized || !SotnBatRuntime_FoodChildValid(c, command.primitive))
                return SDL_SetError("Food draw lost its actual weapon21 child");
            if (!SotnActorGraphics_DrawSurface(
                    session->actor_graphics,
                    (SotnActorSprite){.animation_set = right ? 18 : 16,
                                      .frame = c->state.food.frame,
                                      .palette = right ? 0x128 : 0x110,
                                      .texture_offset = right ? 0x66 : 0x64,
                                      .facing_left = c->source.facing_left},
                    Pixel(c->source.x) - view_x + session->draw_target.offset_x,
                    Pixel(c->source.y) - view_y + session->draw_target.offset_y,
                    (SotnSpriteTransform){.scale_x = 256, .scale_y = 256, .opacity = 128},
                    session->draw_target.surface) ||
                !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return false;
            continue;
        }
        if (command.kind == SOTN_DRAW_SWORD)
        {
            const SotnBatRuntimeChild *c = &session->bat_runtime.children[command.primitive];
            // Weapon53's warp redraws Alucard's current frame like the Alucard-sword image.
            const bool image = c->kind == 0xe1 || c->kind == 0xf1 || c->weapon_warp;
            if (!c->generation || !c->initialized || c->karma || c->weapon_secondary ||
                (c->weapon_warp && !SotnBatRuntime_WarpValid(c, command.primitive)) ||
                (!image && c->kind != 0xe2 && c->kind != 0xf2))
                return SDL_SetError("Sword draw lost its actual initialized child");
            if (image)
            {
                SotnPlayerState copy = player;
                copy.x = c->source.x;
                copy.y = c->source.y;
                copy.facing_left = c->source.facing_left;
                copy.palette = 0x8100;
                copy.body_color_blend = false;
                copy.water_death.active = false;
                if (!DrawRotatedPlayer(session, copy, &alucard,
                                       c->weapon_warp ? c->state.warp.blend
                                                      : c->state.sword.image.blend))
                    return false;
            }
            else
            {
                const Uint16 hand = c->kind == 0xe2 ? 0 : 1;
                const bool rebind = session->hands[hand] != 123;
                const SotnDataView weapons = SotnResource_View(session->weapons);
                SotnEquipment sword = {0}, current = {0};
                if (rebind &&
                    (!SotnEquipment_Read(session->dra, 123, &sword) ||
                     !SotnEquipment_Read(session->dra, session->hands[hand], &current) ||
                     !SotnActorGraphics_LoadWeapon(session->actor_graphics, weapons, hand, sword)))
                    return false;
                const bool drawn = SotnActorGraphics_DrawSurface(
                    session->actor_graphics,
                    (SotnActorSprite){.animation_set = c->kind == 0xe2 ? 16 : 18,
                                      .frame = c->state.sword.strike.frame,
                                      .palette = c->kind == 0xe2 ? 0x110 : 0x128,
                                      .texture_offset = c->kind == 0xe2 ? 0x64 : 0x66,
                                      .blend_mode = 0x30,
                                      .facing_left = c->source.facing_left},
                    Pixel(c->source.x) - view_x + session->draw_target.offset_x,
                    Pixel(c->source.y) - view_y + session->draw_target.offset_y,
                    (SotnSpriteTransform){.rotation = player.gravity.upside_down ? 0x800 : 0,
                                          .scale_x = 256,
                                          .scale_y = 256,
                                          .opacity = 128},
                    session->draw_target.surface);
                char detail[512] = {0};
                if (!drawn)
                    SDL_strlcpy(detail, SDL_GetError(), sizeof(detail));
                if (rebind &&
                    !SotnActorGraphics_LoadWeapon(session->actor_graphics, weapons, hand, current))
                    return false;
                if (!drawn)
                    return SDL_SetError("%s", detail);
            }
            if (!SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return false;
            continue;
        }
        if (command.kind == SOTN_DRAW_FIRE_HIT)
        {
            if (command.primitive >= SOTN_BAT_RUNTIME_SLOTS)
                return SDL_SetError("Fire sprite exceeds its actual child slots");
            const SotnBatRuntimeChild *child = &session->bat_runtime.children[command.primitive];
            const SotnFireHit *fire = &child->state.fire_hit;
            if (child->kind != 16 || !child->initialized || fire->removed || !fire->phase)
                return SDL_SetError("Fire sprite lost its actual rowless child owner");
            if (!SotnActorGraphics_DrawScaledOriginal(
                    session->actor_graphics,
                    (SotnActorSprite){.animation_set = fire->bank,
                                      .frame = fire->frame,
                                      .palette = fire->palette,
                                      .blend_mode = fire->blend_mode,
                                      .facing_left = child->source.facing_left},
                    Pixel(fire->x) + session->draw_target.offset_x + session->shake.x,
                    Pixel(fire->y) + session->draw_target.offset_y + session->shake.y,
                    fire->draw_flags & 1U ? fire->scale_x : 256,
                    fire->draw_flags & 2U ? fire->scale_y : 256, session->draw_target.surface) ||
                !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return false;
            continue;
        }
        if (command.kind == SOTN_DRAW_DARK_HIT)
        {
            if (command.primitive >= SOTN_BAT_RUNTIME_SLOTS)
                return SDL_SetError("Dark sprite exceeds its actual child slots");
            const SotnBatRuntimeChild *child = &session->bat_runtime.children[command.primitive];
            const SotnDarkHit *dark = &child->state.dark;
            if (child->kind != 54 || !child->initialized || dark->removed || !dark->phase)
                return SDL_SetError("Dark sprite lost its actual rowless child owner");
            // Original flags33 omit OPACITY8: its decreasing opacity is not a color fade.
            if (!SotnActorGraphics_DrawScaledOriginal(
                    session->actor_graphics,
                    (SotnActorSprite){.animation_set = 14,
                                      .frame = dark->frame,
                                      .palette = 0x819f,
                                      .texture_offset = 0x79,
                                      .blend_mode = dark->blend_mode,
                                      .facing_left = child->source.facing_left},
                    Pixel(dark->x) + session->draw_target.offset_x + session->shake.x,
                    Pixel(dark->y) + session->draw_target.offset_y + session->shake.y,
                    dark->scale_x, dark->scale_y, session->draw_target.surface) ||
                !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return false;
            continue;
        }
        if (command.kind == SOTN_DRAW_HELLFIRE_SPRITE)
        {
            if (command.primitive >= 48)
                return SDL_SetError("Hellfire sprite exceeds its actual child slots");
            const SotnBatRuntimeChild *child = &session->bat_runtime.children[command.primitive];
            if (!child->generation || !child->initialized ||
                (child->kind != 24 && child->kind != 26 && child->kind != 27))
                return SDL_SetError("Hellfire sprite lost its actual child owner");
            const bool trail = child->kind == 24;
            const SotnHellfireTrail retained =
                trail ? child->state.hellfire_trail.motion : (SotnHellfireTrail){0};
            const SotnHellfireFireball fire =
                trail ? (SotnHellfireFireball){0} : child->state.hellfire_fire.motion;
            if (!SotnActorGraphics_DrawSurface(
                    session->actor_graphics,
                    (SotnActorSprite){.animation_set = trail ? 2 : 9,
                                      .frame = trail ? child->state.hellfire_trail.frame
                                                     : child->state.hellfire_fire.frame,
                                      .palette = trail ? retained.palette : 0,
                                      .blend_mode = trail ? retained.blend_mode : 0,
                                      .facing_left = child->source.facing_left},
                    Pixel(child->source.x) - view_x + session->draw_target.offset_x,
                    Pixel(child->source.y) - view_y + session->draw_target.offset_y,
                    (SotnSpriteTransform){
                        .rotation = trail ? 0 : fire.rotation,
                        .scale_x =
                            trail && (retained.draw_flags & 1U) ? (Sint16)retained.scale_x : 256,
                        .scale_y =
                            trail && (retained.draw_flags & 2U) ? (Sint16)retained.scale_y : 256,
                        .opacity = 128},
                    session->draw_target.surface) ||
                !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return false;
            continue;
        }
        if (command.kind == SOTN_DRAW_BAT_FIREBALL)
        {
            if (command.primitive >= 48)
                return SDL_SetError("Invalid Bat fireball draw slot");
            const SotnBatRuntimeChild *child = &session->bat_runtime.children[command.primitive];
            if (child->kind != 45 || !child->initialized)
                return SDL_SetError("Bat fireball draw lost its owner");
            const SotnBatFireball motion = child->state.fire.motion;
            if (!SotnActorGraphics_DrawScaled(
                    session->actor_graphics,
                    (SotnActorSprite){.animation_set = 9,
                                      .frame = child->state.fire.frame,
                                      .facing_left = motion.facing_left},
                    Pixel(motion.x) - view_x + session->draw_target.offset_x,
                    Pixel(motion.y) - view_y + session->draw_target.offset_y, motion.scale_x,
                    motion.scale_y, session->draw_target.surface) ||
                !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return false;
            continue;
        }
        if (command.kind == SOTN_DRAW_PLAYER_BLINK)
        {
            const Uint16 owner = command.primitive / 8U, q = command.primitive % 8U;
            const SotnPlayerEffectQuad *quads = NULL;
            Uint16 packet_count = 0;
            if (!SotnBatRuntime_Visual(&session->bat_runtime, owner, &quads, &packet_count) ||
                q >= packet_count)
                return SDL_SetError("Bat draw lost its actual owned child packet");
            const bool flash = session->bat_runtime.children[owner].kind == 29;
            const bool drawn =
                flash ? SotnHellfireGraphics_Flash(session->actor_graphics, &quads[q],
                                                   session->draw_target.offset_x + session->shake.x,
                                                   session->draw_target.offset_y + session->shake.y,
                                                   session->draw_target.surface)
                      : DrawPlayerEffectQuad(session, &quads[q]);
            if (!drawn || !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                                     SOTN_FRAME_PIXELS))
                return false;
            continue;
        }
        if (command.kind == SOTN_DRAW_MIST)
        {
            if (!DrawMistQuad(session, command.primitive) ||
                !SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                           SOTN_FRAME_PIXELS))
                return false;
            continue;
        }
        if (command.actor >= count)
            return SDL_SetError("Actor draw command lost its frame snapshot");
        const SotnActorState actor = list[command.actor];
        if (command.kind == SOTN_DRAW_PRIMITIVE)
        {
            SotnActorPrimitive primitive = {0};
            if (!SotnActors_GetPrimitive(session->actors, actor.slot, command.primitive,
                                         &primitive))
                return false;
            if (primitive.target)
            {
                if (!SwitchTarget(session, primitive))
                    return false;
                continue;
            }
            if (primitive.clip)
            {
                if (!SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                               SOTN_FRAME_PIXELS))
                    return false;
                clip = (SotnFrameClip){primitive.clip_left, primitive.clip_top,
                                       primitive.clip_right, primitive.clip_bottom};
                continue;
            }
            if (!DrawPrimitiveRange(session, actor, camera_x, camera_y, command.primitive,
                                    command.primitive + 1U))
                return ActorDrawError(actor);
            goto draw_complete;
        }
        if (actor.id == 0xa || actor.id == 0xb)
        {
            // Persistent pickups can remain far behind a tall-room camera; icons are 16 pixels.
            const Sint32 x = Pixel(actor.x) - view_x - 8, y = Pixel(actor.y) - view_y - 8;
            if (x <= -16 || y <= -16 || x >= SOTN_FRAME_WIDTH || y >= SOTN_FRAME_HEIGHT)
                goto draw_complete;
        }
        if (actor.id == 0xb)
        {
            SotnItem relic = {0};
            if (!SotnRelic_Read(session->dra, actor.parameters & 0x7fff, &relic) ||
                !SotnItemIcon_Draw(session->dra, relic, Pixel(actor.x) - view_x - 8,
                                   Pixel(actor.y) - view_y - 8, session->frame, SOTN_FRAME_PIXELS))
                return false;
            goto draw_complete;
        }
        if (actor.id == 0xa)
        {
            const Uint16 id = actor.parameters & 0x7fff;
            SotnItem item = {0};
            if (!SotnItem_Read(session->dra, id >= SOTN_HAND_ITEMS,
                               id >= SOTN_HAND_ITEMS ? (Uint16)(id - SOTN_HAND_ITEMS) : id,
                               &item) ||
                !SotnItemIcon_Draw(session->dra, item, Pixel(actor.x) - view_x - 8,
                                   Pixel(actor.y) - view_y - 8, session->frame, SOTN_FRAME_PIXELS))
                return false;
            goto draw_complete;
        }
        if (session->stage_id == 65 && actor.id == 0x59 && actor.step >= 3 &&
            !SotnDialogueRenderer_Item(session->dra, session->nz0, actor.parameters,
                                       Pixel(actor.x) - view_x, Pixel(actor.y) - view_y,
                                       (Uint16)(actor.scale_x / 16), (Uint16)(actor.scale_y / 16),
                                       actor.step == 4 && (session->simulation_ticks & 2),
                                       session->frame, SOTN_FRAME_PIXELS))
            return false;
        const SotnActorSprite sprite = {actor.animation_set,  actor.frame,      actor.palette,
                                        actor.texture_offset, actor.blend_mode, actor.facing_left};
        if (command.kind == SOTN_DRAW_ACTOR &&
            !DrawActorPrimitives(session, actor, camera_x, camera_y))
            return ActorDrawError(actor);
        const SotnSpriteTransform transform = SotnActorGraphics_GetTransform(actor);
        if ((actor.frame & 0x7fffU) == 0 || transform.scale_x == 0 || transform.scale_y == 0)
            goto draw_complete;
        if (!SotnActorGraphics_DrawSurface(session->actor_graphics, sprite,
                                           Pixel(actor.x) - view_x + session->draw_target.offset_x,
                                           Pixel(actor.y) - view_y + session->draw_target.offset_y,
                                           transform, session->draw_target.surface))
            return ActorDrawError(actor);
    draw_complete:
        if (!SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                       SOTN_FRAME_PIXELS))
            return false;
    }
    if (session->draw_target.mode)
        return SDL_SetError("Ordered actor presentation did not restore its render target");
    return true;
}

static bool PrepareDialogue(SotnLaboratory *session)
{
    for (Uint16 i = 0; i < 2; ++i)
        if (session->dialogue.portraits[i] &&
            session->portrait_offsets[i] != session->dialogue.portraits[i])
        {
            if (!SotnActorGraphics_LoadPortrait(session->actor_graphics,
                                                session->dialogue.portraits[i], i))
                return false;
            session->portrait_offsets[i] = session->dialogue.portraits[i];
        }
    return !session->dialogue.primitive_text ||
           SotnActorGraphics_LoadDialogue(session->actor_graphics, session->dialogue);
}

static bool DrawDialogue(SotnLaboratory *session)
{
    if (session->stage_id == 25 && session->dialogue.primitive_text)
    {
        size_t count = 0;
        for (Uint16 slot = 0; slot < SOTN_STAGE_ACTOR_SLOTS; ++slot)
        {
            SotnActorState actor = {0};
            if (!SotnActors_Get(session->actors, slot, &actor))
                return false;
            if (!actor.generation || actor.id != 0x2a)
                continue;
            for (Uint16 part = 0; part < actor.primitive_count; ++part)
            {
                SotnActorPrimitive p = {0};
                if (!SotnActors_GetPrimitive(session->actors, slot, part, &p))
                    return false;
                if (p.active && !SotnActorOrder_Add(session->draw_items,
                                                    SDL_arraysize(session->draw_items), &count,
                                                    (SotnActorDraw){.actor = slot,
                                                                    .primitive = part,
                                                                    .priority = p.priority,
                                                                    .kind = SOTN_DRAW_PRIMITIVE}))
                    return false;
            }
        }
        for (size_t i = 0; i < count; ++i)
        {
            const SotnActorDraw command = session->draw_items[i];
            SotnActorState actor = {0};
            if (!SotnActors_Get(session->actors, command.actor, &actor) ||
                !DrawPrimitiveRange(session, actor, 0, 0, command.primitive,
                                    command.primitive + 1U))
                return false;
        }
        return true;
    }
    return SotnDialogueRenderer_Draw(session->actor_graphics, session->dialogue, session->frame,
                                     SOTN_FRAME_PIXELS);
}

static bool DrawGameplay(SotnLaboratory *session)
{
    if (session->engine_flash)
    {
        for (size_t i = 0; i < SOTN_FRAME_PIXELS; ++i)
            session->frame[i] = 0x7fff;
        return true;
    }
    if (session->bat_runtime.screen_flash)
    {
        const SotnBatRuntime *r = &session->bat_runtime;
        const Uint16 color = (Uint16)((r->screen_red >> 3) | (r->screen_green >> 3) << 5 |
                                      (r->screen_blue >> 3) << 10);
        for (size_t i = 0; i < SOTN_FRAME_PIXELS; ++i)
            session->frame[i] = color;
        return true;
    }
    if (session->draw_target.mode)
        return SDL_SetError("Actor presentation retained an unfinished render target");
    session->draw_target.surface = MainSurface(session);
    session->draw_target.offset_x = session->draw_target.offset_y = 0;
    SotnPlayerState state = {0};
    if (!SotnPlayer_GetState(session->player, &state) || !PrepareDialogue(session))
        return false;
    Sint32 camera_x = 0, camera_y = 0;
    Camera(session, state, &camera_x, &camera_y);
    if (!PrepareWolfPresentation(session, state, camera_x, camera_y))
        return false;
    const Sint32 view_x = camera_x - session->shake.x, view_y = camera_y - session->shake.y;
    const SotnTileSources sources = {StageGraphics(session), SotnResource_View(session->common)};
    Uint16 backdrop = 0;
    if (!SotnActors_GetBackdrop(session->actors, &backdrop))
        return false;
    for (size_t i = 0; i < SOTN_FRAME_PIXELS; ++i)
        session->frame[i] = backdrop;
    return ((session->stage_id != 6 && session->stage_id != 25) ||
            DrawActorsAndPlayer(session, state, camera_x, camera_y, true)) &&
           SotnTilemap_Draw(&session->room.background, sources, view_x, view_y, session->frame,
                            SOTN_FRAME_PIXELS) &&
           (session->stage_id != 2 ||
            DrawActorsAndPlayer(session, state, camera_x, camera_y, true)) &&
           SotnDialogueRenderer_Background(session->dialogue.fade, session->frame,
                                           SOTN_FRAME_PIXELS) &&
           SotnTilemap_Draw(&session->room.foreground, sources, view_x, view_y, session->frame,
                            SOTN_FRAME_PIXELS) &&
           (!session->save_room ||
            SotnSavePointRenderer_DrawOffset(session->save_renderer, session->save_point, false,
                                             session->shake.x, session->shake.y, session->frame,
                                             SOTN_FRAME_PIXELS)) &&
           DrawActorsAndPlayer(session, state, camera_x, camera_y, false) &&
           SotnSubweapons_Draw(session->subweapons, session->actor_graphics, session->dra,
                               session->game_timer, view_x, view_y, session->frame,
                               SOTN_FRAME_PIXELS) &&
           (!session->save_room ||
            SotnSavePointRenderer_DrawOffset(session->save_renderer, session->save_point, true,
                                             session->shake.x, session->shake.y, session->frame,
                                             SOTN_FRAME_PIXELS)) &&
           SotnHud_Draw(session->hud, session->frame, SOTN_FRAME_PIXELS) && DrawDialogue(session) &&
           (!session->save_room ||
            SotnSavePointRenderer_Dialog(session->save_renderer, session->save_point,
                                         session->input.pad != NULL, session->frame,
                                         SOTN_FRAME_PIXELS));
}

static bool DrawMap(SotnLaboratory *session)
{
    if (session->map_opacity == 0 || session->game_over != NULL)
        return true;
    if (!SotnMapRenderer_Draw(session->map_renderer, &session->world, false, session->map_opacity,
                              session->frame, SOTN_FRAME_PIXELS))
        return false;
    SotnPlayerState player = {0};
    if (!SotnPlayer_GetState(session->player, &player))
        return false;
    if (session->room.foreground.parameters & 0x10)
        return true;
    const Sint32 cx = SDL_clamp((Sint32)session->room.foreground.left + Pixel(player.x) / 256, 0,
                                63),
                 cy =
                     SDL_clamp((Sint32)session->room.foreground.top + Pixel(player.y) / 256, 0, 63);
    Sint32 tick = 63 - (Sint32)(session->map_draw_cursor & 63);
    tick = 15 - ((tick * tick) >> 8);
    if (tick == 0)
        tick = 15;
    const Sint32 x = cx * 4 - 14 + tick, y = cy * 4 - 13 + tick, size = 32 - tick * 2;
    SotnTextureQuad cursor = {.x = {x, x + size, x, x + size},
                              .y = {y, y, y + size, y + size},
                              .u = {0, 32, 0, 32},
                              .v = {0, 0, 32, 32},
                              .page = 0x1a,
                              .palette = 0x170,
                              .blend = true,
                              .modulate = true,
                              .opacity = (Uint8)(tick * 16 * session->map_opacity / 64)};
    return SotnActorGraphics_Quad(session->actor_graphics, cursor, session->frame,
                                  SOTN_FRAME_PIXELS);
}

bool SotnLaboratory_Draw(SotnLaboratory *session)
{
    if (session == NULL)
        return SDL_InvalidParamError("laboratory");
    if (session->dirty)
    {
        if (session->launch_active)
        {
            if (!SotnLaunchRenderer_Draw(SotnResource_View(session->common), session->launcher,
                                         session->launch_slots, session->launch_error,
                                         session->frame, SOTN_FRAME_PIXELS) ||
                !SotnFrame_ToRgba(session->frame, SOTN_FRAME_PIXELS, session->rgba,
                                  sizeof(session->rgba)) ||
                !SotnPresenter_SetImage(session->presenter, session->rgba, sizeof(session->rgba),
                                        SOTN_FRAME_WIDTH, SOTN_FRAME_HEIGHT))
                return false;
            session->dirty = false;
            return SotnPresenter_Draw(session->presenter);
        }
        if (session->menu.phase != SOTN_MENU_CLOSED)
        {
            if (!SotnInventoryRenderer_Draw(session->inventory_renderer, session->menu,
                                            session->character, session->simulation_ticks,
                                            session->input.pad != NULL, session->menu_rgba,
                                            sizeof(session->menu_rgba), &session->world) ||
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
            !DrawMap(session) ||
            !SotnFrame_ToRgba(session->frame, SOTN_FRAME_PIXELS, session->rgba,
                              sizeof(session->rgba)) ||
            !SotnPresenter_SetImage(session->presenter, session->rgba, sizeof(session->rgba),
                                    SOTN_FRAME_WIDTH, SOTN_FRAME_HEIGHT))
            return false;
        session->dirty = false;
        session->targets_pending = false;
    }
    return SotnPresenter_Draw(session->presenter);
}

bool SotnLaboratory_Finished(const SotnLaboratory *session)
{
    return session != NULL && !session->launch_active && SotnGameOver_Finished(session->game_over);
}

bool SotnLaboratory_Event(SotnLaboratory *session, const SDL_Event *event)
{
    if (session == NULL || event == NULL)
        return SDL_SetError("Laboratory event arguments are invalid");
    if (!LaboratoryInput_Event(&session->input, event))
        return false;
    if (event->type >= SDL_EVENT_WINDOW_FIRST && event->type <= SDL_EVENT_WINDOW_LAST &&
        event->window.windowID != SotnPresenter_WindowID(session->presenter))
        return true;
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
        SotnMapRenderer_Destroy(session->map_renderer);
        SotnResource_Close(session->map_art);
        SDL_free(session->directory);
        SotnGameOver_Destroy(session->game_over);
        SotnResource_Close(session->game_over_art);
        SotnInventoryRenderer_Destroy(session->inventory_renderer);
        if (!session->borrowed_saves)
            SotnSaveStore_Destroy(session->saves);
        SotnSavePointRenderer_Destroy(session->save_renderer);
        SotnHud_Destroy(session->hud);
        SotnMusic_Destroy(session->speech);
        SotnMusic_Destroy(session->music);
        SotnCdAudio_Destroy(session->fairy_cd);
        SotnPlayback_Destroy(session->audio);
        if (!session->borrowed_music)
            SotnStream_Close(session->music_stream);
        SotnSound_Destroy(session->sound);
        SotnResource_Close(session->sequence_resource);
        for (Uint32 i = 0; i < 5; ++i)
            SotnResource_Close(session->sound_resources[i]);
        SotnPresenter_Destroy(session->presenter);
        LaboratoryInput_Quit(&session->input);
        SotnActors_Destroy(session->actors);
        SotnSubweapons_Destroy(session->subweapons);
        SotnCharacter_Destroy(session->character);
        SotnActorLayout_Close(session->actor_layout);
        SotnActorGraphics_Destroy(session->actor_graphics);
        for (Uint16 i = 0; i < 4; ++i)
            SotnResource_Close(session->fairy_resources[i]);
        SotnPlayer_Close(session->player);
        SotnResource_Close(session->weapons);
        SotnResource_Close(session->arc);
        SotnResource_Close(session->common);
        SotnResource_Close(session->tiles);
        SotnStage_Close(session->stage);
        SotnStageAssets_Close(session->stage_assets);
        SDL_free(session);
    }
}

static bool RestoreCharacterAt(SotnLaboratory *session, SotnSaveState state,
                               const SotnPlaytime *retained)
{
    const SotnCharacterState *c = &state.character;
    SotnPlaytime clock = {0};
    if (!SotnPlaytime_FromTicks(state.ticks, &clock))
        return false;
    if (retained)
        clock = *retained;
    if (!SotnCharacter_SetPlaytime(session->character, clock) ||
        !SotnCharacter_Restore(session->character, c) ||
        !SotnCharacter_RestoreFamiliars(session->character, &state.familiars) ||
        !ApplyCharacter(session) ||
        !SotnPlayer_Restore(session->player, c->hp, state.player_random))
        return false;
    session->playtime = clock;
    return true;
}
static bool RestoreCharacter(SotnLaboratory *session, SotnSaveState state)
{
    return RestoreCharacterAt(session, state, NULL);
}

static bool RestoreSessionAt(SotnLaboratory *session, SotnSaveState state, Uint32 slot,
                             const SotnPlaytime *retained)
{
    const SotnCharacterState *c = &state.character;
    const SotnActorProgress progress = {c->hearts, c->subweapon,       c->gold,
                                        c->kills,  state.actor_random, state.combat_random};
    if (!(retained ? RestoreCharacterAt(session, state, retained)
                   : RestoreCharacter(session, state)) ||
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

static bool RestoreSession(SotnLaboratory *session, SotnSaveState state, Uint32 slot)
{
    return RestoreSessionAt(session, state, slot, NULL);
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
    if (!CreateSession(
            data, directory,
            (SessionStart){.stage = state.stage, .room = state.room, .saved = &state, .slot = slot},
            &session))
    {
        SotnSaveStore_Destroy(store);
        return false;
    }
    SotnSaveStore_Destroy(session->saves);
    session->saves = store;
    SDL_Log("Loaded native slot %u in stage %u room %u", slot, state.stage, state.room);
    *out = session;
    return true;
}

bool Laboratory_ContinueStartup(const SotnGameData *data, const char *directory,
                                SotnSaveStore *store, Uint32 slot, Uint32 engine_clock,
                                Uint32 map_cursor, Uint16 buttons, const SotnSound *sound,
                                SotnLaboratory **out)
{
    if (!out)
        return SDL_InvalidParamError("out");
    *out = NULL;
    if (!store || !sound)
        return SDL_SetError("Cold Continue requires actual save and resident sound owners");
    SotnSaveState saved = {0};
    if (!SotnSaveStore_Read(store, slot, &saved))
        return false;
    SotnLaboratory *candidate = NULL;
    if (!CreateSession(data, directory,
                       (SessionStart){.stage = saved.stage,
                                      .room = saved.room,
                                      .slot = slot,
                                      .saved = &saved,
                                      .system_timer = engine_clock,
                                      .startup_buttons = buttons,
                                      .map_cursor = map_cursor,
                                      .map_draw_cursor = map_cursor,
                                      .startup_sound = sound,
                                      .startup_store = store},
                       &candidate))
        return false;
    candidate->borrowed_saves = false;
    *out = candidate;
    return true;
}

static void LaunchError(SotnLaboratory *session, const char *detail)
{
    SDL_strlcpy(session->launch_error, detail, sizeof(session->launch_error));
    for (size_t i = 0; session->launch_error[i]; ++i)
        if ((Uint8)session->launch_error[i] < 32 || (Uint8)session->launch_error[i] > 126)
            session->launch_error[i] = '?';
}

bool SotnLaboratory_ShowLauncher(SotnLaboratory *session)
{
    if (session == NULL)
        return SDL_InvalidParamError("laboratory");
    session->launch_available = 0;
    session->launch_error[0] = '\0';
    SDL_memset(session->launch_slots, 0, sizeof(session->launch_slots));
    if (session->saves == NULL && !SotnSaveStore_Create(&session->saves))
        LaunchError(session, SDL_GetError());
    if (session->saves != NULL)
        for (Uint32 i = 0; i < 15; ++i)
        {
            SotnLaunchSlot *summary = &session->launch_slots[i];
            bool exists = false;
            if (!SotnSaveStore_Exists(session->saves, i + 1, &exists))
            {
                LaunchError(session, SDL_GetError());
                continue;
            }
            summary->exists = exists;
            if (exists)
            {
                SotnSaveState saved = {0};
                if (SotnSaveStore_Read(session->saves, i + 1, &saved))
                {
                    *summary = (SotnLaunchSlot){.exists = true,
                                                .valid = true,
                                                .level = saved.character.level,
                                                .hp = saved.character.hp,
                                                .room = saved.room,
                                                .stage = saved.stage};
                    session->launch_available |= (Uint16)(1U << i);
                }
                else
                    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Native slot %u cannot load: %s",
                                i + 1, SDL_GetError());
            }
        }
    const Uint32 selected = session->save_point.slot ? session->save_point.slot : 1;
    if (!SotnLaunchMenu_Reset(&session->launcher, session->scene_buttons, selected))
        return false;
    session->launch_active = true;
    session->launch_action = SOTN_LAUNCH_NONE;
    session->input.menu_open = true;
    session->map_open = false;
    session->map_opacity = 0;
    session->menu = (SotnInventoryMenu){0};
    SotnMusic_Destroy(session->speech);
    session->speech = NULL;
    SotnMusic_Destroy(session->music);
    session->music = NULL;
    if (session->audio != NULL && !SotnPlayback_Reset(session->audio))
        return false;
    session->last_time = SDL_GetTicksNS();
    session->clock_units = 0;
    session->dirty = true;
    return true;
}

bool SotnLaboratory_UpdateSession(const SotnGameData *data, SotnLaboratory **owner)
{
    if (data == NULL || owner == NULL || *owner == NULL)
        return SDL_SetError("Session update requires owned gameplay and core data");
    SotnLaboratory *old = *owner;
    const SotnLaunchAction action = old->launch_action;
    const bool travel = old->stage_pending;
    old->launch_action = SOTN_LAUNCH_NONE;
    old->stage_pending = false;
    if (action == SOTN_LAUNCH_NONE && !travel)
        return true;
    SotnPlayerState previous_player = {0};
    if (!SotnPlayer_GetState(old->player, &previous_player))
        return false;
    SotnSaveState saved = {0};
    const Uint32 slot = old->launcher.slot;
    SotnLaboratory *next = NULL;
    bool ready = false;
    if (travel && SaveState(old, &saved))
    {
        saved.stage = old->arrival.stage;
        saved.room = old->arrival.room;
        ready =
            CreateSession(data, old->directory,
                          (SessionStart){.stage = saved.stage,
                                         .room = saved.room,
                                         .saved = &saved,
                                         .arrival = &old->arrival,
                                         .teleport_mode = old->teleport_mode,
                                         .music = old->music_stream,
                                         .system_timer = old->system_timer,
                                         .game_timer = old->game_timer,
                                         .playtime = &old->playtime,
                                         .map_cursor = old->map_cursor,
                                         .map_draw_cursor = old->map_draw_cursor,
                                         .stat_buffs = &old->stat_buffs,
                                         .familiars = old,
                                         .spirit_target_cursor = old->bat_runtime.target_cursor,
                                         .water_breath_timer = previous_player.water_breath_timer,
                                         .water_pain_timer = previous_player.water_pain_timer},
                          &next);
    }
    else if (action == SOTN_LAUNCH_CHECKPOINT)
        ready =
            CreateSession(data, old->directory,
                          (SessionStart){.stage = 12,
                                         .room = 1,
                                         .music = old->music_stream,
                                         .system_timer = old->system_timer,
                                         .game_timer = old->game_timer,
                                         .map_cursor = old->map_cursor,
                                         .map_draw_cursor = old->map_draw_cursor,
                                         .spirit_target_cursor = old->bat_runtime.target_cursor,
                                         .water_breath_timer = previous_player.water_breath_timer,
                                         .water_pain_timer = previous_player.water_pain_timer},
                          &next);
    else if (action == SOTN_LAUNCH_LOAD && old->saves != NULL &&
             SotnSaveStore_Read(old->saves, slot, &saved))
    {
        // save_mgr.h:ApplySaveData retains both completion words across in-session loads.
        saved.world.time_attack_visited |= old->world.time_attack_visited;
        saved.world.cutscenes_seen |= old->world.cutscenes_seen;
        ready =
            CreateSession(data, old->directory,
                          (SessionStart){.stage = saved.stage,
                                         .room = saved.room,
                                         .saved = &saved,
                                         .slot = slot,
                                         .music = old->music_stream,
                                         .system_timer = old->system_timer,
                                         .game_timer = old->game_timer,
                                         .map_cursor = old->map_cursor,
                                         .map_draw_cursor = old->map_draw_cursor,
                                         .spirit_target_cursor = old->bat_runtime.target_cursor,
                                         .water_breath_timer = previous_player.water_breath_timer,
                                         .water_pain_timer = previous_player.water_pain_timer},
                          &next);
    }
    if (ready && !SotnPlayer_RestoreHammer(next->player, previous_player.hammer_timer))
    {
        SotnLaboratory_Destroy(next);
        return false;
    }
    if (ready)
    {
        next->bat_runtime.dark_blend_phase = old->bat_runtime.dark_blend_phase;
        // DRA's one shared teleport point array outlives full entity initialization.
        // Incoming birth overwrites it in actual slot order after the handoff.
        next->bat_runtime.teleport = old->bat_runtime.teleport;
        // Weapon17 scratch outlives room-local entities; births overwrite its actual
        // vectors/shards.
        SDL_memcpy(next->bat_runtime.pentagram, old->bat_runtime.pentagram,
                   sizeof(next->bat_runtime.pentagram));
    }
    if (ready && travel)
    {
        // Engine_Init destroys entities and AluInit restores normal form. Mist's global ring
        // fractions/color phases outlive that entity reset and remain available on later entry.
        SDL_memcpy(next->mist_effect.points, old->mist_effect.points,
                   sizeof(next->mist_effect.points));
        SDL_memcpy(next->mist_effect.color_phase, old->mist_effect.color_phase,
                   sizeof(next->mist_effect.color_phase));
        SDL_memcpy(next->mist_effect.color_base, old->mist_effect.color_base,
                   sizeof(next->mist_effect.color_base));
        SotnPlayerState previous = {0};
        if (!SotnPlayer_GetState(old->player, &previous) ||
            !SotnPlayer_RestoreCurse(next->player, previous.curse_ticks) ||
            !SotnPlayer_RestorePoison(next->player, previous.poison_ticks))
        {
            char detail[160] = {0};
            SDL_strlcpy(detail, SDL_GetError(), sizeof(detail));
            SotnLaboratory_Destroy(next);
            next = NULL;
            ready = false;
            SDL_SetError("Cannot restore transient ailments on stage arrival: %s", detail);
        }
    }
    if (!ready)
    {
        char detail[160] = {0};
        SDL_strlcpy(detail, SDL_GetError(), sizeof(detail));
        if (travel && !SotnLaboratory_ShowLauncher(old))
            return false;
        LaunchError(old, detail);
        old->launcher.phase = SOTN_LAUNCH_ERROR;
        old->dirty = true;
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot change native session: %s",
                     old->launch_error);
        return true; // Preserve the previous owner and every ordinary save on failure.
    }
    if (old->saves != NULL)
    {
        SotnSaveStore_Destroy(next->saves);
        next->saves = old->saves;
        old->saves = NULL;
        if (!EnterSaveRoom(next))
        {
            char detail[160] = {0};
            SDL_strlcpy(detail, SDL_GetError(), sizeof(detail));
            old->saves = next->saves;
            next->saves = NULL;
            SotnLaboratory_Destroy(next);
            if (travel && !SotnLaboratory_ShowLauncher(old))
                return false;
            LaunchError(old, detail);
            old->launcher.phase = SOTN_LAUNCH_ERROR;
            old->dirty = true;
            return true;
        }
    }
    // Walking continues through connected rooms; held UI/action buttons require release.
    const Uint16 directions =
        SOTN_BUTTON_LEFT | SOTN_BUTTON_RIGHT | SOTN_BUTTON_UP | SOTN_BUTTON_DOWN;
    next->blocked_buttons =
        travel ? (Uint16)(old->scene_buttons & (Uint16)~directions) : old->scene_buttons;
    old->music_stream = NULL;
    next->borrowed_music = false;
    *owner = next;
    SotnLaboratory_Destroy(old);
    SDL_Log("Native session ready: stage %u room %u (%s)", next->stage_id, next->room_index,
            travel                       ? "connected arrival"
            : action == SOTN_LAUNCH_LOAD ? "ordinary save"
                                         : "checkpoint");
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
