// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/42398.c:MainGame status-clock tail, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. No private code/data is embedded.
#include <SDL3/SDL_error.h>
#include <sotn/playtime.h>
static Sint32 Signed(Uint32 n)
{
    return n <= SDL_MAX_SINT32 ? (Sint32)n : (Sint32)((Sint64)n - 4294967296LL);
}
static Sint32 Add(Sint32 a, Sint32 b)
{
    return Signed((Uint32)a + (Uint32)b);
}
bool SotnPlaytime_Advance(SotnPlaytime *clock, Sint32 vcount, bool menu, bool map_pause)
{
    if (!clock)
        return SDL_InvalidParamError("playtime");
    const Uint32 count = (Uint32)vcount + 255U;
    const Sint32 elapsed =
        count < 0x80000000U ? (Sint32)(count >> 8) : (Sint32)((Sint64)(count >> 8) - 16777216);
    if (!menu && !map_pause)
    {
        clock->frames = Add(clock->frames, elapsed);
        if (clock->frames >= 60)
        {
            clock->frames = Add(clock->frames, -60);
            clock->seconds = Add(clock->seconds, 1);
            if (clock->seconds >= 60)
            {
                clock->seconds = Add(clock->seconds, -60);
                clock->minutes = Add(clock->minutes, 1);
                if (clock->minutes >= 60)
                {
                    clock->minutes = Add(clock->minutes, -60);
                    clock->hours = Add(clock->hours, 1);
                    if (clock->hours >= 100)
                    {
                        clock->seconds = 59;
                        clock->minutes = 59;
                        clock->hours = 99;
                    }
                }
            }
        }
    }
    return true;
}
bool SotnPlaytime_FromTicks(Uint64 ticks, SotnPlaytime *out)
{
    if (!out || ticks > 99999ULL * 3600 * 60)
        return SDL_SetError("Playtime requires the existing bounded native saved tick field");
    *out = (SotnPlaytime){.hours = (Sint32)(ticks / (3600U * 60U)),
                          .minutes = (Sint32)((ticks / 3600U) % 60U),
                          .seconds = (Sint32)((ticks / 60U) % 60U),
                          .frames = (Sint32)(ticks % 60U)};
    return true;
}
bool SotnPlaytime_Ticks(SotnPlaytime clock, Uint64 *out)
{
    if (!out || clock.hours < 0 || clock.minutes < 0 || clock.seconds < 0 || clock.frames < 0)
        return SDL_SetError("Saved playtime requires nonnegative actual status words");
    const Uint64 ticks =
        (((Uint64)clock.hours * 60U + (Uint32)clock.minutes) * 60U + (Uint32)clock.seconds) * 60U +
        (Uint32)clock.frames;
    *out = SDL_min(ticks, 99999ULL * 3600 * 60);
    return true;
}
