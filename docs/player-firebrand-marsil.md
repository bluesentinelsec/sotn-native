# Selected Firebrand and Marsil

The actual shared [ordinary hand owner](shared-ordinary-hands.md) now accepts Firebrand111 and
Marsil117 through the weapon49 program, with their QCF specials179 (kind20) and180 (kind21).
Like [Thunderbrand](player-thunderbrand.md)'s special, both hold pose0 for forty updates, cost
five MP after the factory and play no casting grunt. [Icebrand](player-icebrand.md) uses its
own weapon50 program.
Defaults, grants and v9 saves are unchanged.

Definitions validate category1, family49, the special bytes, variants0..3, kinds20/21 and five
MP for the specials. The program's palette table and live hand word at D_8017B144/D_8017B148
load through the existing palette owner. Its SetSpriteBank2 frames (D_8017AA44) now serve the
puffs and pillars.

## Attack callback

The four descriptors (Firebrand0, its special1, Marsil2, its special3) draw additively from the
program's bank0x10. At birth, descriptor1 requests the WFACTORY(0x5B,0) sparks, and
descriptor3 requests the WFACTORY(0x5F,0) swirls and the WFACTORY(0x40,1) ring, all before
SetWeaponProperties. The first update with pose1 after pose0 (the program's D_8017C8CC word,
shared with weapon48's layout) requests WFACTORY(0x5D) puffs with SFX_FM_EXPLODE_B:
descriptors1/3 with param0, descriptor2 with param1. Descriptor0 requests nothing.

## Children

- **Sparks** (E1/F1, eight in slots17..31). They retire outside animations0x41..0x47 or
  without an attack mode. Each copies its raw parent's attack, element, hit state, stun,
  invincibility and hit effect without a roll. Its group comes from func_80118894 alone, so
  sparks keep their own least-used group instead of the parent's. They orbit the blade (±12,
  -26, or -31 when Y-scaled) at radius64 shrinking by three, from params*512 turning by 0x20.
  Their rotation follows the ratan2 heading of their screen motion. They draw DRA animSet9's
  eight-frame loop rotated and retire once lifetime0x28 falls below0x11. Spark0 requests the
  WFACTORY(0x40,0) ring at lifetime0x18.
- **Ring** (E2/F2, slots32..63). Sixteen mandatory LINE_G2 rows retyped as GT4 wedges
  (white-fill palette, page0x1A, UVs from the SDK circle, additive drawMode0x335) surround the
  blade. Their radius grows from four by six to forty. Red/green fade over the last eight of 25
  updates (forty for param1). Room entry translates every vertex.
- **Puffs** (E3/F3, one per update, eight in slots17..31). Each sits at a private blade anchor
  (D_8017B03C, per player animation) plus a fraction of its extent: three quarters, or two
  thirds for param1, times the child index over eight. Six libc calls place and vary it
  (Y+0..15, X+0..3, palette0x117+0..7, scale0x80..0xBF, priority+0/4, quarter blend on 2/3),
  before SetWeaponProperties copies the raw parent. A puff rises half a pixel per update until
  its thirteen-frame animation ends.
- **Swirls** (E4/F4, eight after four updates in slots17..47). Invisible, they roll properties
  with no hitbox and spiral outward. Past radius0x40 they turn CAMERA_LOCKED, and they retire
  at0x70. Each requests the WFACTORY(0x65,0) pillar whenever its counter (rand%5 at birth) is
  a multiple of seven.
- **Pillars** (E5/F5). SFX_FM_EXPLODE_B, a 12x12 box and the swirl's equip id and enemyId; they
  rise 0.375 per update until their animation ends.

## Evidence

Public tests cover the attack factories, puff parameters, sounds and pose word; the sparks'
copied attack, group, orbit and ring request; the ring's wedges, growth, fade and lifetimes;
the puffs' RNG order, anchors and rise; the swirls' phases and exact pillar schedule; the
pillars; the runtime child counts, roles, rows, the sparks' own groups and attack, the pillars'
raw parents and row release; and Firebrand's kind20 QCF. Mutations of each are caught.

The private original-instruction comparison runs complete w_049 callbacks: 31,136 attack
callbacks and 13,576 child callbacks across all five roles, with real DRA UpdateAnim and the
program's ratan2/rcos/rsin. Its 32 full palette copies match the compiled loader. Debug and ASan
traces are byte-identical. Native Debug/ASan protocols equip both brands through the menu in
either hand and run ordinary swings (Marsil's puffs), both real QCF specials, every visible
child's pixels, secondary-only damage and row release. Supplied stock, protection and position
contexts are not acquisition, route or console evidence.

Adapted AGPL-3.0-or-later algorithms: `weapon/w_049.c`, `weapon/shared.h`, `dra/6E42C.c`,
`dra/7879C.c` and `dra/71830.c` at sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`.
