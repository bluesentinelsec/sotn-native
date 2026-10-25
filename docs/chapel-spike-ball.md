# Chapel Bone Pillar spike ball

The internal [Chapel checkpoint](chapel-runtime.md) now runs the Bone Pillar's
spike-ball variant. The intact ball can be destroyed with ordinary attacks. Killing
its base releases it with original bounce/rotation, first/later impact sounds,
rolling motion and palette flashing. The released ball becomes invulnerable and
continues dealing contact damage. Its separate textured quad retains the
original UV origin and priority behind the main sprite.

The original creates a linked attack outside the stage-actor array. Native code
represents those selected attacks as fifteen bounded records, each borrowing a
ball slot/generation. They follow whole-pixel ball positions and retire when their
owner disappears. Four cooldown groups use the original least-occupied assignment;
reusing a group preserves existing enemy invulnerability. Offscreen attack centers
are disabled. Optional exhausted links omit the attack without stopping the ball.
Other users of the original out-of-stage actor pool remain unintegrated.

Linked attacks reuse the native damage service with original fire power, hit type,
16-tick invulnerability and four-tick stun. They can damage other enemies, including
normal elemental defense/immunity/absorption and supported EXP/death handling.
Player weapons cannot damage the released ball. Room changes clear transient links;
ordinary save data and private input boundaries are unchanged.

## Validation

All 85 public groups pass in Debug and macOS ASan/UBSan. Synthetic tests cover UVs,
release/bounce/roll, palette timing, intact death, link capacity/group balancing,
stale generations, offscreen rejection, allocation exhaustion, actual enemy damage,
shared/reused cooldowns, elemental immunity/absorption and room-entry cleanup.

Private Debug and ASan/UBSan LLDB fixtures start the original placed room-11 variant using a
protected 999-HP/high-strength profile. One sequence destroys the intact ball with
ordinary attack inputs. Another defeats its base, follows its original stair bounce
until it rolls, positions player contact and injects a controlled defense-one skull
target. The linked attack deals 58 damage and player contact deals 59 HP. Forced
player displacement keeps the camera with the ball; target positioning and protection
isolate component behavior. These checks are not ordinary traversal, a full encounter
replay or console parity evidence. Dummy audio verifies decoding/ownership rather
than listening or original PCM comparison.
