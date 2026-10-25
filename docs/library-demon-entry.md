# Lesser Demon arena and reward

The Library now runs the placed Lesser Demon manager, sealing books, original
camera-lock transition, selected combat, music handoffs, defeat record and Life
Max Up reward. Stage 2 remains disconnected while librarian/shop and remaining
scripts are integrated. This is an explicit-room development checkpoint, not a
complete Library route or authentic whole-game claim.

The original manager creates the body and its preceding camera actor in reserved
slots. Crossing the original world-coordinate entry rectangle activates combat
and marks event 17 visited. The eight sealing books fall in sequence and change
their private foreground tiles. Camera parameter 6 retains the Library's original
sixteen-update interpolation; it does not use the other overlays' direct-lock or
debug-cancel behavior. Completing the encounter unlocks its horizontal bounds.

Boss music waits for the previous owner to stop. Physical defeat records the
original HHMMSS time; the manager waits for the body's final completion signal
and an available reward slot before reopening the books and resuming Wood Carving
Partita. Exhausting the optional reward pool keeps the arena closed until the
allocation succeeds, as in the original. Exact CD latency and complete audiovisual
parity remain unproven.

The shared reward effect now accepts the Library's original drop selection and
initialization. It becomes the original Life Max Up, adds five maximum HP and
restores HP through the existing collection service. Reward projection reads the
original private matrix-pair table instead of an approximate rounded sine. A
private 40,000-case comparison matches the pinned SDK algorithm with those original
pairs. This replaces the earlier reward check against the reference backend's
approximate compiled pairs; it is not a general GTE or console comparison.

Native sealing-book signals require matching owner slots and generations. The
original last segment writes an unused field in the following unrelated actor;
the native adapter deliberately omits that unsafe write. Original placed actors
and unrelated fixed-slot occupants cannot be replaced by the book factory.

Ordinary v7 saves already hold all 32 original event records and visited bits.
No format or application version changes. A nonzero event-17 record prevents the
placed manager from creating another boss after Continue/reentry. As in the
original time-attack service, a record at time zero remains zero; no synthetic
permanent defeat bit is added. Transient arena, attack and seal state is not saved.

All 69 public groups pass in Debug and macOS ASan/UBSan; release packaging passes.
Synthetic cases cover entry boundaries, reserved ownership, music waits, reward
pool exhaustion, tile closure/restoration, adjacent-generation protection, camera
interpolation/unlock, reward conversion and recorded-defeat suppression.

A private native LLDB sequence starts explicitly in Library room 0 with a protected
9000-HP/strength-99 character. Original placements create the boss; the original AI
selects dive, spit, bolt and summoning before normal combat inputs defeat it. Debug
and ASan/UBSan agree at tick 3670, 918 renders, 139 observed death ticks and record
52. The sequence observes closing/opening seals, reward effect/collection, then
starts explicitly in save room 14, uses the ordinary slot UI, performs a fresh
Continue and checks defeated room reentry. Maximum HP increases by five beyond
its intervening original level gains. It uses an isolated save directory and
preserves the user's ordinary slots. This is not uninterrupted traversal, an
unmodified-character playtest, poison validation or console parity.

Algorithms adapt pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, `st/lib/unk_3B200.c`,
`unk_3B53C.c`, `e_lock_camera.c` and `st/e_life_up.h` (AGPL-3.0-or-later).
Selected reward matrix/projection algorithms in `boss_projection.c` adapt PSY-Z
`b52003e55c0d1b20719e984dd19bd4d760eeb14a`, `psyz/src/psyz/libgte.c`
(MPL-2.0). All original tile/drop/math/art/audio tables remain private runtime inputs.
