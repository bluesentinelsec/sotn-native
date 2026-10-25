# Chapel Spectral Sword

The internal [Chapel checkpoint](chapel-runtime.md) now runs selected original
Spectral Sword movement, its sixteen orbiting weapons, expansion/retraction,
independent weapon destruction, aura and parent death. Connected Chapel travel,
remaining mechanisms, scripts and boss progression remain incomplete.

The original placed parent reserves twelve small and four large weapons in
adjacent slots. Initialization preflights placements, ownership and generation
capacity before replacing any weapon. Children retain parent generation checks;
an unrelated actor cannot occupy a reserved slot. All sixteen weapons must be
ready or dead before the parent enables their hitboxes.

Movement retains the original SDK square-root quantization, angle lookup and
velocity ordering. Its hidden primitive stores a screen-anchored target; camera
movement therefore changes the vector to that target. Small weapons use the
original 341-unit angle spacing. Ready weapons replace only their integer position
halves and retain their own fractions. Lost weapons modify spin in the original
direction, including when the current spin is zero. The attack expands to radius
168 before retracting to 48; reaching a target by one exact step completes on the
following call.

Each weapon takes independent damage and decrements its parent's count once.
Destroyed weapons fall with selected flames, then remain hidden and owned until
the parent dies. Parent death marks all sixteen dead, retains its prior velocity,
falls and waits for every weapon to hide before retiring the whole family and its
primitive. The growing, fading aura follows its live parent with safe generation
references. Exhausted optional aura/effect pools omit that effect; failed private
data, invalid states and ownership conflicts report explicit errors.

Original initialization, frame hitboxes, artwork, palettes and SDK tables remain
private runtime inputs. Transient actor state does not change ordinary v8 saves.

## Validation

All 87 public test groups pass in Debug and macOS ASan/UBSan; release packaging
passes. The repository guard checks the public source boundary before publishing.

Synthetic regression cases cover all sixteen reservations, readiness, orbit
spacing, private SDK quantization, camera-relative targets, fractional position
retention, exact-step transitions, radius timing, lost-weapon spin, individual and
parent deaths, hidden-child lifetime, aura fading, orphan/re-entry safety and
primitive/generation exhaustion. They also reject invalid frame/count state.

Private native checks use an explicit room-9 start, a protected character and
positioned contact/attack targets. They isolate movement, attack cycling, blade
contact/destruction and parent cleanup; they do not prove ordinary traversal,
an uninterrupted encounter replay or console audiovisual/timing parity. Dummy
audio checks decoding and ownership rather than listening or PCM comparison.
Debug and sanitizer LLDB runs both finish at tick 783 with 196 rendered frames
and successful exit. The selected blade deals 15 HP contact damage, an ordinary
Gladius attack destroys it, its hidden corpse remains owned, and a later high-
strength fixture finishes the parent's ordinary attack kill. All sixteen weapons
and the parent's primitive retire. A captured expanded-orbit frame was visually
inspected. The fixture uses 999 HP, protected intervals and forced positioning;
these are component checks rather than an ordinary-character encounter replay.
