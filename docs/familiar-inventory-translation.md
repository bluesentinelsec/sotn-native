# Familiar inventory, translation and recovery-flash services

The native character and Fairy runtime now supply the inventory and translation
services needed by the upcoming gameplay binding. The shared Blink owner also
accepts the original finite Life Apple recovery flash4A. These services do not
activate Fairy or complete rescue17. Ordinary card lifecycle, saved familiar
progression, whole-pass native transactions and the FFF/player17 handshake remain
required. All six [Alucard gates](alucard-status.md) remain open.

Fairy consumption decrements the original unequipped hand-stock byte at its actual
use pose. It ignores Duplicator and equipped copies. The earlier stock check does
not reserve an item: ordinary hand use can spend the last spare during Fairy's
animation delay, and the later unchecked byte decrement then wraps0 to255. Native
character validation now admits the original full byte range for hand stock;
ordinary pickups still stop at99 or above. Body-stock validation retains its
existing99 bound. The caller must commit this operation after the actual native
factory/player transaction succeeds, including optional factory exhaustion.

The existing4096-byte version-nine save record already stores raw hand bytes.
It now round-trips this reachable255 stock value without a version, layout, field,
grant or encoder change. Existing checks still reject malformed body counts,
inventory order, reserved bytes and unsupported versions. No transient familiar
slots, timers, audio or statistics have been placed in reserved v9 storage.

Post-callback translation keeps full entity fractions and defined signed wrapping.
Camera and player-lock entity flags apply independently. Primitive camera2 takes
priority over player200; player packet translation subtracts the arithmetic high
word of the original displacement. Negating a floored opposite displacement would
move fractional cases by an extra pixel. The hidden DF sentinel participates in
the same packet pass. DD's ordinary435 fan omits camera2 and stays at its original
birth coordinates during a camera move.

Connected-room preparation validates every actual lease before any sibling row
retires, removes slots without20000 and translates retained entities and flag100
packets. Generations, event history, sound state, timers, RNG and callback counts
do not advance. Cold sources can move without initializing an effect cursor.
Destination initialization and actual native camera/player displacement inputs
remain the lifecycle caller's responsibility; this API does not manufacture them.

Blink4A uses the existing real eight-row Blink controller and private profile.
Its ordinary brightness/96-tick hold/fade and afterimage-reset semantics are
preserved, including phase-zero retry on actual allocation failure. The recovery
producer must restore the actual player state and resources; the flash itself
restores neither.

Public checks cover actual character consumption with zero/one/two Duplicators,
manual-use overlap and byte underflow, unchanged equipment/RNG, pickup suppression,
v9 byte round-trip, primitive priority, signed wrapping, cold positions, stale-lease
rollback, room retirement and finite recovery-flash timing. Private comparisons
match6144 translation contexts:4608 retained entity/TILE/G4 instruction fragments
and1536 original room retirements. The original room-prune function executes with
an explicit DestroyEntity adapter. These are bounded original engine fragments,
not a complete engine replay.

The private recovery-flash comparison matches7152 complete controller/shared-angle/
eight-quad snapshots across48 actual normal/Bat image, facing, rotation and position
contexts. Original image lookup, profiles and SDK geometry execute; primitive
allocation/destruction and SDK leading-zero instructions have explicit adapters.
Debug and ASan/UBSan traces match for both comparisons. Supplied flags, positions,
initialization and inventory are fixtures, not ordinary activation, acquisition,
uninterrupted traversal, completed rescue or console evidence.

Provenance: selected `servant/tt_002/faerie.c`, `dra/5D5BC.c`,
`dra/5087C.c:RunMainEngine` and `dra/7879C.c` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
Private tables, assets and source traces remain outside published source.

The later [retirement/whole-namespace validation](familiar-lifecycle.md) closes a
late-sibling failure gap in the original translation precheck: all real rows and
absent-lease orphans are now validated before any sibling retires. Earlier row0-only
prechecks did not establish that complete failure boundary.
