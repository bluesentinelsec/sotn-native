# Laboratory combat integration

## Empty-hand attack dependency

The default development loadout now uses Gladius in the left hand and an empty right hand.
Use `--laboratory-loadout unarmed` to retain the earlier empty-hand test. Z/X/D map to left-hand attack, jump,
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

The explicit development profile has 70 HP, base stats of 6, no armor/accessories,
10/50 hearts, and Cube of Zoe enabled. Its unarmed physical attack is 6 and
critical rate is zero. Equipped attacks use runtime item attack/element metadata,
the original unbuffed STR contribution, and the selected luck/critical calculation. This enables testing; it is not original stage entry or
a recovered save. Alucard takes ordinary physical contact damage, runs hit
reactions, and has recovery invulnerability. Reaching zero HP runs the selected
[physical death/game-over path](laboratory-game-over.md). Title/file selection
and saves remain unintegrated. Inventory changes
appear in SDL logs and the [original HUD](laboratory-hud.md) displays HP, MP, hearts,
and subweapon state.

Other equipped weapons, subweapon use, inventory menus, experience/level progression,
enemy loot, original blood/glow/hit primitives, all elemental player damage,
full shared random-state consumption, startup, and complete audio parity remain incomplete.
Subweapon swapping retains the previous pickup and its toss/recovery timing;
its sparkle primitive is omitted. Unsupported vessels remain marked unsupported.
Do not describe this checkpoint as a complete or fully authentic game port.

## Equipped weapon checkpoint

Gladius (original hand-item ID 86) and Mace (128) now use their original normal
player animations, lock/recovery rules, seven standing/ledge/crouch/air variants,
weapon hitboxes, multipart sprites, palettes, and swing sounds. Either hand can
carry either item. The second attack button remains an empty-hand attack in the
default loadout. `--laboratory-loadout sword|mace|dual|unarmed` selects explicit
development equipment; `dual` equips Gladius on the left and Mace on the right.
These options do not claim an inventory menu, recovered equipment, or stage entry.
Other hand items fail explicitly instead of borrowing an unrelated weapon behavior.

`BIN/WEAPON0.BIN` is an additional verified private input (1,691,648 bytes,
SHA-1 `b8dcd64f95d197d3f59ab3e46f67897ecdc477cb`). Its overlays are parsed as
bounded data; no PS1 code is executed. Equipment definitions use the PS1 record
stride of 52 bytes, including alignment padding. Tables, pixels, palettes, and
weapon frames stay in the user's files. No generated game arrays are included.

The core adaptation additionally follows `src/weapon/w_000.c`, `w_002.c`,
`shared.h`, `src/dra/alu_anim.c:UpdateUnarmedAnim`,
`5D5BC.c:GetEquipProperties`, and `menu.c:CalcAttack` at the pinned reconstruction,
under AGPL-3.0-or-later. Shared global random-state consumption, complete equipment
modifiers/specials, armor and inventory menus remain incomplete. This development
profile is not complete weapon or combat parity.

Synthetic tests exercise equipment stride and bounds, unsupported IDs, frame
references, both hands, attack power, crouch/air attacks, early direction changes,
held recovery, palette/texture uploads, rendering each hand, and malformed-input
failure. A private native LLDB encounter with Gladius/Mace defeated two zombies,
collected a heart, and took physical damage with music and effects enabled.

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
