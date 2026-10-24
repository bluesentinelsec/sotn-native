#ifndef SOTN_PLAYBACK_H
#define SOTN_PLAYBACK_H

#include <sotn/sound.h>

/** @brief Owned default-device stereo PCM playback, used on the simulation thread. */
typedef struct SotnPlayback SotnPlayback;

/** @brief Open a paused 44.1 kHz signed-16 stereo device, without starting playback.
 * @param out Receives ownership on success, NULL on failure when non-NULL.
 * @return Success, or false with SDL_GetError(); releases partial initialization.
 * @note Owns one SDL audio-subsystem reference until destruction. */
bool SotnPlayback_Create(SotnPlayback **out);
/** @brief Queue one simulation tick, then start playback if necessary.
 * @param playback Borrowed playback state.
 * @param pcm Borrowed stereo samples; copied before return.
 * @param count Must be SOTN_SOUND_FRAMES * 2 samples.
 * @return Success, or false with SDL_GetError(); stop after a device failure.
 * @note Discards a backlog exceeding 250 ms, preserving the checkpoint policy. */
bool SotnPlayback_Queue(SotnPlayback *playback, const Sint16 *pcm, size_t count);
/** @brief Pause and discard pending audio after focus changes or debugger gaps.
 * @param playback Borrowed playback state.
 * @return Success, or false with SDL_GetError(). The next queue starts playback. */
bool SotnPlayback_Reset(SotnPlayback *playback);
/** @brief Close the device and release its SDL audio-subsystem reference.
 * @param playback Owned playback state or NULL. */
void SotnPlayback_Destroy(SotnPlayback *playback);

#endif
