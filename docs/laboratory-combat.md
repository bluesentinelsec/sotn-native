# Laboratory combat integration

## Empty-hand attack dependency

The development player has empty hands. Z/X/D map to left-hand attack, jump,
and right-hand attack; Xbox X/A/B provide the same actions. This is a development
loadout, not recovered equipment, a save game, or original stage entry.

The player reads equipment zero, seven attack variants, animation streams,
and hitboxes from private DRA data. It preserves grounded attack locking,
early direction/crouch changes, held-button recovery, airborne motion, and
normal collision. The attack view carries original element, hit type/effect,
stun duration, enemy invincibility, and base critical-rate metadata. No game
arrays or sprite assets are embedded. Ability inputs remain unsupported.

This dependency alone does not resolve hits, damage enemies, break candles,
collect items, or play audio. Those scene services follow in separate PRs.
Weapons, inventory, subweapons, and original startup remain incomplete.

## Provenance and checks

The selected paths adapt `src/dra/6E42C.c`, `71830.c`, `7879C.c`'s
`EntityUnarmedAttack`, and `alu_anim.c`'s `UpdateUnarmedAnim` at the pinned
reconstructed C revision recorded in `config/reference-us.json`.
`modules/core/player.c` retains AGPL-3.0-or-later attribution. Runtime tables
are accessed through checked image-relative addresses, never host pointers.

Synthetic player tests cover attack lock, active versus recovery hitboxes,
held recovery, crouch and airborne variants, both hands, landing, and atomic
failure on bad private references. Keyboard and virtual Xbox tests cover both
attack controls. A local 150-tick sequence rendered our native Laboratory
scene under LLDB, including 32 active hitbox ticks, and exited successfully.
This verifies the exercised path, not complete combat parity.
