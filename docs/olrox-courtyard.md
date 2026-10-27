# Olrox's Quarters courtyard presentation

The NO2 courtyard now presents the original house models, repeating backgrounds,
foreground ornament, backdrop, linked lamp flames and cycling glow. The glow
retains its camera/player trigger and gradually increases the red channel of the
original seven palette rows. These services use NO2's own identities and private
models, placements, animation streams, colors and lookup tables.

House spawners create up to eleven bodies in the original effect-slot order.
Each house owns sixteen panels. The renderer projects nineteen private vertices,
culls faces by winding and draws the split facades, sides and roof with original
UVs, colors and ordering priorities. The model's Y rotation uses the SDK matrix
pair table and its bounded perspective divider; do not substitute the separate
quarter-wave table. Screen saturation and signed midpoint division preserve the
selected original behavior. Houses outside the original horizontal range hide
all their panels. Optional slot/primitive exhaustion follows original omission
or removal; room cleanup releases owned children and primitives.

The repeating background's screen-space grid remains fixed while its placement
controller stays alive. Its quads intentionally overlap; do not anchor them to
the controller's world position or replace their original spacing with a host
texture repeat. Foreground ornament and the original dark RGB backdrop now run.

Lamps watch their original breakable slots. An intact lamp loops its private
animation; losing the breakable resets the animation and requests five selected
explosions. RNG advances only for allocated children. A lamp first created after
its breakable is absent uses the extinguished frame without replaying that burst.
These are transient room relationships, not new permanent pickup/save flags.

The glow cycles its seven private palettes every two ticks. Once the original
camera/player condition is reached, it increments RGB555 red every sixteen
ticks, saturating at 31. A copied command selects the seven palette rows; only
the graphics service mutates its owned GPU-memory representation. It validates
all rows before writing and preserves color zero, green, blue and mask bits.
Borrowed input files remain immutable. Pause freezes these simulation services.
No ordinary-save format change is required.

## Validation and scope

Synthetic public tests cover houses and face emission, offscreen hiding,
malformed model pointers and unchanged projection output, one-shot spawning,
slot/primitive exhaustion, backdrop/grid geometry, lamp breakage, palette cycling,
trigger timing, queued-command cleanup, red saturation, transparency/mask bits,
invalid/duplicate palette rows and atomic graphics updates. Public fixtures are
authored without original tables or artwork.

Private comparisons check 61,750 projected vertices against the pinned SDK
reference, 17,952 root/visible house-panel snapshots against original US
instructions with isolated initialization/allocation and SDK adapters, and
420 glow timing/palette-request snapshots against original US instructions.
The latter uses an isolated LoadClut adapter; complete GPU/PCM/frame parity is
not established by these component comparisons.

Muted native Debug and macOS ASan/UBSan LLDB checks begin in courtyard room 8,
move right, break a placed lamp with ordinary Gladius attacks, render houses and
advance ten palette updates. The fixture supplies an explicit room start and
contact protection, with no stat, relic, equipment or ongoing position grants.
Equivalent CMake-app debugger checks use the same original-input sequence and
normal SDL quit. These explicit starts are not ordinary castle entry, an
uninterrupted whole-area replay, or console parity evidence. User saves remain
untouched; all inputs and diagnostic captures stay ignored.

The original courtyard framebuffer effect (actor `0x20`), further destinations
and full audiovisual/timing parity remain incomplete. Selected
[Olrox combat](olrox-encounter.md) and [connected travel](olrox-connections.md) now run.
Alucard's remaining forms/abilities and the rest of the castle remain separate
work; this batch does not establish complete levels or a complete game.

Provenance: `st/no2_bg.h` and `st/e_background_house.h`, included by NO2's
`33D8C.c` and `e_background_house.c`, at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later). Selected SDK
projection math follows PSY-Z `b52003e55c0d1b20719e984dd19bd4d760eeb14a`
(MPL-2.0); all original model and lookup arrays remain private runtime inputs.
