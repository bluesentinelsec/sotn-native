# Selected weapon combos and every shield spell

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

Only combos with an actual native owner are selected. Every shield spell now has one, with
Mablung (or a Shield rod, once owned): Leather shield's 203, Knight shield's 204, Axe Lord
shield's 206, Goddess shield's 209, Shaman shield's 210, Alucard shield's 214 and Alucart
shield's 215. Dual Heaven swords' 202 now has its own owner too: see
[Heaven sword](player-heaven-sword.md#both-buttons-combo202).

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
Enemy contact reaches the walker through collision.h's attacker `hitFlags` (see
[Sword of Dawn](player-sword-of-dawn.md#enemy-contact)), so striking an enemy stops it.

The spell lacks CAMERA_LOCKED, so it keeps its screen position when the camera moves after the
freeze, as the warp does. The walker and its afterimages are camera-locked.

The afterimages are the same callback with params 0x100, in slots 32..63. Each copies the
walker's current frame, keeps the factory's equipId 0 and fades by two over 0x18 updates. The
walker and its afterimages read their raw parent only at birth. A later, foreign occupant of
that slot is not their parent.

## Goddess shield spell (item209) and its clouds

Goddess shield's spell (w_025) costs forty MP and takes seventeen rows. Its fifteen band angles
are program data loaded once and kept across casts. Then:
1. The shield grows, holds for eight updates and flashes out with TRANSFORM_3X, revealing
   fifteen white bands.
2. The card narrows to 32x120 as its wave decays (`unk92` loses one on every eighth and every
   second g_Timer value).
3. The card darkens by ten per update, waits 0x40 updates and releases `FACTORY(55, 15)`.
4. It brightens past 0x120, the angle step drops from 0x80 to 0x20, and the bands spread for
   sixteen updates.
5. The spell fades for 0x40 updates, then clears the freeze and retires.

During phases 2..4, every tenth timer value requests a `FACTORY(90,1)` cloud with the combo's
`<<14` hand bits. That is the shield program's vector9 as an E9/F9 child in slots 32..63, params
0x100. It inherits the spell's 0x10000, so it runs during the freeze. Each cloud:
- takes one mandatory GT4 before six rand calls: its position around the raw parent's words, its
  priority, first timer, spin (never zero) and orbit angle;
- grows, then orbits a point circling 0x20 above the parent. Every fourth update it re-aims with
  SDK `ratan2`/`rsin`/`rcos` at speed 0.5, spinning as it goes;
- fades by six per update once its timer ends or its parent slot empties.

The raw parent is read every aim. An empty slot's zeroed words are screen position 0, and a later
foreign occupant supplies its tracked origin. Neither the spell nor its clouds are
CAMERA_LOCKED, so both keep their screen position when the camera moves. The lab's GT4 draw
commands now pack `owner*32 + row`, because the spell's seventeenth row overflowed the old stride
of 16.

## Shaman shield spell (item210) and its stars

Shaman shield's spell (w_026) costs ten MP and takes fourteen rows. Its twelve band angles, a
green cycle (`D_8017AA98`/`AA9A`) and a glass-shard counter (`D_8017CC3C`) are program words. The
first two are loaded once from overlay data. The sequence:
1. The shield grows, holds and flashes out.
2. The card and twelve vertical bands open. Each band's red, green and blue use angles a third
   of the circle apart.
3. Blueprint92 releases eight stars with GLASS_SHARDS. The sound repeats every 22 counter steps
   through phase 7.
4. The card darkens, waits 0x30 updates and brightens past 0x120 for `FACTORY(55, 6)`.
5. The bands spread (both waves) and fade, then the spell ends the freeze.

Every spell callback bounces the green cycle 0..0x1F. It rewrites the green bits of palette 0x112
(0x12A for the right hand) colour 13, as `(word & 0xFC1F) + (green << 5)` in `g_Clut`, and uploads
the weapon rows. The edit persists until a palette reload. Native code queues the read-modify word
and commits it after the pass.

Each star is the program's vector9 as an E9/F9 child in slots 17..31, with params 0..7 and one
GT4. It rolls a delay of 0..14 and requests its `FACTORY(90,1)` trail. Its orbit:
- It turns around Y by 0x28 per update from `index << 9`.
- `RotMatrix`, `TransMatrix` (0, 0x10, 0x180), `SetGeomScreen(0x280)` and `RotTransPers4`
  place it around its birth point. It shows for 0 < OTZ <= 0x1BE, in front (priority 0x1BE) for
  angles 0x901..0xEFF.
- Its offsets shrink by six to zero after the delay.

When the raw parent spell reaches step 8, the star waits `rand & 15` and flies off until its Y
offset passes -0x80.

Each trail (params 0x100, four GT4s in 32..63) copies its raw parent star's row into row
`(count >> 1) % 4` while the star's X offset is positive, and retires when the star's slot
empties. The star's projection uses PSY-Z-derived SDK matrix and divider math, scoped to this
effect. A foreign occupant of a raw parent slot reads as step 0 with offset 0 here. Stars and
trails run during the freeze and keep their screen position.

## Alucard shield spell (item214) and its armed shield

Alucard shield's spell (w_052) costs twenty MP and takes twenty-three rows:
- the glow;
- eleven letters (overlay UVs, palettes base+0..10, each with a 1..16 countdown from one of the
  eleven birth rand calls);
- eleven untextured G4 ribbons, coloured from the overlay table and drawn as gouraud quads.

The sequence:
1. The letters circle a point 0x1C ahead of and 0x30 above the player. Their radius shrinks from
   0x78 and their angle steps from 0x40. Meanwhile the shield grows, holds 0x40 updates, plays
   TRANSFORM and flashes out.
2. The letters keep circling while the glow fades. Their last centres and priorities stay in the
   program (`D_8017CB88/CBA0/CBB8`).
3. With TELEPORT_BANG_B the letters stretch away and hide. The ribbons between neighbouring
   retained centres open by twelve per update.
4. Past 0xE0 the spell requests BlinkWhite `FACTORY(44, 1)` and sets `D_8017ABBC`. The ribbons fade
   for 0x20 updates.
5. It sets the timer `D_8017ABC0 = 0x1000` and ends the freeze.

Those two program words arm the shield's own `EntityWeaponAttack` (the family-52 slot16 owner).
While armed, every callback:
- sets `D_80097BF8` bit 1, which holds the HUD's MP regeneration for that frame;
- drains one MP on every sixteenth `g_GameTimer`;
- restores the attack it cached at birth.

While the shield is raised (steps 1/2) it also:
- adds hitbox bit 2, so it attacks with that attack instead of only guarding;
- on a hit (actual collision `hitFlags`), calls `func_8010E168(0, 0x60)` (BlinkWhite 0x15 and the
  timer-13 floor), queues kind-1 eight-HP healing unless kind 1 is pending, and adds a heart;
- counts its timer down. When the timer reaches zero, or MP would reach zero, it clears the flag,
  rolls `SetWeaponProperties` again, sets attack 1 and releases `((hand+1)<<14)|0x3A`: the
  E1/F1 letters.

The letters child (`func_ptr_80170004`, eleven GT4s in slots 32..63) makes three rand calls per
letter: a countdown and two velocities (`D_8017CBD0`). The letters burst from 8 above the player,
grow and fall for 0x20 updates, count down, streak upward and hide. WEAPON_SWISH_C plays on every
fourth update from 0x60 to 0x48 of their 0x60-update tail.

Native code commits these effects as follows:
- MP drain and the heart go to the character within the runtime pass.
- The heal and timer-13 floor go through `SotnPlayer_CommitShieldHit` after the pass.
- The regeneration hold is an explicit HUD input.

The original's signed MP can reach -1 while the shield is lowered and still draining; native MP
stops at zero (the shield still expires on its next raised callback). BlinkWhite 0x15 joins the
selected BlinkWhite profiles.

Rows are screen pixels, because the original drawMode lacks DRAW_UNK02. The entity is
screen-fixed, but the camera is static for its whole frozen life.

## Evidence

Public tests cover:
- **Selection:** combo resolution and hand selection (both arrangements), `<<14` factories, ten,
  twenty or forty MP and kind 5, plus refusal by a live spell or too little MP.
- **Spell:** birth, freeze set and clear, growth timing, flash, card sizes and brightening, the
  pink factory, retained angles and row exhaustion, for Leather and Knight. Alucart's hold,
  flash priority, accumulations and question-mark display are covered too. Axe Lord's angle
  reset, phase lengths, band slicing and same-update freeze release plus walker factory are
  covered as well.
- **Walker:** orphan retirement, the raw-parent copy and property roll, falling to the 6.0 cap,
  landing, stomp/afterimage cadences, walls and hits while falling or walking, the final
  animation and the afterimage fade.
- **Goddess:** phase lengths, angle retention and steps, cloud requests, the exact wave decay,
  band seams, card sizes and spread. For the cloud: birth placement and the six rolls (including
  the zero-spin case), growth, aims and corners recomputed from the original constants, its timer,
  and parent-loss fades.
- **Shaman:** the program loads, green bounce and palette events, the GLASS_SHARDS cadence, the
  star release, phase lengths, RGB bands and the spread. For the star: its delay roll, UVs, trail
  request, spin, shrink, release, fly-off and projected bounds. For the trail: row selection,
  copy and hide, and parent-loss retirement. Read-modify palette words are covered too.
- **Alucard:** the spell's rolls, rows, phase lengths, letter orbit, streaks, ribbon geometry and
  fade, BlinkWhite request and arming. The letters' rolls, fall, growth, swishes and streaks.
  The armed shield (hand tests): the hold and drain cadence, attack restore, hitbox bit, hit
  requests, timer and MP expiry, and the lowered-shield exceptions. The runtime armed shield:
  character MP drain, heal, heart, BlinkWhite 0x15 and timer-13 outputs, and the letters through
  retirement. The HUD hold and `SotnPlayer_CommitShieldHit` are covered too.
- **Runtime:** the spell, E9/F9 walker (spell item, least-used group, params0) and afterimages
  (32..63, equipId0, no roll) through the actual factories. Another hand's Shotel boomerangs take
  the retired walker's slot while its afterimages fade. Camera moves after the freeze keep the
  spell's screen position. Goddess's clouds run through the actual factories (one row, flag
  0x10000, no roll or attack). They keep their screen position, and fade once the spell's slot is
  emptied mid-flight. Shaman's eight stars and eight four-row trails run through blueprint92 and
  FACTORY(90,1); trails copy their stars' rows and one palette word is queued per spell
  callback.
- **Scene freeze:** the scene freeze now accepts request 1.

Twenty-nine targeted mutations are caught. Two more are equivalent: Knight's `i*80/10` slice
equals `i*8`, and its fade steps by 8 from 0x130, so thresholds 4 and 8 trigger together. Of
forty-one Axe Lord mutations, forty are caught. The remaining one changes a vertical-wave
angle that multiplies the always-zero `unk94`, so it is equivalent. Of forty-nine Goddess
mutations, forty-five are caught. Two are equivalent: no phase-2 timer value is a multiple of 9,
and the fade reaches 2 either way. One does not compile, because it leaves a variable unused. The
last changes the empty-slot birth origin, which native play cannot reach (clouds are born only
while the spell lives); the private comparison covers it. Of sixty-four Shaman mutations,
sixty-three are caught. The remaining one moves the fly-off bound by one, but the star's Y offset
moves in steps of six from multiples of six, so -127 and -128 are never reached. (The OTZ
visibility shift is also invisible: the star's depth stays 0x180 +/- 216, always inside the test.) Of sixty-one Alucard
mutations, fifty-eight are caught. Two are equivalent: the vertical scale only falls to 160, so its
0x80 floor is never reached, and letter widths stay even, so a gap of 3 never occurs. One does not
compile.

The private original-instruction comparisons run complete original callbacks:
- **Spells:** 19,552 w_008/w_009/w_058/w_011/w_025/w_026 callbacks across 120 contexts, comparing
  229,544 GT4 rows. Contexts include a second cast that continues the retained angles. They also
  cover w_011's twelve walker factories, w_025's eighty cloud factories and w_026's twelve star
  factories (each omitted without a free factory). Shaman's green cycle and glass counter match
  through 4,032 real `LoadImage` uploads.
- **Axe Lord walker:** 2,748 w_011 `func_ptr_80170024` callbacks (1,808 walker, 940 afterimage).
  The real DRA UpdateAnim runs as the post-callback (1,768 calls), with 4,516 synthetic-world
  CheckCollision probes and 298 afterimage factories whose origins are compared.
- **Goddess clouds:** 4,525 w_025 callbacks across 40 contexts, with 1,049 real `ratan2` aims and
  4,485 rows. Contexts include an empty parent at birth and parents retiring mid-flight.
- **Shaman stars:** 2,175 star and 488 trail w_026 callbacks. They run the real SLUS
  `RotMatrix`/`TransMatrix`/`RotTransPers4` code, with a private PSY-Z COP2 register model
  executing 4,302 GTE commands.
- **Alucard:** 3,012 w_052 spell callbacks across 20 contexts and 776 letter callbacks across 8,
  comparing 77,264 rows. The comparison includes the retained centres, arming, BlinkWhite and
  freeze. The armed shield runs inside the hand-weapon comparison: 888 armed callbacks of the
  real `EntityWeaponAttack`, with 124 raised hits through `func_8010E168`/`AddHearts`, heal kind,
  MP drain and hold, and expiry rolls.
- **BlinkWhite 0x15:** compared in the BlinkWhite oracle alongside the other selected profiles.

Debug and ASan traces are byte-identical.

Native Debug/ASan protocols equip Mablung with each of the seven shields through the menu in both
arrangements. They check:
- the actual combo word, ten (twenty, forty) MP and the spell's pixels;
- about 196 frozen frames (88 for Axe Lord, 288 for Goddess), both Blinks and the pink effect;
- up to seven Goddess clouds, which draw pixels and move during the freeze;
- Shaman's eight stars and eight trails, whose projected rows draw and move during its 253 frozen
  updates;
- Alucard's 188 frozen updates with its ribbons, then the armed shield. Held raised with little MP,
  it holds regeneration, drains MP to zero, expires and releases letters that draw and retire;
- release without a Mablung swing;
- Alucart's question-mark pixels;
- Axe Lord's walker in Laboratory room 5. It holds its actual group, hitbox and active 206
  attack, walks 137 pixels with drawn pixels after the freeze, and keeps up to seven afterimages.

Too little MP keeps the left-hand swing. Supplied stock, protection and MP contexts are not
acquisition, route or console evidence.

Adapted AGPL-3.0-or-later algorithms: `weapon/w_008.c`, `weapon/w_009.c`, `weapon/w_011.c`,
`weapon/w_025.c`, `weapon/w_026.c`, `weapon/w_052.c`,
`weapon/w_058.c`, `dra/menu.c` (CheckWeaponCombo),
`dra/6E42C.c`, `dra/6DF70.c` and `dra/7879C.c` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`.
