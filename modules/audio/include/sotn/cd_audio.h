#ifndef SOTN_CD_AUDIO_H
#define SOTN_CD_AUDIO_H
#include <sotn/music.h>

/** @brief Selected original CD command owner backed by the existing native XA decoder.
 * Owns the current decoder and original two saved positions, never a second voice pool.
 * Native read/configure completion does not claim original CD hardware latency/interpolation. */
typedef struct SotnCdAudio SotnCdAudio;
/** @brief Committed original queue/acknowledgement/playback state, distinct from PCM completion. */
typedef struct SotnCdAudioState
{
    Uint32 started, duration;
    Uint16 playing, queued, command, commands;
    Uint8 phase, saved, volume, target_volume;
    bool acknowledged, queue_empty, playing_xa, busy, serial;
    bool decoder_present;
    SotnMusicState decoder;
} SotnCdAudioState;
/** @brief Create the selected command owner and adopt existing native stage music on success.
 * @param dra Borrowed private DRA. @param stream Borrowed verified music stream.
 * @param music Owned decoder pointer, set NULL only on success; *music may be NULL.
 * @param clock Actual engine/VSync clock. @param out Owned output or NULL.
 * @return Success or SDL error. Borrowed inputs must outlive this owner.
 * The existing native stage intro/loop decoder retains its own loop service. */
bool SotnCdAudio_Create(SotnDataView dra, SotnStream *stream, SotnMusic **music, Uint32 clock,
                        SotnCdAudio **out);
/** @brief Queue original10/11/A or one of the eight selected US Fairy narrative IDs.
 * Sets the original acknowledgement-busy flag immediately for10/11. Commands subsequently
 * enter the original256-entry command ring and CD queue, with original overflow behavior.
 * @return Success or SDL error without mutation for unsupported commands. */
bool SotnCdAudio_Queue(SotnCdAudio *audio, Uint16 command);
/** @brief Execute sound commands, original expiry, then one CD-command phase and PCM tick.
 * @param audio Actual owner. @param clock Actual wrapping engine/VSync clock, not save playtime.
 * @param pcm Full stereo output. @param count At least SOTN_SOUND_FRAMES*2 samples.
 * @return Success or SDL error; stop on an actual decoder/I/O failure. Native configuration,
 * first-sector reads, retained decoder positions and pause/resume commit before acknowledgement.
 * Caller owns menu/freeze engine/audio scheduling. This enables no Fairy controls by itself. */
bool SotnCdAudio_Tick(SotnCdAudio *audio, Uint32 clock, Sint16 *pcm, size_t count);
/** @brief Read the actual selected owner without advancing clocks, decoding or queues. */
bool SotnCdAudio_Get(const SotnCdAudio *audio, SotnCdAudioState *out);
/** @brief Destroy current/saved decoders, retaining borrowed private input/stream ownership. */
void SotnCdAudio_Destroy(SotnCdAudio *audio);
#endif
