# Selected Monster vial 3

Monster vial 3 (item3) is a kind130 consumable running the weapon42 program (`w_042`) in either
hand. Each press throws a vial that breaks on the floor and summons a skeleton archer, which looses
three arrows. Defaults, grants and saves are unchanged; the stock is ordinary inventory.

## Use

Kind130 takes the existing throw path:

- CheckChainLimit (chain eight; the live vial carries item3);
- the blueprint50 factory (E0/F0 in slots17..31, with the hand bit);
- the casting grunt and animation 0x60;
- one consumed vial after the player's successful update (Duplicator keeps it).

## Vial

`EntityWeaponAttack` (role0) is [Monster vial 1](player-monster-vial-1.md)'s vial with its own
tables:

- `D_12A000_8017A6B4` in flight;
- one of four `D_12A000_8017A6F4` speeds, picked by the program cursor `D_12A000_8017B5F0`
  (reduced modulo four, then advanced);
- the `D_12A000_8017A684` break.

The arc, the side and ceiling probes, the floor break (GLASS_BREAK_A, `WFACTORY(56,0)`) and END
retirement are unchanged.

## Archer

`func_ptr_80170004` (role1, slots17..31) copies its raw parent vial's animSet, unk5A, palette and
zPriority-2 at birth. It turns to face the opposite way, plays `D_12A000_8017A604` and grows from
scale 0 by four per update (SCALEX|Y around rotPivotY 0x14, after rising 0x14). At 0x100 it
switches to `D_12A000_8017A620` and calls `SetWeaponProperties` with the raw parent's **current**
equip id. As with Monster vial 1, the vial has retired by then, so the archer and its arrows take
the empty hand's properties. The archer keeps `func_80118894`'s least-used group.

On the last update of poses 6, 0xC and 0x12 it requests `WFACTORY(0x3E, n)`. Its counter n
(`ext.weapon.unk7E`) advances only when a factory was free, so a refused arrow is retried at the
next pose with the same index. There is no sound. At END it switches to `D_12A000_8017A6BC`
silently, requests `WFACTORY(70,0)` once at scale 0x40 and shrinks by four per update once past
pose0, retiring below zero.

Every callback ends with:

- DRA `UpdateAnim` over its private frame-property hitboxes (`D_12A000_8017A6DC`);
- `func_12A000_8017AC08`, which writes colour8 of the hand's first weapon palette (0x039C or
  0x199D, alternating every two game updates). The session commits the write after the pass.

Natively the counter shares the union word that holds other secondaries' velocityY; the archer
never moves vertically, so nothing reads it.

## Arrows and burst

`func_ptr_80170008` (role2, slots17..31) takes its index n from `params & 0x7F00`. It destroys
itself when its raw parent's slot is empty. Otherwise it:

- copies the archer's sprite words at zPriority-2 with ENTITY_ROTATE;
- moves 0x10 up and 8 pixels behind its facing;
- launches at `SetSpeedX(D_12A000_8017A740[n])` and velocityY `D_12A000_8017A74C[n]`;
- plays `D_12A000_8017A6AC`, calls `SetWeaponProperties` and plays ARROW_SHOT_D.

Each update it turns by -0x60, moves and gains 0.15625 velocityY. Its first hit swaps it for DRA
animSet2's spark: palette 0x8170, `D_12A000_8017A704`, unk5A 0, no draw flags and velocityY -0.5.
The spark rises without horizontal motion and retires at END. Its frames carry no hitbox. Unhit
arrows fall until offscreen culling retires them. The arrow animates through `UpdateAnim` with the
archer's hitbox table.

`func_ptr_8017000C` (role3, slots17..47) is Monster vial 1's burst with `D_12A000_8017A684`.

## Graphics

The program's palette table (0x8017A758) uploads twenty-four palettes, with the hand word at
0x8017A75C. Sprites use the standard weapon bank table; the spark uses DRA bank 2.

## Evidence

Public tests cover:

- The vial's speeds and cursor, flight, floor break with its sound and factory, and END
  retirement.
- The archer's parent words, flipped facing, growth, properties at 0x100 and arrow cadence. The
  cases are all factories free, none free, and the first arrow refused (its index is reused).
- The archer's silent END, burst, shrink past pose0, frame-property hitboxes and palette flash.
- The arrows' parent check, indices, placement, launch tables, sound, rotation, gravity, hit
  spark, rise and END retirement, and the burst.
- A shared-runtime throw in both hands: one vial, one archer with empty-hand properties and its
  own group, three indexed arrows (one struck into a spark), one burst and the flashes.
- The player's consumption.

A private MIPS oracle executes all four original callbacks over 7,692 traced callbacks, running
DRA's real `UpdateAnim`. It covers breaks, bounces, ceilings, growth, arrows (including refused
factories), shrinks, bursts, missing parents, hits, sparks and palette flashes. Debug and ASan
traces are byte-identical.

A private native LLDB probe in Laboratory room 3 throws real vials from the equipment menu in
both hands, by actual pad input:

- the vial breaks and the archer grows;
- it looses three drawn arrows, flashes, bursts and shrinks away;
- an enemy positioned on an arrow's path takes damage, and the arrow turns into the drawn spark;
- one vial is consumed.

These fixtures supply items, stock, protection and positions; they are not acquisition, route or
console evidence.

Adapted from `src/weapon/w_042.c` at sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, under
AGPL-3.0-or-later. All original definitions, tables, palettes and sounds remain private.
