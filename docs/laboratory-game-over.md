# Physical death and game over

Lethal physical contact now takes Alucard through the selected original death
path instead of returning an application error. The player stops accepting
movement/attack actions, launches away from the hit, runs animation 0xC0,
changes tint/rotation, and dissolves over the original 64-tick mask sequence.
Recovery immunity prevents repeated damage; dying players cannot collect prizes.
The later [native physical/elemental owner](player-physical-death.md) replaces the
actual app's CPU mask with original normal/fire/thunder/ice effects, resident
capture and64 actual uploads. The older selected standalone mask and evidence
below do not establish those owners; remaining interrupted/fade contexts stay open.

The scene then replaces stage music with the original US game-over track and
shows the original game-over artwork. The captured scene falls away in columns,
the title reveals, and the presentation holds/fades. Enter or Xbox Start advances
after the reveal. HUD and presentation animation advance on simulation ticks,
not draw calls. Full GTE rasterization, shared global RNG consumption, effect
ordering, and original timing parity remain unproven.

The current checkpoint ends successfully after the fade: original title/file
selection menus and full progression are not integrated. It does not invent an
automatic respawn or claim that the original title flow works. The [native save-room checkpoint](laboratory-save-rooms.md) supplies explicit
`--laboratory-continue SLOT` loading on the next launch.

The additional private input is `BIN/F_GO.BIN`, 114,688 bytes,
SHA-1 `fa0556554d4b4e68410cf0270cb826b4a42f36ff`. Its artwork and palette bytes
are parsed at runtime; public builds contain none of them. The original music
plays from the already verified raw music container. Its selected one-shot path
returns silence after completion. All owned decoder streams are destroyed before
releasing the playback service’s final SDL audio-subsystem reference.

The selected death logic adapts `src/dra/71830.c:PlayerStepKill`; the game-over
column/reveal logic adapts `src/dra/game_handlers.c:HandleGameOver` and
`func_800E5358`, at the pinned reconstructed revision under AGPL-3.0-or-later.
CPU sprite transforms replace GTE services and retain explicit parity limits.

Synthetic tests cover lethal damage, immunity/input locking, atomic failure on
bad death animation data, full mask coverage, tint/visibility, game-over reveal,
Start and timed completion, render-independent state, invalid resources, and
one-shot music/device cleanup. A native LLDB test took seven ordinary enemy hits
from 70 HP through death, game over, and successful cleanup. A separate targeted
fatal-damage run under ASan/UBSan also passed. Private captures remain ignored.
These are exercised-path checks, not complete original-game parity.

The application now returns to the [native Continue shell](native-session-flow.md)
after the fade, rather than exiting. Original title/card menus remain incomplete.


When the actual shared CD owner is bound, native physical/water death128 now enters
its original selected audio lifecycle. The first engine frame queues81, and the
following Init frame queues11. The wait uses actual pause/restore acknowledgement
and XA activity (pending fade or playing identity). Queue-empty remains a separate
snapshot. On success it queues12/B; the following MuteSound retires actual current,
saved or in-flight transports and resets original metadata. The next resource phase
queues original US33e through real configuration, ReadN and playback ownership.
The existing game-over artwork is already verified/resident; its presentation clocks
stay held through these phases and start while command4 is still configuring. No
replacement fade/readiness timer or legacy music decoder is introduced.

Bound game-over frames continue actual SPU command, voice/DSP and device updates.
Player/game/stage clocks keep their existing holds and HP stays zero. Drawing does
not advance audio or presentation. Mute runs after actual SEQ closure, retains live
voice/envelope/register/DSP/RAM state, copied scripts and the original reverb restore
byte, and clears the original command/effect queues, current script/SEQ metadata and
abandoned bookmarks. It resets master127/multiplier32 without a sample tick. The
existing unbound game-over path retains its separate legacy behavior.

Authored tests cover closed-SEQ validation, failed preconditions without mutation,
actual81/11/B/12/16 work, retained SPU/copy owners, pending-effect/attachment reset,
active/in-flight CD cancellation and a subsequent real33e read. Four private-data
command contexts compare1,040 updates,36 requests,12 mute boundaries and both full
256-entry queues against original instructions, including live/copied script words,
SEQ access/attachment/volume, pending-effect buffer clearing and actual SDK RR/master
MMIO. These command-only checks use explicit CD/SDK playback/key-latch adapters and
do not advance DSP or SDK SEQ scheduling. Separate32 complete original engine/handler
calls verify successive81/11, acknowledgement/fade/playing gates independent of queue
count, following12/B and Mute, and four resource-load gates before33e. Capture,
entity/GPU cleanup, loader, sound and mute callees are explicit adapters in that check.

The actual Debug/ASan native checks each cover physical, water and silent-music deaths through
resident capture/decay and game-over completion. Each passes1,226 SPU/device frames,
984 presentation updates, three metadata resets retaining complete SPU state and
1,418,757 nonzero game-over PCM samples. An actual old BGM bookmark/speech context
restores before shutdown. The silent context reaches the source acknowledgement gate
with a nonempty CD queue. Producer frames are checked directly; five repeated draws
per context hold clocks, and actual Start input finishes the reveal/fade. Card/load,
protection, lethal contact, HP1/water-result and stage-audio fixtures remain separate
from ordinary acquisition, water routes and console evidence. All six final CD/rescue/actual-app regressions pass with normal exits and no
sanitizer errors; exact-head platform results accompany the PR.

This owns selected bound audio transitions. Unbound bootstrap/game-over, original
whole-scene/entity/GPU cleanup, original shared RNG/raster/SDK timing, session/title
continuity and the remaining [milestone gates](alucard-status.md) remain unfinished.
