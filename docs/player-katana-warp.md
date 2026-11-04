# Selected Osafune and Masamune

The actual shared [ordinary hand owner](shared-ordinary-hands.md) now accepts the two-handed
Osafune katana139 and Masamune140 through the weapon53 program shared with
[Katana](player-katana.md). Osafune's back-forward special192 is a slash root10; Masamune's
ordinary swing already spawns slash root0x14. Their QCF specials 193 and 194 are kind135
warps: the sword-warp player state with weapon53's own slot16 afterimage. Yasutsuna165 (kind129
thrown) stays unavailable.

Definitions validate category5, family53, the special bytes, variants2/3/4 and, for 193/194,
kind135, animation0xD8, lock0 and blueprint60; the definition reader now accepts a per-item
blueprint. Both use Namakura's [paired two-hand reservation](player-two-hand-equipment.md).
Defaults, grants and v9 saves are unchanged.

## Kind135 player start

`dra/6E42C.c` case135 plays SFX_VO_ALU_ATTACK_C, sets timer9 and calls func_8010ED54: both
velocities stop, the player enters Player_SwordWarp with animation0xD8 and requests the
profile20 outline; no casting grunt, attack mode or block_98 animation is set. The existing
sword-warp step then waits for slot16 to empty, requests the profile21 outline and recovers
after sixteen updates, exactly as for the Alucard sword.

## Warp afterimage

Blueprint60 replaces slot16 with the E2/F2 warp, whose equip id is the factory argument
(193/194). It redraws Alucard's current frame with the Alucard palette, transparent except
while it holds. It leaves at speed8 and half a pixel upward, decelerating by 15/32 each update;
Masamune's variant requests slash root0x28 at birth. Pose6 requests root0x1E and holds it
opaque, pose8's final update requests another root0x1E, pose10 sends it back the same way, and
pose15 retires it. It copies the player's draw flags, Y scale and pivot. Lacking
CAMERA_LOCKED, it keeps its screen position after camera work, like Neutron Bomb.

Its slash roots keep their birth offset and copy the warp's equip id and enemyId0, so they use
the special's properties on the zero attack channel. Root0x28's chained children (41..49) also
read properties; rows48 and49 lie past the 48-row private table, and row48 overlaps the
program's live g_HandId word, which reads as the resident hand. Their garbage radii produce
vertices far off screen; such original-data quads now follow the GPU's signed 11-bit vertex
wrap and drop any triangle spanning more than 1023x511 pixels, instead of failing. Other quad
paths keep their native fail-fast bound.

## Evidence

Public tests cover the warp phases, motion, blends, pose gates and slash requests; kind135's
blueprint60 reservation, voice, grunt omission, sword-warp step, animation and outline;
Osafune's back-forward slash; the runtime warp in slot16 and its root copying item193 and
group0 from that raw parent; rows48/49 reading the live hand word; and the opt-in GPU wrap and
span rule. Mutations of each are caught.

The private original-instruction comparison runs 2,680 complete warp callbacks (48 contexts)
with the program's own SetSpeedX/DecelerateX, and the katana slash comparison now covers all 50
parameters, including rows48/49 with the live hand word (6,880 slash callbacks). Debug and ASan
traces are byte-identical; a deliberate deceleration mutation is caught. Native Debug/ASan
protocols equip both two-handed swords through the menu and run Osafune's ordinary swing and
back-forward slash, Masamune's ordinary slash, and both real QCF warps in both hands: the
sword-warp step and recovery, the afterimage pixels, roots0x1E (and0x28), root damage and row
release. Supplied stock, protection and position contexts are not acquisition, route or
console evidence.

Adapted AGPL-3.0-or-later algorithms: `weapon/w_053.c`, `weapon/shared.h`, `decelerate.h`,
`set_speed_x.h`, `dra/6E42C.c` and `dra/71830.c` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`.
