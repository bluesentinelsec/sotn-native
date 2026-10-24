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

The native scene resolves empty-hand hits against candles and Bloody Zombies.
Zombies use runtime health, defense, hitboxes, cooldowns, stun, hit reactions,
and death animation. Candles become their original heart/gold/subweapon prize
and retain their persistence flag when revisiting rooms. Prize motion, expiry,
and collection update a stage-session development inventory. Common explosions
render from private data. Damage and item events emit original sound IDs for
the following audio integration.

The explicit development profile has 70 HP, base stats of 6, empty equipment,
10/50 hearts, and Cube of Zoe enabled. Its unarmed physical attack is 6 and
critical rate is zero. This enables testing; it is not original stage entry or
a recovered save. Alucard takes ordinary physical contact damage, runs hit
reactions, and has recovery invulnerability. Reaching zero HP stops with a clear
message because death/continue and saves remain unintegrated. Inventory changes
appear in SDL logs; the original HUD is not implemented.

Equipped weapons, subweapon use, inventory menus, experience/level progression,
enemy loot, original blood/glow/hit primitives, all elemental player damage,
full shared random-state consumption, startup, and complete audio parity remain incomplete.
Subweapon swapping retains the previous pickup and its toss/recovery timing;
its sparkle primitive is omitted. Unsupported vessels remain marked unsupported.
Do not describe this checkpoint as a complete or fully authentic game port.

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

The combat service adapts selected `src/st/collision.h`, `st_update.h`,
`st_common.h`, `e_collect.h`, `collect_subweapon.h`, and
`src/dra/5D5BC.c:DealDamage` paths under the same AGPL-3.0-or-later license.
All prize, enemy, animation, and explosion tables remain runtime data.

Synthetic tests cover intersection edges, wrapped hitbox arithmetic, elemental
immunity/absorption/resistance/weakness, critical hits, minimum damage, bad inputs,
candle replacement/collection/persistence, target cooldowns, zombie death and
removal, player HP/recovery, and the explicit unsupported continue path.
A local comparison matched 131,072 synthetic damage cases against the unchanged
pinned `DealDamage` with equipment modifiers disabled and an integer-root adapter.
That comparison does not establish full combat, effects, or global RNG parity.

Native checks under LLDB rendered and defeated two zombies. A separate targeted
room-3 placement, with enemy contact suppressed to isolate the pickup path,
broke a candle, rendered its explosion/drop, and collected a heart (10 to 11).
The targeted check is not an unmodified full gameplay replay.

The [native sound service](laboratory-audio.md) now plays selected original
combat sound effects and Laboratory music; reverb and full audio parity remain incomplete.
