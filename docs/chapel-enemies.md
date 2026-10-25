# Chapel bird and Skelerang encounters

The internal [Royal Chapel checkpoint](chapel-runtime.md) now runs selected original
Black Crow, Blue Raven and Skelerang encounters with their own stage identities,
initialization, animations, hitboxes, combat and sounds. Chapel travel remains gated;
other enemy families, full area effects/scripts and boss progression remain incomplete.

Black Crows retain proximity wake-up, facing at animation loops, downward swoop,
reference-height recovery, 128-tick hover and rotating/falling animated death.
Blue Ravens retain dismount, height matching, 32-tick hover and accelerating attack.
Their original death exception during height matching remains intact: a dead-marked
Raven finishes that phase before its death handler runs. Dismount compares integer
pixel heights, including when fractions differ within the same pixel. Private
per-frame hitboxes, positional caw/death sounds and selected explosion paths remain
distinct from the other flying enemies.

Skelerangs retain their shared original ground/wake, cower, throw/catch, return and
death behavior. Chapel selects its own body/boomerang/target IDs, initialization,
contiguous animation/sensor/position data and original 256-entry sine table. Boomerang
velocity reads private stage samples; it does not regenerate that table with host
floating-point math. Their homing angle uses the existing private SDK lookup.
Standing and cowering deaths retain separate fragmentation/explosion branches.

Shared Skelerang re-entry now replaces only matching children owned by the same
root slot. It validates all three reservations before retiring any surviving child,
protects original placed and unrelated actors, and preserves generation checks for
orphaned boomerangs. This also fixes the native Marble adapter's former termination
when its own child survived re-entry. Ordinary saves still recreate transient enemies;
no save format or character grant changes.

## Validation

All 81 public test groups passed in Debug and macOS ASan/UBSan; packaging passed.
Synthetic cases cover bird flight/recovery/hover/deaths, the Raven death exception,
integer-height decision, frame/motion bounds, Chapel-specific Skelerang identities,
private sine selection, fragments/cower death, child replacement and ownership
conflicts. Marble's tests cover its corresponding child replacement and rejection.
No original animation, hitbox, sine, palette or artwork arrays enter public fixtures.

Private LLDB checks start the original placed Crow (room 14), Raven and Skelerang
(room 13). They use a protected 999-HP/high-strength character, observe active
swoop/attack/boomerang behavior, then position contacts and use ordinary attack
inputs to verify damage and deaths. Crow/Raven/Skelerang finish at ticks 77/242/264
with 148 rendered frames in both Debug and sanitizer builds; both exit successfully.
A separate protected/injected native Marble Skelerang/Diplocephalus regression passes.
Dummy audio checks decoding and ownership, not listening or console PCM parity.
These explicit starts and positioned encounters are not ordinary traversal,
uninterrupted connected routes, whole-area or console parity evidence.
