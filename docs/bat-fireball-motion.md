# Bat fireball motion component

The core now reconstructs the motion/lifecycle of the projectile requested by
Fire of Bat. [Selected live Fire of Bat](player-fire-of-bat.md) now connects this component
to actual factories, attacks, animation, rendering and feedback. The comparisons
below retain their motion-only scope; the component itself reserves no slots.

An actual child starts at its factory's fixed-point origin. Birth takes facing
and priority from Alucard, including the original unconditional facing overwrite
for both player and familiar parameter variants. It offsets the shot seven pixels
forward and four upward, preserving fixed-point fractions and signed-word wrap.
Travel continues at the original 3.5 pixels per simulation tick. The shot begins
at quarter scale; X grows to 1.5 scale while Y stops at full scale. Later player
turns and priority changes never redirect or reprioritize an existing shot.

Birth emits the original sound, spell-nine initialization and four ticks of
player timer ten (`ALU_T_USE_SUBWPN`). That timer is not invincibility. The cursor
reports bank nine, priority and hitbox dimensions without inventing initialized
combat or copying private animation data. Any subsequent nonzero hit flags retire
the shot before movement/growth. Birth itself ignores preceding hit flags.
The owner still handles original offscreen removal and camera/room rebases.

Authored public fixtures cover both facings, launch/fraction preservation,
independent growth caps, captured priority/facing, impact ordering, one-shot
requests, signed coordinate boundaries and atomic invalid-input failures.
Private muted LLDB comparisons exercise 324 supplied contexts and 6,048 records,
including player/familiar parameters, priority extremes and different impact
frames. Original projectile and speed instructions execute; sound, spell
initialization and destruction use explicit adapters. This proves the selected
motion/requests, not spell damage, animation, rendering, live Fire of Bat or console
parity. Original images and traces remain private.

Provenance: `dra/84B88.c:EntityBatFireball`, pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
