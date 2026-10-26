# Olrox's Quarters Valhalla Knights

The internal NO2 profile now advances Valhalla Knight bodies, two adjacent contact
actors and thirteen death pieces using original private initialization, animation,
sensor, hitbox and launch profiles. The [area remains disconnected](olrox-quarter-runtime.md)
while other enemies, mechanisms and Olrox still need integration.

Bodies retain the original landing, charge, brake/turn, player-airborne decision,
run-up, leap, landing recovery and physical death paths. The patrol anchor uses
room-world coordinates; camera movement must not add another scroll offset.
Leap gravity combines the original ground helper's increment and the subsequent
per-state adjustment. Braking/landing dust uses the original optional gray-puff pool.

The first adjacent child extends the body's damage region. Hits route to the
body's HP and share its attack cooldown, stun and death flags, preventing duplicate
damage from overlapping regions. The second child supplies independent weapon
contact. It retains its own attack definition and does not route damage to the
body. All links validate slot, generation, identity and parameter; stale children
retire and cannot hurt the player while awaiting their update. Re-entry replaces
only matching owned children, preserving unrelated reservations.

Defeat creates three selected explosions and thirteen rotating sprite pieces.
Pieces retain inherited body velocity, private launch/rotation, gravity and
simulation-frame explosion/sound phases. Original optional pool exhaustion omits
effects without overwriting another actor or consuming an unallocated explosion's
random choice. Motion and private indices receive explicit bounds checks.

Original runtime arrays, artwork and sound stay private. This adds no save fields,
development grants, new public stage start or connected route. Ordinary saves
remain version nine. Complete hit/death effects, loot, sound/reverb, contact priority
and whole-encounter audiovisual/timing parity still require separate verification.

## Validation

Public synthetic cases cover charge/turn/leap/recovery, frame-selected body and
child hitboxes, linked damage/cooldowns, independent weapon contact, stale owners,
all thirteen death profiles, full optional pools, owned re-entry, occupied slots,
invalid states/tables/parameters and motion overflow. Fixtures contain no private
game arrays or assets; existing actor/combat tests remain enabled.

Ignored comparisons pass 8,000 original/native instruction ticks across both
facings and grounded/airborne player cases, covering every normal motion state
and leap substate with both contact children. A further 2,496 instruction ticks
cover all thirteen death pieces in both facings, including periodic explosion
allocation, random consumption and sound. They execute original motion, animation,
decisions and factories with isolated initializer, synthetic flat-floor collision,
allocation, destruction and sound adapters. These are component comparisons,
not a full original engine or console replay.

A private native LLDB sequence starts explicitly beside an original placed
room-eight Knight. An ordinary development character with Gladius deals damage
to its original 161 HP, completes physical death, awards level-adjusted EXP from
0 to 303 and releases both contact children. The sequence takes 1,598 combat
ticks plus 180 effect ticks and renders 445 frames. It protects the player without
stat, equipment, HP or defeat grants. The rendered encounter was inspected;
the absent house/background mechanisms remain visible limitations. This fixture
does not prove ordinary combat balance, uninterrupted traversal or console parity.
Audio uses SDL's dummy device; real-device listening and complete audio parity
remain separate. Existing user saves remain untouched.

Provenance: `st/e_valhalla_knight.h`, NO2 identities/initializers, selected
`st/collision.h` link handling and `st/e_misc.h` gray-puff factory at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
