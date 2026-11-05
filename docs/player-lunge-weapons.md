# Selected Rapier and Were bane lunges

The actual shared [ordinary hand owner](shared-ordinary-hands.md) now accepts Rapier23 and
Were bane22 through their own weapon38/weapon39 programs, together with their QCF
[weapon specials](player-weapon-specials.md) 191 and 190. Each special spawns a lunge spark
child through the shared [weapon secondary owner](player-holy-sword.md).

Owned selection uses the ordinary inventory menu and either hand. Definitions validate the
actual category0, family38/39, kind16/26, animation0xAE, blueprint48, variant, lock13,
chain0x80, special byte and zero MP. Defaults, grants and v9 saves are unchanged. No new
weapon, effect or primitive pool is introduced.

## Attack callbacks

Both programs run the sword template without blending (descriptors0/1 share frameStart0xAE
and sound0x60B). Descriptor1 requests `WFACTORY(0x38,0)` from the weapon's position on
pose4's final update, after the frame/hitbox update; it never repeats within one pose.

## Kind26 lunge

A completed grounded QCF selects the special (no MP). Kind26 stands still with both
velocities cleared, sets `D_80139824`=40 and plays SFX_VO_ALU_ATTACK_B (0x6EF) instead of the
casting grunt and its RNG. Substep0x5A decrements the count each update, holds pose6 while it
lasts, and resumes the item's ordinary substep0x50 at pose10, on a Down/Up tap or while the
backward direction is held. The player's count starts at the attack, a few updates before the
secondary's own countdown, so either can end the lunge first.

## Lunge secondaries

The next-pass E1/F1 child in slots17..31 retires whenever unk46 clears or the player's
step_s leaves 0x5A. It reserves mandatory LINE_G2 rows (Rapier 24, Were bane 32) before its
first callback; exhaustion retires it before any RNG or property roll. It copies the raw slot16
parent's equip id and enemyId like the Holy sword secondary, sets flags0x860000 (no sprite, no
UpdateAnim), a 24x10 hitbox at (38,-12), rows at player priority+4 with drawMode0x339
(additive, initially hidden), and writes 40 to the program's BSS countdown.

Later callbacks queue SFX_ANIME_SWORD_B (0x640) whenever the countdown&7 is7, decrement it and
retire with SFX_VO_ALU_ATTACK_C (0x6F0) when it reaches zero. Rows follow the player's screen
position (34,-14, three pixels higher with the Y-scale flag):

- Rapier: each row's countdown (initially its index+1) places a 28-pixel streak at a random
  offset (two rand() calls), shows it for three updates, hides it for two and repeats. Birth
  rolls one rand() per row for its red level.
- Were bane: rows appear one at a time every two updates and stay nine updates, chaining from
  the previous row's end. Those ends live in the program's BSS arrays, as in the original.

The per-hand program BSS (countdown and chain arrays) belongs to the resident program: it
survives menus, room changes and repeated specials, resets on a genuine program load or Axe
Armor, and is never saved. Draws skip hidden rows, apply the current shake once and never
tick. Holds, rebases, shoves, death, retirement and failure atomicity follow the shared
secondary owner.

## Evidence

Public tests cover the pose4 factory gate and missing blend; both lunge secondaries' birth
RNG, flags, hitbox, rows and countdown; exact row placement including the Y-scale offset;
Rapier's show/hide cycle; Were bane's chained endpoints; the ANIME_SWORD_B/VO_ALU_ATTACK_C
schedule over 41 callbacks; step_s/unk46/exhaustion retirement without RNG; failure atomicity
of program BSS; and kind26's no-grunt setup, velocity stop, pose6 hold, pose10/Down/backward
resumption. Mutations of each are caught.

The private original-instruction comparison runs 23,408 complete attack callbacks (896
contexts) and 3,968 complete secondary callbacks (128 contexts: both programs, hands, facings,
hit types, cameras, seeded BSS, exhaustion, step_s/unk46 retirement and facing changes), with
the real DRA UpdateUnarmedAnim and SLUS rand, including 107,520 row and 63,488 chain-word
comparisons. Debug and ASan traces are byte-identical; a deliberate timer mutation is caught.
The compiled palette loader matches the original full384-word copies for both hands; both
programs' copies cross the live hand word. Native Debug/ASan protocols exercise each weapon's
ordinary attack, the real QCF lunge to its natural end and a Down-tap cancel in both hands:
the real secondary with its 24/32 shared rows, line pixels, BSS countdown, secondary-only
enemy damage, repeated-draw holds and row release. Supplied stock, protection and position
contexts are not acquisition, route or console evidence.

Adapted AGPL-3.0-or-later algorithms: `weapon/w_038.c`, `weapon/w_039.c`, `weapon/shared.h`,
`dra/6E42C.c` and `dra/71830.c` at sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`.
