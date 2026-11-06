# Selected Holbein dagger and Blue knuckles

Holbein dagger (item136) and Blue knuckles (item137) are kind134 weapons running the weapon46
program (`w_046`) in either hand. A press releases a short-lived attack entity instead of a
slot16 swing, and the item is never consumed. Defaults, grants and saves are unchanged.

## Kind134 use

Case134 first calls `func_8010DFF0(1, 0x20)`: it resets the afterimages and holds timer15 at
0x20. It then takes the kind129 throw path:

- the item's factory (blueprint50 for Holbein, blueprint56 for Blue knuckles, both in
  slots17..31, carrying the hand bit);
- the casting grunt;
- the item animation (0xD2).

Each item's chain limit is one: CheckChainLimit counts the live root carrying its item. Holbein's
two extra slashes carry no item and do not count. A refused press resets nothing.

Both roots run `DestroyEntityWeapon(true)` (a raised shield in slot16 retires), set
`g_Player.timers[9]` to four and roll properties through `SetWeaponProperties`. That roll takes
the least-used group.

## Holbein dagger

`EntityWeaponAttack` (E0/F0) copies the player's position and facing and takes one mandatory GT4.
Failure destroys it before properties. Its row of `D_146000_8017AAD8` (selected by params) gives:

- the UV flips;
- a flat colour;
- a radius (`ext.timer.t`);
- a start angle (`rotate`).

The quad uses tpage0x19, priority+2 and drawMode0x335. The entity moves ±20 pixels toward the
facing and -5 (11 when crouched) down; the root (params0) also takes a 15x20 hitbox at offY -5
and plays SWISH_A.

Every callback:

1. The root requests `WFACTORY(0x32,1)` and `WFACTORY(0x32,2)` at unk7E 3 and 6, from its own
   position, and plays `PlaySfxVolPan(SWISH_A, 0x50, 0)` when unk84 reaches 8.
2. The CLUT steps through `D_146000_8017AAB0` (or `AAC4` when the row's flag is set), plus 0x18
   for the right hand. The ninth callback retires the slash.
3. The four corners spin around the **live** player at angles rotate+0x600, then -0x400, -0x800
   and +0x400, at the row's radius, mirrored when facing left.

The 0x32 slashes are the same callback with params1/2: no properties, hitbox or sound. Their raw
parent is the root's slot.

## Blue knuckles

`func_ptr_80170004` (E1/F1) follows the player's position every callback and takes thirty-two
mandatory G4s (untextured) at priority+4, drawMode0x335. Standing or crouched selects the arc:

| | head/tail | end | radius | step | pivot | hitbox |
| --- | --- | --- | --- | --- | --- | --- |
| Standing | -0x500 | 0x700 | 0x13 | 0x200 | (0,-0x10) | offset (8,-0x13), 16x22 |
| Crouched | -0x200 | 0x280 | 0x2E | 0x80 | (-0x20,4) | offset (0xB,2), 8x24 |

It plays SWISH_C, then:

1. The head advances by the step until it reaches the end.
2. The brightness then falls by 0x10 per update while the tail catches up at a quarter step. The
   hitbox clears below 0x80, and the arc retires at zero.

Each row pair spans one sixteenth of the head-tail arc. `D_146000_8017AC10` glow (scaled by
brightness) colours the inner vertices blue, and `D_146000_8017AB90` widens alternate rows. The
whole arc mirrors around the player when facing left. A params-selected red branch with
`D_146000_8017ABD0` and larger hitboxes exists in the original; no item reaches it, but the
component reproduces it.

## Graphics

The program's palette table (0x8017AC50) uploads twenty-four palettes from its first entry, with
the live hand word at 0x8017AC60. The rows are screen pixels rebuilt from the live player every
callback; room entry translates them once.

## Evidence

Public tests cover:

- Every slash row, uv/colour/CLUT/radius/corner geometry and the root's factories and sounds.
- The arc's standing/crouched/red setups, sweep, fade, hitbox clear, glow/shape rows and mirror.
- Shared-runtime throws in both hands, with real rows, the root's properties and group, the
  slashes' raw parent, and retirement.
- The player's kind134 path: afterimage reset, grunt, animation, chain limit and no consumption.

A private MIPS oracle executes both original callbacks over 444 slash and 737 arc traced
callbacks. These cover all thirteen rows, both arc branches, crouching, facing, exhaustion,
shield retirement and the moving player. Debug and ASan traces are byte-identical.

A private native LLDB probe in Laboratory room 3 uses actual pad input and the equipment menu, in
both hands (Blue knuckles also crouched). Each throw draws, hits a positioned enemy, resets the
afterimages, stays equipped and refuses a second press while it lives. These fixtures supply
items, protection and positions; they are not acquisition, route or console evidence.

Adapted from `src/weapon/w_046.c` and `src/dra/6E42C.c` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, under AGPL-3.0-or-later. All original definitions,
tables, palettes and sounds remain private.
