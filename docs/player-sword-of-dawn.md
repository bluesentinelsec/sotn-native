# Selected Sword of Dawn

The two-handed Sword of Dawn17 runs the weapon30 program through the shared
[ordinary hand owner](shared-ordinary-hands.md) and Namakura's
[paired two-hand reservation](player-two-hand-equipment.md). Its QCF special177 summons a
skeleton squad, hosted by the shared weapon-secondary owner outside slot16. The palette table
and live hand word (D_8017A674/D_8017A67C) load through the existing palette owner. Defaults,
grants and v10 saves are unchanged.

## Swing

Sword of Dawn is kind 6 with animation 0x68. Its `EntityWeaponAttack` is the w_002 template run
from D_8017A5E4. It draws from DRA sprite bank 10 with no per-hand bank offset, uses
BLEND_TRANSP|BLEND_QUARTER, and takes the casting grunt.

## Summoning special

A completed QCF selects special177 when CheckChainLimit allows it (chain 8) and MP covers five.
Sword of Dawn alone also refuses ground that sets vram flag 0x40. Special177 is kind 5, Moon
rod's path: a grounded start stops in substep 0x45 and five MP is paid after the blueprint56
factory.

### Circle (E1/F1, `func_ptr_80170004`)

The circle appears in slots 17..31 and carries 177 from birth.

**Birth.**
1. One rand chooses one soldier, or six when `rand()&7` is zero.
2. Eleven mandatory GT4s follow.
3. A second rand picks the first soldier type from D_8017A618.

**Rise and scan.** The circle rises for eight updates at `SetSpeedX(1.5)` and -5.125, while its
quad widens and grows.
- It then scans fifteen ground points from the player's birth screen position: seven-pixel
  steps forward, then behind, using CheckCollision with the current camera.
- Each soldier takes the first free point in D_8017A608 order. A point is skipped when it
  failed the scan or when a live E2/F2 of the hand stands within three pixels.
- Each placed soldier gets one `WFACTORY2(0x3E, type + (six ? 0x10 : 0))` factory moved to its
  point. Placement stops when the free factory entities run out.
- The circle then plays TELEPORT_BANG_A at volume 0x58.

**Bands and tether.**
- Nine bands shade by cosine phases (D_8017A5F4), which every circle of the program shares and
  resets.
- A LINE_G2 tether reaches toward the last placement.
- `weapon_030.unk84 = 1 + (count-10)/4` releases the soldiers wave by wave.
- The circle then flashes red and retires.

### Soldiers (E2/F2, `func_ptr_80170008`)

Each soldier waits until its raw parent circle's wave equals its type plus one. It then:
- rolls once and takes fourteen GT4s (rise mask and eight smoke puffs);
- shakes the camera (SHAKE_Y_MEDIUM) and plays EXPLODE_A;
- rises, then takes attack 4 (HIT) through `func_80118894`.

By type:

| Types | Behaviour |
|---|---|
| 0, 1 | Walk with CheckCollision, slope speed scaling and wall checks. Leaving the camera bounds blinks them out. Type 1 survives two hits. |
| 2, 3 | Archers: an arrow at pose 5, twice (three times on the odd roll). |
| 4 | Throws two or three homing swords per volley. |
| 5 | Throws the bomb pair at pose 4. |

Hits blink a soldier out after 0x18 updates. Its smoke rows are world pixels, because the
original DRAW_UNK02 screen vertices follow the camera.

### Arrows and swords (E3/F3, `func_ptr_8017000C`)

Arrows fly back at 5.5, rewound five updates. Swords appear around their thrower at
D_8017A66C, indexed by the program's counter D_8017CC44. They spin for `0x30 + 4*index`
updates, then:
- select a target through `func_80118970`, sharing the spirits' marks;
- home through `func_80118B18` with a turn limit that shrinks to the gap and grows by 0x18.

A hit blinks either one out after ten updates.

### Bombs (E4/F4, `func_ptr_80170010`)

The bombs draw from DRA bank 9 and retire when their raw parent slot empties. They grow and spin,
with EXPLODE_FAST_A rising in volume, until the bomber's pose 0x20. At that point:
- index 0 bursts;
- index 1 turns subtractive and rolls at 3.5, scattering DRA `FACTORY(0x24,1)` sparks (Hellfire's
  high1 trail), and shrinks after a hit.

## Enemy contact

collision.h writes each attacker's `hitFlags` on contact (1, 3 with hitbox bit 8, 2 after a
metal clang) unless both entities carry FLAG_UNK_100000. Native combat now delivers that word to
every weapon secondary and to the slot16 hand weapon, not only the fireball and crescents. So
soldiers lose lives and blink out, arrows and swords blink out, and rolling bombs shrink when they
actually strike an enemy. Earlier native builds dealt the damage but never told the child.

## Concessions and gaps

- **Uninitialized point.** The original leaves unscanned points' stack words uninitialized. A
  soldier with no free point reads point 0 (D_8017A608[15] is the zero pad). Native code
  zero-initializes those words.
- **Row timing.** Soldiers' fourteen rows are leased when their factory child is born, not when
  they emerge. Only shared row contention during the wait differs.
- **Spacing scan.** The scan compares weapon-secondary E2/F2 occupants only. While Sword of Dawn
  is equipped, no other E2/F2 producer exists, except a throw still flying from before the equip
  change.

## Evidence

Public unit tests cover:
- **Circle:** one- and six-soldier rolls (including a roll with `&7 == 4`), the rise, scan,
  spacing and factory limits, bands, waves, tether and the flashing exit.
- **Soldiers:** all six types, wave gating, rows, rise, walking and bounds, lives, archer,
  thrower and bomber requests.
- **Arrows and swords:** arrows, swords, homing and target loss.
- **Bombs:** burst, roll and shrink.

Hand use covers the kind5 special, five MP and the 0x40 gate. The plain-weapon test covers family
30's descriptor, bank and blend. Twenty-six targeted mutations are caught. A twenty-seventh, the
circle's width clamp, is equivalent because the rise lasts exactly eight updates.

Private original-instruction comparisons run the following, with Debug and ASan traces
byte-identical:
- 33,785 complete w_030 callbacks over 176 contexts: 3,782 circle, 18,877 soldier, 6,846
  arrow/sword and 4,280 bomb. They compare 206,607 GT4 rows and 12,387 collision queries, and run
  real DRA UpdateAnim (11,903 calls) and func_80118B18 against a synthetic collision world.
- The swing within the shared plain-weapon comparison (14,384 callbacks across eleven programs).
- Both hands' palette copies.

Native Debug/ASan protocols equip Sword of Dawn through the menu and cast with varied character
RNG seeds. They reach the six-soldier roll and all six types, measure pixels for every role and
real enemy damage from soldiers, projectiles and bombs, and check row release in both hands.
Supplied stock, protection, position and RNG-seed contexts are not acquisition, route or console
evidence.

Adapted AGPL-3.0-or-later algorithms: `weapon/w_030.c`, `weapon/shared.h`, `dra/7879C.c` and
`dra/6E42C.c` at sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`.
