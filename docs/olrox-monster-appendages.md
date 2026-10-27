# Olrox owned appendage components

The monster family now has reconstructed components for its foot and knee
hitboxes, growing eight-packet tail, paired articulated arms and independently
breakable hands. Arms retain the original fifteen command states and position
and rotate their generation-owned hands. Damaged hands disappear, wait, grow
back and restore their original physical hitbox. Selected defeat commands retain
captured original sprites until the body's next signal.

These components remain outside gameplay dispatch while the monster body, head,
attack children and complete defeat/reward flow are integrated. They are not a
playable second form or a completed boss. Existing ordinary saves are unchanged.

## Ownership and original behavior

One monster generation owns the reserved parts and articulation service. Missing
parents retire dependent parts without changing replacement actors. Missing or
reused hands fail before an arm writes their state. Foot metadata copies complete
fixed-point coordinates; knee and arm attachments preserve the original fractional
position words when replacing only the integer coordinates.

Tail poses retain separate angles and velocities, the original zero endpoint,
private pose pointers, growth and strict arrival behavior. Arm commands retain
packed target/speed semantics and the original distinction between signed byte
extension decisions and unsigned byte geometry. Hand recovery restores original
HP and attack state; it does not grant player equipment or progress.

Original burst allocation creates six independent red-fireball particles and
consumes two RNG samples even when its optional pool is full. Particle motion,
late animation damping, rotation, opacity and lifetime use their original private
animation and SDK inputs. Optional primitive exhaustion retries a tail/arm
constructor or omits a hand capture without overwriting another owner's packets.
The native capacity fallback avoids the source's unchecked null-packet access;
it is not console parity evidence under artificial pool exhaustion.

The selected sprite-capture helper applies original scale/rotation with zero
rotation pivots. Captured vertices stay in the part's own primitive allocation;
private frame definitions and artwork remain immutable. It preserves the
bank-seven sprite helper's 127-unit offsets and ignored part cropping.

## Validation and limits

Public authored tests cover fixed-point ownership, parent replacement, tail
growth/poses, arm commands and attack selection, hand damage/recovery and retained
defeat capture, malformed roles/private pointers and optional capacity limits.
Independent particle fixtures cover all four burst variants and retirement.

Private Debug and macOS sanitizer comparisons match 88,960 original-instruction
appendage snapshots and 864 particle snapshots. They cover all fifteen arm
commands, tail poses, both hand roles, damage/recovery/defeat signals, rotated
captures, camera origins, palette overrides and burst allocation/RNG. Original
controllers, SDK routines, animation and burst factories execute. Initialization,
primitive allocation, nonclearing destruction and audio boundaries use observers;
leading-zero adapters supply the GTE operations used by SDK square roots.
These traces seed component commands and body signals. They do not advance the
real body, establish ordinary boss damage/defeat, or prove encounter/console parity.
Muted Debug/sanitizer CMake application checks additionally verify normal Entrance
startup and successful SDL quit; they do not exercise the unfinished monster.

Provenance: selected BO0 `313A8.c` routines `func_us_801B5470` through
`func_us_801B7BAC`, shared `st/e_misc.h` burst behavior and selected
`dra/4B758.c:func_800EB758`, at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
Private profiles, SDK tables, animation, artwork and sound stay user-supplied.
Authoring disassembly, generated analysis and private traces are not shipped.
