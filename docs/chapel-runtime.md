# Royal Chapel runtime foundation

Royal Chapel (DAI, stage 6) now has an internal native resource profile, selected
shared services, Winged Guards and non-inverted Bats. Connected travel remains
disabled while its other encounters, mechanisms, Maria event and boss are missing.
This is not a playable whole-stage or complete-game milestone.

The profile validates 24 sprite banks, original actor IDs below 0x49, sixteen
permanent prize entries with flag base 0x98, three red-door layouts, seven camera
configurations and ordinary save rooms 16/18. Shared pickups, doors, explosions
and camera locks use Chapel tables. Its camera uses the original immediate-lock
variant, independently of Library's interpolation. Unmapped shared table requests
fail explicitly instead of reading another overlay's data. Unknown actors remain
explicitly marked and omitted in this internal checkpoint.

Winged Guards retain their original initial facing, fixed-point horizontal flight,
SDK sine-based vertical velocity, flap timing, positional sound and death. Their
eight independent pieces retain source velocity/facing/priority, whole-pixel
initialization, private launch offsets, rotation, gravity and occasional explosions.
The original spawner uses its private world-coordinate ranges, bitwise randomized
wait and four-slot global 168–171 pool (native slots 104–107). Pool exhaustion
omits optional spawns/effects. Bats use Chapel initialization and drop/flight
animation tables, proximity wake-up, fixed-point drop/wave and positional death
sound. Shared combat now recognizes these two Chapel body identities.

All original tables, artwork, sound banks and music remain private runtime files.
The canonical catalog already fingerprints DAI.BIN/F_DAI.BIN. Optional on-demand
ST/DAI/SD_ZKDAI.VH and SD_ZKDAI.VB fingerprints were checked byte-for-byte against
the user's local disc extents; metadata alone enters the public manifest. Original
Requiem for the Gods intro/loop tracks use the existing private music stream.
Public builds and packages require none of these inputs. Version-eight ordinary
save encoding accepts the two Chapel save rooms without changing its layout or
granting connected travel.

## Validation and remaining work

The public synthetic `chapel-actors` group covers initialization, flight/flap,
death pieces, bounded spawner ranges/pool exhaustion, Bat phases, unsupported IDs,
malformed ranges/velocities and motion overflow. Music tests exercise the Chapel
track selection; ordinary-save tests round-trip its supported save-room records.
All 78 test groups passed in Debug and macOS ASan/UBSan; release packaging passed.

A private LLDB native check starts rooms 2 and 13 explicitly. It uses a protected
999-HP/high-strength character, an inserted Bat, the original placed Winged Guard
spawner and positioned contacts. Ordinary attack inputs complete both deaths,
including eight Guard pieces, across 56 rendered frames. Both Debug and sanitizer
checks must exit successfully. Dummy audio exercises ownership/decoding, not an
auditory or console-PCM comparison. These fixtures are not ordinary traversal,
whole-stage, uninterrupted-route or console parity evidence.

Next integration includes remaining Chapel enemy families, lamps, staircase and
other mechanisms, scenery, original encounters/progression and connected-route
validation. Full effects/audio/timing parity and the wider game remain incomplete.
