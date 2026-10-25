#ifndef SOTN_MUSIC_H
#define SOTN_MUSIC_H

#include <sotn/sound.h>
#include <sotn/stream.h>

/** @brief Owned streaming Laboratory music decoder and rate-conversion state. */
typedef struct SotnMusic SotnMusic;

/** @brief Start the original Laboratory track using private DRA music metadata.
 * @param dra Borrowed immutable US DRA image, alive until destruction.
 * @param stream Borrowed verified raw music container, alive until destruction.
 * @param out Receives ownership on success, NULL on failure when non-NULL.
 * @return Success, or false with SDL_GetError() for invalid data or allocation failure.
 * @note Supports the selected 4-bit stereo 37.8 kHz XA path. The SDL rate converter is a
 * temporary service; original CD interpolation/timing parity is not established. */
bool SotnMusic_Create(SotnDataView dra, SotnStream *stream, SotnMusic **out);
/** @brief Start Entrance wind or Dracula's Castle from private intro/loop metadata.
 * @param dra Borrowed DRA. @param stream Borrowed verified XA container.
 * @param castle True for post-Death music, false for outdoor wind.
 * @param out Receives ownership or NULL. @return Success or SDL error.
 */
bool SotnMusic_CreateEntrance(SotnDataView dra, SotnStream *stream, bool castle, SotnMusic **out);
/** @brief Start the original quiet wind at the first Entrance hallway.
 * @param dra Borrowed DRA. @param stream Borrowed verified XA container.
 * @param out Receives ownership or NULL. @return Success or SDL error.
 */
bool SotnMusic_CreateEntranceQuiet(SotnDataView dra, SotnStream *stream, SotnMusic **out);
/** @brief Play one of the original opening Death dialogue XA clips once.
 * @param dra Borrowed DRA. @param stream Borrowed verified XA container.
 * @param id Original sound ID 0x35f..0x363. @param out Receives ownership or NULL.
 * @return Success or SDL error. Both borrowed inputs must outlive playback.
 */
bool SotnMusic_CreateDeathSpeech(SotnDataView dra, SotnStream *stream, Uint16 id, SotnMusic **out);
/** @brief Play a selected original Death or Laboratory Maria XA speech clip once.
 * @param dra Borrowed private DRA. @param stream Borrowed verified XA container.
 * @param id Original selected dialogue ID. @param out Receives ownership or NULL.
 * @return Success or SDL error. Inputs must outlive playback. */
bool SotnMusic_CreateSpeech(SotnDataView dra, SotnStream *stream, Uint16 id, SotnMusic **out);
/** @brief Whether playback has remaining ticks; NULL and completed speech report false. */
bool SotnMusic_Playing(const SotnMusic *music);
/** @brief Start original Slogra/Gaibon boss music with its private introduction/loop metadata.
 * @param dra Borrowed verified US DRA, alive through playback. @param stream Borrowed verified
 * music container, alive through playback. @param out Owned playback, NULL on failure.
 * @return Success or SDL error. Uses the same selected XA decoder as Laboratory music. */
bool SotnMusic_CreateBoss(SotnDataView dra, SotnStream *stream, SotnMusic **out);
/** @brief Play the original US game-over music once, then return silence.
 * @param dra Borrowed immutable DRA. @param stream Borrowed verified music container.
 * @param out Receives ownership, NULL on failure. @return Success, or false with SDL_GetError().
 * @note Both borrowed inputs must outlive playback. */
bool SotnMusic_CreateGameOver(SotnDataView dra, SotnStream *stream, SotnMusic **out);
/** @brief Decode and return one development tick of stereo 44.1 kHz music.
 * @param music Borrowed owned playback state, advanced on success.
 * @param pcm Receives SOTN_SOUND_FRAMES * 2 initialized signed samples.
 * @param count Sample capacity, at least SOTN_SOUND_FRAMES * 2.
 * @return Success, or false with SDL_GetError(); stop after a data/I/O failure.
 * @note Preserves track selection/filter and introductory/loop metadata; no entire-file loading.
 * Reads are bounded and run on the caller's thread. */
bool SotnMusic_Tick(SotnMusic *music, Sint16 *pcm, size_t count);
/** @brief Destroy playback without closing its borrowed stream or DRA image.
 * @param music Owned playback or NULL. */
void SotnMusic_Destroy(SotnMusic *music);

#endif
