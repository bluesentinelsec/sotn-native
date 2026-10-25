# Chapel Bone Pillar skulls and fire

The internal [Chapel checkpoint](chapel-runtime.md) now runs selected original
stacked Bone Pillar skulls, side-dependent fire attacks, embers and delayed death
pieces. The [spike-ball variant](chapel-spike-ball.md) is also integrated. Connected Chapel
travel, remaining encounters and tower/Maria/boss progression remain incomplete.

The bottom skull grounds through its private sensor table and creates an adjacent
owned top skull. The top and bottom retain different frame/hitbox profiles and
attack toward opposite sides. Near/far thresholds, interval cycling, side-dependent
countdown, mouth animation and palette flashing follow the selected original path.
Only the original animation-entry cursor emits a fire attack. The native loader
respects alignment padding between the private animation tables.

Fire uses the original directional quadrilateral, private sprite UVs, separate
contact hitbox and sound. Its animation ends before damage stops; the following
update replaces it with sixteen additive one-pixel embers. Original RNG order,
per-ember clock cadence, color decay and 48-tick lifetime are preserved. Optional
primitive exhaustion omits the effect. Original artwork/tables remain private.

Destroying a bottom skull spawns its two delayed pieces and releases only the
matching owned top skull. The released skull preserves original rolling rotation,
radial collision probes, gravity and bounce; another ordinary attack turns it into
a fireball explosion. Pieces wait, fall, settle and wait again before becoming
small explosions. Invalid profiles, child reservations and motion fail explicitly.
Transient actors do not enter ordinary saves; the v8 save format is unchanged.

## Validation

All 84 public test groups, macOS ASan/UBSan and release packaging pass. Original
synthetic fixtures cover both attack sides, side-dependent waits, near/far resets,
fire geometry/UVs, ember timing/colors/blend, death offsets, upper-skull release,
bounce conversion, piece delays/settling, allocation exhaustion and invalid input.

A private LLDB fixture uses the original placed room-11 skull pair from each side,
with a protected 999-HP/high-strength character. It observes original attacks and
embers, positions a fire hitbox to verify damage, then uses ordinary attack inputs
to kill the base and released upper skull, disabling subsequent contact while
positioning the attack targets. Each side deals 23 HP of fire damage and completes
the kill sequence at tick 271 with 108 rendered frames in Debug and ASan/UBSan;
the sanitizer fixture also verifies death-piece retirement. Controlled starts and actor positioning
are component checks, not an ordinary route, full encounter or console parity replay.
Dummy audio validates decoding/ownership, not listening or original PCM comparison.
