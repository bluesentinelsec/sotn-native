# Wolf retained-image component

The core reconstructs the selected original US Wolf root's six textured
primitives: five retained-position images and one live speed-dependent overlay.
It consumes [root requests](wolf-root-head.md) and copied actual primitive/image
snapshots. It owns no pool rows or captured pixels, and enables no live Wolf
controls or relic switches.

Preserve the first-update ordering: selecting an uninitialized row initializes
and hides it; a later selection starts its retained image. A selection can replace
an existing image. Positions and facing remain captured while colors fade, with
different decay on each side and a separate hide update after timer expiry.
Priority follows the live player. The sixth row follows the current position and
facing, retaining the original unsigned-byte color wrap and upper-only speed cap.
UVs do not mirror with facing. Palette words remain owned by the caller.

All six packets reference the original shared captured texture. They do not own
six frozen pixel copies. The eventual renderer must supply the current articulated
Wolf capture and preserve source drawing/target order, palette, clipping and
primitive capacity. Room/camera rebases must move retained geometry without
advancing phases. Requests and computed packets never establish a successful
allocation, finished capture or transformation readiness.

Public authored tests cover delayed first selection, retained origin/facing,
reselection, asymmetric fade, final visible/hide order, live priority, speed-color
wrap, signed-word coordinates and atomic invalid-input failure. Private muted
LLDB comparisons cover 49,152 complete six-row updates, including supplied retained
phases and primitive metadata. Original instructions execute without service or
math adapters. Debug and ASan/UBSan traces are identical. These are packet/controller
comparisons, not proof of captured-pixel rendering, native form ownership, ordinary
relic acquisition, uninterrupted movement or console parity.

Provenance: `dra/wolfForm.c:func_8012F178`, pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
All runtime artwork and comparison traces remain private.
