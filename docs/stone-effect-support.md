# Shared stone-effect support

The existing shared native runtime now accepts the original stone text3, scattered
smoke3/7, tile-particle3/8 and entity24/high0 factories. They use actual original
roots, child slots, stage primitive leases, private animation/artwork and rendering.
These are supporting owners. The [shared dissolve owner](stone-dissolve.md) now
supplies actual entity32/capture/uploads; the petrification controller's connection
and complete effect production remain unfinished. Supplied factory
requests do not establish a completed petrification or stone-death sequence.

Smoke3 and7 consume three shared character RNG calls only on an actual birth.
Their horizontal ranges differ; facing flips the resulting offset, and separate
rolls select scale/velocity index2..5 and the whole-word Y adjustment. Retained
fractions and the complete factory parameter survive. Later animation/motion uses
no RNG. The undefined US status20000 birth branch remains explicitly unsupported,
before any RNG consumption; no deterministic replacement is introduced.

Particle3 uses its actual behavior2, eight tiles plus a sentinel and four calls
per tile. Particle8 uses behavior8, thirty tiles plus a sentinel and four calls
per tile. They retain distinct horizontal pre-displacement, live facing, Y/delay
ranges and signed arithmetic. Their47/63 lifetime words decrement twice on motion
updates, with retirement before tile work at zero. Individual delay words wrap,
hide only at zero, and continue updating hidden tiles. Motion preserves packed
fractions, source whole-word origins, gravity and constant colors. Allocation
failure retires before RNG. Existing child row-index capacity now accommodates31
actual indices; tile draw tokens use a30-tile stride in ordering and decoding.
This changes no entity or primitive pool.

The existing entity24 owner also accepts its original high0 path. It consumes two
birth calls, retains full fractions and unused low parameters, uses normal scale,
original palette195/transparency and the same private timeline as Hellfire's high1
path. High1 keeps its single call and half scale. Native rendering reads the actual
scale flags; an unscaled child does not acquire an invented half-size transform.
Text3 uses its original private mode1 record and existing eighteen-row text/trail
owner, with actual delayed grounding, packed target, one tink and retirement.

Authored tests cover random birth ownership, malformed-animation RNG rollback,
full low parameters, full row capacity, signed motion/delay wrap, both lifetime
boundaries, actual sprite-scale flags, text3 lifecycle/leases and draw-token bounds.
Private instruction comparisons match192 particle contexts/2,880 updates/55,296
complete tile records,384 smoke contexts/9,984 complete behavior-and-animation
updates,432 text3 contexts/65,016 updates and96 entity24 contexts/1,152 supplied-
animation snapshots. Particle comparisons also execute four actual AluInit vector
loops without substituting those vectors for stone particle behavior2. RNG,
allocation/destruction, text diagnostic/audio services and supplied entity24
animation durations have explicit adapters. Five Debug/ASan trace pairs match
byte for byte. Original tables, artwork and raw traces remain private.

Muted native protocols exercise each of the six supplied factories separately.
They start with default70HP/20MP and charge actual character birth RNG0/30/72/64/240/48,
reserve eighteen/nine/thirty-one rows where required, draw private artwork, hold
full runtime through actual inventory open/hold/close and repeated drawing, run
actual Summon Spirit's33 frozen updates plus release, then retire real children
and rows. These controlled factory/protection checks establish their shared
owners, separately from an actual petrification producer or ordinary encounters.

Both actual CMake apps separately supply all six factories simultaneously. All
six classes contribute pixels: text147, smoke56/28, particles7/30 and normal
entity24 sprite710. Actual inventory open/hold/close preserves the full runtime
and gameplay clock. The original kind0 range32..63 fills: an actual Summon command
spends MP and recovers normally after its optional effect cannot allocate. A
later ordinary command, after the effects retire, runs33 frozen updates plus the
release frame. All original supplied children retire, followed by normal SDL quit.
These are explicit factory/protection and contention contexts, separate from
actual stone production or full sharing parity.

All208 tests pass in Debug/Release/macOS ASan-UBSan.34 Python tests, repository
policy, formatting, static SDL linkage and ZIP packaging accompany the batch.
Exact-head platform results are recorded with the PR. Source comparisons and
native/app protocols retain their explicit fixture boundaries.

The actual petrification/dissolve producer, rare statue transitions, remaining
stone deaths, common captured-image contention and all six
[Alucard milestone gates](alucard-status.md) remain open at31/34 dispatched states.
No ordinary savev9 fields, default grants or acquisition/route/console claims change.

Provenance: `dra/7879C.c:EntitySmokePuff`/`EntityGuardText`/`EntityUnkId24`,
`7E4BC.c:func_8011E4BC` and original private factory scheduling, pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
