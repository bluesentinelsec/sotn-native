# Chapel tower exits

Selected Royal Chapel tower doors now change rooms inside the overlapping tower,
skyway and corridor bounds. Chapel remains an internal development profile;
connected castle travel stays disabled until its remaining scripts and boss work.
This is not a complete stage or game milestone.

The adapter follows US `st/dai/e_tower_exit.c` and
`dra/5087C.c:{Engine_Normal,func_800F0CD8,SetNextRoomToLoad}` at the pinned
reconstruction. All fifteen exit variants retain their strict X thresholds,
48-pixel vertical interval, initialization delay and four successful updates
before dormancy. Both pre-movement and predicted post-movement X must pass the
threshold. Dead-player contact stops horizontal velocity without changing vertical
motion, death state or the window.

The original mutates tilemap left/right cells and horizontal limits. Some left
exits also subtract a screen width from its moving room X. The native service
keeps those limits in a transient copied window while preserving its stable
physical player/map coordinates. The paired origin changes cancel in world space;
they are not a physical player shove. Neighbor lookup uses the adjusted boundary
cell, then original room-header order, including boss overrides before ordinary
rooms. Unsupported stage destinations remain explicit failures.

For Chapel, stage actors update after player movement and before the same tick's
room lookup. The departing actor/audio requests complete before new room entry.
Destination initialization runs before its first draw or player collision, without
a second projectile update, duplicate hand-attack sound or second audio synthesis
frame. Its initial audio requests queue for the next synthesis frame. This native
entry boundary still needs full transition/audio/camera timing comparison against
the console. Other stage update ordering remains unchanged.

Room entry resets every window field and the saved pre-movement coordinate.
Malformed limits fail before arithmetic or output mutation. An internal horizontal
exit with no valid destination fails explicitly. Early sealed vertical probes keep
their existing collision behavior. The owned collision map remains full-sized;
no private tile/layout tables enter public code. Windows are transient and require
no ordinary-save format change (v8) or equipment/progression grants.

## Validation and remaining work

Public synthetic tests cover every exit variant, strict threshold/equality and
height edges, old/current movement ordering, initialization/dormancy, shifted
origins, neighbor probes, dead horizontal stopping and room-entry reset. Shared
route tests compare default-window/ordinary edges and reject malformed inputs
without changing outputs. Player tests retain living/dead vertical motion.
All 91 groups passed in Debug, macOS ASan/UBSan and Release; packaging passed.

A private native LLDB sequence crosses tower 1 to the skyway and back, then to
the spike corridor and back: four actual room changes over 77 ticks and 19 rendered
frames. Each crossing checks same-frame routing, fractional/world X continuity,
horizontal momentum, fresh destination initialization and no immediate bounce.
It uses explicit room starts, a protected 999-HP character and granted Spike
Breaker armor. Those fixtures are not ordinary traversal, all-fifteen-door route
coverage, an uninterrupted castle replay or console parity evidence. Dummy audio
exercises ownership and decoding, not listening or PCM parity.
A shared private regression also passed three Wall/Library connectors, isolated
ordinary save/Continue and failed-arrival owner retention (1,379 ticks, 344 renders).
Existing user save slots remain untouched.

Chapel Maria, Hippogryph, scenery, connected progression and whole-route testing
remain incomplete. Full transition/camera/effects/audio timing parity remains
unproven; do not expose a geometry-only public Chapel route on this evidence.
