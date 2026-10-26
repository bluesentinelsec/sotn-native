# Olrox's Quarters stone door

NO2's placed stone door now runs its original opening, forced movement, arrival
and closing sequence. It patches its own four collision tiles and renders three
owned shaded panels. This ordinary door has no Jewel of Open requirement; keep
it distinct from the Laboratory's magical barrier and NO2's shared red doors.

The private layouts place this actor in room 11 with the alternate stone artwork.
Both orientations, alternate/ordinary graphics and four tile layouts retain the
original rules. The player must walk toward the door within the original 24-by-32
proximity test. Opening pauses movement for the angular sweep, then requests
three-tick walking intervals. Arrival starts open, halts downward motion and walks
the player clear before closing. Selected transient protection follows the door
sequence. Complete afterimage suppression remains unavailable with that service.

Geometry uses the original SDK quarter-wave `rsin`/`rcos` table and arithmetic
shifts, distinct from the matrix sine/cosine-pair table. Preserve the asymmetrical
right-side shading and the retained edge UV at angle `0xA00`. Opening/closing
sound requests retain original volume, pan and the original duplicate opening
request. UVs, replacement tiles, artwork and lookup arrays remain private.

Owned primitives follow generation checks and room cleanup. Primitive exhaustion
removes the door actor as in the original. Validate the complete collision-column
patch before changing any tiles. No transient door pose or movement request is
stored in ordinary saves; arrival/re-entry recreates them from private layouts.
Version-nine saves and earlier migrations remain unchanged.

## Validation and remaining work

Synthetic tests cover both orientations, all four tile layouts and both graphics
variants, facing/proximity, opening/arrival/closing timing, forced controls,
fraction retention, panel shading, sound requests, pool exhaustion, invalid
identities/parameters, truncated lookup data and atomic rejection of malformed
tile patches. Fixtures contain no original arrays or artwork.

A private comparison checks 9,600 root/visible-primitive snapshots against the
original US `EntitySealedDoor`, proximity/side helpers and SDK `rsin`/`rcos`
instructions. Initialization, allocation, sound and protection use isolated
adapters. Compared fields include states, angular sweep, root position, control
buttons/timers, vertical halt, collision hashes, sound volume/pan, visible panel
coordinates, colors, UVs and ordering priority. This is component evidence, not
PS1 framebuffer, full audio or uninterrupted-route parity.

Muted Debug and macOS ASan/UBSan LLDB checks, including the CMake-built app,
start at the placed room-11 door, observe forced entry
and closing, then use ordinary Left input to reopen it and pass the collision
column. The ordinary development character receives no stats, relics, equipment
or position changes during play. This explicit room start does not establish
connected travel, ordinary whole-castle traversal or console parity. NO2 remains
disconnected while its other mechanisms/scenery, Olrox and progression are
integrated. The full castle and Alucard's abilities remain incomplete.

Provenance: `st/e_sealed_door_no2.h`, included by `st/no2/e_sealed_door.c`, at
sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
