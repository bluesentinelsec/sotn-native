# Selected original player camera shake

Wolf wall and ceiling impact requests now reach the native scene's original
single-request shake service. It reads private pattern pointers and signed samples
from DRA, accepts original identities zero through six, and preserves the no-op
identities zero/five. Requests replace the prior cursor; the last request wins.
The next-sample terminator clears the active axis immediately, so the last pattern
sample is intentionally discarded. Do not smooth or extend the original sequence.

The service retains the other axis across replacement. Original presentation
offsets are not reset every frame: replacing a horizontal request with a vertical
one can leave a horizontal offset until another request or initialization changes
it. Normal simulation advances the cursor once, after owned player/actor updates.
Pause and repeated rendering retain it. Room entry/rebases do not advance it.
The native scene still initializes transient presentation state with its stage.

Logical camera follow, collision, player/body positions and capture bounds remain
unchanged. Rendering applies the original additive offset to tile layers, player,
weapons, entities, selected world primitives, secondary effects, save-point world
meshes and selected subweapons. Original menu/absolute player-packet flags bypass
shake while retaining the active target offset. The fixed native HUD/save dialog
remain fixed. Projected stage primitives expose an explicit `camera_fixed` selector;
`screen_space` alone does not mean absolute original draw coordinates.

Wolf normal/capture packets receive the offset during entity projection, before
ordered capture rendering. Their logical player anchor remains unshifted. The
later draw must not apply a second offset. Retained root images/Blink/Mist and
particle primitives follow their own original draw flags and environment offsets.
Repeated-frame capture restoration and simulation-side capture flushing remain
required; a post-render framebuffer shift cannot reproduce these relationships.

All 152 tests pass in Debug, Release and macOS ASan/UBSan, alongside 34 Python
checks and release packaging. Public authored fixtures cover all request identities,
signed samples, interrupted axes, replacement, final-sample discard, no-op cursors and atomic malformed-input
failures. The save-point fixture checks projected translation and unchanged output
on invalid offsets. Private instruction comparisons cover 7,056 original request/
update ticks without adapters, and 5,400 supplied Wolf-render contexts/8,716 normal
and capture packets with varying offsets. The latter executes original renderer,
SDK/ordering/capacity instructions with a GTE leading-zero adapter and supplied
GPU/CLUT context. Debug/ASan traces match; these are scoped component comparisons.

Private muted native Step/Draw fixtures connect a real Wolf entry to a controlled
wall-contact/speed snapshot, then check actual request dispatch, visible world
translation, fixed HUD/menu/absolute packets, unchanged camera/physics/cursor on
repeated draws, termination and real form return/retirement. They grant Soul/Power
of Wolf and contact protection, retain ordinary development HP/MP/stats and enemy
behavior, and write no saves. Supplied contact is not physical wall traversal or
an unmodified original encounter. Separate native checks keep the body alive while
the real pause menu freezes/resumes the cursor. A CMake-executable LLDB check also
verifies actual impact dispatch, stable repeated pixel draws, logical-camera
retention, original return/cleanup and an ordinary SDL quit. Debug and ASan/UBSan
pass. Runtime inputs and rendered pixels stay private.

[Selected Gravity Boots ceilings](player-gravity-impact.md) now request the same
service. Other player/actor shake producers, complete original primitive-flag propagation,
full palette/GPU contention and raster/audio/timing/route parity remain incomplete.
This service does not establish a complete Wolf, Alucard, area, castle or game.
Ordinary starts and version-nine saves remain unchanged.

Provenance: selected `dra/624DC.c:{ShakeCamera,StepShakeRequest,UpdateCameraShake}`,
`wolfForm.c`, `4B758.c`, `4CE2C.c`, `4DA70.c` and `62DEC.c` at pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later. Pattern values remain
private runtime data, not public fixtures or embedded game tables.
