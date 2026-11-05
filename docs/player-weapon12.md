# Selected Chakram, Vorpal blade and Crissaegrim

Chakram133, Vorpal blade163 and Crissaegrim164 run the weapon12 program, hosted by the shared
weapon-secondary owner outside slot16. Their palette table and live hand word
(D_8017AAB0/D_8017AAC8) load through the existing palette owner. Defaults, grants and v10 saves
are unchanged.

## Use

All three take the thrown-item path in `dra/6E42C.c`: CheckChainLimit, the factory, the casting
grunt and the throw animation. Their isConsumable bytes are clear, so the hand keeps them.
- Chakram is kind130 with blueprint56 (chain 2, animation 0x60).
- Vorpal blade is kind129 with blueprint50 and descriptor1 (chain 3).
- Crissaegrim is kind129 with blueprint52 and descriptor2 (chain 3). It releases four roots one
  every four updates, each with its index in the low parameter byte.

CheckChainLimit counts the roots and the Chakram, which carry their item from birth; afterimages
never take one.

## Slashes

`EntityWeaponAttack` reserves one mandatory GT4. It then copies the player's position and
facing and starts the slash from four corner offsets. While the player is crouched outside crouch
substep2, the slash drops 0x18 pixels.

The program's own counter (`D_8017BB28`, kept per hand in the program's state) chooses one of
three shapes, `count % 3`, each with its own spread steps. Descriptors 1/2 slide: SetSpeedX(4),
starting four updates back. Vorpal decelerates by 0x3000 and Crissaegrim by 0x1000. Crissaegrim
also shades its GT4 at 0x80 and widens its leading edge by 7 per update.

Each slash then:
- runs SetWeaponProperties (least-used group 3..6, own item);
- takes the hitbox (0x2B, drop-6, 0x1E x 0x1C-drop/2);
- runs DestroyEntityWeapon(true), which retires a raised slot16 shield;
- plays SWISH_B and sets player timer9 (hand-weapon startup) to 4.

The CLUT starts at 0x110 (0x128 for the right hand) and steps once per update. The slash retires
when it reaches the tenth fade palette. Corners are offsets from the entity's whole pixels
(DRAW_UNK02 follows the camera).

## Chakram and afterimages

The Chakram starts at the player's hitbox, 16 pixels up (8 while the player step is crouch). Its
horizontal speed is `((rand()&0x3FFF)*2)|FIX(4.5)` plus PLAYER.velocityX. Its vertical speed is
`(rand()&0x7FFF)*2+FIX(2)`. It accelerates by ∓FIX(0.15625) horizontally and by a vertical
acceleration that starts at -FIX(0.125) and drops by FIX(7/1024) each update. It runs
SetWeaponProperties, DestroyEntityWeapon(true) and timer10=4.

After twenty-eight updates (frames 3→4→5) it takes its ratan2 heading. It then steers toward the
player's hitbox at four pixels per update, with a turn limit growing by 0x10 from 0x80. It faces
right, rotates to its heading, and retires inside half of the summed hitboxes. Every eighth
update plays PlaySfxVolPan(SWISH_A). The volume starts at 0x60 and fades by 0xC during homing.
Every fourth g_GameTimer requests a `WFACTORY(0x40,0)` afterimage. Afterimages match
[Heaven sword's](player-heaven-sword.md) but copy the Chakram's own palette (PAL_ALUCARD).

## Evidence

Public tests cover all three slash shapes, sliding, deceleration and widening. They also cover
the crouch drop and its substep exception, hitbox, palette fade and retirement, row exhaustion
before properties, and the program counter. For the Chakram they cover its launch rolls,
accelerations, frames, turn-limit growth, swish fade and afterimage schedule. The runtime tests
check Crissaegrim's four staggered roots and timer9, and the Chakram's afterimages naming it with
its palette. Hand use (packed factories, grunt, non-consumption, chain limits) is covered too.
Sixteen targeted mutations are caught. A seventeenth, the speed mask 0x3FFF versus 0x7FFF, is
equivalent because the OR with FIX(4.5) already sets bit 15.

The private original-instruction comparison runs complete w_012 callbacks. It covers 1,344 slash
callbacks (1,200 GT4 comparisons, thirty-two shield retirements), 3,885 Chakram callbacks and
340 afterimage callbacks, plus both hands' palette copies. Debug and ASan traces are
byte-identical. Native Debug/ASan protocols equip each item through the menu in either hand. They
measure slash, Chakram and afterimage pixels and see Crissaegrim's three overlapping slashes and
four timer9 starts. They check real enemy damage, non-consumption and row release. Supplied
stock, protection and position contexts are not acquisition, route or console evidence.

Adapted AGPL-3.0-or-later algorithms: `weapon/w_012.c`, `weapon/shared.h`, `set_speed_x.h`,
`decelerate.h` and `dra/6E42C.c` at sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`.
