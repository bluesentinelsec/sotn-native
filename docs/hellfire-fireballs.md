# Hellfire projectile motion components

The core reconstructs the original normal and large Hellfire fireballs. These are
motion/initialization services. The [selected native owner](player-hellfire.md) now
connects player casting, actual factories, private animation/artwork, spell/groups,
rendering and enemy combat. These interfaces alone enable no command or full parity.
These components allocate no child, initialize no actual attack, charge no MP,
learn no spell and write no ordinary save data.

Normal birth reads its parameter 0..2 launch vectors from bounded private DRA
records. It retains the original inverted player facing, bank nine, priority+2,
spell-two request, four-pixel attack extents, sound only for parameter zero and
no movement on birth. Subsequent nonzero **byte-width** hit flags retire it before
motion or timer decrement. The twentieth curved update still applies the retained
vertical velocity, then later updates move horizontally while retaining that field.
Malformed vector bounds, parameter or role/phase fail before mutation.

Large birth shifts the original Y high word while preserving its fraction, retains
existing draw flags plus rotation, and starts with an absolute raw fixed-point
speed 16, or 16/65536 pixels per update. Its pre-animation pose must reach 23 before
motion/acceleration/rotation/RNG occurs. It ignores hit feedback and keeps piercing;
actual enemy cooldown belongs to the native attack-group owner. Later updates
accelerate in the current velocity's sign, preserve signed fixed-point wrap and
apply the retained Y velocity. Original rotation advances by a quarter turn on
clock multiples of four and wraps as a signed word.

On each eligible even game clock, large motion requires exactly one actual
fifteen-bit rand result. An odd result requests blueprint 36 with parameter 1;
odd-clock updates consume no RNG. The owner must preserve that call even when an
optional factory cannot allocate. Missing/invalid required RNG fails transactionally.
Birth's separate spell initialization also needs its real private INT/equipment/
critical/RNG/group service; the motion's rand input does not replace it.

The original normal and large private animation offsets are emitted at birth.
The owner advances those actual timelines after the child controller and supplies
the next frame's pose, rather than inventing a launch delay. Actual post-controller
animation, offscreen culling, child/factory allocation and native combat are outside
these components. A factory/spell intent never establishes readiness.

Public tests use deliberately authored launch vectors. They cover both facings,
all normal parameters, birth/no-birth sounds, no first-tick motion, twenty curved
updates, impact retirement, retained velocity, large Y shifts/flag preservation,
pre-animation gates, piercing, RNG cadence/factory request, signed fixed/rotation
wrap and malformed/unready-input rollback. No private motion tables are published.

Private comparisons match 144 supplied vector/facing/pose/contact/fixed-word
contexts and 11,196 complete original controller snapshots for both variants.
They include supplied velocity/rotation boundary states and pose timelines, not
an ordinary game replay. Actual SetSpeedX executes; spell initialization, rand,
factory, sound and destruction use explicit boundary adapters. Debug and ASan/
UBSan snapshots match. This proves scoped motion/request behavior, not actual
animation, native projectile combat, playable Hellfire, full-frame, Alucard/castle
or console parity. Original tables, images and traces remain private.

Provenance: selected `dra/84B88.c:{EntityHellfireNormalFireball,
EntityHellfireBigFireball}`, pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
See the [beam lifecycle](hellfire-beam.md) and [command component](normal-spell-commands.md)
for their separate ownership boundaries.
