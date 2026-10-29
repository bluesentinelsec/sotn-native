# Spirit casting and scene freeze foundation

Alucard's core now supports the original Summon Spirit and Tetra Spirit cast
states when an actual owner explicitly binds their context. The native game
leaves that context disabled: portal capture, stage freeze dispatch, actual spirit
children, attack groups, artwork and enemy damage still need connected owners.
Selected Dark Metamorphosis remains the only live ordinary spell. This foundation
creates no effects, learns no progress and grants no relics or resources.

Successful commands charge the private spell cost once, retain the original low
attack mode, remove the real shield cursor and emit the original root factory117
with parameter0 or1. They select F0/F1, request the original voice and set timer12
to four. Timer12 is separate from subweapon use and invincibility; it contributes
the original spell-casting status. Both cast states retain the crouch status and
return to crouch timeline17 after the animation terminates. Crouch dispatch now
requires an actual attack substate before running weapon recovery; a retained low
attack flag must not override ordinary crouch after a spell. Room rebases translate
the pending source without ticking it. Full stage initialization clears transients;
ordinary inventory and version-nine saves remain unchanged.

The scene-freeze service models the original engine boundaries. A portal's birth
request takes effect at the end of its ordinary frame, after that frame's stage
update. Only the entry edge requests the native afterimage reset and sound-script
pause. The owner snapshots the next frame's mode before updating children. A
portal that clears its request still completes that frame under frozen rules. The
following frame performs only the original release protection and sound-script
resume; normal simulation starts one frame later. This is separate from menu pause.
The engine adapter must continue the system clock and palette jobs while preserving
the game clock, player motion/resources, hit detection and ordinary stage timers.

The player freeze adapter advances only original timelines BA/BB/BE/F0/F1. It
retains position, velocity, collision, HP/MP, ailment/buff/use/invincibility timers,
pending blood healing, spell commands and the simulation frame number. Consumed
one-update events are cleared so effects and MP costs cannot replay. The native
input adapter observes physical buttons on frozen and release frames, preventing
a held button from becoming a new tap after thaw. The release frame advances no
animation and applies original protection max(timer14,48).

The [selected stage-freeze interface](stage-spell-freeze.md) now supplies the
dispatcher that native integration must call: new step-zero
actors initialize; initialized actors advance only with flag0x10000, including the
original hit-palette countdown. It must not perform layout scrolling, hit detection
or ordinary stun/cooldown updates. Actual effects continue in original slot order.
The portal's delayed capture must copy the real ordered framebuffer into its
shared target. No timeout or invented readiness can replace that operation.

Public authored tests cover both commands/facings, exact and insufficient MP,
disabled ownership, retained attack/shield state, original recovery, rebase,
transactional malformed-animation failure, all five selected freeze timelines,
unselected timelines, retained player state, input edges and the release-only frame.
Private original-instruction comparisons cover 864 command/constructor contexts,
including 144 actual casts, and 1,620 complete animation-selector/PlayAnimation
updates. The cast comparison adapts factory, sound, learning and destruction
boundaries; the freeze selector executes without helper adapters. These are
component comparisons, not native spell, full-frame or console parity evidence.
Original tables and traces remain private.

Provenance: selected `dra/6E42C.c:{PerformSummonSpirit,PerformTetraSpirit}`,
`71830.c:{PlayerStepSummonSpells,PlayerStepCrouch}`, `692E8.c` status construction,
`6DF70.c:{func_8010DF70,func_8010E0D0}` and `5087C.c` engine-five boundaries,
pinned sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`,
AGPL-3.0-or-later. See [spirit motion/portal/target scope](spirit-spells.md).
