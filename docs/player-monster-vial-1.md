# Selected Monster vial 1

Monster vial 1 (item1) is a kind130 consumable running the weapon40 program (`w_040`) in either
hand. Each press throws a vial that breaks on the floor and summons a small fire-breathing monster.
Defaults, grants and saves are unchanged; the stock is ordinary inventory.

## Use

Kind130 takes the existing throw path:

- CheckChainLimit (chain eight; the live vial carries item1);
- the blueprint50 factory (E0/F0 in slots17..31, with the hand bit);
- the casting grunt and animation 0x60;
- one consumed vial after the player's successful update (Duplicator keeps it).

## Vial

`EntityWeaponAttack` (role0) at birth:

- `SetSpriteBank1`, then the hand's animSet (0x8010/0x8012), palette (0x110/0x128) and unk5A
  (0x64/0x66);
- zPriority+2, the player's facing, CAMERA_LOCKED|0x100000 and `anim_4`;
- four pixels up;
- a speed from `D_11C000_8017A85C[cursor % 4]`, where the cursor is the program's
  `D_11C000_8017B540` and advances per throw;
- velocityY -2.5 and `g_Player.timers[10]` = 4.

It falls with 0.15625 gravity. A solid point under it breaks the vial: top correction,
`anim_3`, BLEND_TRANSP|ADD, `PlaySfxVolPan(GLASS_BREAK_A, 0x50, 0)` and `WFACTORY(56,0)`. Before
that, a side probe (x±4) reverses it at half speed, and a ceiling probe (y-8) sets velocityY 1.0
and halves X. The broken vial retires at its animation's END.

## Monster

`func_ptr_80170004` (role1, slots17..31) copies its raw parent vial's animSet, unk5A, palette and
zPriority-2 at birth. It turns to face the opposite way, plays `anim_0`, and grows from scale 0
by four per update (SCALEX|Y around rotPivotY 0x16, after rising 0x16). At 0x100 it switches to
`anim_1` and calls `SetWeaponProperties`.

That call copies the raw parent's **current** equip id. The vial has retired long before, so the
slot is empty (equip id 0): the monster and its fireballs take the empty hand's properties, as
in the original. The monster keeps `func_80118894`'s least-used group (no parent copy).

While attacking, it requests `WFACTORY(62,0)` on the last update of poses 0xB, 0xF and 0x13.
EXPLODE_B plays only when a factory was free. At END it switches to `anim_5`, cries out
(VO_ALU_PAIN_A), requests `WFACTORY(70,0)` once at scale 0x40, and shrinks by four per update
from pose5 until below zero.

Every callback ends with:

- DRA `UpdateAnim` over its private frame-property hitboxes (`D_11C000_8017A844`);
- `func_11C000_8017AC14`, which writes colour10 of the hand's first weapon palette (0x039C or
  0x199D, alternating every two game updates). The session commits the write after the pass.

## Fireballs and burst

`func_ptr_80170008` (role2, slots17..31) destroys itself when its raw parent's slot is empty.
Otherwise it:

- copies the monster's sprite words at zPriority-2;
- moves to ±0x18 ahead and 0xA up;
- flies at 2.5 (`SetSpeedX(-2.5)` against its facing);
- plays `anim_2` and calls `SetWeaponProperties` (its fire element is overwritten there);
- retires on its first hit.

It animates through `UpdateAnim` with the same hitboxes. Offscreen culling retires unhit ones.

`func_ptr_8017000C` (role3, slots17..47) copies the monster's sprite words at zPriority+2, plays
the additive `anim_3` 0x10 lower and retires at END.

## Graphics

The program's palette table (0x8017A86C) uploads twenty-four palettes, with the hand word at
0x8017A870. Sprites use the standard weapon bank table.

## Evidence

Public tests cover:

- The vial's speeds and cursor, gravity, bounce, ceiling and floor break with its sound and
  factory, and END retirement.
- The monster's parent words, flipped facing, growth, properties at 0x100 and fireball cadence
  (with and without free factories).
- The pain cry, burst, shrink, frame-property hitboxes and palette flash.
- The fireballs' parent check, placement, motion and hit retirement, and the burst.
- A shared-runtime throw in both hands: one vial, one monster with empty-hand properties and its
  own group, three fireballs, one burst and the flashes.
- The player's consumption.

A private MIPS oracle executes all four original callbacks over 6,860 traced callbacks, running
DRA's real `UpdateAnim`: breaks, bounces, ceilings, growth, fireballs (including refused
factories), pains, bursts, missing parents, hits and palette flashes. Debug and ASan traces are
byte-identical.

A private native LLDB probe in Laboratory room 3 throws real vials from the equipment menu in
both hands, by actual pad input:

- the vial breaks and the monster grows;
- it breathes three drawn fireballs, flashes, bursts and shrinks away;
- a positioned enemy takes damage;
- one vial is consumed.

These fixtures supply items, stock, protection and positions; they are not acquisition, route or
console evidence.

Adapted from `src/weapon/w_040.c` at sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, under
AGPL-3.0-or-later. All original definitions, tables, palettes and sounds remain private.
