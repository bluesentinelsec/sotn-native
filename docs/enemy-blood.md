# Enemy blood and deferred healing

Successful cutting hits on enemies with the original blood flag now create the
common blood actor in the real stage slots160..191. It reserves exactly twelve
contiguous rows in the existing stage primitive pool, renders the original red
tiles, and releases its rows on retirement. Exhaustion omits this optional effect;
it never replaces another actor or consumes particle RNG before allocation.

The component retains the original 48-update lifetime, root acceleration, growing
absorption hitbox and permanent absorption cutoff. Its twelve particles preserve
four stage `Random` calls each, descending launch order, independent packed
fractions, signed-word positions and shrinking-tile cadence. This uses stage
`Random`, not the separate combat `rand` stream. Room/camera rebases move visible
tiles and the root without advancing simulation or replacing retained fractions.

An actual absorbing-player status and blood contact request 8 HP, doubled to 16 by
an equipped Bloodstone. An already pending heal wins while every consumed contact
still increments the original counter. The next successful player update applies
and clamps HP once. The session synchronizes ordinary character/HUD HP and requests
the original pickup sound and both shared healing-flash factories, except that
stone suppresses the flashes. Both flashes use actual existing child slots, real
eight-row stage leases and uploaded body artwork. Allocation remains a retry;
no pending child signals readiness. Healing does not revive a death state.

[Selected Dark Metamorphosis](player-dark-metamorphosis.md) now supplies the actual
casting/timer/status owner. The earlier private contact/queued-heal fixtures
establish these component boundaries separately from that playable integration. Floating HP
numbers, other healing producers, all-ability contention and full raster/audio/
whole-route/console parity remain incomplete. Bloodstone can now equip through the
ordinary owned accessory path; no default item or ability is granted. Saves remain
version nine and retain no transient blood actors, pending heals or effects.

Public authored tests cover simultaneous real actor contacts during invulnerability,
allocation failure before RNG, twelve-row ownership,
fragmented-pool exhaustion, both facings, lifetime/contact cutoff, pending-heal and
Bloodstone amounts, no-tick rebases, invalid-input rollback, deferred HP/clamping,
shared flash lifetimes and ordinary accessory ownership. Synthetic SDK samples
remain distinct from private comparisons.

Private instruction comparisons match 288 contexts and 7,200 complete blood/body/
twelve-tile/healing/RNG snapshots against the original Laboratory instructions,
including word/fraction wrap, disabled absorption and the cutoff-frame contact.
Actual stage RNG and SDK trigonometry execute; allocation, destruction and
accessory-count calls use explicit boundary adapters. Debug and ASan traces match.
The two added flash profiles also match the original controller/shared angles/SDK
quads within 288 normal/Bat-image contexts and 24,544 updates, including existing
profiles. These comparisons do not establish a complete game frame.

A muted native Step/Draw check uses placed Laboratory enemies, explicit Gladius
and X124 entry positioning with ordinary initial stats/HP/MP. It produces four
cutting-hit blood births and 11,302 cumulative contributed pixels, then verifies
complete actor retirement. A separate later HP/queued8 fixture verifies the HP
update and both actual flash owners. These fixtures write no saves and grant no
relics or contact protection; they are separate from ordinary traversal or casting.
The actual CMake application also verifies four distinct generations, contiguous
leases, healing, both flash owners and normal SDL quit in Debug and ASan/UBSan.

Provenance: selected `st/e_particles.h:EntityEnemyBlood`,
`st/st_common.h:UnkEntityFunc0`, `st/st_update.h:Random`, `st/collision.h`,
`dra/692E8.c:EntityAlucard` healing and `dra/7879C.c:EntityPlayerBlinkWhite`,
pinned sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
Original definitions, SDK tables and traces remain private runtime inputs.
