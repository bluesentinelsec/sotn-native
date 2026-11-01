# Alucard control completion inventory

Milestone 1 remains open. Its original control dispatch has 34 handled states;
32 states have selected native producers. State17 has an additional gated response
whose Fairy/engine availability remains unfinished. An enum member, command recognizer,
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
| 10: hit | [Selected kind promotion, physical births/recovery, shared wall/ceiling/slope/skid owners and palette counters](player-hurt-motion.md) | Selected [poison/curse text/Blink](player-ailment-effects.md); [cut feedback](player-cut-effects.md), [holy hits](player-holy-hit.md), [dark hits](player-dark-hit.md), [fire hits](player-fire-hit.md), [thunder hits](player-thunder-hit.md) and [ice hits](player-ice-hit.md) and [HUD/secondary impacts](player-hud-hp-hold.md); remaining elemental/death variants, rare/contact/death contexts and full contention; [damage inventory](player-contact-absorption.md) |
| 11: stone | [Selected queued entry, ordinary/rare statue, actual shared effects/capture, wiggle/escape and stone-death choice](player-petrification.md) | [Hammer word/actual stone escape protocol](player-medicines.md) now runs; Fairy Hammer producer, stone grab/thrown contexts, complete death/fade and shared frame/contention boundaries |
| 12: boss grab | [Selected normal/stone-reaction control, supplied boss-velocity release4 and Hippogryph drop](player-boss-grab.md), [normal held-grab damage/number/equipment feedback](player-grab-damage.md) | Actual throwing-boss and stone-reaction producers, dead release, broader interruptions and remaining dependent effects |
| 13: water kill | [Water death](player-water-death.md) | Global-fade preemption and remaining death contexts |
| 15: sword warp | [Alucard sword command, actual slot16 retirement, outlines20/21 and sixteen-update recovery](player-sword-warp.md) | Osafune and other weapon-specific contexts, all interruption/shared-overlay combinations |
| 16: kill | [Selected native normal/fire/thunder/ice C0 effects, resident capture and common64 uploads](player-physical-death.md), water common death and stone death | Interrupted/grab/fade contexts, general cleanup and whole-stage failure/shared timing boundaries |
| 17: Fairy Life Apple rescue hold | [Bound interruption/hold/resource/recovery and shared Blink4A](player-fairy-rescue.md); no ordinary Fairy producer/FFF engine availability | Actual Fairy selection, saved progress, item-use/lifecycle and native FFF/cleanup/resource transactions; remaining death contexts. Supplied scene/resource/clock fixtures do not implement the familiar |
| 18: teleport | [Owned Library Card](player-library-card.md), actual shared121/65 effects, Library travel/arrival ACK; six controller substates and integer alignment helper | Other-castle triggers/native TOP/RTOP, broader sharing/GPU/audio and route/console evidence |
| 24/25: Wolf/return | [Wolf](player-wolf.md), [conversions](player-wolf-cross-forms.md) | All entry/return contexts, interrupted handshakes and shared contention |
| 32: Dark Metamorphosis | [Dark](player-dark-metamorphosis.md) | Full transition/helper/palette contention evidence |
| 33/35: Summon/Tetra Spirit | [Spirits](player-spirit-spells.md) | Remaining capture/raster/audio contention and interrupted transition contexts |
| 34: Hellfire | [Hellfire](player-hellfire.md) | Full hidden collision/steering/recovery contexts and shared effects |
| 37: Soul Steal | [Soul Steal](player-soul-steal.md) | Full contact/group/factory contention and interruption contexts |
| 39: Sword Brothers | No native dispatch | Actual Sword familiar lifecycle/command/attack and shared outline producer; recognizer alone is unavailable |
| 40/41/42/43: Axe Armor stand/walk/jump/hit | [Owned armor25/weapon45, actual controllers, slot16 Spell11 combat, shadow49 and private SDK rendering](player-axe-armor.md) | All interruption/transition/shared-overlay contexts; common remaining damage/death branches |
| 48: scripted reaction | [Death theft producer, state/reset/palette, actual smoke/Blink58 and recovery](player-script-control.md) | Remaining form/script contexts and integrated transition evidence |
| 49: scripted hold | [Raw modes0/1/2, shared input timer and US release baseline](player-script-control.md) | Remaining script producers/contexts and Axe Armor's separate state50 |
| 50: Axe Armor scripted hold | [Actual equipped Axe owner, raw-mode hold, CF/palette restoration](player-axe-armor.md) | Remaining script producer/transition contexts |

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
timers. The selected [unified word-bank owner](player-timers.md) now supplies all16
countdowns, signed protection and original expiry/helper order; remaining producers
and broader frame/collision/transition contexts still require their actual owners. Frozen/release/menu/map/draw paths and failed native updates must preserve
their existing ownership and rollback boundaries.

Full acceptance needs original comparisons for each implemented branch and
repeatable actual-app sequences crossing normal movement, hands, forms, scripts,
grabs, damage and death. Supplied poses, equipment, protection, clocks or arrivals
are controlled evidence, not ordinary acquisition, traversal or console parity.
No default grants or ordinary-save format changes are part of this inventory.

The earlier "global-fade" label for17 referred to its shared graphics word.
The US producer is `servant/tt_002/faerie.c:UpdateServantUseLifeApple`; the alternate
Sprite familiar has its own corresponding producer. Fairy waits beyond24, checks
and consumes an owned Life Apple, then sets that word toFFF. Its actual flight,
animation and event children precede HP/MP restoration and slots8..63 cleanup;
a later phase clears the word. `dra/5087C.c`'s FFF engine branch runs the actual
player rather than Spirit's animation-only path. Closing17 therefore depends on
the familiar, consumable and common scene/effect owners, not a generic fade timer.

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
missing original script producers; the later [Axe Armor owner](player-axe-armor.md) supplies its separate states. The subsequent
[scripted input/reaction owner](player-script-control.md) adds actual modes0/1/2,
the shared remaining interval, Death's mode1 producer and its presentation. The
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

Selected [Talisman response2](player-contact-talisman.md) now uses actual equipped
count/LCK/RNG, shared entity46/Blink58 and the existing guard timer3 recovery. It
adds no dispatched state; remaining hurt/death/stone and equipment dependencies
keep milestone1 at26/34 handlers and incomplete.

[Selected neutral hurt feedback](player-neutral-hurt.md) adds original timer2/red/Blink58 and hurt-exit protection/color/afterimage work to the existing physical adapter. It adds no dispatch state or kind promotion; the remaining damage/motion/elemental gates stay open.

The subsequent [selected hurt motion/palette owner](player-hurt-motion.md) supplies
kind0/1 promotion, physical births/recovery, original wall/ceiling/slope/skid calls,
secondary impact and timer8, preserving actual shared owners and SDK pivots. Its
palette/counter coverage is distinct from remaining elemental child/text/Blink
availability. It adds no handler; all common/cross-owner milestone gates remain open.

The subsequent [owned medicine owner](player-medicines.md) connects the17
original equippable medicines to the existing normal-state hand path, without a
new dispatched state. Life Apple146/Hammer147 are forbidden hand selections in
the actual original menu. Their component profiles and the real stone Hammer
word/escape protocol do not supply the Fairy producer or state17 rescue. Actual
4096 buffs/cures/healing/MP, circle/outline/Blink/label effects and all-root room
cleanup retain their documented evidence and failure boundaries. Milestone1
remains32/34 with all six common/cross-owner gates open.
