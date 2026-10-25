#include "input.h"
#include <sotn/actor_graphics.h>
#include <sotn/actor_order.h>
#include <sotn/actors.h>
#include <sotn/alucard_sprite.h>
#include <sotn/castle_map.h>
#include <sotn/combat.h>
#include <sotn/dialogue_renderer.h>
#include <sotn/frame_clip.h>
#include <sotn/game_over.h>
#include <sotn/hud.h>
#include <sotn/inventory.h>
#include <sotn/laboratory.h>
#include <sotn/launch_renderer.h>
#include <sotn/music.h>
#include <sotn/playback.h>
#include <sotn/player.h>
#include <sotn/presenter.h>
#include <sotn/relic.h>
#include <sotn/resource.h>
#include <sotn/save.h>
#include <sotn/save_point_renderer.h>
#include <sotn/sound.h>
#include <sotn/stage_assets.h>
#include <sotn/stage_profile.h>
#include <sotn/stage_route.h>
#include <sotn/subweapon_renderer.h>

struct SotnLaboratory
{
    SotnStage *stage;
    SotnStageAssets *stage_assets;
    Uint16 stage_id;
    SotnStageRoute arrival;
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
    Uint8 map_opacity;
    Uint64 map_ticks;
    SotnStageRoom room;
    SotnWorldState world;
    SotnCamera camera;
    SotnDataView dra, nz0, executable;
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
    Uint16 cape_glow_target;
    Uint8 cape_glow_delay;
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
    bool save_room, clock_room;
    LaboratoryInput input;
    SotnActorDraw draw_items[SOTN_STAGE_ACTOR_SLOTS + SOTN_ACTOR_PRIMITIVES_MAX];
    Uint16 frame[SOTN_FRAME_PIXELS];
    Uint16 clip_backup[SOTN_FRAME_PIXELS];
    Uint8 rgba[SOTN_FRAME_PIXELS * 4];
    Uint32 room_index;
    Uint64 last_time, clock_units;
    bool dirty, paused;
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
    const SotnSaveState *saved;
    const SotnStageRoute *arrival;
    SotnStream *music; /**< Borrowed during candidate construction; transferred only at commit. */
} SessionStart;

static bool CreateStageMusic(SotnLaboratory *s, SotnMusic **out)
{
    if (s->stage_id == 26)
    {
        // The original BO4 stage record selects MU_NO_AUDIO until its encounter event.
        *out = NULL;
        return true;
    }
    if (s->stage_id == 0 || s->stage_id == 1 || s->stage_id == 2 || s->stage_id == 6)
        return SotnMusic_CreateStage(s->dra, s->music_stream, s->stage_id, out);
    if (s->stage_id == 12)
        return SotnMusic_Create(s->dra, s->music_stream, out);
    return SotnMusic_CreateEntrance(s->dra, s->music_stream,
                                    s->stage_id == 7 || s->world.flags[SOTN_ENTRANCE_ON] ||
                                        s->world.flags[SOTN_ENTRANCE_DEATH],
                                    out);
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

static bool OpenAudio(SotnLaboratory *session, const char *directory)
{
    static const struct
    {
        Uint16 stage;
        SotnResourceId header, body;
    } profiles[] = {{12, SOTN_RESOURCE_LAB_AUDIO_HEADER, SOTN_RESOURCE_LAB_AUDIO_BODY},
                    {65, SOTN_RESOURCE_ENTRANCE_AUDIO_HEADER, SOTN_RESOURCE_ENTRANCE_AUDIO_BODY},
                    {7, SOTN_RESOURCE_RETURN_AUDIO_HEADER, SOTN_RESOURCE_RETURN_AUDIO_BODY},
                    {0, SOTN_RESOURCE_MARBLE_AUDIO_HEADER, SOTN_RESOURCE_MARBLE_AUDIO_BODY},
                    {1, SOTN_RESOURCE_WALL_AUDIO_HEADER, SOTN_RESOURCE_WALL_AUDIO_BODY},
                    {26, SOTN_RESOURCE_DOPPLE_AUDIO_HEADER, SOTN_RESOURCE_DOPPLE_AUDIO_BODY},
                    {2, SOTN_RESOURCE_LIBRARY_AUDIO_HEADER, SOTN_RESOURCE_LIBRARY_AUDIO_BODY},
                    {6, SOTN_RESOURCE_CHAPEL_AUDIO_HEADER, SOTN_RESOURCE_CHAPEL_AUDIO_BODY}};
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
    if (!SotnSound_Create(sources, &session->sound))
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
    if (!CreateStageMusic(session, &session->music))
        return false;
    return SotnPlayback_Create(&session->audio);
}

static bool QueueAudio(SotnLaboratory *session, const SotnActorCombat *combat, Uint16 player_sound,
                       Uint16 player_extra_sound)
{
    if (session->sound == NULL)
        return true;
    if (combat->restart_stage_music)
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
        if (combat->sequence == 0)
            SotnSound_StopSequence(session->sound);
        else if (!SotnSound_PlaySequence(session->sound, combat->sequence))
            return false;
    }
    if (combat->music_change)
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
                return SDL_SetError("Unsupported Laboratory event music request");
        }
        SotnMusic_Destroy(session->music);
        session->music = next;
    }
    if (combat->cd_command)
    {
        if (combat->cd_command == 0x90 || combat->cd_command == 0x80)
        {
            session->xa_command = combat->cd_command == 0x90 ? 1 : 3;
            session->xa_fade_counter = 0;
            session->xa_fade_multiplier = combat->cd_command == 0x90 ? 128 : 32;
        }
        else if (combat->cd_command != 0x10 && combat->cd_command != 0x11)
            return SDL_SetError("Dialogue issued an unsupported XA control command");
        if (combat->cd_command == 0x10 || combat->cd_command == 0x11)
            session->music_fading = combat->cd_command == 0x10;
        if (combat->cd_command == 0x11)
        {
            session->music_paused = false;
            session->music_gain = 127;
        }
    }
    if (combat->voice_stop)
    {
        SotnMusic_Destroy(session->speech);
        session->speech = NULL;
    }
    if (combat->voice_change)
    {
        SotnMusic *next = NULL;
        if (!SotnMusic_CreateSpeech(session->dra, session->music_stream, combat->voice, &next))
            return false;
        SotnMusic_Destroy(session->speech);
        session->speech = next;
    }
    for (Uint32 i = 0; i < combat->sound_count; ++i)
        if (combat->volumes[i] == SDL_MAX_UINT16
                ? !SotnSound_Queue(session->sound, combat->sounds[i])
                : !SotnSound_QueueVolumePan(session->sound, combat->sounds[i], combat->volumes[i],
                                            combat->pans[i]))
            return false;
    if (combat->ambience_change &&
        !SotnSound_QueueAmbienceVolume(session->sound, combat->ambience_volume,
                                       combat->ambience_pan))
        return false;
    if (player_sound != 0 && !SotnSound_Queue(session->sound, player_sound))
        return false;
    if (player_extra_sound != 0 && !SotnSound_Queue(session->sound, player_extra_sound))
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
    return true;
}

static bool TickAudio(SotnLaboratory *session, const SotnActorCombat *combat, Uint16 player_sound,
                      Uint16 player_extra_sound)
{
    if (!QueueAudio(session, combat, player_sound, player_extra_sound))
        return false;
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
    if (session->game_over == NULL && !SotnSound_Tick(session->sound, pcm, SDL_arraysize(pcm)))
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
        const Uint8 *base = session->palette_ready ? session->player_common
                                                   : SotnResource_View(session->common).bytes;
        for (Uint32 i = 0; i < 4; ++i)
            colors[i] = (Uint16)(base[0x4000c + i * 2] | (Uint16)base[0x4000d + i * 2] << 8);
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
        session->cape_glow_target = 0;
        session->cape_glow_delay = 0;
        session->cape = state.equipment[4];
        session->palette_ready = true;
    }
    session->mp = state.mp;
    return SotnPlayer_SetCharacter(session->player, &state, stats);
}

static bool TickCape(SotnLaboratory *session)
{
    if (session->cape != 56)
        return true;
    Uint16 colors[4] = {0};
    for (Uint32 i = 0; i < 4; ++i)
        colors[i] = (Uint16)(session->player_common[0x4000c + i * 2] |
                             (Uint16)session->player_common[0x4000d + i * 2] << 8);
    if (!SotnAlucard_TwilightTick(session->dra, colors, &session->cape_glow_target,
                                  &session->cape_glow_delay))
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

static bool UpdateActors(SotnLaboratory *session, bool entering)
{
    SotnPlayerState player = {0};
    if (!SotnPlayer_GetState(session->player, &player))
        return false;
    if (!VisitMap(session) ||
        !SotnCamera_Tick(&session->camera, Pixel(player.x), Pixel(player.y), false))
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
    // Initializing a newly entered room must not advance projectiles or replay the
    // hand-attack sound already consumed by the departing room in this frame.
    if (entering)
        player.attack.sound = false;
    if (!SotnActors_SetServices(session->actors, session->executable, session->simulation_ticks,
                                session->music != NULL) ||
        !SotnActors_SetButtons(session->actors, session->scene_buttons) ||
        !SotnActors_Dialogue(session->actors, SotnMusic_Playing(session->speech), NULL) ||
        !SotnActors_SetDialogueBusy(session->actors, session->music_fading) ||
        (!entering && !SotnSubweapons_Tick(session->subweapons, player, camera_x, camera_y)) ||
        !SotnActors_Tick(session->actors, player, camera_x, camera_y))
        return false;
    if (!SotnActors_Dialogue(session->actors, SotnMusic_Playing(session->speech),
                             &session->dialogue))
        return false;
    SotnPaletteBlend palette = {0};
    if (!SotnActors_GetPaletteBlend(session->actors, &palette) ||
        !SotnActorGraphics_BlendPalette(session->actor_graphics, palette))
        return false;
    if (session->dialogue.background_enabled)
        session->room.background.flags |= 1;
    SotnActorMotion motion = {0};
    if (!SotnActors_GetMotion(session->actors, &motion) ||
        !SotnPlayer_Displace(session->player, motion.x, motion.y, motion.contacts) ||
        (motion.stop_velocity && !SotnPlayer_StageStop(session->player)) ||
        (motion.stop_horizontal && !SotnPlayer_StageStopHorizontal(session->player)) ||
        (motion.protect && !SotnPlayer_StageGuard(session->player, motion.halt_vertical)) ||
        !SotnPlayer_BellInside(session->player, motion.bell_inside))
        return false;
    if (!SotnPlayer_SetChairKind(session->player, motion.chair_kind))
        return false;
    SotnActorCombat combat = {0};
    SotnCharacterStats attributes = {0};
    if (!SotnActors_GetCombat(session->actors, &combat) ||
        !SotnCharacter_Get(session->character, &profile, &attributes))
        return false;
    if (player.step != SOTN_PLAYER_KILL && !ApplyCharacter(session))
        return false;
    SotnContactPower power = {0};
    const SotnCombatEnemy elements = {.weaknesses = attributes.weakness,
                                      .strengths = attributes.resistance,
                                      .immunes = attributes.immunity,
                                      .absorbs = attributes.absorption};
    if (!SotnCombat_ContactPower(combat.contact_damage, combat.contact_element, elements, &power))
        return false;
    if (combat.contact_damage && power.absorb && player.hp && player.step != SOTN_PLAYER_KILL)
    {
        profile.hp = (Uint16)SDL_min(profile.hp_max, (Uint32)player.hp + power.attack);
        if (!SotnCharacter_Restore(session->character, &profile) || !ApplyCharacter(session))
            return false;
    }
    else
    {
        const bool stone = (power.effects & 0x200) || player.step == SOTN_PLAYER_STONE;
        if (combat.contact_damage && !power.immune && stone)
        {
            const Uint16 damage =
                player.step == SOTN_PLAYER_STONE
                    ? (Uint16)(player.hp_max / 8)
                    : (Uint16)SDL_clamp((Sint32)power.attack - (Sint32)attributes.defense * 2, 0,
                                        9999);
            if (!SotnPlayer_Petrify(session->player, damage, combat.contact_x,
                                    combat.contact_velocity))
                return false;
        }
        else
        {
            Uint16 damage = 0;
            if (combat.contact_damage &&
                !((combat.contact_element & 31U) >= 16
                      ? SotnCombat_EnvironmentalContact(power, player.hp_max,
                                                        player.poison_ticks != 0, &damage)
                      : SotnCombat_NormalContact(power, attributes.defense,
                                                 player.poison_ticks != 0, &damage)))
                return false;
            if ((power.effects & 0x100U)
                    ? !SotnPlayer_CurseDamage(session->player, damage, combat.contact_x,
                                              combat.contact_velocity)
                : (power.effects & 0x80U)
                    ? !SotnPlayer_PoisonDamage(session->player, damage, combat.contact_x,
                                               combat.contact_velocity)
                    : !SotnPlayer_Damage(session->player, damage, combat.contact_x,
                                         combat.contact_velocity))
                return false;
        }
    }
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
        if (!session->save_room && !session->clock_room)
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
    if (!SotnCharacter_Get(session->character, &character, NULL))
        return false;
    character.hp = player.hp;
    *out = (SotnSaveState){.stage = session->stage_id,
                           .room = session->room_index,
                           .character = character,
                           .world = session->world,
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
                                           (state->contacts & 1) != 0, &crossing))
            return false;
    }
    else if (!SotnStageRoute_Crossing(local_x, local_y, (Uint32)width, (Uint32)height,
                                      (state->contacts & 1) != 0, &crossing))
        return false;
    if (!crossing.active)
        return true;
    const Sint32 world_x = (Sint32)session->room.left * 256 + crossing.x;
    const Sint32 world_y = (Sint32)session->room.top * 256 + crossing.y;
    if (session->stage_id == 1 || session->stage_id == 6)
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
        if (!SotnPlayer_ChangeRoom(session->player, room.foreground.collision, offset_x, offset_y))
            return false;
        if (!SotnSubweapons_Enter(session->subweapons, room.foreground.collision))
            return false;
        session->room = room;
        session->room_index = i;
        if (!EnterActors(session) || !EnterSaveRoom(session))
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

static bool PrimeAudio(SotnLaboratory *session)
{
    SotnActorCombat combat = {0};
    return SotnActors_GetCombat(session->actors, &combat) && TickAudio(session, &combat, 0, 0);
}

static bool RestoreCharacter(SotnLaboratory *session, SotnSaveState state);
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
            : stage_id == 65                               ? (start_room == 11 ? 16 : 128)
            : start_room >= 22                             ? 128
            : (start_room == 5 || start_room == 6 || start_room == 9 || start_room == 10) ? 80
                                                                                          : 32,
            start.arrival                        ? start.arrival->y
            : stage_id == 65 && start_room == 11 ? 388
                                                 : 135,
            &session->player) ||
        !SotnSubweapons_Create(session->dra, session->character, session->room.foreground.collision,
                               &session->subweapons) ||
        !SotnPlayer_SetSubweapons(session->player, session->subweapons) ||
        (start.arrival != NULL &&
         !SotnPlayer_SetArrivalFacing(session->player, (start.arrival->x & 255) >= 128)) ||
        (start.arrival == NULL && !SotnPlayer_Tick(session->player, 0)) ||
        !SotnActors_CreateStage(session->dra, session->nz0, stage_id, &session->actors) ||
        !SotnActors_SetCamera(session->actors, &session->camera) ||
        !SotnActors_SetWorld(session->actors, &session->world) ||
        !SotnActors_SetCharacter(session->actors, session->character) ||
        !SotnActors_SetSubweapons(session->actors, session->subweapons) ||
        !SotnInventoryRenderer_Create(session->dra, SotnResource_View(session->common),
                                      &session->inventory_renderer) ||
        !SotnActorGraphics_CreateStage(
            session->dra, session->nz0,
            (SotnTileSources){StageGraphics(session), SotnResource_View(session->common)}, stage_id,
            &session->actor_graphics) ||
        !InitializeEquipment(session, saved) ||
        !SotnSavePointRenderer_Create(session->dra, SotnResource_View(session->common),
                                      &session->save_renderer) ||
        !SotnHud_Create(session->dra, SotnResource_View(session->common),
                        (SotnHudStatus){70, 70, 20, 20, 10, 0}, &session->hud) ||
        (saved != NULL && !RestoreCharacter(session, *saved)) || !EnterActors(session) ||
        // Teleports may begin inside a closed red-door column. Its actor must open
        // the owned tiles before the first player collision, as in original room entry.
        !TickActors(session) || (start.arrival != NULL && !SotnPlayer_Tick(session->player, 0)) ||
        (saved != NULL && !RestoreSession(session, *saved, slot)) ||
        !LaboratoryInput_Init(&session->input) ||
        !SotnPresenter_Create("SOTN Native - Castle checkpoint (development)", 1024, 960,
                              &session->presenter) ||
        !OpenAudio(session, directory) || !EnterSaveRoom(session) || !PrimeAudio(session))
    {
        char detail[512] = {0};
        SDL_strlcpy(detail, SDL_GetError(), sizeof(detail));
        SotnLaboratory_Destroy(session);
        return SDL_SetError("Cannot initialize native stage %u room %u: %s", stage_id, start_room,
                            detail);
    }
    session->last_time = SDL_GetTicksNS();
    session->dirty = true;
    SDL_Log("Native development start: stage %u room %u", stage_id, (unsigned int)start_room);
    if (stage_id == 26)
        SDL_Log("Doppleganger checkpoint: placed opening, selected combat, physical defeat, "
                "seals, reward and connected Outer Wall progression. Full effects/parity remain "
                "incomplete.");
    else if (stage_id == 2)
        SDL_Log("Long Library development adapters: common pickups, selected breakables, "
                "Skeleton/Flea, Flea Armor, Dhuron, plants, Ectoplasm and book combat, music and "
                "saves; placed Lesser Demon entry, selected combat, seals and reward. "
                "Selected Librarian introduction, buy/sell, speech and exit run. Full shop panels, "
                "optional services and remaining scripts are incomplete. Selected Outer Wall "
                "travel is enabled; this is not a complete Library or authentic game release.");
    else if (stage_id == 6)
        SDL_Log("Royal Chapel internal checkpoint: private resources, selected shared actors, "
                "lamps/debris, furnishings, shortcut, selected scenery/gargoyles and Winged "
                "Guard/Bat/Crow/Raven/Skelerang/Halberd/Corner Guard/Bone Pillar/Hunting Girl/"
                "Spectral Sword combat, Spike Breaker corridor tiles/damage/debris, bells "
                "and selected internal tower exits, clouds, sky land, stained glass, chairs "
                "and confessional ghosts/blades/chime. Maria and boss encounters "
                "and connected travel "
                "remain incomplete.");
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
    SDL_Log("Character checkpoint: inventory/equipment, EXP/levels, selected subweapon attacks, "
            "and version-eight saves with stage, explored cells, and permanent castle events.");
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
    if (session->stage_pending)
        return true; // The application commits an arrival once, after its catch-up ticks.
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
        return TickAudio(session, &silent, 0, 0);
    }

    if (session->game_over != NULL)
    {
        const SotnActorCombat combat = {0};
        if (!SotnGameOver_Tick(session->game_over, start) || !TickAudio(session, &combat, 0, 0))
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
    if (session->map_open || (select && state.can_pause && !owns_save && control.timer == 0 &&
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
        ++session->map_ticks;
        const SotnActorCombat silent = {0};
        session->dirty = true;
        return TickAudio(session, &silent, 0, 0);
    }
    if (session->map_opacity)
        session->map_opacity = (Uint8)SDL_max(0, session->map_opacity - 8);

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
        if (!TickAudio(session, &silent, 0, 0))
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
    player_buttons &= (Uint16) ~(SOTN_BUTTON_START | SOTN_BUTTON_CANCEL | SOTN_BUTTON_SELECT);
    session->simulation_ticks = SDL_min(session->simulation_ticks + 1, 99999ULL * 3600 * 60);
    if (!SotnPlayer_SetScriptHold(session->player, control.hold) ||
        !SotnSubweapons_SetPaused(session->subweapons, control.block_stopwatch) ||
        !SotnPlayer_Tick(session->player, player_buttons) ||
        !SotnPlayer_GetState(session->player, &state) ||
        (session->stage_id != 6 && state.step != SOTN_PLAYER_KILL && !Transition(session, &state)))
        return false;
    if (session->stage_pending)
    {
        session->dirty = true;
        return true;
    }
    if (!TickActors(session) || !TickCape(session) ||
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
        !TickAudio(session, &combat, state.sound, state.extra_sound))
        return false;
    if (session->stage_id == 6 && state.step != SOTN_PLAYER_KILL)
    {
        // Original DAI exit actors change limits after player motion and before
        // the same frame's room lookup. Preserve departing sounds and synthesize
        // only one audio frame; entry initialization queues its sounds for the next.
        const Uint32 previous_room = session->room_index;
        if (!Transition(session, &state))
            return false;
        if (!session->stage_pending && session->room_index != previous_room)
        {
            SotnActorCombat entered = {0};
            if (!UpdateActors(session, true) || !SotnActors_GetCombat(session->actors, &entered) ||
                !QueueAudio(session, &entered, 0, 0))
                return false;
        }
    }
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
                "the reveal; Continue returns through the native menu. Original title/card menus "
                "remain unintegrated.");
    }
    session->dirty = true;
    return true;
}

static bool DrawPrimitiveRange(SotnLaboratory *session, SotnActorState actor, Sint32 camera_x,
                               Sint32 camera_y, Uint32 begin, Uint32 limit)
{
    if (begin > limit || limit > actor.primitive_count)
        return SDL_SetError("Actor draw range exceeds its owned primitives");
    for (Uint32 i = begin; i < limit; ++i)
    {
        SotnActorPrimitive p = {0};
        if (!SotnActors_GetPrimitive(session->actors, actor.slot, i, &p))
            return false;
        if (!p.active)
            continue;
        if (p.clip)
            return SDL_SetError("Actor draw-area changes require ordered stage presentation");
        const Sint32 x = Pixel(p.x) - (p.screen_space ? 0 : camera_x),
                     y = Pixel(p.y) - (p.screen_space ? 0 : camera_y);
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
            if (!SotnActorGraphics_Line(line, session->frame, SOTN_FRAME_PIXELS))
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
        if (right <= 0 || left >= SOTN_FRAME_WIDTH || bottom <= 0 || top >= SOTN_FRAME_HEIGHT)
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
            if (!SotnActorGraphics_Quad(session->actor_graphics, q, session->frame,
                                        SOTN_FRAME_PIXELS))
                return false;
        }
        else
        {
            const Uint16 front =
                (Uint16)((p.red / 8) | ((p.green / 8) << 5) | ((p.blue / 8) << 10));
            const Sint32 start = SDL_max(0, left), end = SDL_min(SOTN_FRAME_WIDTH, right);
            for (Sint32 row = SDL_max(0, top); row < SDL_min(SOTN_FRAME_HEIGHT, bottom); ++row)
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
                    if (!SotnActorGraphics_Line(tile, session->frame, SOTN_FRAME_PIXELS))
                        return false;
                }
                else
                    for (Sint32 col = start; col < end; ++col)
                        session->frame[(size_t)row * SOTN_FRAME_WIDTH + (size_t)col] = front;
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

// LIB parallax uses priorities 0x20..0x60 before its foreground tile bucket 0x60.
// Original later-submitted primitive packets precede tile packets at the tied bucket.
static bool DrawActorsAndPlayer(SotnLaboratory *session, SotnPlayerState player, Sint32 camera_x,
                                Sint32 camera_y, bool background)
{
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
        const bool library_background =
            session->stage_id == 2 && state.id >= 0x16 && state.id <= 0x19;
        if (library_background != background)
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
    if (!SotnAlucard_DecodeSprite(session->dra, SotnResource_View(session->arc),
                                  player.animation.sprite, &alucard))
        return false;
    bool player_drawn = false, weapon_drawn = false;
    SotnFrameClip clip = {0, 0, SOTN_FRAME_WIDTH, SOTN_FRAME_HEIGHT};
    for (size_t i = 0; i <= draw_count; ++i)
    {
        if (!SotnFrameClip_CopyOutside(clip, session->frame, session->clip_backup,
                                       SOTN_FRAME_PIXELS))
            return false;
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
                    .blend_mode =
                        (Uint16)(item.equipment == 86 || item.equipment == 123 ? 0x30 : 0),
                    .facing_left = player.facing_left};
                if (!SotnActorGraphics_Draw(session->actor_graphics, weapon,
                                            Pixel(player.x) - camera_x, Pixel(player.y) - camera_y,
                                            session->frame, SOTN_FRAME_PIXELS))
                    return false;
            }
            weapon_drawn = true;
        }
        if (!player_drawn && (i == draw_count || session->draw_items[i].priority > 0x94))
        {
            const SotnDataView common = {session->player_common, sizeof(session->player_common)};
            const bool drawn =
                player.stone.active
                    ? SotnAlucard_DrawPetrified(&alucard, common, Pixel(player.x) - camera_x,
                                                Pixel(player.y) - camera_y, player.facing_left,
                                                player.stone, session->frame, SOTN_FRAME_PIXELS)
                    : SotnAlucard_DrawDeath(&alucard, common, Pixel(player.x) - camera_x,
                                            Pixel(player.y) - camera_y, player.facing_left,
                                            player.death, session->frame, SOTN_FRAME_PIXELS);
            if (!drawn)
                return false;
            if (player.sleep.active && player.sleep.frame &&
                !SotnActorGraphics_Draw(session->actor_graphics,
                                        (SotnActorSprite){.animation_set = 17,
                                                          .frame = player.sleep.frame,
                                                          .palette = 0x819f,
                                                          .texture_offset = 0x50},
                                        Pixel(player.sleep.x) - camera_x,
                                        Pixel(player.sleep.y) - camera_y, session->frame,
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
        if (command.actor >= count)
            return SDL_SetError("Actor draw command lost its frame snapshot");
        const SotnActorState actor = list[command.actor];
        if (command.kind == SOTN_DRAW_PRIMITIVE)
        {
            SotnActorPrimitive primitive = {0};
            if (!SotnActors_GetPrimitive(session->actors, actor.slot, command.primitive,
                                         &primitive))
                return false;
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
            const Sint32 x = Pixel(actor.x) - camera_x - 8, y = Pixel(actor.y) - camera_y - 8;
            if (x <= -16 || y <= -16 || x >= SOTN_FRAME_WIDTH || y >= SOTN_FRAME_HEIGHT)
                goto draw_complete;
        }
        if (actor.id == 0xb)
        {
            SotnItem relic = {0};
            if (!SotnRelic_Read(session->dra, actor.parameters & 0x7fff, &relic) ||
                !SotnItemIcon_Draw(session->dra, relic, Pixel(actor.x) - camera_x - 8,
                                   Pixel(actor.y) - camera_y - 8, session->frame,
                                   SOTN_FRAME_PIXELS))
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
                !SotnItemIcon_Draw(session->dra, item, Pixel(actor.x) - camera_x - 8,
                                   Pixel(actor.y) - camera_y - 8, session->frame,
                                   SOTN_FRAME_PIXELS))
                return false;
            goto draw_complete;
        }
        if (session->stage_id == 65 && actor.id == 0x59 && actor.step >= 3 &&
            !SotnDialogueRenderer_Item(session->dra, session->nz0, actor.parameters,
                                       Pixel(actor.x) - camera_x, Pixel(actor.y) - camera_y,
                                       (Uint16)(actor.scale_x / 16), (Uint16)(actor.scale_y / 16),
                                       actor.step == 4 && (session->simulation_ticks & 2),
                                       session->frame, SOTN_FRAME_PIXELS))
            return false;
        const SotnActorSprite sprite = {actor.animation_set,  actor.frame,      actor.palette,
                                        actor.texture_offset, actor.blend_mode, actor.facing_left};
        if (command.kind == SOTN_DRAW_ACTOR &&
            !DrawActorPrimitives(session, actor, camera_x, camera_y))
            return ActorDrawError(actor);
        if ((actor.frame & 0x7fffU) == 0 ||
            (actor.scale_set && (actor.scale_x == 0 || actor.scale_y == 0)))
            goto draw_complete;
        if (!SotnActorGraphics_DrawTransform(
                session->actor_graphics, sprite, Pixel(actor.x) - camera_x,
                Pixel(actor.y) - camera_y,
                (SotnSpriteTransform){.rotation = actor.rotated ? actor.rotation : 0,
                                      .scale_x = actor.scale_x ? actor.scale_x : 256,
                                      .scale_y = actor.scale_y ? actor.scale_y : 256,
                                      .opacity =
                                          actor.opacity_set || actor.opacity ? actor.opacity : 128,
                                      .tint = actor.tinted,
                                      .red = actor.red,
                                      .green = actor.green,
                                      .blue = actor.blue},
                session->frame, SOTN_FRAME_PIXELS))
            return ActorDrawError(actor);
    draw_complete:
        if (!SotnFrameClip_CopyOutside(clip, session->clip_backup, session->frame,
                                       SOTN_FRAME_PIXELS))
            return false;
    }
    return true;
}

static bool DrawDialogue(SotnLaboratory *session)
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
    return SotnDialogueRenderer_Draw(session->actor_graphics, session->dialogue, session->frame,
                                     SOTN_FRAME_PIXELS);
}

static bool DrawGameplay(SotnLaboratory *session)
{
    SotnPlayerState state = {0};
    if (!SotnPlayer_GetState(session->player, &state))
        return false;
    Sint32 camera_x = 0, camera_y = 0;
    Camera(session, state, &camera_x, &camera_y);
    const SotnTileSources sources = {StageGraphics(session), SotnResource_View(session->common)};
    Uint16 backdrop = 0;
    if (!SotnActors_GetBackdrop(session->actors, &backdrop))
        return false;
    for (size_t i = 0; i < SOTN_FRAME_PIXELS; ++i)
        session->frame[i] = backdrop;
    return SotnTilemap_Draw(&session->room.background, sources, camera_x, camera_y, session->frame,
                            SOTN_FRAME_PIXELS) &&
           (session->stage_id != 2 ||
            DrawActorsAndPlayer(session, state, camera_x, camera_y, true)) &&
           SotnDialogueRenderer_Background(session->dialogue.fade, session->frame,
                                           SOTN_FRAME_PIXELS) &&
           SotnTilemap_Draw(&session->room.foreground, sources, camera_x, camera_y, session->frame,
                            SOTN_FRAME_PIXELS) &&
           (!session->save_room ||
            SotnSavePointRenderer_Draw(session->save_renderer, session->save_point, false,
                                       session->frame, SOTN_FRAME_PIXELS)) &&
           DrawActorsAndPlayer(session, state, camera_x, camera_y, false) &&
           SotnSubweapons_Draw(session->subweapons, session->actor_graphics, session->dra,
                               session->simulation_ticks, camera_x, camera_y, session->frame,
                               SOTN_FRAME_PIXELS) &&
           (!session->save_room ||
            SotnSavePointRenderer_Draw(session->save_renderer, session->save_point, true,
                                       session->frame, SOTN_FRAME_PIXELS)) &&
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
    Sint32 tick = 63 - (Sint32)(session->map_ticks & 63);
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
            !DrawMap(session) ||
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
    return session != NULL && !session->launch_active && SotnGameOver_Finished(session->game_over);
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
        SotnMapRenderer_Destroy(session->map_renderer);
        SotnResource_Close(session->map_art);
        SDL_free(session->directory);
        SotnGameOver_Destroy(session->game_over);
        SotnResource_Close(session->game_over_art);
        SotnInventoryRenderer_Destroy(session->inventory_renderer);
        SotnSaveStore_Destroy(session->saves);
        SotnSavePointRenderer_Destroy(session->save_renderer);
        SotnHud_Destroy(session->hud);
        SotnMusic_Destroy(session->speech);
        SotnMusic_Destroy(session->music);
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

static bool RestoreCharacter(SotnLaboratory *session, SotnSaveState state)
{
    const SotnCharacterState *c = &state.character;
    if (!SotnCharacter_Restore(session->character, c) || !ApplyCharacter(session) ||
        !SotnPlayer_Restore(session->player, c->hp, state.player_random))
        return false;
    return true;
}

static bool RestoreSession(SotnLaboratory *session, SotnSaveState state, Uint32 slot)
{
    const SotnCharacterState *c = &state.character;
    const SotnActorProgress progress = {c->hearts, c->subweapon,       c->gold,
                                        c->kills,  state.actor_random, state.combat_random};
    if (!RestoreCharacter(session, state) || !SotnActors_Restore(session->actors, progress))
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
    SotnSaveState saved = {0};
    const Uint32 slot = old->launcher.slot;
    SotnLaboratory *next = NULL;
    bool ready = false;
    if (travel && SaveState(old, &saved))
    {
        saved.stage = old->arrival.stage;
        saved.room = old->arrival.room;
        ready = CreateSession(data, old->directory,
                              (SessionStart){.stage = saved.stage,
                                             .room = saved.room,
                                             .saved = &saved,
                                             .arrival = &old->arrival,
                                             .music = old->music_stream},
                              &next);
    }
    else if (action == SOTN_LAUNCH_CHECKPOINT)
        ready = CreateSession(data, old->directory,
                              (SessionStart){.stage = 12, .room = 1, .music = old->music_stream},
                              &next);
    else if (action == SOTN_LAUNCH_LOAD && old->saves != NULL &&
             SotnSaveStore_Read(old->saves, slot, &saved))
        ready = CreateSession(data, old->directory,
                              (SessionStart){.stage = saved.stage,
                                             .room = saved.room,
                                             .saved = &saved,
                                             .slot = slot,
                                             .music = old->music_stream},
                              &next);
    if (ready && travel)
    {
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
