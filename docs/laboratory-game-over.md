# Physical death and game over

Lethal physical contact now takes Alucard through the selected original death
path instead of returning an application error. The player stops accepting
movement/attack actions, launches away from the hit, runs animation 0xC0,
changes tint/rotation, and dissolves over the original 64-tick mask sequence.
Recovery immunity prevents repeated damage; dying players cannot collect prizes.
Elemental deaths and all related primitives remain incomplete.

The scene then replaces stage music with the original US game-over track and
shows the original game-over artwork. The captured scene falls away in columns,
the title reveals, and the presentation holds/fades. Enter or Xbox Start advances
after the reveal. HUD and presentation animation advance on simulation ticks,
not draw calls. Full GTE rasterization, shared global RNG consumption, effect
ordering, and original timing parity remain unproven.

The current checkpoint ends successfully after the fade: original title/file
selection menus and full progression are not integrated. It does not invent an
automatic respawn or claim that the original title flow works. Save-room loading
is a separate integration step.

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
