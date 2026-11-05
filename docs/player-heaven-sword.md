# Selected Heaven sword

Heaven sword119 is a kind129 sword thrown through blueprint50, like
[Runesword](player-runesword.md). It runs the weapon13 program hosted by the shared weapon-secondary
owner outside slot16. Its palette table and live hand word (D_8017A5FC/D_8017A604) load through
the existing palette owner. Defaults, grants and v9 saves are unchanged.

## Throw

The hand-use path matches Runesword: CheckChainLimit (chain1), `FACTORY(50 | hand,
variant + hand*128)` with equip id119, the casting grunt and animation 0x5D. Its isConsumable byte
is clear, so the hand keeps the sword. CheckChainLimit counts only the root; afterimages never take
an equip id.

## Root

Blueprint50 creates the E0/F0 root in slots17..31. It owns no primitives. At birth it takes the
hand's bank1 frame6 (palette 0x110/0x128, unk5A 0x64/0x66) at the player's animation hitbox, 16
pixels up (8 while the player step is Player_Crouch). Its speed is SetSpeedX(6) plus
PLAYER.velocityX. It runs SetWeaponProperties (least-used group 3..6, own item) and
DestroyEntityWeapon(1), so a raised slot16 shield retires. It takes an 18x4 hitbox, plays
SWISH_C, starts its swish volume at 0x50 and sets player timer10 to4.

After twenty-two updates (DecelerateX(FIX(1)) below seven), it rises backwards: velocityY -1.5
with gravity 1/16 for twenty-four updates, spinning by -11.25° with a 12x12 hitbox. It then
steers toward the player's hitbox at four pixels per update. Each turn is limited to a step that
grows by 0x20 from 0x80, and never overshoots a smaller heading change. It retires inside half of
the combined hitbox sizes. During both phases, every even g_GameTimer requests a
`WFACTORY(0x3A,0)` afterimage. Its childPalette follows the private fourteen-step cycle
(D_8017A5E0), and every eighth timer plays PlaySfxVolPan(SWISH_A) at the fading volume.

## Afterimages

Blueprint0x3A creates E1/F1 afterimages in slots32..63. On their first callback they retire if the
raw parent slot is empty. Otherwise they copy its frame, animSet, unk5A, facing, childPalette and
rotation, add ENTITY_OPACITY to its drawFlags and sit two priorities lower. They fade by eight
from 0x80 and retire below 48. They never take an equip id, properties or hitbox. Native drawing
shades them by entity opacity.

## Both-buttons combo202

Two Heaven swords carry combo main and sub bit1, so CheckWeaponCombo resolves item202 with the left
hand (flag clear). Pressing both attack buttons follows the shared combo path: CheckChainLimit
(chain1) on the live combo children, five MP, `FACTORY(62 + (hand+1)<<14, hand<<7)` with equip id
202, animation 0x66 and kind5's grounded stop. A live sweep or too little MP leaves the left hand's
ordinary throw.

Blueprint62 creates the E2/F2 sweep (func_ptr_80170008) in slots17..31. It lacks CAMERA_LOCKED, so
its position stays in screen space while the camera scrolls. At birth it takes the hand's bank1
frame6, sits two priorities above the player and starts at the player's animation hitbox (16 pixels
up, 8 while crouching). It keeps that point as its screen centre, starts angle -0x200 and radius
0x100, aims its target 0x78 pixels ahead, runs SetWeaponProperties (own item202, least-used group)
and DestroyEntityWeapon(1), takes a 14x14 hitbox, starts its swish volume at 0x60 and sets player
timer10 to4.

Each sweep update spins by 0x100, turns by -0x20 and grows by 0x10, tracing an arc from the centre.
Below -0x400, every even g_GameTimer requests a strike sword through `FACTORY(0x46 + (hand+1)<<14)`.
Its params carry the next D_8017A5E0 palette offset, and the counter advances even when no factory
entity is free. Every sixth g_GameTimer plays PlaySfxVolPan(SWISH_A) at the fading volume. Past
-0xB00 it blinks for eight updates. It then counts to0x34, snapping to its target with an 18x4
hitbox past0x10 and playing SWISH_B every fourth update below0x20. At0x34 it plays
MAGIC_WEAPON_APPEAR_B, switches to PAL_FILL_WHITE with X/Y scaling and allocates one LINE_G2 row.
Allocation failure destroys it. The additive white-pink line grows by 16 pixels each way to ±0x110
screen pixels, then fades. The sprite flattens (scale X +0x80, Y 0x20) and disappears at 0x700. A
0xFE x2 hitbox clamps its offset to the screen edges.

The strike swords (func_ptr_8017000C, E3/F3 in slots17..47) retire whenever their raw parent slot is
empty. At birth they copy the sweep's frame, bank, facing, priority, palette plus their params
offset, drawFlags|BLINK, rotation and target, then take one rand for a 1..32 countdown. That
countdown starts on the update after the sweep reaches step3. At zero they ratan2 toward the target
at 0x100 times the trig step, copy the sweep's equip id and enemyId through SetWeaponProperties and
take an 18x4 hitbox. They retire once past the target. Until launch they carry equip id0, so
CheckChainLimit counts the sweep and launched strikes on the left hand.

The native callbacks take a retained camera (the actual camera less this pass's screen
retention), which matches the pre-scroll camera the original callbacks see. The sweep reserves its
row at its own slot-order turn on the update that reaches0x34.

## Evidence

Public tests cover the root's birth fields and origin, deceleration, rise, gravity, spin,
afterimage/swish schedules, palette cycle, volume fade and clamp, homing turn limit and exact
snap, retirement window and phase bounds; afterimage copies, fade and orphan retirement; the
runtime's root, slots, raw-parent naming, palette range and cadence; and the throw's
non-consumption, grunt and chain limit. All fifteen targeted mutations are caught.

The private original-instruction comparison runs complete w_013 callbacks: 4,134 root callbacks
(1,589 afterimage requests, 437 sounds, eight shield retirements, thirty-six homing retirements)
and 340 afterimage callbacks. Both hands' full palette copies match the compiled loader. Debug and
ASan traces are byte-identical. Native Debug/ASan protocols equip Heaven sword through the menu in
either hand. They throw it, measure sprite and afterimage pixels, damage a target, confirm the
chain-limit block, non-consumption, row release and a rethrow, and throw while the other hand's
shield is raised. Supplied stock, protection and position contexts are not acquisition, route or
console evidence.

Combo202 evidence: public tests cover the both-button selection, packing, MP and fallbacks. They
also cover the sweep's birth, arc, strike schedule (including the exact -0x400 boundary and starved
factories), sounds, blink, target snap, late row, slash growth, fade and both hitbox clamps; the
strike swords' copies, countdown, launch and exact retirement column; and runtime ownership, raw
parents, row leases and screen retention under scrolling. 48 of 49 targeted mutations are caught.
The survivor (ratan2's Y sign) needs the real arctangent table, which only the private comparison
has. That comparison runs complete w_013 func_ptr_80170008/8017000C callbacks: 5,602 sweep callbacks
(912 strike factories, 60 starved, 32 line allocations with four failures, 1,314 line updates) and
1,224 strike callbacks (29 launches, four raw-parent retirements). Debug and ASan traces are
byte-identical. Native Debug/ASan protocols equip two Heaven swords through the menu, cast the combo
and measure the sweep, line and strike pixels. They also check enemy hits, the chain block, the
ordinary throw after the strikes retire and the four-MP fallback. A scrolling room checks retained
screen positions. While the strikes fill slots17..31, an ordinary throw finds no free slot and
spawns nothing, as in the original.

Adapted AGPL-3.0-or-later algorithms: `weapon/w_013.c`, `weapon/shared.h`, `set_speed_x.h`,
`decelerate.h`, `dra/6E42C.c` and `dra/menu.c` (CheckWeaponCombo) at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`.
