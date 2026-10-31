# Scripted Alucard input and theft reaction

The native Entrance Death manager now produces original simulated-pad mode 1
when it takes Alucard's equipment. The following player update enters state 48,
resets normal presentation, stops both velocities, selects animation 33 and
requests BP0 smoke followed by Blink58. Actual shared factories, children,
eight-row Blink allocation and rendering consume the requests after player
success. Animation completion plays WHAT once and returns to normal control.
This adds one handled state to the [control inventory](player-control-audit.md).
The complete milestone remains open.

## Input and frame ownership

`SotnPlayerSimulatedPad` retains the original pad word and shared `demo_timer`.
Mode 0 supplies buttons, mode 1 enters the reaction when not already in state 48,
and mode 2 selects the normal-form hold without selecting a new animation.
Physical input resumes when the timer expires. A retained mode-2 word continues
holding state 49 until the producer changes the word, even with a zero timer.
The US first physical frame rebases held/tapped buttons; a later release/new
press works normally. The raw mode-2 path retains the existing hand word.

Bat/Wolf command holds write this same counter. Stage controls bind their actual
prior-update interval before player control. Death supplies timer 1 and changes
mode 1 back to mode 0 on its next manager update. Existing Boolean NPC-hold
callers retain their adapter that renews one mode-2 frame; the native binding
preserves the raw timer and word. Axe Armor's mode-2 state 50 remains unsupported.

This state is transient. Room rebases translate captured factory origins without
advancing input, animation, colors or effects. Menus, maps, frozen/release-only
frames and draws hold the input and palette timers. Dead players retain their
input/timers; scripts cannot revive them or interrupt death. Ordinary saves stay
version nine, and ordinary starts retain their existing grants/resources.

## Palette, reset and effects

The reaction writes hit timer 2 to six and damage CLUT to 8166. The birth frame
retains its prior palette. The next five displayed updates use red; the sixth
timer update restores normal palette 8100. The existing normal boss-grab producer
also supplies its actual red CLUT to this owner. Grab numbers, thrown release and
stone reactions remain unfinished.

Leaving selected normal scripted states restores protection and, without poison
or curse, timer 4's twelve-update recovery, timer 15's afterimage boundary and the
normal palette. The subsequent eleven displayed updates use the original four
vertex colors from the private SDK sine table and actual wrapping gameplay clock.
Expiry clears color blending. Player packets use actual CLUT/colors and retain
rotation/upside-down geometry. Stone and death keep their drawing owners. This
does not complete every original player timer or palette helper.

Blink58 reads private profile 18 and original 7007's condition: hold while the
player is in Hit state 10. State 48 itself does not extend that hold. Preserve
full-parameter duplicate suppression, shared fan phases, color/rotation math,
actual eight-row retries, afterimage resets and retirement. Optional exhaustion
omits the visual without delaying the reaction or inventing successful ownership.

The normal reset can destroy an actual live Mist visual by its generation instead
of waiting for a return handshake. Presentation/Gas Cloud consumers retire from
the interrupted owner. Palette jobs keep their own lifetimes; the reset supplies
no fake ready phase or palette-service reset. Other form/script combinations
still need the full milestone's transition evidence.

## Evidence and remaining scope

Authored public regressions cover modes 0/1/2, timers and retained mode-2 words,
tap rebasing, reaction/palette/recovery timing, grab palette pulses, frozen holds,
malformed input/SDK/animation rollback, rebases and Death's actual producer.
Shared-runtime tests cover real Blink58 rows, allocation retries, duplicates,
7007's Hit condition, retirement and failed frozen requests. Fixtures contain no
original game tables, scripts, artwork or audio.

Private instruction comparisons match 5,184 state-48 contexts, 11,648 timer-2/4
palette/SDK-color contexts, and all 7,248 Blink controller/phase/quad snapshots in
96 normal-image contexts. Simulated-pad comparisons match 768 supplied sequences,
4,992 original input updates. Factory/sound/allocation/destruction adapters isolate
these components; they are not complete original main-loop/GPU/console comparisons.

Muted LLDB native checks run the actual room-11 manager/dialogue through five
speech requests, equipment theft, one reaction birth, five red and eleven recovery
frames, real eight-row Blink rendering/retirement and restored ordinary jump.
A draw-only check measures 905 changed palette pixels without ticking the player.
Explicit relic/mode fixtures check live Mist interruption; a neutral recovery
projection matches the existing upside-down CPU coverage. A lethal/protection/
palette/pad fixture verifies dead-state exclusion. The CMake-built app also passes
the actual script/reaction/rows/control sequence and quits through SDL.
These explicit starts/fixtures are separate from acquisition, uninterrupted routes
and console evidence. Full opening presentation, the other nine missing states
and remaining control/equipment/familiar/damage/shared branches are incomplete.

Adapted AGPL-3.0-or-later algorithms: `dra/692E8.c`, `6DF70.c`,
`71830.c:PlayerStepUnk48/49`, `7879C.c:EntityPlayerBlinkWhite` and
`st/no3/e_death.c` at revision `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`.
Runtime definitions and SDK lookup values remain private inputs.
