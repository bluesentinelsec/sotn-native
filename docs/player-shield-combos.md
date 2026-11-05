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

Only combos with an actual native owner are selected. These shield spells have one, with
Mablung or [Shield rod](player-shield-rod.md): Leather shield's 203, Knight shield's 204, Iron
shield's 205, Axe Lord shield's 206, Herald shield's 207, Dark shield's 208, Goddess shield's
209, Shaman shield's 210, Medusa shield's 211, Skull shield's 212, Fire shield's 213, Alucard
shield's 214 and Alucart shield's 215: every shield spell. Dual Heaven swords' 202 now has its own owner too: see
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

## Iron shield spell (item205) and its swords

Blueprint86 creates the family10 E8/F8 spell with 33 GT4s. Row0 is the shield glow. Rows 1..32
are sixteen band/copy pairs on a disc: each band spans two adjacent 1/16 spokes of the shield
image (UVs around 0xC0 with radius 0x3E, mirrored when facing left), and each copy is an
additive white slice. It retires a raised shield, plays ANIME_SWORD_C, freezes the scene and
drifts back while the shield image grows to full scale. Like the card spells it holds, then
flashes the glow while the image shrinks. MAGIC_WEAPON_APPEAR_B unhides the disc, which opens to
radius 0x3E. Its inner radius then closes, releasing `FACTORY(88,0)` and `FACTORY(88,1)` (each
skipped silently without a free factory entity) and ending the freeze. The copies dim to black,
hold for 0x40 updates, brighten to 0x120, spread (TELEPORT_BANG_B, the bands hide) and fade out.
Each update the copies take their corner colours from the program's retained shieldAngles
(overlay data at 0x8017ABBC), which advance by 0xC0. Copies are forced visible from birth, as
the original's colour pass rewrites their drawMode every update.

The swords (`func_ptr_80170024`, E9/F9 in slots17..31) retire once their raw parent spell's slot
empties. At birth they copy its facing, child palette (plus one and their index) and v band, take
eighteen GT4s (failure retires them) and copy its equip id205 through SetWeaponProperties
(least-used group, no enemyId copy). Row0 is the original FakePrim: a screen-space centre
drifting 2.25 pixels per update toward the facing once a 0x20-update delay ends, which also
makes the sword CAMERA_LOCKED and drops KEEP_ALIVE. The blade (row1) and its sixteen white slices
then circle that centre on a radius opening by 0x18 to 0x280 while spinning, and the second sword
plays WEAPON_SCRAPE_ECHO each half turn. Slice colours follow the program's shieldAngles2 as the
swords fade. Their chain count keeps the combo blocked while either lives; a sword whose orbit
leaves the screen is culled.

## Herald shield spell (item207), its swirl and sprites

Herald shield's spell (w_023, ten MP) takes eighteen GT4s: the glow, the shield image laid on its
side (u runs down the screen, v across it) and sixteen additive white slices of it. Like the card
spells it retires a raised shield, plays ANIME_SWORD_C, freezes the scene, drifts back while the
shield image grows, holds and flashes out with TRANSFORM_3X. The image then opens to 36x64 half
extents while the glow fades; when it is full size the glow hides and the spell requests
`FACTORY(94)` (skipped without a free factory entity). The slices dim to black, and after 0x30
of the next 0x70 updates the spell requests `FACTORY(90)` and then the `FACTORY(55,9)` pink
effect in the same update; `FACTORY(55,10)` follows at 0x70. The slices brighten past 0x130
(the image hides), then wave apart as they fade, and the spell ends the freeze. Slice blues and
waves come from the program's retained `D_8017AB70` angles (s32 overlay data), which advance by
0x80 each update.

The swirl (`func_ptr_80170024`, E9/F9, four GT4s) starts 0x48 below the spell. Its birth rotates
the program's sixteen model vectors by `RotMatrix(0x80,0,0xC0)` with `RotTrans` into overlay BSS
(`D_8017AC90`'s targets; native program words). It rises 7/8 pixel per update, brightens by ten
to 0xFA, holds 0x20 updates and dims below nine, retiring early if it starts the brightening
above screen Y 0x10. Each update it spins (`RotMatrix(0,unk7E,0)`, depth 0x140, H 0x200) and
projects each face with `RotTransPers4` into x0, x1, x3 and x2; a face shows only while its OTZ
is in 1..0x1BE.

`FACTORY(94)` creates twenty-four sprites (`func_ptr_80170028`, E10/F10, slots 63 downward, params
0..23). Each copies its raw parent's palette and unk5A at birth, starts `D_8017AB58` at a random
pose (one rand call) and waits params*4 updates with no animSet; then it copies the parent's
animSet and for 0x40 updates rises 1.5 pixels per update, swaying about its birth screen X in
front of and behind the spell (priorities 0x1B4/0x1BC) as its scale pulses. The params-0 sprite's
signed countdown starts at zero and wraps, so it waits 65536 updates before it can appear: an
original quirk kept natively (it holds a slot meanwhile). The swirl and sprites inherit the
spell's freeze flag through their factories, so they run during the freeze. None of them attacks.

All three keep screen positions (none is CAMERA_LOCKED): native callbacks use the camera before
the pass's retained-screen shift. The pink request now follows a spell's factory requests, which
reproduces Herald's swirl-then-pink order; no other spell requests both in one update.

## Dark shield spell (item208) and its rocks

Dark shield's spell (w_024, thirty MP) writes `g_Player.unk20 = 8` on every callback, including
its last: the transform lockout (native `mist_lockout`, committed after the pass), so Bat/Mist/
Wolf stay unavailable until eight normal updates after it ends. It takes eighteen GT4 rows: two
screen-fixed TILEs (0x90x0xE8 each, drawMode 0x51: subtractive) that darken the screen, the glow,
the shield image on its side with fixed 0x1C/0x38 half extents and fourteen white slices. It
retires a raised shield, plays ANIME_SWORD_C, freezes the scene and grows like the card spells;
its hold ends with ShakeCamera(SHAKE_Y_MEDIUM), and the flash unhides the slices with
WEAPON_APPEAR while the tiles darken (by 2, then 3, below 0xF8). The image opens from half scale.
On the opening's 32nd update the spell requests `FACTORY(96)` (skipped without a free factory
entity), ends the freeze and calls SetWeaponProperties for its own item208 with a 0x7F hitbox:
it strikes for three callbacks (the freeze's release-only frame can add one native pass). At full
scale the original's row pointer has moved on to the image, so its last blue becomes 6. The slices
dim, hold 0x70 updates, brighten past 0x120 (the image hides) and fade as the tiles lift, waving by
their `unk92`, which shrinks to zero while opening and grows while fading. Colours come from the
program's retained `D_8017AD08` angles (fourteen of sixteen used).

`FACTORY(96)` creates up to thirty-two rocks (`func_ptr_80170024`, E9/F9, one GT4, slots 63
downward; the noncritical factory stops at the first occupied slot). A rock retires unless its
raw parent slot holds an entity, copies the spell's v band and child palette plus one, then rolls
its size class, screen X anchor, screen Y below the screen and facing (four rand calls).
Small rocks (size<8) and large ones take their own UVs, extents, shading and priority. It copies
the spell's equip id through SetWeaponProperties (least-used group, no parent enemyId) with a
0x40 hitbox, so every rock strikes. It creeps up at a sixteenth of its speed until the raw parent
passes step5, trembles for a rolled 10..57 updates, plays START_SLAM_A and launches, stretching
while above screen Y 0x80 and retiring above -0x40. Spell and rocks keep screen positions.

## Medusa shield spell (item211), its lasers and hit points

Medusa shield's spell (w_027, thirty MP) takes one GT4 (the glow) and keeps the fixed priority
0x1B6. It retires a raised shield, plays ANIME_SWORD_C, freezes the scene and drifts back while
the shield image grows; at full size it sets FLAG_UNK_100000 and plays `D_8017AC8C` (the head
opening its eyes) for eight updates, then `D_8017ACB4` while the glow spreads by eight and fades.
At glow 160 it hides the glow, requests `FACTORY(98)` (two lasers; skipped without a free
factory entity) and ends the freeze; it holds eighty updates (`D_8017AC8C` again with 64 left) and
shrinks away by twelve. Native animation uses the spell's own timeline word.

Each laser (`func_ptr_80170024`, E9/F9 in 17..31, params0/1) takes eighteen GT4s (failure retires
it), steps 6 or 14 ahead, aims back-up or higher by its index and facing, requests a target with
func_80118970 (the shared spell-target selector, marked as for the spirits) and plays
BIBLE_SCRAPE. Every update it turns toward that target with func_80118B18 (or the screen-edge
point on its facing side when it has none), limited by a step that tightens to the remaining gap
and relaxes by eight to 0x100; it swings on an SDK SquareRoot0 wave whose speed is damped by a
decaying 0x300 counter, moving three units per step of that speed, and plays BIBLE_SCRAPE once
per wave period while on screen. On even g_Timer it requests `FACTORY(100)`. Its ribbon grows to
eighteen segments and then slides, each new segment's width following the swing. It retires far
off screen; a laser homing on a live target can circle it until something releases it.

Each hit point (`func_ptr_80170028`, E10/F10 in 17..47) retires once its raw parent laser's slot
empties. The laser's equip id is its factory argument (zero), so the hit point's
SetWeaponProperties rolls item0's (empty-hand) properties with a 6x6 hitbox and least-used group;
it lives six updates. Hits count, and a sixth would clear the laser's target, but a hit point can
record at most five in its life, so that release never happens in play (the private comparison
exercises it with a seeded count). Lasers and hit points keep screen positions.

## Skull shield spell (item212) and its beam

Skull shield's spell (w_028, forty MP) takes thirteen GT4s: the glow, the skull image, a copy behind
it on palette+2 and ten white slices. It grows, flashes out with WEAPON_APPEAR and opens the image
to scale/8 half extents (its palette alternates on every second g_Timer); then it dims and holds
160 updates. At the hold's 64th update it requests `FACTORY(88)` (skipped without a free factory
entity), ends the freeze and recoils 3.5 back, decelerating by 22/128 and trembling (one pixel on
alternate g_Timer, then on alternate pairs) until the hold ends. It brightens past 0x120, hides the
image and its copy, and slides the slices apart for 24 updates as they fade. Slice colours and
waves come from the program's retained `D_8017AB1C` angles (ten used). Every callback that draws
also runs `func_C8000_8017B3D4`: a program ramp (`D_8017AB18`/`AB1A`) ping-pongs over 0..31 into
colour 11's red bits of palette 0x113/0x12B, uploaded after the pass like Shaman's green cycle.

The beam (`func_ptr_80170024`, E9/F9 in 17..31) retires at birth unless its raw parent slot holds
an entity, takes three GT4s, copies the spell's v band and child palette plus three, and sits 24
ahead of its raw parent's X every update (an empty slot reads screen X 0). A black mask grows by two
to 32, then gives way to the skull and a beam to the screen edge with FIREBALL_SHOT_A. For 0x40
updates it copies the raw parent's equip id and calls SetWeaponProperties every update (least-used
group, re-picked each time) with a hitbox reaching from its position to the screen edge; once the
spell's slot empties that copy is zero, so it rolls item0's properties. Then it shrinks away. The
beam fires level with the spell, so floor-level enemies below a risen spell are missed.

## Fire shield spell (item213) and its dragons

Fire shield's spell (w_029, fifty MP) takes one GT4, the glow. Like the card spells it retires a
raised shield, plays ANIME_SWORD_C, freezes the scene and drifts back while the shield image grows
by 12 (its palette steps `D_8017ACF8` on every second g_Timer, as the guard's does on g_GameTimer).
After eight updates at full size it shrinks by 16 (now on g_GameTimer); at zero it starts the
looping flame `D_8017AD24` (FLAG_UNK_100000, eight three-update frames) at priority 0x1B6 with its
base palette, plays TRANSFORM and grows back by 16 while the glow spreads by eight and fades. After
0x20 updates the glow hides, FIREBALL_SHOT_A plays, `FACTORY(102)` releases the dragons (skipped
without a free factory entity) and the freeze ends; 0x18 updates later the flame shrinks away.

`FACTORY(102)` creates three E9/F9 dragons in slots17..31, six updates apart (params0..2). Each takes
nineteen GT4s (failure retires it), copies the spell's equip id and rolls SetWeaponProperties
(least-used group) with a 20x20 hitbox. Its program-BSS row `D_8017C9A0[dragon]` holds twenty
segments in screen 16.16 words the camera never moves; birth sets each at the dragon's position,
heading 0x540 per dragon (0x800 more facing left), wave/turn 0xC00, speed 0x70, swell 256, widths
20 (10 and 0 for the last two) and wake delays of one plus twice the index. A woken segment's wave
advances 0x80 and its swell shrinks four; it moves by its heading times speed, scaled by half the
wave's sine plus 0x1000, and turns by its signed turn word >> 4. Segments curl tighter (turn -40) until
the wave passes 0x1400, then slow (turn -80, speed -4; dragon0 plays FIREBALL_SHOT_A as each passes
below 0x40), then unwind (wave -0x60, turn -48) and speed off by two; off screen (outside X ±288 or
Y 0..256) their turn stops, and the first to pass speed 0x100 retires the dragon. The head size
ramps from 0x40 by two to 0x100. Rows chain each segment's perpendicular edges, scaled by width and
the swollen wave, with `D_8017AD10` palettes on every second g_Timer; the head row sits on segment1
(sized from segment0) and the entity takes segment1's position before requesting `FACTORY(100)`
every update. Because each dragon alone writes its own row while the chain limit stands, the
native dragon keeps its segments beside its rows.

Each `FACTORY(100)` hit point (E10/F10, slots17..47) retires once its raw parent dragon's slot
empties; it copies the dragon's equip id (213), rolls SetWeaponProperties with a 14x14 hitbox and
lives ten more updates. A dragon therefore trails up to ten live hitboxes. The spell, dragons and
hit points keep screen positions.

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
- **Iron:** the guard's raised-hit thunder and BlinkWhite/protection order (no healing), the
  thunder's birth copies, player-following high words, THUNDER_A and END retirement, slot16 item
  checks; the spell's birth rows, its phase timeline and sounds, every update's spoke geometry
  and retained-angle copy colours, one or two sword factories, and freeze set/clear; the swords'
  birth copies, delay, CAMERA_LOCKED switch, screen retention before it, centre drift, radius,
  spin, scrape sound, slices and shieldAngles2 colours, chain counting, groups and row release.
  59 of 63 Iron mutations are caught: three are equivalent and one (slot16's item7 check) is
  covered by the private comparison. That comparison runs complete `w_010` callbacks: 2,992
  spell, 2,584 sword and 105 thunder callbacks, with 141,728 row comparisons. Debug and ASan traces
  are byte-identical. Native Debug/ASan protocols cast spell205 with Mablung in both hand
  arrangements, measure spell and sword pixels, check the freeze, chain and MP, and deliver a
  supplied hit to a raised Iron shield for the thunder and protection.
- **Herald:** the guard's hitFlags3 rule, cooldown (counted every update, stale blocks granted
  when it ends), table offsets and the runtime heart with its rising heart and HEART_PICKUP (none
  at the maximum; the Alucard shield's hit gains them too). The spell's birth rows, phase
  timeline, sounds, FACTORY(94)/(90) and pink9/10 requests, every update's glow/image/slice
  geometry and retained-angle blues; the swirl's rotated program vectors, rise, timing, top
  retirement and per-face projection; the sprites' birth copies, rand pose, waits (including the
  params-0 wrap), animSet copy, sway, scale and priority. The runtime casts the spell through the
  actual factories: twenty-four sprites with distinct indices, the swirl before pink9, screen
  retention while the camera scrolls, one freeze and release, and the waiting params-0 sprite
  after the spell. 80 of 82 Herald mutations are caught; two are equivalent (the swirl's dimming
  bound, which steps by ten to zero, and an OTZ limit its depths never approach). The private
  comparison runs complete `w_023` callbacks: 3,472 spell, 338 swirl and 646 sprite callbacks
  (with the real DRA UpdateAnim and COP2 model), 63,528 row comparisons; Debug and ASan traces
  are byte-identical. Native Debug/ASan protocols cast spell207 with Mablung in both hand
  arrangements (spell, swirl and sprite pixels, freeze, the waiting sprite), deliver supplied
  blocks to a raised Herald shield (hearts, cooldown, rising heart, none at the maximum) and
  fall back to a swing below ten MP.
- **Dark:** the guard's palette sequences (birth roll before properties, stepping, 0xFFFF
  restart and reroll, knock-away). The spell's lockout every update, birth rows (subtractive
  TILEs, image, slices), phase timeline, shake and sounds, tile darkening and lifting, wave, slice
  geometry and colours from the retained angles, the FACTORY(96) request with the freeze's end
  and the three-callback 0x7F attack, and the image's moved-pointer blue. The rocks' parent and row
  gates, four rolls (every size class), UVs, extents, shading, properties and 0x40 hitbox, creep
  (held through parent step5), tremble, slam, launch, stretch and retirement. The runtime casts the
  spell: thirty-two rocks with groups 7..10 and the spell's v band and palette, the spell's own
  three attack passes, every rock slamming, one freeze, the lockout on every spell pass and full
  release. 86 of 90 Dark mutations are caught; four are equivalent (a slice copy source whose
  copied fields are all rewritten, a hitbox already cleared, a tile cap the tiles never reach and
  a rock-factory parameter the spell never requests). The private comparison runs complete
  `w_024` callbacks: 3,640 spell and 1,851 rock callbacks with 67,067 row comparisons; Debug and
  ASan traces are byte-identical. Native Debug/ASan protocols cast spell208 in both hand
  arrangements (spell, darkness, rock pixels; enemies defeated; lockout each spell pass), watch a
  raised Dark shield cycle fifteen palettes and fall back to a swing below thirty MP.
- **Medusa:** the plain guard's own table. The spell's birth row and fixed priority, phase
  timeline, both animations and their switches, glow growth/spread/fade, the FACTORY(98) request
  with the freeze's end, the hold and the shrink. The laser against a model of the original
  arithmetic: row gate, offsets and aims, rows, the target request and scrape, turning with its
  limit, the SquareRoot0 swing with damping (to zero), movement, the scrape's screen bounds,
  FACTORY(100) on even g_Timer, the growing and sliding ribbon (both width clamps) with its UVs,
  and retirement bounds. The hit point's parent gate, properties, life, hit counting and the
  (seeded) sixth-hit release. The runtime casts the spell: two lasers with item0, hit points with
  item0 properties and least-used groups, one freeze. 70 of 75 Medusa mutations are caught by the
  public tests; of the rest, two runtime target paths are caught by the native protocol, one
  shrink bound is equivalent, the sixth-hit release is unreachable in play and one is a defensive
  foreign-occupant check. The private comparison runs complete `w_027` callbacks: 1,188 spell,
  3,914 laser and 46 hit-point callbacks with 71,412 row comparisons (real DRA UpdateAnim and
  func_80118B18); Debug and ASan traces are byte-identical. Native Debug/ASan protocols cast
  spell211 in both hand arrangements (spell/laser pixels, animations, lasers locking targets, an
  enemy defeated), raise the guarding Medusa shield and fall back to a swing below thirty MP.
- **Skull:** the plain guard's own table. The spell's birth rows (glow, image, palette+2 copy,
  slices), phase timeline, sounds, the FACTORY(88) request with the freeze's end, the recoil's
  speed/deceleration and tremble, image geometry and alternating palette, the image copy's
  visibility, slice colours from the ten retained angles, the final glow blue and the phase-8
  height growth, and the red ramp's palette request each callback. The beam's gates, band and
  palette, the three rows' UVs, the mask growth and FIREBALL_SHOT_A, sixty-four property rolls
  with the hitbox reaching the screen edge, the shrink and X following the raw parent. The
  runtime casts the spell: one beam with the spell's band, item212 and least-used groups, a
  palette word every spell callback, one freeze. 60 of 65 Skull mutations are caught; three are
  equivalent (the always-zero opening wave and an unrequested factory parameter) and two do not
  compile. The private comparison runs complete `w_028` callbacks: 3,580 spell and 692 beam
  callbacks with 48,372 row comparisons and every g_Clut write and upload; Debug and ASan traces
  are byte-identical. It caught a native glow-row pointer slip and the beam's per-update equipId
  re-read (an emptied spell slot rolls item0), both fixed. Native Debug/ASan protocols cast
  spell212 in both hand arrangements (spell/beam pixels, sixty-three attack passes, palette
  writes, freeze), raise the guarding Skull shield and fall back to a swing below forty MP.
- **Fire:** the guard's own table, its palette cycle and every-fifth-update afterimage request
  (none once knocked away). The afterimage's parent retirement, birth copies, fade and palettes.
  The spell's birth, phase timeline, sounds, freeze, palettes on both clocks, flame timeline and
  FACTORY(102). An exact model of the dragon: segments, states, sounds, turn stops, retirement,
  head ramp, chained rows and their palettes, the head row, the entity on segment1 and FACTORY(100)
  every update. The hit point's gate, properties, hitbox and lifetime. The runtime casts the spell:
  three dragons (indices 0..2, item213, least-used groups, 20x20 attacks) that keep their screen
  place across screen shifts, up to 26 live 14x14 hit points, one freeze. 81 of 88 Fire mutations
  are caught; of the rest one does not compile, one is equivalent (the opacity never equals 7), the
  private comparison catches the dragon's truncating half-sine and the native protocol the
  afterimage palette base, and three survive (the far-left turn-stop bound and two permissive
  request/parameter validators). The private comparison runs complete `w_029` callbacks: 92
  afterimage, 956 spell (576 real UpdateAnim), 1,836 dragon and 27 hit-point callbacks with 36,720
  segment and 35,524 row comparisons, plus 64 Fire guard contexts in the hand-weapon comparison;
  Debug and ASan traces are byte-identical. Native Debug/ASan protocols cast spell213 in both hand
  arrangements (spell/dragon pixels, three dragons, 26 live hit points, exactly fifty MP, freeze;
  zombies held on a dragon's hitbox are destroyed, a positioned-contact fixture), raise the guarding
  Fire shield (palette cycle, red-tinted afterimages over all five palettes) and fall back to a
  swing below fifty MP.
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
