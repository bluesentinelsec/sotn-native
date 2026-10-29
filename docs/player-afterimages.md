# Original afterimage controller

Selected normal Alucard afterimages now run in the native session. The original
six retained samples drive three real global display slots, using the existing
common stage lease and private ARC artwork. The player captures before collision,
fades/selects displays afterward, and consumes actual later BlinkWhite reset intents.

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

All 155 tests pass in Debug, Release and macOS ASan/UBSan, alongside 34 Python
checks and release packaging.

Full original GPU contention, remaining status/script/palette effects and complete
Alucard/route/console parity remain incomplete. Ordinary starts grant no relics and
save/application versions remain unchanged.

## Shared stage lease

Native initialization now reserves the original eight linked primitive rows before
stage actors prime: six hidden afterimage metadata rows and two common capture
rows. They use the existing stage pool and one global player lifetime. The final
two rows are the same environment owner used by selected Wolf presentation; native
Wolf entry allocates its six root rows around that existing lease. There is no
second afterimage pool or later native Wolf-only environment allocation.

The shared update retains all eight rows before factory/child allocations. Commit
validates the entire plan before writing rows, rejects duplicate/colliding indices,
and prevents a live common lease from being detached or remapped by an effect.
A standalone Wolf commit cannot bypass that shared owner. Metadata rows stay hidden
and cannot be read/freed through stage-actor or Echo APIs, even with an equal numeric
generation. Rows alone confer no display texture or effect readiness; native player/display
ownership now connects their retained state and actual uploads.

Room entry retires stage actors while preserving these global player rows. Body
return retires transient Wolf parts/root effects while retaining all eight common
rows. Full stage initialization/Continue creates a fresh owner. Transient factory/
child occupancy masks exclude the permanently reserved global slots and rows.
The normal native presentation now uses its actual common target from stage start.
Original GPU environment allocation phases and complete raster contention remain
outside this selected software-target adaptation.

Public fixtures verify initialization ordering, eight-row capacity/exhaustion,
unchanged output on failure, common/stage ownership separation, malformed commits,
room-entry retention and actual Wolf entry/root/Blink/return allocation around the
lease. Private muted native checks cover ordinary Laboratory/Entrance starts,
normal jump, repeated/skipped presentation and explicit same-room reload; other
checks exercise twelve directed form conversions with both facings. Separate CMake
executable checks retain the lease through eight consecutive conversions, resource
spending, normal return and ordinary SDL quit. The form checks use explicit relic
and contact-protection fixtures, ordinary HP/MP/stats/enemies, and write no saves.
They do not establish uninterrupted route or console parity. Debug and ASan/UBSan
pass; runtime inputs/pixels stay private.

Provenance: selected `dra/alu_anim.c:{InitPlayerAfterImage,DrawPlayerAfterImage}`,
`dra/6DF70.c:{func_8010DFF0,EnableAfterImage,ForceAfterImageOn}` and previous-button
assignment and global initialization in `dra/692E8.c`, pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.

## Native player and presentation integration

The actual common stage generation binds the player's three initialized display
identities/slots. Existing public component callers remain unbound unless they
supply a real owner. Bound updates require a new engine-clock context; another
Tick on the same context fails without mutation. Context updates and room rebases
do not advance capture, fade or timer15. The selected original 16-bit timer15 loop
runs before movement, including the original poison-expiry request before its
own decrement; attack/crouch/status resets and later BlinkWhite child resets
retain their original update boundaries. Normal movement/jump/dive-kick can retain
afterimages; selected transformed, seated, hurt, dead, stone, grabbed and script
states suppress them. Full status recovery/palette and unintegrated spell/script
producers remain incomplete.

The native engine clock advances through pause, map, launcher and game-over updates.
Gameplay and afterimage cursors stay frozen while paused. Successful stage/session
replacement carries that current unsigned clock; it is not part of ordinary save
data. A fresh player/common lease initializes new displays on stage/Continue entry.
Rendering never advances either clock or cursor, and room/camera translation never
recaptures a pose at its later collision position.

Private display records initialize each slot's upload coordinates, flip and page.
Rendering decodes immutable ARC poses and uploads into those actual separate slots,
with original right/bottom margins and frame-load skips. Palette identities sample
current owned colors rather than storing old framebuffer pixels. Preserve the
source's distinct facing/slot-flip anchor rules, signed-word geometry, byte UVs,
culling after camera shake, original color modulation and STP-gated blending.
Shake enters packet projection once; later rasterization adds only the active target
offset. Equal-priority global displays draw in original slot order 3,2,1 after
later stage/weapon slots. Full ordering-table/GPU capacity contention remains unproven.

The 155 public tests include authored pre-collision samples, clock ownership/wrap,
reset/expiry, no-tick room rebases, status gates and atomic failure. Renderer cases
exercise separate slots, private descriptor flips, current palettes/modulation,
frame-load skips, culling and invalid metadata without publishing original profiles.
Private Debug/ASan traces match 4,734 supplied original RenderEntities contexts:
2,338 packed packets and 1,170 uploads, including CRC comparisons of actual ARC
uploads and right/bottom clear margins. They execute original packet/OT/metadata
instructions with explicit LoadImage/ClearImage driver adapters. They do not prove
complete GPU raster or console parity.

Private muted native checks verify twelve pre-collision captures, 678 contributed
pixels, stable repeated/skipped presentation, paused cursors with an advancing engine
clock, and attack/damage resets while ordinary development HP falls from 70 to 60.
Separate checks of the CMake executable verify 861 contributed pixels, pause/clock
and real ownership/input/damage, followed by ordinary SDL quit. Debug and ASan/UBSan
pass. They use the ordinary development character/enemies without relic, protection,
position, HP/MP or stat grants, and write no saves. In-memory ordinary-save and
checkpoint replacement checks cover fresh displays, retained gameplay ticks,
current engine clock and unsigned wrap; they do not write user slots or prove an
uninterrupted route. Transformation regression evidence remains scoped to its
explicit relic/contact fixtures, not ordinary acquisition or whole-form parity.
