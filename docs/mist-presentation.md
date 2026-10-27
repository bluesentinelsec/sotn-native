# Mist sprite capture and palette ownership

The owned actor texture service now captures real, unrotated normal Alucard poses
for the [Mist visual cursor](mist-effect.md). It uploads borrowed ARC pixels into
the original private display slot, clears the original right/bottom margins and
returns the eight original BlinkWhite snapshot wedges. It never changes ARC, DRA
or common artwork. The player/effect owners still need to call these services in
gameplay; Mist controls remain unavailable.

Capture retains original pivot/facing conventions, half-edge and center rounding,
negative screen coordinates and UV origin. Odd-height bottom edges use the original
half-height value. It validates the private unperturbed snapshot profile and display
slot against the source UV layout before writing. Bat, Wolf, rotated and alternate
sheet snapshots need their own original adapters; this API cannot stand in for them.

Selected original palette jobs now queue, cancel and advance in owned texture
memory. Job 27 copies the initial Mist palette. Jobs `0x11C` and `0x11F` blend toward
ordinary/Gas Cloud colors; `0x11D` restores current character colors. All source,
destination, interval and color references come from checked private descriptors
and live owned palette rows. The character owner can refresh its normal row as
equipment colors change; restoration never relies on an obsolete startup copy.

The selected queue keeps the original 32-slot first-free order, channel-eight
cancellation, countdown-before-work and 32 color-stepping applications. A full
queue omits the new optional request and preserves existing owners. Canceled
flagged blends do not cancel the unflagged initial copy. Color stepping preserves
the destination mask bit while moving each RGB555 channel one unit toward its
target. All selected operations validate before mutating colors or job cursors.
This queue covers isolated Mist jobs; contention with other original palette jobs
still needs shared integration. Unsupported descriptors fail explicitly.

The geometry cursor's texture pages are original primitive values. A renderer
must apply draw-mode `0x537`, including the `+0x20` additive texture-page adjustment,
vertex modulation, semi-transparency, dithering and hide flags. These services
upload pixels and animate palettes; they do not establish final framebuffer/GPU
parity or enable the live player transformation and sensor restoration.

## Validation

All 128 test groups pass in Debug, Release and macOS ASan/UBSan; release packaging
passes. Authored public fixtures cover captured geometry, odd heights, negative
facing, exact packed-pixel residency, original clear margins, copy/blend timing,
live restoration colors, mask bits, cancellation, capacity and atomic failures.

Private CMake-library helpers run through muted LLDB and exit successfully.
They validate 1,578 real ARC/DRA pose captures, including every supported normal
pose in both facings at three screen origins, and compare all eight wedges with
the original BlinkWhite instructions. SDK calls and the afterimage helper execute;
the fixture gives afterimages their own independent primitives, and an explicit
adapter supplies successful capture primitive allocation. Native upload pixels
and clear margins are checked separately through the owned surface API.

Another 1,492 records match original palette request acceptance and every resulting
color across initialization, ordinary/Gas Cloud fades, live character-palette
updates, restoration, repeated cancellation and full queues. Original queue,
cancellation, color stepping and live source reads execute; only GPU `LoadClut`
uses an adapter. Debug/sanitizer capture and palette traces are byte-identical.
These are component comparisons, not shared-pool, framebuffer, playable-form,
uninterrupted-route or console parity. Private images and traces stay ignored.

Provenance: `dra/7879C.c:EntityPlayerBlinkWhite`, `4B758.c:RenderEntities` and
`4A538.c` selected palette services, sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
