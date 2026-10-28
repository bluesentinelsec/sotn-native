#ifndef SOTN_PLAYER_PARTICLES_H
#define SOTN_PLAYER_PARTICLES_H
#include <sotn/player_factory.h>
/** @brief Selected original tile particles; the actual owner reserves count+one stage rows. */
#define SOTN_PLAYER_PARTICLE_LIMIT 16
/** @brief Original fixed-point particle data and emitted signed-word tile position. */
typedef struct SotnPlayerParticle
{
    Sint32 x, y, velocity_x, velocity_y;
    Sint16 screen_x, screen_y;
    Uint16 timer, draw_mode;
    Uint8 red, green, blue, width, height;
} SotnPlayerParticle;
/** @brief Selected entity-five profiles1/9/13, with no primitive pool or factory ownership. */
typedef struct SotnPlayerParticles
{
    SotnPlayerParticle tiles[SOTN_PLAYER_PARTICLE_LIMIT];
    Uint32 flags;
    Uint16 parameter, priority, lifetime;
    Uint8 count, phase;
    bool removed;
} SotnPlayerParticles;
/** @brief Screen-space source/player and successful shared allocation at this entity update. */
typedef struct SotnPlayerParticlesInput
{
    SotnPlayerFactorySource
        source; /**< Owner projects the actual factory source into screen space. */
    Sint16 player_y;
    Sint32 water_level;
    Uint16 parameter, player_priority;
    bool allocated;
} SotnPlayerParticlesInput;
/** @brief Read selected private profile capacity, including the original final sentinel row.
 * @param dra Borrowed private profile pointers/records. @param parameter Actual child parameter.
 * @param count Receives required rows. @return Success or SDL error, preserving output on failure.
 * This reports capacity only; it allocates no rows and signals no readiness. */
bool SotnPlayerParticles_Rows(SotnDataView dra, Uint16 parameter, Uint16 *count);
/** @brief Advance selected original tile-particle behavior once.
 * @param particles Zero-initialized actual child's copied state. @param dra Borrowed private data.
 * @param input Current source/player and real allocation result. @param random Owned RNG snapshot.
 * @return Success or SDL error, preserving state/RNG on malformed inputs. Failed initial allocation
 * retires the child before RNG consumption. The owner commits actual row/generation ownership,
 * draws nonhidden tiles, and retires the sentinel together with the child. */
bool SotnPlayerParticles_Tick(SotnPlayerParticles *particles, SotnDataView dra,
                              SotnPlayerParticlesInput input, Uint32 *random);
/** @brief Move retained tile positions by signed-word camera/room pixels without advancing time.
 * @param particles Actual child. @param x Horizontal shift. @param y Vertical shift.
 * @return Success or SDL error without mutation on malformed child metadata. */
bool SotnPlayerParticles_Rebase(SotnPlayerParticles *particles, Sint16 x, Sint16 y);
#endif
