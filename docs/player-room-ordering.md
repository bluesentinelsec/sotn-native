# Selected room callback and effect cleanup ownership

Ordinary native room lookup now follows the departing player, DRA/shared effect,
stage, HUD and audio updates. This extends the ordering previously selected only
for Chapel. Actual spell freeze and a committed teleport warp preempt the lookup.
Destination initialization adds no second player, shared effect, gameplay-clock or
audio synthesis update; its sound requests wait for the next audio frame.

After choosing a real neighboring room, the native owner prunes selected dissolve32,
ice33 and teleport65 children before destination actor allocations. Dissolve and
teleport have no20000 retention flag. Attached ice retains its real24 row indices;
detached ice retires its actual rows. Selected pending factories39/46/121 follow
original cold-root and initialized-origin retention tests. Cleanup validates a
copied runtime and commits the existing stage-pool row plan before replacing the
live runtime. The copy is a transaction, not another entity or primitive pool.

A raw contact sampled after the departing player update belongs to the old room.
Successful room entry clears that queued contact before collecting destination
contacts. It preserves pending healing, all16 player words, HP, RNG and player
frame. The next original hit-detection phase belongs to the destination. This
native bridge does not change contact priority or player-success commit ordering.

The private comparison executes1,056 complete original room-cleanup calls across
all48 selected child slots, dissolve phases, attached/detached ice, all teleport
phases and pending/initialized factory roots. Exactly48 attached ice contexts retain
their20000 owners; unrelated retained children survive. Debug and ASan traces match.
Entity destruction is an explicit service adapter; those comparisons alone do not
establish native row allocation or frame ordering.

Native and actual application checks supply explicit effect/contact, protection,
neighbor-edge position and camera-history contexts. They distinguish one old-room
callback from no-tick cleanup, including a pending121 factory's96 actual birth RNG
calls before its36 rows retire. Attached ice retains its24 leases and nonposition
state after the old callback; dissolve and detached ice retire. A placed Library
ghost supplies a real raw contact after the old player update, then actual entry
clears only that contact. These are scoped ownership checks, not uninterrupted
traversal, item acquisition or console comparisons. Unsupported stage actors remain
outside their evidence.

All213 authored tests pass in Debug, Release and ASan. All ten native crossing
protocols and ten actual CMake application protocols pass with normal exits. Both
native placed-ghost and cleanup-rejection checks pass; a stale actual target mark
rejects the row commit without changing the live runtime, rows or player. Actual
ordinary Summon holds lookup through33 frozen updates and release, then the next
ordinary callback crosses; an actual terminal teleport warp preempts the neighbor.
Library Card travel/menu/white-frame/ACK checks pass in both native app builds.
Connected-stage walking, isolated save/reload and an ordinary placed-enemy death
through resident capture/64 uploads/game over also pass. The connector check uses
explicit development entries and retained flag/relic fixtures; it is not an
uninterrupted acquisition route.

Other shared child/root cleanup, full room loading/fade/engine timing and whole
native HP/map/clock/audio rollback remain unfinished. Default grants and savev9
are unchanged. All six [Alucard milestone gates](alucard-status.md) remain open;
this ordering is a dependency for further ordinary consumables, not evidence that
they are available.

Provenance: `dra/5087C.c:RunMainEngine`, `dra/7879C.c:func_8011A9D8`,
`dra/5087C.c:func_800F0CD8`, pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
