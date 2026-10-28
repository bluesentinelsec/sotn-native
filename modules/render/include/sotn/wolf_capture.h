#ifndef SOTN_WOLF_CAPTURE_H
#define SOTN_WOLF_CAPTURE_H
#include <sotn/actor_graphics.h>
#include <sotn/wolf_presentation.h>
/** @brief Maximum selected player plus fourteen-body normal/capture packets. */
#define SOTN_WOLF_FRAME_PACKETS (15 * SOTN_WOLF_DRAW_MAX_PACKETS)
/** @brief Copy the original unperturbed Wolf outline used by Mist's snapshot.
 * @param dra Borrowed private BlinkWhite profiles. @param bounds Retained actual body bounds.
 * @param x Signed-word screen anchor X. @param y Signed-word screen anchor Y.
 * @param facing_left Original facing, selecting the capture texture origin only.
 * @param out Receives eight unmirrored wedges, unchanged on failure.
 * @return Success or SDL error. Signed/inverted bounds and byte UV wrap are retained.
 * This copies geometry only: the session must retain its actual ordered capture pixels.
 * It uploads no normal sprite and advances no body, effect or readiness signal. */
bool SotnWolfCapture_Snapshot(SotnDataView dra, SotnWolfCaptureBounds bounds, Sint32 x, Sint32 y,
                              bool facing_left, SotnMistWedge out[8]);
/** @brief Render selected captured Wolf packets into the original owned 63x63 target.
 * @param graphics Borrowed texture owner, with no active target interval.
 * @param packets Borrowed generated packets in original entity/piece emission order.
 * @param count Rows, at most SOTN_WOLF_FRAME_PACKETS; normal packets are ignored.
 * @param clear Original shared-environment background-clear bit from current player status.
 * @return Success or SDL error; failed draws cancel this interval without changing owned pixels.
 * Captures draw by ascending original priority and reverse emission order at ties. Caller owns
 * actual environment/GPU reservations and ordered stage/render integration. This method captures
 * no private source memory, advances no simulation and signals no transformation readiness. */
bool SotnWolfCapture_Render(SotnActorGraphics *graphics, const SotnWolfDrawPacket *packets,
                            size_t count, bool clear);
#endif
