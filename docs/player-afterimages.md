# Original afterimage controller

The selected controller reconstructs the original six retained samples and three
alternating display states. It does not yet run in gameplay. Actual entity/primitive
ownership, movement/reset hooks, frame-clock ownership, private ARC texture uploads
and rendering must connect before enabling normal Alucard afterimages.

Capture runs at the original pre-collision point. It keeps the captured signed-word
position, frame-load bit, palette and facing, and samples once every three eligible
updates. New game-button input, release of the previous complete button word and
falling faster than half a pixel restart the activity cursor. Original Start/Select
words do not restart it. Excluded animations and disabled/expired cursors retain
original early returns; byte timers wrap rather than saturate.

Presentation runs after collision. All six samples fade using private DRA tables;
system-clock parity chooses one sample from each pair for the three display slots.
Captured positions replace only the high words, retaining display fractions. Preserve
original red subtraction, blue increment/overshoot, blend threshold and retired
frames. Metadata green remains unchanged while displayed red/green share the faded
red value. Repeated rendering must not run capture or presentation again.

Reset can clear frames without changing retained colors, positions, capture cadence
or the table timer. It disables capture and requests the original player timer 15;
enable clears reset/disable flags, while force changes only the activity index.
Room rebases move retained metadata and display positions with signed-word wrap and
preserve fractions without advancing either update. These copied cursors confer no
factory, child, row, texture or readiness ownership.

The player also retains the previous complete button word instead of its intersection
with current input. This restores the original Gravity Boots requirement for neutral
input before the initial Down. A directional release immediately into Down no longer
starts that command; a neutral update before Down does. Existing airborne-command
coverage now supplies that required neutral update explicitly.

Public authored fixtures cover capture cadence/ring wrap, activity expiry, menu
exclusion, directional release, excluded animations, byte-timer wrap, alternating
slots, color thresholds/overshoot, fraction retention, reset/force/enable, duration
word wrap, no-tick rebases and atomic invalid-input failures. Tables are authored;
no original timing/color values are published. Gravity Boots tests cover previous
Left/Right/Up and a subsequent clean neutral-start command.

Private muted LLDB comparisons match 4,096 supplied original capture, fade, reset,
enable and force instruction contexts without adapters. Debug and ASan/UBSan traces
match. They supply original linked metadata rows, player/display state and the
system-clock word; they do not execute a native gameplay frame or console replay.
Private tables, traces and input binaries stay ignored.

All 153 tests pass in Debug, Release and macOS ASan/UBSan, alongside 34 Python
checks and release packaging.

Full shared resource ownership, live afterimages, all-ability contention and complete
Alucard/route/console parity remain incomplete. Ordinary starts grant no relics and
save/application versions remain unchanged.

Provenance: selected `dra/alu_anim.c:{InitPlayerAfterImage,DrawPlayerAfterImage}`,
`dra/6DF70.c:{func_8010DFF0,EnableAfterImage,ForceAfterImageOn}` and previous-button
assignment in `dra/692E8.c`, pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
