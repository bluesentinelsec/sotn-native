# Selected Library parallax backgrounds

The Library now presents the original repeating bookcases, furniture, column
shadows and distant shelves. Model depth/rotation, lighting grids and distant
anchor coordinates stay in the user's private LIB overlay. Camera movement uses
the original projected anchor and repeated/culled geometry. The distant grids
retain alternating UV direction and per-vertex private brightness.

The selected projector uses the existing SDK divider and saturation math, with
the original screen distance and the pinned models' identity rotations. It rejects
unselected nonzero rotations, invalid depth/control bounds and truncated private
records. Signed GPU rectangle coordinates are checked before narrowing.

Backdrop color follows the original background-actor initialization and persists
within the stage actor owner across room changes. It is transient presentation,
not ordinary save data. New stage owners begin with black until an actor changes
it. No application or v8 save-format version changes.

Library background primitives use original priorities 0x20–0x60, ahead of their
foreground tile bucket 0x60. The native renderer presents these actors between
the background and foreground tile layers. Original primitive packets precede
previously submitted tile packets at that tied bucket. Normal actors and the
player remain in the existing subsequent pass. This is the selected Library
layering path, not complete general PS1 GPU packet emulation.

## Evidence and limits

Public synthetic tests cover axial projection, signed rounding/saturation,
malformed/truncated records, repeated geometry, column-shadow tail behavior,
distant private anchor/lighting decoding, camera displacement, backdrop ownership
and invalid indices/coordinate narrowing.

A private comparison matches 100000 anchors against the pinned SDK backend with
original matrix pairs, including negative and saturated screen coordinates. This
checks selected projection math, not console rasterization or a full scene replay.

Native checks use original placements in rooms 9, 8 and 0 with an explicitly
protected character. They exercise all four background kinds and 72 rendered
frames. The room-0 start uses its original background placement window; starting
below that window does not initialize those actors and is not coverage. These
are explicit room starts, not uninterrupted route or console parity evidence.

Exact GPU dithering/color interpolation, general priority ties, the rotating
secret bookcase, sitting, Mist-form interaction, remaining shop services and
ordinary Library travel remain incomplete. Do not describe the area as complete.

Behavior adapts `st/lib/e_library_bg.c` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later). The existing
bounded projection backend derives from PSY-Z
`b52003e55c0d1b20719e984dd19bd4d760eeb14a` (MPL-2.0). Original model, texture,
palette, anchor and lighting tables remain private.
