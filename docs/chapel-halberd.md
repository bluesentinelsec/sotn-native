# Chapel Bone Halberd combat

The internal [Chapel checkpoint](chapel-runtime.md) now runs selected original
Bone Halberd movement, spin/stab, jump, repeated lunge, separate weapon contact and
nine-piece death. Connected Chapel travel, other enemy families, mechanisms, Maria
and boss progression remain incomplete.

The body retains original grounded initialization, walking/retreat hysteresis,
loop-based facing, wall/ledge decisions, attack intervals and randomized lunge/jump
choices. Spin sounds follow animation advancement; repeated-lunge sounds precede
it. Preserve the original lunge cursor reset: `SetSubStep` clears the temporary
pose/timer assignments before repeated stabs begin. Jump landing and side sensors
use the private Chapel tables.

An owned adjacent weapon follows the body's full position/facing and selects its
private hitbox from frames 13–20. It retires when its parent generation disappears.
Re-entry replaces only a matching owned survivor and protects placed/unrelated
slots. Generation exhaustion fails before destroying an existing child.

Death reads nine private launch positions, velocities, lifetimes and rotation
increments. Pieces retain their own facing, rotation, original quarter-pixel gravity
and six-pixel terminal threshold, then become small explosions. The conversion
retains the piece's frame and rotation until explosion initialization, as the
original does. Its adjacent weapon retires and the original death sound plays.
Optional exhausted effect slots omit pieces; corrupt profiles fail explicitly.
All artwork and original tables remain private. Ordinary save layout is unchanged.

## Validation

All 82 public test groups passed in Debug and macOS ASan/UBSan; release packaging
passed. Synthetic tests cover movement, facing/hysteresis, spin timing, lunge cursor
and sound order, jump/landing, private weapon hitboxes, nine pieces, conversion,
re-entry/orphans, reservation conflicts and generation exhaustion.

A private LLDB fixture starts the original placed room-2 Halberd. It uses a protected
999-HP/high-strength character, observes its attack, positions weapon-only contact
while temporarily disabling body contact, and verifies a 16-HP hit. Ordinary attack
inputs then defeat it at tick 144, retire the owned weapon and create all nine pieces.
The subsequent 160 ticks render the pieces/explosions and verify retirement. Debug
and sanitizer checks each render 77 frames and exit successfully. Dummy audio checks
decoding/ownership, not listening or console PCM. Controlled contact/start fixtures
are not ordinary traversal, connected-route, whole-stage or console parity evidence.
