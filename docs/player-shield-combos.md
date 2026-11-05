# Selected weapon combos and the Leather/Knight/Alucart/Axe Lord shield spells

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

Only combos with an actual native owner are selected. Currently those are Leather shield's 203,
Knight shield's 204, Axe Lord shield's 206 and Alucart shield's 215, with Mablung (or a Shield
rod, once owned). Other combos keep the ordinary left-hand attack, a documented gap until their
spells exist.

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

Alucart shield's spell (item215, w_058) is the joke one. It has two rows:
1. The shield grows the same way, holds 0x40 updates, plays SFX_TRANSFORM and flashes out
   (glow priority 0x1FC).
2. While the original's unused unk86/unk92 accumulations continue, the page-0x1E question mark
   appears above the player (current facing and position) for 0x28 updates.
3. The freeze ends with no pink effect.

## Axe Lord shield spell (item206) and its walker

Axe Lord shield's spell (w_011) costs twenty MP and takes ten rows. Its birth resets the
program's sixteen band angles to `(i<<9)*2`, so every cast starts from the same phases. Then:
1. The shield grows, holds for eight updates and flashes out over sixteen updates. The flash plays
   MAGIC_WEAPON_APPEAR_A and reveals eight green bands.
2. The bands narrow onto a 16-wide, 64-row card as their wave decays (`unk92` drops by 12 per
   update). The vertical wave (`unk94`) stays zero.
3. The card darkens. As it hides, the spell clears the freeze itself and creates the blueprint88
   walker on the same update.
4. It brightens back to 0x80 and retires four updates later. There is no pink effect.

The walker is the shield program's vector9 (`func_ptr_80170024`) as an E9/F9 child in slots
17..31. Its params lack the hand bit, so CheckChainLimit counts it only for a left-hand combo.
At birth it:
- retires if its raw parent slot (the spell) has emptied;
- copies the spell's bank, texture, palette and facing, and its equip id (206);
- rolls SetWeaponProperties. func_80118894's least-used group stays its enemyId (there is no
  parent copy).

It then falls under 1/8 gravity, capped at 6.0, and lands with STOMP_SOFT_A. It walks at 2.5
(1.25 on matching slopes) with a stomp every ten counts. Its afterimages use FACTORY(90,1): every
fourth g_Timer while falling, every fourth own count while walking. A wall probe, a ceiling probe
eight pixels above it, or a hit plays CANDLE_HIT, its final animation and one last afterimage.
The spell rises about 48 pixels, so a low ceiling stops the walker at once, as in the original.

The spell lacks CAMERA_LOCKED, so it keeps its screen position when the camera moves after the
freeze, as the warp does. The walker and its afterimages are camera-locked.

The afterimages are the same callback with params 0x100, in slots 32..63. Each copies the
walker's current frame, keeps the factory's equipId 0 and fades by two over 0x18 updates. The
walker and its afterimages read their raw parent only at birth. A later, foreign occupant of
that slot is not their parent.

Rows are screen pixels, because the original drawMode lacks DRAW_UNK02. The entity is
screen-fixed, but the camera is static for its whole frozen life.

## Evidence

Public tests cover:
- **Selection:** combo resolution and hand selection (both arrangements), `<<14` factories, ten
  or twenty MP and kind 5, plus refusal by a live spell or too little MP.
- **Spell:** birth, freeze set and clear, growth timing, flash, card sizes and brightening, the
  pink factory, retained angles and row exhaustion, for Leather and Knight. Alucart's hold,
  flash priority, accumulations and question-mark display are covered too. Axe Lord's angle
  reset, phase lengths, band slicing and same-update freeze release plus walker factory are
  covered as well.
- **Walker:** orphan retirement, the raw-parent copy and property roll, falling to the 6.0 cap,
  landing, stomp/afterimage cadences, walls and hits while falling or walking, the final
  animation and the afterimage fade.
- **Runtime:** the spell, E9/F9 walker (spell item, least-used group, params0) and afterimages
  (32..63, equipId0, no roll) through the actual factories. Another hand's Shotel boomerangs take
  the retired walker's slot while its afterimages fade. Camera moves after the freeze keep the
  spell's screen position.
- **Scene freeze:** the scene freeze now accepts request 1.

Twenty-nine targeted mutations are caught. Two more are equivalent: Knight's `i*80/10` slice
equals `i*8`, and its fade steps by 8 from 0x130, so thresholds 4 and 8 trigger together. Of
forty-one Axe Lord mutations, forty are caught. The remaining one changes a vertical-wave
angle that multiplies the always-zero `unk94`, so it is equivalent.

The private original-instruction comparison runs 10,912 complete w_008/w_009/w_058/w_011 spell
callbacks, comparing 95,440 GT4 rows across 80 contexts. These include a second cast that
continues the retained angles, plus w_011's twelve walker factories (it omits the walker without a
free factory). A second comparison runs 2,748 complete w_011 `func_ptr_80170024` callbacks: 1,808
walker and 940 afterimage. It uses the real DRA UpdateAnim as the post-callback (1,768 calls),
4,516 synthetic-world CheckCollision probes and 298 afterimage factories, with origins compared.
Debug and ASan traces are byte-identical.

Native Debug/ASan protocols equip Mablung with Leather, Knight, Alucart or Axe Lord shield
through the menu in both arrangements. They check:
- the actual combo word, ten (twenty) MP and the spell's pixels;
- about 196 frozen frames (88 for Axe Lord), both Blinks and the pink effect;
- release without a Mablung swing;
- Alucart's question-mark pixels;
- Axe Lord's walker in Laboratory room 5. It holds its actual group, hitbox and active 206
  attack, walks 137 pixels with drawn pixels after the freeze, and keeps up to seven afterimages.

Too little MP keeps the left-hand swing. Supplied stock, protection and MP contexts are not
acquisition, route or console evidence.

Adapted AGPL-3.0-or-later algorithms: `weapon/w_008.c`, `weapon/w_009.c`, `weapon/w_011.c`,
`weapon/w_058.c`, `dra/menu.c` (CheckWeaponCombo),
`dra/6E42C.c`, `dra/6DF70.c` and `dra/7879C.c` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`.
