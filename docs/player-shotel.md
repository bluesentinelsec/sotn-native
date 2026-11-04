# Selected Shotel

Shotel28 runs the weapon34 program in either hand through the shared
[ordinary hand owner](shared-ordinary-hands.md). Its QCF special176 throws it as a boomerang,
hosted by the shared weapon-secondary owner outside slot16. The palette table and live hand word
(D_8017A944/D_8017A94C) load through the existing palette owner. Defaults, grants and v10 saves
are unchanged.

## Swing

Shotel is kind 12 with animation 0x92. Its `EntityWeaponAttack` is the w_002 template, run from
its own descriptor (D_8017A934, frameStart 0x92, SWISH_B on pose 4), with BLEND_TRANSP only.
The swing takes the casting grunt and the ordinary block_98 states.

## Thrown special

A completed grounded QCF selects the special176 when CheckChainLimit allows it (chain 1, counted
per hand from live children carrying 176). It is kind129, so it takes the thrown-item path:
- the blueprint56 factory (`56 | (hand+1)<<12`, parameter +128 for the right hand);
- the casting grunt, then the throw animation 0x5D, set only from Player_Stand or a crouch, as in
  the original case129;
- no MP. Shotel stays in hand because its isConsumable byte is clear.

The E1/F1 boomerang (`func_ptr_80170004`) appears in slots 17..31 and carries 176 from birth.
At birth it:
- starts at the player's hitbox, 16 pixels up (8 while the player step is crouch);
- moves at `SetSpeedX(FIX(4.5))` plus the player's speed, and FIX(-0.25) vertically;
- runs SetWeaponProperties (least-used group 3..6), DestroyEntityWeapon(true) and timer10=4,
  with a 9x9 hitbox.

It then decelerates by ∓FIX(0.125) horizontally and gains FIX(1/64) vertically for 0x1C updates,
spinning ROT(45) every update. Next it takes its ratan2 heading and homes on the player's hitbox
at four pixels per update. The turn limit starts at 0x200 and grows by 0x10 per update. The
boomerang retires inside half of the summed hitboxes. Every eighth g_GameTimer plays SWISH_B,
except on the retiring update.

## Punch while thrown

Case 12 checks CheckChainLimit for the special with both hands' params. While a 176 lives in
either hand, a Shotel swing (or a QCF refused by the chain limit) becomes a punch:
- Shotel's own factory is already reserved;
- the empty-hand factory 42+hand follows, and its result is ignored;
- item 0 supplies the animation, kind and lock.

The punch child replaces Shotel's in slot16 before Shotel's runs. Native player state takes the
same-hand count (`SotnPlayer_SetSpecialChain`) and the other hand's count
(`SotnPlayer_SetSpecialCross`) from the session's live children.

## Evidence

Public unit tests cover the boomerang's launch, accelerations, spin, flight length, heading,
turn-limit growth, retirement and sound schedule in both hands. Runtime tests cover the
blueprint56 E1/F1 child with its own item, group and row-free release in both hands. Hand-use
tests cover:
- the QCF throw: packed factory, grunt, non-consumption and no MP;
- the chain refusal falling back to a punch;
- the punch from the other hand's throw;
- the ordinary swing.

The plain-weapon test covers family 34's descriptor and blend. Twenty targeted mutations are
caught.

Private original-instruction comparisons cover:
- 3,846 complete w_034 boomerang callbacks, including 36 homing retirements and 8 shield
  retirements;
- the swing within the shared plain-weapon comparison (13,936 callbacks across ten programs);
- both hands' palette copies.

Debug and ASan traces are byte-identical.

Native Debug/ASan protocols equip Shotel through the menu in either hand. They measure swing and
boomerang pixels, real damage from both, and the boomerang's return and retirement. They also
check the punch while it lives (Shotel's slot16 child never appears) and a Shotel swing after
its return. Supplied stock, protection and position contexts are not acquisition, route or
console evidence.

Adapted AGPL-3.0-or-later algorithms: `weapon/w_034.c`, `weapon/shared.h` and `dra/6E42C.c` at
sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`.
