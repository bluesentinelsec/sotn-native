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
 * @note Supports selected direct/scripted sound effects. CD music, SEQ, reverb, and full
 * hardware/audio scheduling parity are not implemented by this component. */
bool SotnSound_Create(SotnSoundSources sources, SotnSound **out);
/** @brief Queue an original sound-effect ID for the next simulation tick.
 * @param sound Borrowed owned simulation. @param id Original US ID, 0x601..0x8e0, or 0xa1 to
 * release ambience channels 20/21.
 * @return Success, or false with SDL_GetError() for invalid input or a full 256-event queue. */
bool SotnSound_Queue(SotnSound *sound, Uint16 id);
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
