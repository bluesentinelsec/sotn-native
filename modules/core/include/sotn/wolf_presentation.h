#ifndef SOTN_WOLF_PRESENTATION_H
#define SOTN_WOLF_PRESENTATION_H
#include <sotn/player_blink.h>
#include <sotn/wolf_articulation.h>
/** @brief Original bank15 part and current rendering context, with actual remaining GPU capacity.
 */
typedef struct SotnWolfDrawInput
{
    SotnWolfPart part;
    Sint16 player_x, player_y;
    Uint16 player_priority, slot, available;
    Uint32 game_timer;
} SotnWolfDrawInput;
/** @brief One original normal/capture packet, in source emission order. */
typedef struct SotnWolfDrawPacket
{
    SotnPlayerEffectQuad quad;
    bool capture;
} SotnWolfDrawPacket;
/** @brief Maximum packets from one selected original Wolf frame (two multipart pieces). */
#define SOTN_WOLF_DRAW_MAX_PACKETS 4
/** @brief Reconstruct bank15 multipart geometry and its original capture duplicate.
 * @param dra Borrowed private bank15 definitions. @param executable Borrowed SDK tables.
 * @param input Copied actual screen-space part and player context; owner checks generation.
 * @param out Caller-owned packet buffer, disjoint from input images. @param capacity Buffer rows.
 * @param count Receives emitted rows. @return Success or SDL error, preserving outputs on failure.
 * Preserves cropping, source pivots/SDK rotation, palette offsets, raw-color semantics, priority
 * and normal-before-capture exhaustion. The owner sorts actual priorities/ties, reserves real
 * GPU capacity and applies packets to normal/owned-capture surfaces. No pixels are captured and
 * no transformation readiness or live Wolf controls are enabled by this component. */
bool SotnWolfPresentation_Packets(SotnDataView dra, SotnDataView executable,
                                  SotnWolfDrawInput input, SotnWolfDrawPacket *out, size_t capacity,
                                  size_t *count);
#endif
