#ifndef SOTN_SOUND_H
#define SOTN_SOUND_H

#include <sotn/game_data.h>

/** @brief Borrowed private sound-bank inputs for a supported US stage. */
typedef struct SotnSoundSources
{
    SotnDataView dra, common_body, player_header, player_body, stage_header, stage_body;
} SotnSoundSources;

/** @brief Owned sound simulation with bounded SPU memory, voices, and scripts. */
typedef struct SotnSound SotnSound;

/** @brief Stereo 44.1 kHz sample frames per development simulation tick. */
enum
{
    SOTN_SOUND_FRAMES = 735
};

/** @brief Create a sound simulation without opening an audio device.
 * @param sources Immutable private inputs, alive until destruction; sample bodies are copied.
 * @param out Receives ownership on success, NULL on failure when non-NULL.
 * @return Success, or false with SDL_GetError() for invalid data or allocation failure.
 * @note Supports selected direct/scripted sound effects. CD music, reverb, and full
 * hardware/audio scheduling parity are not implemented by this component. */
bool SotnSound_Create(SotnSoundSources sources, SotnSound **out);
/** @brief Attach verified immutable Librarian SEQ and original SDK pitch tables; no playback yet.
 * @param sound Borrowed initialized sound owner. @param executable Borrowed private boot image.
 * @param data Borrowed version-one Library SEQ. Both views must outlive sound destruction.
 * @return Success or SDL error, leaving the existing attachment unchanged on failure. */
bool SotnSound_SetSequence(SotnSound *sound, SotnDataView executable, SotnDataView data);
/** @brief Start the selected original Librarian sequence with private DRA settings.
 * @param sound Borrowed initialized owner with attached sequence input. @param id US ID 0x202.
 * @return Success or SDL error. Voices share the original bounded hardware pool with effects.
 * Full reverb/hardware timing parity remains incomplete. */
bool SotnSound_PlaySequence(SotnSound *sound, Uint16 id);
/** @brief Release only sequence-owned voices and stop its cursor; other effects remain active.
 * @param sound Borrowed initialized owner or NULL. */
void SotnSound_StopSequence(SotnSound *sound);

/** @brief Queue an original sound-effect ID for the next simulation tick.
 * @param sound Borrowed owned simulation. @param id Original US ID, 0x601..0x8e0, or 0xa1 to
 * release ambience channels 20/21.
 * @return Success, or false with SDL_GetError() for invalid input or a full 256-event queue. */
bool SotnSound_Queue(SotnSound *sound, Uint16 id);
/** @brief Queue an original effect with explicit volume and signed stereo distance.
 * @param sound Borrowed simulation. @param id Original effect ID, 0x601..0x8e0.
 * @param volume Original 0..127 multiplier. @param pan Original distance, -8..8.
 * @return Success or SDL error; the queue is unchanged on invalid input or exhaustion.
 * Pan coefficients remain in the private DRA image. */
bool SotnSound_QueueVolumePan(SotnSound *sound, Uint16 id, Uint16 volume, Sint8 pan);

/** @brief Queue original channels 22/23 gain without restarting their loop or envelope.
 * @param sound Borrowed simulation. @param volume Original multiplier 0..127.
 * @param pan Original signed distance -8..8. @return Success or SDL error on invalid input
 * or queue exhaustion. A valid update with no active ambience has no effect. */
bool SotnSound_QueueAmbienceVolume(SotnSound *sound, Uint16 volume, Sint8 pan);

/** @brief Advance scripts/voices and render one tick of signed native-endian stereo PCM.
 * @param sound Borrowed owned simulation, advanced on success.
 * @param pcm Receives SOTN_SOUND_FRAMES * 2 initialized samples.
 * @param count Sample capacity, at least SOTN_SOUND_FRAMES * 2.
 * @return Success, or false with SDL_GetError(); stop the session after a data failure.
 * @note No device access, allocations, or filesystem reads occur in this method. */
bool SotnSound_Tick(SotnSound *sound, Sint16 *pcm, size_t count);
/** @brief Release sound state without freeing borrowed image/header bytes.
 * @param sound Owned sound simulation, or NULL. */
void SotnSound_Destroy(SotnSound *sound);

#endif
