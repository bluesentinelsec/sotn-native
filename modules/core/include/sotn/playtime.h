#ifndef SOTN_PLAYTIME_H
#define SOTN_PLAYTIME_H
#include <SDL3/SDL_stdinc.h>

/** @brief Actual signed status words updated at the end of MainGame. */
typedef struct SotnPlaytime
{
    Sint32 hours, minutes, seconds, frames;
} SotnPlaytime;
/** @brief Normalize the raw signed SDK count with original signed32 wrap.
 * @param vcount Actual supplied vertical count. @return Arithmetic (vcount+255)>>8. */
Sint32 SotnPlaytime_FrameDelta(Sint32 vcount);
/** @brief Advance the original status clock using SDK vertical-count normalization.
 * @param clock Borrowed mutable status words; preserved on failure.
 * @param vcount Signed SDK count. Native fixed60 frames supply256.
 * @param menu Actual post-update inventory pause flag.
 * @param map_pause Actual post-update map/status pause flag.
 * @return Success, or SDL error for a null clock. Carry occurs once per word,
 * preserving signed32 wrap and the original99:59:59 hour-carry cap. This does not
 * update gameplay clocks, the map cursor or global RNG. */
bool SotnPlaytime_Advance(SotnPlaytime *clock, Sint32 vcount, bool menu, bool map_pause);
/** @brief Decode the existing native v9 aggregate field without clamping live hours.
 * @param ticks Saved ticks, bounded by the existing99999-hour native ceiling.
 * @param out Receives copied words; preserved on failure.
 * @return Success or SDL error. Legacy hours remain until the original hour carry. */
bool SotnPlaytime_FromTicks(Uint64 ticks, SotnPlaytime *out);
/** @brief Encode the existing aggregate field without mutating live words.
 * @param clock Copied status words; all must be nonnegative for serialization.
 * @param out Receives aggregate ticks, capped at the existing99999-hour ceiling.
 * @return Success or SDL error, preserving output on failure. Adds no save fields. */
bool SotnPlaytime_Ticks(SotnPlaytime clock, Uint64 *out);
#endif
