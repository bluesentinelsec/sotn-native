# Chapel spike corridor

The internal [Chapel checkpoint](chapel-runtime.md) now runs the original spike
corridor controller, its owned damage actor, Spike Breaker tile destruction,
oriented fragments, dust and selected explosion cloud. Chapel travel, remaining
tower mechanisms, scripts and boss progression remain incomplete.

The controller scans three vertically adjacent tiles at the player's column in
the original eighty-tile-wide room. Collision types 244 through 247 identify
spikes. Without Spike Breaker equipped in the armor slot, the controller moves
its adjacent damage actor to the last matching tile. Otherwise it clears each
matching tile, reads the original private solid-neighbor orientation recipe and
spawns three fragments, dust and a cloud with the original sound. An unequipped
inventory copy does not protect the player. Exhausted optional effect pools omit
those effects while preserving tile destruction.

Spikes retain element/kind `0x51`. The selected environmental damage branch uses
maximum HP divided by eight, doubles that amount under poison, then applies the
original minimum-one rule. Ordinary armor defense does not reduce this loss.
Equipment element immunity/absorption still uses the existing prepass; ordinary
contact continues through its separate defense calculation. Complete damage-kind
reaction/effect and special-equipment parity remain outstanding.

Fragment orientation controls initial movement and rotation. Original owned RNG
consumption, gravity, collision bounce, side reversal, rotating fragments and
off-camera cleanup are retained. Dust uses the private stage sine table, original
quantized direction and animation. Original initialization, collision definitions,
orientation/animation tables and artwork remain private runtime inputs.

Native ownership protects the adjacent damage slot on re-entry and rejects
unrelated placements. Orphan damage actors cannot hit before retirement. Scans
outside room bounds during a crossing skip inaccessible rows instead of reading
outside the map; invalid private definitions and neighborhoods fail explicitly.
Destroyed spike tiles are transient room state, as in the selected reference.
Ordinary v8 saves retain equipped armor through the existing character fields;
they do not persist cleared spike tiles or transient effects. No save-format change
or grant of Spike Breaker to ordinary characters is introduced.

## Validation

All 88 public test groups pass in Debug and macOS ASan/UBSan; release packaging
passes. The private Debug and sanitizer LLDB runs both exit successfully.

Synthetic checks cover the three-tile scan and final damage position, collision
type bounds, environmental damage/poison/minimum/immunity, equipped versus stored
armor, private orientation selection, optional pool exhaustion, debris RNG/motion/
bounce/reversal, dust direction/lifetime, room edges, ownership/re-entry/orphans,
generation capacity and malformed private definitions.

A private LLDB component sequence starts in original room 1 with granted Alucard
armor: defense 20, HP 70. Spike contact removes 8 HP. After granting/equipping Spike
Breaker it walks right past pixel 400, clears at least sixteen original spike
tiles, observes all three fragment/dust/cloud families and takes no further damage.
A fresh session restores all 110 original spike tiles, then accepts the retained
equipped character. The sequence finishes at tick 213 with 54 rendered frames.
A captured destruction frame was visually inspected. This explicit start and
granted-equipment fixture is not ordinary progression, a full-corridor replay or
console audiovisual/timing parity evidence. Dummy audio checks decoding and
ownership rather than listening or PCM comparison.
