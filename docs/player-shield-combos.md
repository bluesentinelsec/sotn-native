# Selected weapon combos and the Leather/Knight shield spells

Pressing both attack buttons can cast a combo spell. CheckWeaponCombo (`menu.c`) runs with each
weapon load and records `D_8013AEE4`: the combo item, plus bit 31 when the right hand owns it.
It pairs the left hand's comboSub with the right hand's comboMain first (left hand), then the
reverse (right hand), and takes the first item 0xAA..0xD8 whose comboSub matches. Shield rod4 and
Mablung124 carry main 0x3FFE, each shield one sub bit, and dual Heaven swords pair with each
other. Native code resolves this with `SotnEquipment_Combo` whenever the player's equipment is
set. Defaults, grants and v10 saves are unchanged.

## Both buttons

When both buttons arrive together (a held shield's button counts as pressed), `6E42C.c` checks
the combo before the hand. If it exists, the player is not cramped, the combo's CheckChainLimit
allows another (live children carrying its item with its hand's params) and MP covers it, the
spell is used with the combo's hand. Otherwise the left hand attacks normally.

A combo spell:
- skips the QCF and back-forward selection;
- reserves its factory with the hand bits at `<<14`, the plain DRA factory entity with the
  E0/F0 child offset;
- pays its MP after the factory, as the original's `HasEnoughMp(mpUsage, 1)` does.

Kind 5 stops a grounded player in substep 0x45 with the casting grunt.

Only combos with an actual native owner are selected. Currently those are Leather shield's 203
and Knight shield's 204, with Mablung (or a Shield rod, once owned). Other combos keep the ordinary left-hand attack, a
documented gap until their spells exist.

## Leather shield spell (item203)

Blueprint86 creates the Leather program's E8/F8 `EntityWeaponShieldSpell` in slots 17..31. It
carries 203 from birth.

At birth it:
- takes eleven mandatory GT4s before anything else;
- retires a raised shield (DestroyEntityWeapon(1)) and plays ANIME_SWORD_C;
- sets `g_unkGraphicsStruct.unk28 = 1`.

The engine then enters the spell freeze shared with the spirits. Freeze 1 also creates BlinkWhite
0x47 and 0x40 from the player, flagged 0x10000, before frozen frames begin.

The spell's sequence:
1. The shield image (bank 0x10/0x12, frame 0x3E) grows by 12 per update while drifting back and
   rising, holds, then flashes out with TRANSFORM_3X.
2. A card of nine wave bands opens. Its phases are the program's retained `shieldAngles` (overlay
   data, loaded at first use and kept across casts until a genuine program reload).
3. The card darkens, brightens past 0x120 and releases the `FACTORY(55, 8)` pink medicine effect.
4. The spell clears the freeze and retires.

Knight shield's spell (item204, w_009) is the same program shape. It differs in these ways:
- twelve rows, slicing the card into ten horizontal bands with their own red/green/blue phases;
- a card of 48x40 half extents;
- pink profile 7;
- a fade that lowers brightness by 8 and steps the angle increment by 4 until brightness is
  below 4.

Rows are screen pixels, because the original drawMode lacks DRAW_UNK02. The entity is
screen-fixed, but the camera is static for its whole frozen life.

## Evidence

Public tests cover:
- **Selection:** combo resolution and hand selection (both arrangements), `<<14` factories, ten
  MP and kind 5, plus refusal by a live spell or too little MP.
- **Spell:** birth, freeze set and clear, growth timing, flash, card sizes and brightening, the
  pink factory, retained angles and row exhaustion, for both Leather and Knight.
- **Scene freeze:** the scene freeze now accepts request 1.

Twenty-two targeted mutations are caught. Two more are equivalent: Knight's `i*80/10` slice
equals `i*8`, and its fade steps by 8 from 0x130, so thresholds 4 and 8 trigger together.

The private original-instruction comparison runs 6,360 complete w_008/w_009 spell callbacks,
comparing 72,720 GT4 rows across 40 contexts. These include a second cast that continues the retained
angles. Debug and ASan traces are byte-identical.

Native Debug/ASan protocols equip Mablung with Leather or Knight shield through the menu in both
arrangements. They check:
- the actual combo word, ten MP and the spell's pixels;
- about 196 frozen frames, both Blinks and the pink effect;
- release without a Mablung swing.

Too little MP keeps the left-hand swing. Supplied stock, protection and MP contexts are not
acquisition, route or console evidence.

Adapted AGPL-3.0-or-later algorithms: `weapon/w_008.c`, `weapon/w_009.c`, `dra/menu.c` (CheckWeaponCombo),
`dra/6E42C.c`, `dra/6DF70.c` and `dra/7879C.c` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`.
