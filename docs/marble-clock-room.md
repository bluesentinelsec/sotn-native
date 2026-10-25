# Early-route Marble clock room

The native Marble clock room now presents its original hands, birdcages, statues,
selected shadows, gears, floor doors and nearby pendulum. The stage actor controller
stops Marble music on entry; leaving the clock room restores the stage track.

The left statue opens on even game-time minutes and closes on odd minutes when
Alucard is below the original height threshold. The right statue responds to a
new stopwatch interval and closes after it ends when Alucard is below that threshold.
Both retain the original entry-from-above rule. Moving statues update their collision
tiles and player contact over the original 96 ticks. Birdcage transitions retain the
original four 64-tick phases. Hand positions follow game time independently of
presentation rate. The original US half-hour bell expression remains unchanged.

The actor service now receives full elapsed simulation ticks, preserving seconds
and frame-within-second for clock sounds. Hourly bell scheduling, distant tick
volume/pan, and the pendulum's selected motion/sound path run against private data.
The sound service carries explicit volume and signed stereo distance through direct
and scripted events, using the original private pan coefficients and SDK channel
orientation. Explicit zero opacity now remains representable for selected shadows.

The central floor uses its original persistent CEN_OPEN flag, retained by existing
version-six saves. Loaded open/closed states and selected floor opening movement are
supported. The later ring ritual remains unintegrated and fails explicitly if its
original equipment condition is met. Underground switches, complete primitive
priority ordering, camera shake, the ritual's effects, and full audiovisual/timing
parity remain incomplete. This is an early-route checkpoint, not complete clock-room
or castle progression.

## Validation and provenance

Synthetic tests check statue tile transitions, hand positions, birdcage timing,
floor restoration, shadow opacity, pendulum motion, invalid indices/time bounds,
and distant sound metadata. Audio tests exercise direct/scripted left/center/right
output, silence, malformed coefficients and queue bounds. Save tests retain the
floor event across supported stage profiles.

A private native LLDB fixture changes game time, supplies a stopwatch, verifies
both statue transitions, renders the room, and walks/jumps out through its original
right exit with stage music restored. It uses an explicit room/time fixture and a
protected player; it does not establish a continuous route or console parity.
Dummy audio validates decoding rather than real-device listening.

Adapted from `st/no0/clock_room.c`, `st/clock_room_entities.h`, selected
`st/no0/42A34.c` paths, `dra/sound.c` and SDK key-on volume/pan rules at the pinned
reference revision. Actor/audio algorithms retain their documented AGPL/MIT provenance.
No original position, animation, tile, instrument, pan or graphics tables are embedded
in public code; users supply their private runtime data.
