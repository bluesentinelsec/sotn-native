#ifndef SOTN_SEQUENCE_H
#define SOTN_SEQUENCE_H
#include <sotn/game_data.h>
/** @brief Selected original SEQ channel events. Meta timing and loop commands stay internal. */
typedef enum SotnSequenceEventKind
{
    SOTN_SEQUENCE_NOTE,
    SOTN_SEQUENCE_VOLUME,
    SOTN_SEQUENCE_PAN,
    SOTN_SEQUENCE_BEND,
    SOTN_SEQUENCE_STOP
} SotnSequenceEventKind;
/** @brief Copied original channel state at an event; zero velocity releases a note. */
typedef struct SotnSequenceEvent
{
    SotnSequenceEventKind kind;
    Uint8 channel, bank, program, note, value, volume, pan;
} SotnSequenceEvent;
/** @brief Borrowed event sink. Return false with SDL error to stop the owning audio session. */
typedef bool (*SotnSequenceSink)(void *context, SotnSequenceEvent event);
/** @brief Caller-owned bounded SEQ cursor. Initialize through SotnSequence_Open, never raw bytes.
 */
typedef struct SotnSequence
{
    SotnDataView data;
    size_t cursor, start, loop;
    Sint32 delta, step, period;
    Uint16 resolution, repeats, completed;
    Uint8 program[16], volume[16], pan[16], channel, running, bank, nrpn, loop_count;
    bool playing, loop_enabled, loop_count_set;
} SotnSequence;
/** @brief Open original version-one SEQ data without allocating or playing it.
 * @param data Immutable borrowed input, alive until the sequence is discarded.
 * @param bank Original initialized VAB index, 0..3. @param repeats Zero loops indefinitely.
 * @param out Caller-owned state, unchanged on failure. @return Success or SDL error.
 * Supports selected original note/control/program/bend/tempo/loop commands, not general MIDI.
 */
bool SotnSequence_Open(SotnDataView data, Uint8 bank, Uint16 repeats, SotnSequence *out);
/** @brief Advance the original SDK's quantized 60-Hz scheduling once; never call per render.
 * @param sequence Initialized writable state. @param sink Borrowed synchronous event receiver.
 * @param context Borrowed receiver context; no callback may reenter this owner.
 * @return Success or SDL error. Stop the session after a failure; emitted events cannot roll back.
 * A stopped sequence emits nothing. Instruction and input bounds prevent unbounded loops.
 */
bool SotnSequence_Tick(SotnSequence *sequence, SotnSequenceSink sink, void *context);
#endif
