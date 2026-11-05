#ifndef SOTN_ACTOR_PRIMITIVE_H
#define SOTN_ACTOR_PRIMITIVE_H
#include <SDL3/SDL.h>
/** @brief Copied stage primitive, owned by its actor; positions are room-local 16.16 pixels. */
typedef struct SotnActorPrimitive
{
    Sint32 x, y;
    Uint16 width, height, page, palette;
    Uint8 u[4], v[4], red, green, blue;
    bool active, textured, blend, modulate;
    bool framebuffer;  /**< Sample the ordered RGB555 framebuffer snapshot, not stage artwork. */
    bool screen_space; /**< Projected screen coordinates; camera is already applied. */
    bool camera_fixed; /**< Original DRAW_ABSPOS/DRAW_MENU: omit shared camera-shake offsets. */
    bool priority_set; /**< Use this primitive's original ordering-table priority. */
    Uint16 priority;
    bool line; /**< Two-color line through offset vertices 0 and 1. */
    Uint8 line_red, line_green, line_blue, line_blend_mode;
    bool gouraud;
    Uint8 vertex_red[4], vertex_green[4], vertex_blue[4];
    bool quad; /**< Use explicit pixel offsets from x/y for a textured or colored quadrilateral. */
    Sint16 offset_x[4], offset_y[4];
    bool clip; /**< Ordered draw-environment change; bounds are screen pixels, right/bottom
                  exclusive. */
    Sint16 clip_left, clip_top, clip_right, clip_bottom;
    Uint8 target; /**< Ordered target: 0 normal, 1/2 strict begin/restore,
                      3/4 original replaceable environment begin/restore. */
    Sint16 target_offset_x, target_offset_y;
    bool target_clear;
} SotnActorPrimitive;
#endif
