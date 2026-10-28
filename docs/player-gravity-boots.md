# Selected Gravity Boots service

An owned, enabled Gravity Boots relic now accepts the original Down → Up → Jump
command. Each command interval lasts sixteen simulation ticks. Keyboard arrows
plus X/Space and Xbox d-pad plus A use the existing bindings. The original gate
requires more than four MP, then spends four; four MP alone does not launch.
The player must be crouching or in the normal Jump state after using Leap Stone.
An attack lock or tight space prevents launch. Development starts grant no relics.

The selected launch uses original animation 33, vertical speed -12 pixels/tick
and directional speed three pixels/tick. Original launch/deceleration, low-ceiling
recovery, inverted ceiling pose, total-duration recovery and return to normal
jumping run. The late-ceiling branch retains its original unconditional facing
flip. Damage, petrification, boss grabs and
script holds restore facing when they interrupt an inverted pose.

High jumps and other movement at least two pixels/tick horizontally now use the
original eight collision passes. Signed division preserves arithmetic-shift
rounding, frame contact history and the original negative-Y clamp. This also
changes the existing dive-kick and knockback paths. Normal slower movement retains
one collision pass.

Four owned cosmetic beam slots retain original anchored position/fractions,
four narrowing Gouraud quads, additive color, priority and fade rules. Room
changes rebase their anchors. Pool exhaustion omits the optional visual and
preserves movement. The inverted sprite uses an integer 180-degree rotation
about the original Y pivot two. [Selected ceiling dust and camera shake](player-gravity-impact.md)
now run through actual shared owners. Dithering, GPU edge rasterization and complete
shared factory-capacity contention remain incomplete.

The player owns an MP snapshot and reports the current tick's approved cost.
The session spends it once from ordinary character progress after a successful
step, before room/stage transfer. Failed animation data rolls back the player
step without spending character MP. Existing recovery and save-room healing
remain in effect. Version-nine saves retain MP and relics; they store no command
cursor, jump pose or beam state. No save format or application version changes.

## Evidence and remaining scope

All 110 Debug, Release and macOS ASan/UBSan test groups and asset-free packaging
pass. Authored fixtures cover ownership, the MP threshold/cost, command expiry,
held-button restrictions, airborne eligibility, tight spaces, atomic failure,
low/early/late ceilings, interruption, room rebasing and mirrored/rotated pixels.

Private instruction comparisons match 32,256 command/MP/launch snapshots and
15,840 high-jump/recovery snapshots, including signed timer wrap and the late
facing flip. Another 360 beam ticks and 476 quads match original state, anchored
fractions, fade and vertex/color/priority/draw-mode equations. Adapters isolate
relic lookup, optional factories, destruction, sound and camera shake as noted
by each check. These do not establish complete player/frame/GPU/console parity.

Muted Debug and macOS ASan/UBSan LLDB runs of this repository's CMake executable
verify the command, MP cost, low-ceiling recovery and normal SDL quit. A debugger
fixture supplies the relic; the ordinary development character remains unchanged.

A separate muted native integration route starts explicitly in NO2 room 15 and
grants the two movement relics plus contact protection. Fifteen boosts and fifteen
ordinary Gladius hits break the 128-HP secret ceiling over 2,705 ticks and 677
draws. MP spending/recovery, the inverted pose/beam, ordinary save encoding and
the actual isolated save-room UI/fresh v9 load pass. Save-room healing restores
MP; transient effects stay absent after loading. No position, stat, MP or enemy-hit
injection supplies that route. It does not prove ordinary relic acquisition or
an uninterrupted castle traversal. Existing user slots remain untouched.

The faster collision path also retains the existing private dive-kick instruction
comparison and native damage/rebound/death route. Full transformations, spells,
relic/settings menus, other abilities, remaining stages and whole-route parity
remain incomplete. Stage 4 remains disconnected; do not call the castle or
Alucard complete.

## Provenance

Selected `player_gravity.c`, `player.c` and collision paths adapt
`dra/704D0.c:CheckGravityBootsInput`, `5D5BC.c:HandleGravityBootsMP`,
`6E42C.c:DoGravityJump`, `71830.c:PlayerStepHighJump/CreateCeilingHitEffect`,
`7879C.c:EntityGravityBootBeam` and `692E8.c` from the
[pinned reference](../config/reference-us.json), under AGPL-3.0-or-later.
All original definitions, animation, sprites and sound remain private runtime data.
