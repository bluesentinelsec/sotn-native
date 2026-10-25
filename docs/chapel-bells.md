# Chapel bells

The internal [Chapel checkpoint](chapel-runtime.md) now runs the two original bell
variants, their spawners, ropes, projected bodies, damped swing, ringing and selected
player interaction. Tower routing, remaining scenery, confessional/Maria scripts,
boss progression and connected Chapel travel remain incomplete.

Each spawner reads its two original positions and variants from private data.
Both reservations, placements and generation capacity are checked before replacing
either child. Bells retain independent behavior after spawning, as in the selected
reference. Private profiles select two or four rope segments, length and priority.
An exhausted primitive pool leaves the bell waiting to retry initialization.

Swing uses original fixed-point velocity, acceleration, damping, snapping and
clamping. Ground and ceiling contacts come from the existing original room tile
collision; this adapter introduces no replacement bell platform. Contact changes
retain the original direction and impulse ordering, including ceiling overriding
ground when both are present. A 64-tick ring interval suppresses repeated sound
requests without suppressing swing/contact state changes.

Rope vertices use the private SDK sine table. Three body faces read private model
pointers and vertices and use the selected SDK matrix/projection path. Anchors
remain in room coordinates; projected vertices and draw areas use screen coordinates.
Mist priority and bat/mist input exceptions retain their original status branches,
but this does not enable transformed-player abilities or prove their play behavior.

Inside a bell, two ordered draw-area commands conceal Alucard and restore the
original area afterward. They occupy priorities immediately before and after the
player; every intervening draw observes the active area. Native presentation saves
and restores pixels outside that area around each ordered draw. Empty/inverted
areas hide the draw, and bounds clip safely to the frame. These owned records
replace the selected draw-environment objects; they do not implement a general
PS1 GPU or its separate environment-allocation pool.

The selected inside behavior refreshes 32-tick protection, exposes the original
transient status bits and simulates an empty pad for two ticks in normal form.
Hand inputs are masked for the following player update, then the transient bits
clear. Leaving the bell lets the short input interval expire. Original afterimage
reset/disable effects, transformed-player validation, full raster/audio/timing
comparison and remaining stage scripts are separate work. All bell profiles,
coordinates, model data, artwork and SDK tables stay private. Ordinary v8 saves
exclude transient bells, swing, input intervals and draw areas.

## Validation

All 90 public test groups pass in Debug and macOS ASan/UBSan; release packaging
passes. Private Debug and sanitizer LLDB bell sequences both exit successfully.

Synthetic checks cover both variants, private spawner order, atomic reservation
preflight/re-entry, pool retry, original impulses/damping/target changes, simultaneous
contacts, ring suppression, camera-relative rope/body geometry, SDK rotation,
inside status/control/protection, mist priority, invalid profiles/models/states,
draw-area preservation and the player's hand-input recovery.

A private LLDB component sequence starts with the original placed room-14 bells
and a protected 999-HP character. Original tile contacts trigger ringing and swing.
An explicit inside-position fixture verifies status and simulated-pad ownership,
then ordinary right input resumes after leaving. A diagnostic temporarily hides
the bell artwork and compares otherwise identical renders with the clipping
commands enabled/disabled: 637 player-area pixels differ. Original full bell
artwork remains enabled for the inspected capture. The sequence finishes at tick
111 with 28 regular rendered frames. These protected/positioned and body-hidden
checks are not ordinary traversal, a full tower replay or console parity evidence.
Dummy audio checks decoding/ownership rather than listening or PCM comparison.
An existing Wall/Library connector and ordinary save/Continue sequence also passes
against the shared renderer changes: 1,379 ticks and 344 rendered frames. Its saves
use a separate local fixture directory; existing user slots stay untouched. The
deliberately missing destination still preserves the current session and save owner.
