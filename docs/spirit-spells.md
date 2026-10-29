# Spirit spell components

The core now reconstructs the shared portal, spirit flight/trails and target
selection used by Summon Spirit and Tetra Spirit. These are component services.
The pure components enable no additional spell controls and own no factories,
combat, framebuffer capture or scene freeze. The [shared runtime](shared-spirit-runtime.md)
now supplies selected actual factory/child/target/combat ownership, and the
[playable integration](player-spirit-spells.md) binds native capture/presentation/
freeze. Actual native owners must connect those boundaries before enabling further
spell availability. Selected Dark/Summon/Tetra now have scoped gameplay owners.
The [casting/freeze foundation](spirit-casting.md)
supplies selected player states and engine boundaries.

The portal retains its original ten-update delay, nine-row allocation request,
63×63 capture into the shared target at 512,448, private outer shape and eight raw
textured wedges. It preserves screen-word positions and birth fractions, player
height selection, SDK rounding, front/back priorities, eleven-update expansion,
actual factory requests and shrinking retirement. The returned freeze/flash,
capture, sound and factory intents are requests, not completed operations.
The owner must also draw the original black rectangle from the ninth row.

The original allocation-failure branch destroys the portal without releasing its
scene freeze. The component preserves that result. An eventual native owner must
handle this failure explicitly; silently releasing the freeze or inventing capture
readiness would change the original path. Secure bounds checks replace unsafe
memory access, not the original lifetime or allocation result.

A spirit requests sixteen actual rows, with eight metadata/trail pairs. Successful
birth requests the appropriate original spell initialization, launches immediately
and captures its first trail before the later animation update. Flight retains
signed-word wobble/age/angle behavior, fixed-point wrap, original upward departure,
turn-rate thresholds, shortest-turn comparisons and fallback targets. Losing a
retained target moves into departure without performing that later state's work
on the same update. Target acquisition remains an explicit owner request.

Every trail retains its captured position/angle while shrinking and fading through
the original eight-entry cursor. Packet geometry preserves nested arithmetic
shifts and SDK quantization. Birth sprite zero reads the two bytes immediately
before the original UV array. The service reads those validated private-image
bytes with bounded offsets; it does not introduce a C out-of-bounds access, guessed
sentinel or `FIX_UB` artwork change. The later owner supplies actual animation,
body artwork, rotation, private spell/critical metadata, RNG, attack groups,
cooldown, real row ownership and native enemy damage/death/EXP.

The target service examines exactly 128 original stage slots. It first marks and
returns the first eligible unmarked actor without changing the global cursor.
Once all candidates are marked, it searches circularly and advances the retained
cursor. It preserves original identity/hitbox/flags, screen-edge and HP tests,
including their inclusive boundaries. It consumes no RNG. The owner commits a
copied mark against the same actual slot identity/generation; later steering must
represent the original retained slot honestly, including its identity-zero test.
It must not silently substitute another target-selection policy.

Before live integration, preserve the original freeze behavior: advance only the
selected player animation, real player children and eligible stage updates while
ordinary player motion, collision, resource timers and game time remain frozen.
The engine clock and palette jobs continue. Stage initialization during freeze
has its own original allowance; do not merely stop every actor. Actual capture
must use the ordered framebuffer and shared target, not a placeholder image.
Full allocation/group/capture contention and audiovisual/frame timing remain
unproven. Ordinary starts, saves and live ability availability remain unchanged.

Public authored tests cover target filtering/marking/cursor progression, both
facings, successful and failed row requests, captured fractions, trail birth UVs,
retirement and target loss, no-tick translation, delayed capture, both factory
paths, the retained freeze on exhaustion and malformed-data rollback. Fixtures
use authored geometry and synthetic SDK samples, not original game tables.

Private comparisons match 28 portal contexts/630 full lifecycle/capture/request/
SDK packet updates, 96 spirit contexts/17,280 motion/target/trail packets and 576
complete original 128-slot target searches. Spirit comparisons include supplied
signed-word/fixed-point boundary states and original adjacent birth UV bytes.
Actual SDK instructions execute. Portal allocation/destruction, capture, factory
and sound calls use explicit boundary adapters; spirit attack initialization and
target acquisition use separate adapters. The target-search comparison executes
the complete original selector without helper adapters and retains stage RNG.
Debug and ASan/UBSan snapshots match. This is not native spell, uninterrupted
route, full-frame, Alucard or console parity evidence.

Provenance: selected `dra/84B88.c:{func_80129864,EntitySummonSpirit}` and
`dra/7879C.c:func_80118970`, pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
All original shape, UV, launch-angle and SDK tables and traces remain private.
