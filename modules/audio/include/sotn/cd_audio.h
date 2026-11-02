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
    Sint16 fade_multiplier, fade_counter; /**< Original signed CD-volume fade words. */
    Sint16 master_volume, master_target;  /**< Original signed master fade words. */
    Sint32 release_counter;               /**< Original D_800BD1C4 countdown. */
    Uint8 phase, saved, volume, target_volume, mode;
    bool acknowledged, queue_empty, playing_xa, busy, serial, managed;
    bool fade_acknowledged; /**< Original single fade flag, distinct from10/11 and queue empty. */
    bool xa_active; /**< Original xa_play_ck: pending fade or playing ID, never data readiness. */
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
/** @brief Adopt an existing decoder into an empty prepared CD owner, without allocating/ticking.
 * @param audio Actual owner with no decoder, bookmark or pending command.
 * @param music Owned decoder pointer, set NULL only on success; *music may be NULL.
 * @param clock Actual engine clock. Both owners borrow the same live verified stream.
 * @return Success or SDL error preserving both owners on failure. Retains the decoder's
 * real resampler/sector/ADPCM position. This lets a resource transaction allocate first
 * and transfer ownership only at commit; it supplies no playback acknowledgement. */
bool SotnCdAudio_AdoptMusic(SotnCdAudio *audio, SotnMusic **music, Uint32 clock);
/** @brief Bind the actual configured SPU owner for CD-produced reverb depth work.
 * @param audio Actual CD owner. @param sound Borrowed live sound owner sharing the same DRA.
 * @return Success or SDL error without mutation. No queues, decoders or DSP clocks advance.
 * The sound owner must remain live until replacement or CD destruction. Native callers rebind
 * prepared sound replacement before retiring the departing owner. This does not enable CD sends. */
bool SotnCdAudio_BindSound(SotnCdAudio *audio, SotnSound *sound);
/** @brief Validate a prepared SPU dependency without changing the live binding or clocks.
 * @param audio Actual CD owner. @param sound Prepared configured owner sharing its DRA.
 * @return Success or SDL error without mutation. A successful main-thread check permits
 * BindSound after other preparation, before retiring the previous SPU owner. */
bool SotnCdAudio_CheckSoundBinding(const SotnCdAudio *audio, const SotnSound *sound);
/** @brief Original MuteSound/InitSoundVars1 after StopSeq, without a sample tick.
 * @param audio Actual bound owner with closed SEQ. Retires its current/saved transports and serial
 * mix.
 * @return Success or SDL error before mutation for invalid/open-SEQ inputs. Clears command/effect
 * queues, abandoned bookmarks and SEQ attachment, resets master127/multiplier32, retaining
 * DSP/RAM, envelopes, copied scripts, original restore byte and retained XA configuration words. */
bool SotnCdAudio_Mute(SotnCdAudio *audio);
/** @brief Queue selected original SPU/CD commands or an original US XA ID301..532.
 * Sets the original acknowledgement-busy flag immediately for10/11. Commands subsequently
 * enter the original256-entry command ring and CD queue, with original overflow behavior.
 * Commands90..94 immediately set the separate fade flag; command8 owns the original
 *128/8/4/2/1 multipliers, signed counter and deferred stop2. Shared multiplier changes
 * can raise gain; the original flag can clear while another fade remains queued.
 * Commands80..84 require the bound actual SPU: command10 controls master gain, depth0,
 * StopSeq, delayed all24-voice RR8/cleanup and queued CdFadeOut1 (command3) restoring master127.
 * Apply master_volume once to the final native mix, including SPU, CD and legacy inputs.
 * With the bound SPU,1/7/E/F/A0..A8/202/204 share this actual ring: retained22/23 gain,
 * SEQ close, script pause/resume, all24 RR14 cleanup,20/22 release/rekey and attached
 * Library/Chapel SEQ start. A3/A7 release conditionally without clearing current IDs;
 * A4/A8 rekey those identities with their retained gain/distance. Native callers update
 * effects/scripts before executing the ring. Duplicate XA early return holds later SPU controls;
 * resume does not replay a script until the following script update. Unbound callers retain
 * the selected standalone sound APIs; these commands fail before queuing without a real binding.
 * Bound B owns stop2/StopSeq/all24 RR8;12 owns original CD16 bookmark-cursor reset.
 * Actual private metadata/format and native reads are validated before playback acknowledgement.
 * Modes0/1 use original continuation/command6; adopted legacy intro/loop decoders keep
 * their existing native loop service. Enables no additional stage/familiar controls.
 * Managed loop seeks require the retained configured base; changed global queued-base
 * contention fails explicitly before seeking and still requires its actual owner.
 * @return Success or SDL error without mutation for commands outside the original domain. */
bool SotnCdAudio_Queue(SotnCdAudio *audio, Uint16 command);
/** @brief Retain original22/23 gain/distance now and queue command1 in the shared ring.
 * @param audio Actual bound owner. @param volume Multiplier0..127. @param pan Distance-8..8.
 * @return Success or SDL error before mutation on invalid input. No current ID means no change
 * or queued command. Execution uses the then-current ID, including entry0 after a release;
 * this never retriggers a voice or advances scripts, decoders or samples. */
bool SotnCdAudio_QueueAmbienceVolume(SotnCdAudio *audio, Uint16 volume, Sint8 pan);
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
