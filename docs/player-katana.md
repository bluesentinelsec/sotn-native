# Selected Katana

The actual shared [ordinary hand owner](shared-ordinary-hands.md) now accepts the two-handed
Katana100 through the weapon53 program, together with its QCF
[weapon special](player-weapon-specials.md) 189. The special spawns the original GT4 slash
chain through the shared [weapon secondary owner](player-holy-sword.md). Katana uses Namakura's
[paired two-hand reservation](player-two-hand-equipment.md).

Osafune139 and Masamune140 share the program, but their QCF specials (193/194, kind135) use
the sword-warp player state with weapon53's own warp entities, and Masamune's ordinary swing
already spawns a slash; Yasutsuna165 is a kind129 thrown item. They stay unavailable until
those owners exist. Muramasa's secondary runs the same slash code, but Muramasa itself waits
for a save-format decision (see [Gurthang](player-gurthang.md)).

Definitions validate category5, family53, kind1, animation0x41, blueprint48, variant, lock13,
chain0x80 and special byte189. Defaults, grants and v9 saves are unchanged.

## Attack callback

Weapon53 runs the sword template with additive blending; descriptors1 and3 use
BLEND_TRANSP|BLEND_QUARTER instead. At birth, before `SetWeaponProperties`, descriptor1
requests `WFACTORY(56,0)` (descriptor3 and4 request roots10 and0x14 for the unavailable
items).

## Slash chain

Each slash child owns one mandatory GT4 row; exhaustion retires it before any RNG. At birth it
captures the player's facing, offsets its factory position by (+-20, -5 or11 when crouched),
sets flags0x08820000 and copies its private row: UV flips, colors, radius and rotation. Roots
(0, 10, 20, 30) and parameters40..49 read properties from their raw parent and copy its
enemyId; others are visual only. Every fourth parameter plays SFX_WEAPON_SWISH_C.

Roots request `WFACTORY(0x38, age+param+1)` every callback, including the last, so a root
spawns nine chained slashes. Each child's raw parent is the root's slot, even when a chained
child is reborn into that vacated slot (it then reads its own cleared words). A slash selects
its CLUT from one of two nine-entry schedules, retires on its ninth callback and otherwise
follows the player at (+-38, -5/11) keeping its own fraction (parameters below0x1E). Its
corners use the SDK quarter-wave table at the rotation plus 0x600/-0x400/-0x800/+0x400,
scaled by the private radius.

The original vertices carry DRAW_UNK02 and move with the camera like the entity, so native
stores them relative to the entity and translates at draw time. Draws use the textured
additive GT4 path, apply shake once and never tick. Holds, rebases, shoves, retirement and
failure atomicity follow the shared secondary owner.

## Evidence

Public tests cover the descriptor blends and slash roots; slash birth, UV/CLUT/color/radius
selection, crouch and facing offsets, root properties and sounds, chained requests, the
nine-callback lifetime, exact SDK corners, table bounds and exhaustion; and the runtime
chain: root raw parent slot16 and group, nine chained children naming slot17 with no
properties, at most eight overlapping, and full row release. Mutations of each are caught.
The synthetic quarter-wave fixture now sits at the SDK table offset, which also made the
Holy sword corner assertions sensitive.

The private original-instruction comparison runs 17,668 complete attack callbacks (1,120
contexts, all five descriptors) and 6,592 complete slash callbacks across all 48 private
parameters, both hands, facings, crouch changes, hit types and exhaustion, with the real DRA
UpdateUnarmedAnim and SLUS rcos/rsin, including 5,824 GT4 comparisons and 540 chained
requests. Debug and ASan traces are byte-identical; a deliberate reposition mutation is
caught. The compiled palette loader matches the original copies for both hands. Native
Debug/ASan protocols equip the two-handed Katana through the menu, perform the ordinary
attack and the real QCF slash in both hands, and check the root and nine chained children,
textured quad pixels, root-only enemy damage, repeated-draw holds and row release. Supplied
stock, protection and position contexts are not acquisition, route or console evidence.

Adapted AGPL-3.0-or-later algorithms: `weapon/w_053.c` (shared with `weapon/w_056.c`),
`weapon/shared.h` and `dra/6E42C.c` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`.
