# Laboratory movement integration

`--laboratory` runs this repository's C executable, built through its normal
CMake/Make entrypoint. It loads original room tables, collision definitions,
animation tables, palettes, and sprite pixels from the user's private US dump.
No reference executable, MIPS interpreter, generated game arrays, or private
assets are linked into the app or copied into packages.

## Current behavior

The checkpoint starts normal Alucard at development room 1, position (32,135),
or the verified room-3 corridor start with `--laboratory-room 3`.
It supports standing, looking up, walking, turning, crouching, jumping, falling,
landing, one-way platform drop-through, slope/wall/ceiling collision, and
transitions between adjacent Laboratory rooms. Motion uses original 16.16
fixed-point values and sensor rules; animation uses the private DRA tables.
The native session renders the existing tile/sprite pipeline through OpenGL.

The [actor checkpoint](laboratory-actors.md) now adds selected original room
objects and Bloody Zombie active motion/animation to this movement foundation.
Original game startup, complete stage actors/scripts, combat, equipment, damage,
HUD, audio, saves, transformations, and full-game progression remain incomplete. Reaching an
unsupported external stage/boundary fails with a clear message. Movement
support does not establish parity for the full original player state machine.

The camera centers/clamps to the current room. The clock advances at 60 ticks
per second, independent of presentation, with at most eight catch-up ticks;
focus loss or a debugger gap over 250 ms freezes it. These are development
adapters. Original camera, scrolling transitions, NTSC timing, layer/entity
composition, and audiovisual parity remain acceptance work.

## Inputs and playtest

Use the five [core files](private-data.md) and the three verified
[graphics resources](laboratory-presentation.md) from the same supported US dump.
Keep them outside the repository or in ignored `local/game-data/us`.

```sh
make && lldb -o run -- build/debug/bin/sotn-native.app/Contents/MacOS/sotn-native --game-data /path/to/private/extracted/us --laboratory
```

On Linux, substitute `build/debug/bin/sotn-native`; on Windows, substitute the
generator's `sotn-native.exe` path. The command launches this project's app.
On a crash, run `thread backtrace all` in LLDB; `quit` leaves the debugger.

| Action | Keyboard | Xbox layout |
|---|---|---|
| Move / look / crouch | Arrow keys | D-pad or left stick |
| Jump | X or Space | A |
| Quit | Escape | Window close |

Z/D and Xbox X/B now run empty-hand attacks; see [combat integration](laboratory-combat.md).
Ability controls remain unsupported and warn once without ending the session.
The core rejects unsupported raw input from direct API/replay callers.
Optional controller-open failures retain
keyboard control and log a warning. Rebinding and vibration remain future work.

## Source and boundaries

`modules/core/player.c` adapts normal-form paths from the pinned reconstructed
C reference's `src/dra/6DF70.c`, `6E42C.c`, `6FDF8.c`, `71830.c`, and `692E8.c`.
`player_sensors.c` adapts floor/ceiling/wall sensors from `6BF64.c`, tight-space
detection from `70394.c`, and normal-form `CheckStageCollision` from `692E8.c`.
Both retain AGPL-3.0-or-later provenance; see [third-party notices](../THIRD_PARTY.md).
Transitional sensor names preserve the source mapping for review.

`SotnPlayer` owns its state and borrows immutable room and DRA views. Each tick
uses a candidate state and commits only on success. Map extents, tile IDs,
start positions, animation access, and room rebases are checked. Explicit
16-bit wrap behavior preserves original sensor stores without signed overflow.
Unsupported original movement families are not presented as implemented.

The original project code in `modules/game/` owns the session, graphics
resources, input, development camera/clock, and room rebasing. It is separate
from the reconstructed simulation and the thin `src/main.c` callback entrypoint.

## Validation and remaining acceptance

Public synthetic tests cover fixed-point walking/jump release/landing, crouch
recovery, direction priority, walls, deterministic replay, room rebasing,
unsupported input atomicity, malformed data, and SDL virtual-controller
mapping/dead zones/disconnection, ignored attack/ability inputs alongside
movement/jump, and the single-session warning. They need no game assets.

Local checks against unchanged pinned collision/sensor routines matched
131,072 synthetic cases spanning all 256 collision codes, tile positions,
standing/crouched sensors, contacts, and varied velocities. This validates the
adapted sensor path; it is not a full movement or game parity result.
A private 300-tick movement/jump sequence ran through our native session and
transitioned from room 1 to room 2. Actual framebuffer captures were inspected
locally and remain ignored, alongside all private inputs and test tools.

Before calling this the first authentic playable project milestone, integrate
combat, remaining entities/effects, audio, original timing/camera and entry scripts, then compare
a repeatable sequence with the original. Broader progression and ordinary
save/load follow. Human playtesting of this checkpoint evaluates its limited
movement and rendering; it cannot establish those missing features.
