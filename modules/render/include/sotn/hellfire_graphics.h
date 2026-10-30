#ifndef SOTN_HELLFIRE_GRAPHICS_H
#define SOTN_HELLFIRE_GRAPHICS_H
#include <sotn/actor_graphics.h>
/** @brief Upload both original private Hellfire gradient rows to the actual VRAM owner.
 * @param graphics Owned idle target service. @param dra Borrowed private image.
 * @return Success or SDL error. Validates both rectangles/pixel bounds before writing.
 * Original gradient pixels remain private; this advances no simulation or animation. */
bool SotnHellfireGraphics_Upload(SotnActorGraphics *graphics, SotnDataView dra);
/** @brief Draw the original untextured G4 flash using its retained packet and real target.
 * @param graphics Actual texture/target owner. @param quad Borrowed real flash packet.
 * @param x Horizontal projection offset. @param y Vertical projection offset.
 * @param surface Actual destination. @return Success or SDL error. Never ticks the child. */
bool SotnHellfireGraphics_Flash(SotnActorGraphics *graphics, const SotnPlayerEffectQuad *quad,
                                Sint32 x, Sint32 y, SotnActorSurface surface);
#endif
