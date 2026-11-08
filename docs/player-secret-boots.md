# Selected Secret boots

Secret boots (body item 88, an accessory) make Alucard look taller. Equip them in either
accessory slot; the native inventory now offers them. Defaults, grants and saves are unchanged.

## Player

Each player update, just before the afterimages are sampled, `EntityAlucard` checks the boots
(`dra/692E8.c`). It stretches the body when all of these hold:

- the player is not stuck or held (status `0x10`/`0x40`);
- at least one equipped accessory is item 88;
- the player is not transformed, in a hurt, stone or scripted reaction (status `0x10000`), dead or
  in Axe Armor;
- no rotation or scale flag is set yet this update.

Then it sets `ENTITY_SCALEY`, `scaleY = 0x110` and `rotPivotY = 0x18`. The player's draw flags
reset at the start of every update, but the scale and pivot words stay until `AluInit` at the next
stage start. A later rotation (Gravity Boots, stone, death) writes its own pivot.

## Drawing

`func_800EB758` scales before it rotates: rows 0/1 take the scaled `y0` and rows 2/3 the scaled
`y2`, each `(scaleY * (y - pivot)) >> 8` from the pivot (screen Y plus `rotPivotY`). The scale
reaches every owner that copies the player's draw flags and scale:

- **Body.** The standing, walking or jumping body is drawn through the same quad path as a
  rotated one.
- **BlinkWhite.** The flash copies the flags every update and scales each strip before rotating.
- **Weapons and shields.** Every hand program copies the flags and scale. The sprite stretches,
  and the programs' own `ENTITY_SCALEY` offsets (fist trails, sword sparkles, special anchors)
  now apply.
- **Outlines.** Spell outlines copy the retained `scaleY` at birth.
- **Stopwatch tint.** The [tint](player-stopwatch.md#player-effects) copies the flags with its own
  zero scale, so it draws nothing while the boots are on.

Afterimages, hitboxes and collision are unchanged, as in the original.

## Validation

- **Public tests.** The scale step (floored rows, both pivots), BlinkWhite's stretched strips, the
  player's per-update flag, both accessory slots, the stuck exclusion, and the scale/pivot
  retained after unequipping. They also cover the tint's copied draw flags.
- **Private emulator comparison.** The original `func_800EB758` with `ENTITY_SCALEY` alone and
  with `ENTITY_ROTATE` (actual SDK square root, arctangent and trig) matches 6,000 sampled quads
  (1,500 rotated). A truncating-division mutant fails it.
- **Private LLDB probe.** It equips the boots through the inventory owner in the Laboratory,
  facing both ways:
  - the drawn body changes by 257 to 547 pixels, all at or above the feet;
  - the swinging sword's sprite stretches with its copied flag;
  - the stopwatch tint lives its 125 updates with the copied flag and draws no pixels;
  - unequipping clears the flag and keeps the scale and pivot.

These use explicit item/hearts/protection/position fixtures, not acquisition, route or console
evidence.

Adapted source: `src/dra/692E8.c`, `4B758.c`, `7879C.c` and `7E4BC.c`, and the weapon programs,
at the pinned reconstructed revision `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`,
AGPL-3.0-or-later.
