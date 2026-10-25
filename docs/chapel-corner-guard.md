# Chapel Corner Guard combat

The internal [Chapel checkpoint](chapel-runtime.md) now runs selected original
Corner Guard stair movement, slash/lunge, separate weapon contact and sprite-part
death. Chapel travel, remaining encounters, tower progression, Maria and boss
integration remain incomplete.

The body grounds through its private sensors, waits for the player and changes
between the original head shake, rightward slash and leftward lunge. Movement
checks the stair floor ahead and undoes horizontal motion at its end. Separate
remembered/player direction flags control retreats: the body sprite's facing flag
normally stays unchanged because its frames encode both directions. Attack waits,
animations and weapon hitboxes come from the user's private stage file.

The lunge preserves its windup, original velocity decay and concluding look phase.
Selected dust uses the shared original particle service with Chapel's private
scale/velocity/frame/lifetime tables. The adjacent owned weapon copies the body's
full fixed-point position and current frame's signed-byte hitbox. Re-entry replaces
only a matching owned survivor; unrelated or placed reservations fail explicitly.
Stale parent generations retire the weapon.

Death reserves two primitives per private sprite part. Visible parts use the
original base palette, texture page, UV origins and priority. A bounded equal-depth
Z transform reads the private SDK matrix table; it projects the preceding angle
and velocity before advancing rotation, gravity and lifetime. The original gravity
increment is 6144 in 16.16 units. Each expired part creates a small explosion; the
body retires on the following empty tick. Optional primitive exhaustion retires the
body and lets the orphaned weapon retire on its update. Invalid profiles fail.
The ordinary save format is unchanged; all artwork/tables remain private.

## Validation

All 83 public test groups passed in Debug and macOS ASan/UBSan; release packaging
also passed. Synthetic coverage includes directional
state, stair rejection, slash/lunge transitions and sounds, crouched hitboxes,
weapon frame validation, re-entry/orphans, reservation conflicts, death palette/
UVs, old-angle projection, fragment motion/expiry, pool exhaustion and dust tables.

A private LLDB fixture starts the original room-11 guard with a protected
999-HP/high-strength character. It observes the original attack, positions
weapon-only contact while disabling body contact, verifies a 16-HP hit, then
uses ordinary attack inputs to defeat it. Its four original sprite parts render
and expire, the weapon retires, and the fixture exits successfully at tick 239
with 75 rendered frames in both Debug and sanitizer builds. This explicit start/contact fixture is not ordinary
traversal, a connected-stage replay or console parity evidence. Dummy audio
validates decoding and lifetime, not listening or console PCM comparisons.
