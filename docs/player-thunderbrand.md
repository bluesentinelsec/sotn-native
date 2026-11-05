# Selected Thunderbrand

The actual shared [ordinary hand owner](shared-ordinary-hands.md) now accepts Thunderbrand112
through the weapon48 program, together with its QCF [weapon special](player-weapon-specials.md)
178. Both its ordinary swing and its special spawn lightning through the shared
[weapon secondary owner](player-holy-sword.md), which now supports a second callback role
(func_ptr_80170008) and children in slots 32..63.

Firebrand111, Marsil117 and Icebrand113 (specials 179..181, kinds20..22) share the player state
but run their own programs with several more blueprints and callbacks; they stay unavailable.

Definitions validate category1, family48, kind1/19, animation0x41, blueprint48, variant,
lock13, chain0x80, special byte178 and its five MP. Defaults, grants and v9 saves are unchanged.

## Attack callback

Weapon48 runs the sword template with additive blending. Descriptor1's birth requests, in order
and before `SetWeaponProperties`, a bolt root `WFACTORY(0x38,0)`, three branches
`WFACTORY(0x3A,1..3)` and a ring `WFACTORY(0x40,0)`, then clears the program's pose BSS. The
first update the player's pose becomes1 after0 requests a root and three branches again with
parameters 0x10 (ordinary) or 0x20 (special); the BSS then records the pose every update and
survives as program state like the other weapon BSS. A full factory table drops requests.

## Kind19 hold

The special stands still with zero velocity, plays SFX_VO_ALU_ATTACK_B instead of the casting
grunt, charges five MP after the factory and holds pose0 for forty updates; substeps 0x53..0x56
share 71830.c's handler with Holy sword's 0x5C.

## Lightning (role1)

Roots land in slots17..31, branches (blueprint58) and the ring (blueprint64) are reverse-
allocated from slot63. A bolt retires outside animations0x41..0x47 or when unk46 clears. Below
parameter16 it starts at (+-12, -26) and aims 48 pixels up; otherwise it follows the private
per-animation anchor row (whole extent from0x20, halved below), and the Y-scale flag lifts it
one or three pixels (three more below16). It reserves ten mandatory LINE_G2 rows before one
rand() angle per row, fades the row colors along the bolt, copies raw-parent properties and
enemyId, and keeps a 4x20 hitbox (below16), a 32x6 hitbox (0x20) or none (other 0x2x: hit state
cleared). It lives forty (below16) or six updates. Each update the ten segments step toward
the target with SDK quarter-wave jitter of three pixels and the angles advance 768.

## Ring (role2)

The ring follows (+-12, -26, five higher scaled) and reserves sixteen rows as GT4 wedges on page
0x1A with the white-fill palette 0x15F, the original UVs and a blue centre. It plays
SFX_THUNDER_B, grows its radius from4 by4 to24, fades the centre over its last sixteen of forty
updates and has no attack. Its rows carry the E2/F2 primitive tag, which the shared row owner
now recognizes so they are released.

Draws use the additive line and textured GT4 paths, apply shake once and never tick. Holds,
rebases, shoves, retirement and failure atomicity follow the shared secondary owner. Room
entry translates all four vertices of each ring GT4 (not the LINE_G2 row storage they share).

## Evidence

Public tests cover the birth and pose-1 requests and their once-only BSS gate; kind19's hold,
grunt omission and MP; bolt RNG, colors, hitboxes, anchors, halved extents, scale lifts,
lifetimes and gates; ring UVs, radius, fade, sound and lifetime; role/row tables; and the
runtime slot ranges, roles, groups, attack masks and full row release. Mutations of each are
caught, including the missing E2 row tag the runtime test found.

The private original-instruction comparison runs 15,568 attack callbacks (448 contexts) with
9,632 factory requests, 3,552 lightning callbacks over all twelve parameters and 576 ring
callbacks, with the real DRA UpdateUnarmedAnim and SLUS rand/rcos/rsin, including 32,160 row
and 8,768 ring-quad comparisons. Debug and ASan traces are byte-identical; a deliberate lift
mutation is caught. The compiled palette loader matches the original copies for both hands.
Native Debug/ASan protocols perform the ordinary swing and the real special in both hands and
check every bolt's parameters and slot range, the ring, line and ring pixels, secondary-only
damage, that hidden secondaries draw nothing else, repeated-draw holds and row release.
Supplied stock, protection and position contexts are not acquisition, route or console
evidence.

Adapted AGPL-3.0-or-later algorithms: `weapon/w_048.c`, `weapon/shared.h`, `dra/6E42C.c` and
`dra/71830.c` at sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`.
