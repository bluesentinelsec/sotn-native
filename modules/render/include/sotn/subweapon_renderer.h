#ifndef SOTN_SUBWEAPON_RENDERER_H
#define SOTN_SUBWEAPON_RENDERER_H
#include <sotn/actor_graphics.h>
#include <sotn/subweapon.h>
/** @brief Draw selected original projectile artwork without advancing simulation.
 * @param service Borrowed projectile state. @param graphics Borrowed loaded private textures.
 * @param dra Immutable private flame UV definitions. @param ticks Gameplay animation clock.
 * @param camera_x Room camera. @param camera_y Room camera.
 * @param frame Full RGB555 framebuffer. @param count Available pixels.
 * @return Success or SDL error; stop presentation on failure. Complete trails/glass effects remain
 * outside this selected primitive path. */
bool SotnSubweapons_Draw(const SotnSubweapons *service, const SotnActorGraphics *graphics,
                         SotnDataView dra, Uint64 ticks, Sint32 camera_x, Sint32 camera_y,
                         Uint16 *frame, size_t count);
#endif
