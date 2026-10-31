# Alucard control completion inventory

Milestone 1 remains open. Its original control dispatch has 34 handled states;
the native player currently dispatches 24. An enum member, command recognizer,
animation or component request does not implement a missing state. This inventory
records the dependencies needed to close the complete milestone, including those
also covered by the other [Alucard workstreams](alucard-status.md).

The source baseline is the US `EntityAlucard` switch in `src/dra/692E8.c` and its
handlers in `71830.c`, at reconstructed revision
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`. It dispatches the following states.
Numbers named only in the shared enum, including Richter states and unused spell
36/38, are not additional US Alucard handlers.

| Original state IDs | Native ownership | Remaining completion gates |
| --- | --- | --- |
| 0/1/2/3/4: stand, walk, crouch, fall, jump | Normal controller and collision sensors | All attack substates and held/tapped cancel windows; original timer expiry, landing, restoration and collision comparisons |
| 5/9: Bat/return | [Bat](player-bat.md) | All transition contexts and shared factory/capture contention |
| 6: stuck | [Mist restoration](player-mist.md) | Restoration and cramped-space combinations across every entering state |
| 7/14: Mist/return | [Mist](player-mist.md) | All transition contexts, helpers and shared palette/capture contention |
| 8: high jump | [Gravity Boots](player-gravity-boots.md) | Collision/ceiling recovery and interruptions across remaining damage/equipment states |
| 10: hit | Selected physical reaction | Remaining damage kinds, promoted reactions, wall/ceiling/sloped recovery, helpers and effects; [damage inventory](player-contact-absorption.md) |
| 11: stone | [Selected stone](player-petrification.md) | Rare statue, repeated-hit and dependent effect branches |
| 12: boss grab | [Selected grab](player-boss-grab.md) | Thrown release, stone reaction, damage palette/number effects and remaining boss producers |
| 13: water kill | [Water death](player-water-death.md) | Global-fade preemption and remaining death contexts |
| 15: sword warp | No native dispatch | Actual Alucard sword/Osafune special, weapon-slot retirement, outlines 20/21, palette and sixteen-update recovery |
| 16: kill | Selected physical and water death | Remaining elemental deaths, helpers and fade branches |
| 17: global-fade hold | Classified dead, no native dispatch | Real fade handshake, retained pose/draw attributes, ground/fall recovery and Blink 4A; classification alone is insufficient |
| 18: teleport | No native dispatch | All six substates, actual teleport factory/retry, warp acknowledgement and TOP/RTOP position adjustment |
| 24/25: Wolf/return | [Wolf](player-wolf.md), [conversions](player-wolf-cross-forms.md) | All entry/return contexts, interrupted handshakes and shared contention |
| 32: Dark Metamorphosis | [Dark](player-dark-metamorphosis.md) | Full transition/helper/palette contention evidence |
| 33/35: Summon/Tetra Spirit | [Spirits](player-spirit-spells.md) | Remaining capture/raster/audio contention and interrupted transition contexts |
| 34: Hellfire | [Hellfire](player-hellfire.md) | Full hidden collision/steering/recovery contexts and shared effects |
| 37: Soul Steal | [Soul Steal](player-soul-steal.md) | Full contact/group/factory contention and interruption contexts |
| 39: Sword Brothers | No native dispatch | Actual Sword familiar lifecycle/command/attack and shared outline producer; recognizer alone is unavailable |
| 40/41/42/43: Axe Armor stand/walk/jump/hit | No native dispatch | Actual equipment/form owner, four private overlay callbacks, artwork, weapon/combat/collision and damage transitions |
| 48: scripted reaction | No native dispatch | Original simulated-input mode 1, red palette/timer, smoke/Blink 58 and animation/sound recovery |
| 49: scripted hold | Boolean stage hold adapter, US physical-input rebase | Full original simulated-input duration/mode and all actual script producers |
| 50: Axe Armor scripted hold | No native dispatch | Actual Axe Armor owner, simulated-input mode and animation CF restoration |

## Substates and common frame ordering

The normal states contain weapon-family behavior as well as movement. Their full
completion requires the actual weapon owners from workstream 3:

- Standing: idle/look/shield/backdash/chair 0..4; unarmed 40/59; ordinary
  weapon recoveries 41..50/5D; curse 51; retained startup counters 53..56/5C;
  dash/air follow-through 57/5B; looping attacks 58/5A. Hexadecimal IDs here
  correspond to the original switch cases; 52 has no standing handler.
- Crouching: idle/lower/rise/landing/cramped 0..5; unarmed 40/59, curse 51
  and the original ordinary weapon recovery cases. Shield presentation follows
  the actual hand child and must respect the crouch/rise controller boundary.
- Jumping: rising/falling 0/1; ordinary hand recoveries and curse 51;
  separate 57/5B and 58 loops; dive kick 70. The original early return before
  the final horizontal-lock test is significant.
- Fall uses the eight-update ledge grace and timer 6 animation expiry. Transition
  helper `6FDF8.c:func_8010FDF8` owns landing priority, ordinary and heavy
  landing distinctions, gravity/ceiling gates and attack/jump/crouch/backdash
  priority. Hand destruction belongs only at its original cancellation points.

Closing this inventory also requires the outer `692E8.c` frame: all sixteen
original player timers and expiry helpers; physical versus simulated input;
grab/contact/special/form priority; previous-state snapshots; status and recovery
helper updates; animation; pre-collision afterimages; original one/eight-pass
collision and restoration; water scaling; post-collision presentation. The
sixteen HUD stat-buff counters have a different owner and cannot replace player
timers. Frozen/release/menu/map/draw paths and failed native updates must preserve
their existing ownership and rollback boundaries.

Full acceptance needs original comparisons for each implemented branch and
repeatable actual-app sequences crossing normal movement, hands, forms, scripts,
grabs, damage and death. Supplied poses, equipment, protection, clocks or arrivals
are controlled evidence, not ordinary acquisition, traversal or console parity.
No default grants or ordinary-save format changes are part of this inventory.

## Selected normal-control corrections

The native common controller now preserves the original eight-update ledge-fall
animation switch and walking-carry clear. Heavy ordinary Jump landings use the
sliding crouch recovery with half horizontal speed; attack landings retain their
separate recovery. Fast soft landings retain momentum and select the original
speed-dependent standing animation. Bat and Bat return always use eight collision
passes, including signed velocity truncation at low horizontal speeds.

Standing and crouching apply their recovery, look/turn/rise/movement and hand
destruction flags in the original order. This permits unlocked Up cancellation,
preserves Basilard's held pose, and fixes fist/curse completion on a crouch-release
frame. Jump/crouch/fall transitions retain the unlocked hand word until an original
destruction boundary; the existing weapon owner emits no hitbox outside its frame
range. Airborne recovery respects the original early return before the final
horizontal lock. Shield state comes from its prior actual child update, and crouch
animation changes occur in the controller before the common animation update.
Held shield startup does not synthesize a subweapon tap or charge hearts.

After the existing script or Bat/Wolf input hold ends, the US first physical frame
rebases held/tapped input to the current buttons. A later release/new press works
normally. This corrects the selected hold adapters; it does not implement the
missing original simulated-input modes, durations or script producers. The new
input flag is transient player state, held on draws and failed updates and retained
by ordinary room rebases. It adds no save fields or new ability availability.

Authored public `player-control-recovery` tests exercise actual floor-edge departure,
grace/expiry, animation failure rollback, attack versus ordinary heavy landings,
fast landings, hand cancellations, Basilard/shield recovery, real subweapon heart
costs, low-speed Bat collision and simulated-input release/rollback. The existing
backdash test now expects the original sliding recovery for a heavy ordinary Jump
landing. These fixtures contain no original game tables or assets.

Private comparisons match 48,000 sampled original US return-helper, transition,
standing, crouching and jumping instruction contexts. They vary hand locks,
held/tapped direction, pose/end boundaries, shield snapshots, tight space, water
depth and momentum. Factory, sound and afterimage service adapters isolate these
controller outputs; this is not a complete original frame/collision comparison.
Debug and macOS ASan/UBSan traces are identical. A separate comparison matches
512 supplied simulation/physical-input sequences, 3,328 original input updates,
including the actual US remapping and terminal-frame tap rebase.

Muted LLDB native checks pass with actual resources in Debug and macOS ASan/UBSan:
ordinary development Gladius Up cancellation; explicit empty-hand and Basilard
equipment selection, held/released hand recovery; an explicit heavy-landing pose
and protection fixture producing real shared BP0 smoke children/rows/rendering;
and map/repeated-draw holds. The actual CMake-built app also passes Gladius Up
cancel, selected empty-hand crouch recovery and ordinary walk/jump/landing, then
quits through SDL normally. Equipment/pose/protection fixtures remain distinct
from ordinary acquisition, uninterrupted routes and console comparisons.
