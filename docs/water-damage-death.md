# Water damage and water-death controllers

The selected water-damage component reconstructs the original waist-depth contact,
ordinary damage priority and kind-six HP loss. It does not yet drain native HP or
enable water death. Actual equipment, contact, effect, capture and death owners
must consume its requests before those behaviors become playable.

Water without Holy Symbol replaces a pending contact only when its low five bits
are zero, discarding any retained high bits. Injection precedes grab and damage
invincibility gates. Other pending contacts retain priority; stone and non-exact
kind-six words delegate to the actual general damage owner. Selected ordinary
water damage loses one HP on absolute game-clock multiples of ten, independent of
defense or poison. A lethal hit selects state13, not physical death. Nonlethal
responses request Blink4E every eligible update, refresh the original32-update
pain-voice counter, and request the scrape sound only when HP actually falls.
Equipped Fury Plate requests at least512 defense-buff updates even on zero-loss
responses. These are intents, not successful factories, playback or buffs.

The selected state13 component preserves ordered death voice, normal reset,
animationC1, global effect1, Blink59 and factory49/parameter6 requests. Birth clears
rotation drawing through the normal helper while retaining rotation512; subsequent
updates restore the rotation flag. Its blue increases toward248 while red/green
fall toward8. Vertical speed wraps before the original strict positive reset; it
is not a conventional cap. Cramped-space birth cancels upward velocity. A finished
animation requests the original framebuffer capture and common disintegration
state16; an already completed global fade preempts initialized state13 into17.
The component never moves, animates, copies pixels or substitutes physical death.

Public authored tests cover sustained exposure, clock wrap, voice expiration,
priority, protection, Holy Symbol, stone delegation, Fury Plate, lethal HP and
ordered death/fade/capture boundaries. Private Debug and ASan/UBSan traces match
byte for byte. Instruction comparisons check34,560 supplied damage contexts and
8,100 state13 contexts against the original executable. The damage comparison runs
original kind-six HP/Fury work and dispatch, with equipment/recalculation/factory/
sound adapters and explicit stone/other-contact delegation. The death comparison
runs the actual normal-reset helper; global effect, factory, sound and SDK capture
calls are adapted. These are component checks, not native water damage, actual
captured pixels, complete death/game-over, route or console evidence.

Selected [water Blink flashes](water-blink-effects.md) now supply their shared
child/row/rendering owners and native input snapshots; actual damage/death producers
remain unwired. Selected [death particles](player-death-particles.md) now supply
their initialization/factory/row/rendering dependency; actual capture and common
death operations still need their owners.
Ordinary starts, grants and version-nine saves remain unchanged. The existing
unsupported US wet-smoke birth guard remains. Native water damage/death, full
equipment and shared effects, Alucard, castle and console parity remain incomplete.

Provenance: selected `dra/692E8.c:EntityAlucard`, `dra/5D5BC.c:HandleDamage` and
`dra/71830.c:PlayerStepKillWater`, pinned `sotn-decomp`
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
Private inputs and comparison traces remain excluded from public files.
