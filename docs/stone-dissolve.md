# Shared stone dissolve owner

The shared native runtime now accepts original factory39/entity32 with thirty-six
actual GT4 rows, resident texture capture and private stone-pattern uploads. This
is the effect owner. The later [selected stone controller](player-petrification.md)
now produces its actual factory and selected supporting effects. Supplied factory requests establish neither complete
petrification nor stone death.

Birth retries a failed real row allocation before RNG. A successful birth consumes
37 character RNG calls and retains original fractions, palette and sprite margins.
The following callback copies the previous resident display texture, not source
artwork, into the existing shared5120-byte DRA image. Directional taps select the
original eight-update pattern sequence, with forty calls per update and packed
nibble uploads to the actual primary rectangle. Frame bit8000 skips artwork
upload; it does not hide the body. Rendering uses the resident texture and original
stone palette, SDK rotation and pivots.

Leaving the statue animations copies that same image to the adjacent secondary
rectangle. Original engine-clock modulo3, or modulo2 during state16, gates48
committed decay updates. Six groups of forty nibbles, cursor+35 wrapping, signed
byte motion, delay underflow and hidden-row motion retain their original order.
The final upload retires the actual child and rows before geometry/reset work.
Menus, frozen spell updates, release and repeated draws hold the owner; rebases
translate the source-owned entity/packets without ticking. Scoped room-entry
cleanup retires entity32 and releases its leases.

All siblings use the same existing image in actual slot order. Native effect-update
rollback restores both real texture rectangles and their validity flags, image,
runtime, stage leases, character RNG and affected frame/afterimage/timer15 state.
It does not establish atomic rollback for the entire player HP/motion/audio update.
A later water death may reuse the scratch through its actual new capture generation;
an already-completed capture of that same generation remains invalid. No second
image pool or ordinary-save fields are introduced.

Authored tests cover allocation retry, lifecycle, clocks, image service failure,
nibble completion, no-tick rebases, actual36-row stage lease retention/release,
both texture rectangles and unloaded-texture validity rollback. Private instruction
comparisons match192 contexts,15,360 callbacks and538,272 complete GT4 records,
including all5120 image bytes after every callback. Debug and ASan traces match
byte for byte. Allocation, RNG, image services, destruction, afterimage and camera
services have explicit comparison adapters. Private artwork/tables/raw traces are
not published.

Muted native protocols in Debug and ASan start with default70HP/20MP, apply an
explicit zero-damage petrification request, use actual controls/collision to land,
then supply factory39. One child uses36 rows and37 birth calls, actual previous-
resident capture and primary uploads, then contributes765 secondary-texture pixels
after escape and completes49 secondary uploads: the initial copy plus48 decays.
Two supplied siblings use72 rows and one scratch; failure after two real uploads
restores actual textures, validity and effect transaction state. A separate HP1/
lethal-water-result fixture subsequently completes a new water capture and64
decays to real game over. Natural enemy contacts after escape may still damage
the player; default starting resources are not a final-HP invariant.

Both actual CMake apps independently render765 secondary pixels, hold the full
runtime and gameplay clock through actual inventory open/hold/close, hold the
active dissolve through actual Summon Spirit's33 frozen updates plus release,
then resume, retire and quit normally through SDL. The menu/spell checks occur
after escape. Petrification, factory, protection and lethal-result fixtures remain
separate from ordinary producers, acquisition, routes and console evidence.

All209 tests pass in Debug, Release and macOS ASan/UBSan. Python tests, repository
policy, formatting, static SDL linkage and ZIP packaging accompany the batch;
exact-head platform checks are recorded with the PR. The selected controller has
separate evidence; remaining stone/grab/death/fade and captured-image contention, and all six
[Alucard milestone gates](alucard-status.md) remain open at31/34 dispatched states.
Default grants and ordinary savev9 remain unchanged.

Provenance: `dra/7E4BC.c:EntityPlayerDissolves`, original image descriptors,
factory scheduling and afterimage services, pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
