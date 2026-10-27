# Owned offscreen render targets

Olrox's original transformation first renders its monster image into a 127x256
VRAM rectangle, then renders the morph into a 128x128 target. The existing
240-row game frame cannot hold the complete first target. This batch adds the
bounded renderer and ordered scene services needed for that integration; it does
not yet advance the transformation or second-form actors. The
[morph controller component](olrox-morph-controller.md) retains its original
command/timing sequence, with gameplay dispatch still gated.

Sprite, transformed-sprite, quad and line drawing now share a borrowed tightly
packed RGB555 surface with explicit width, height and pixel count. Dimensions
must be 1..256 and every raster path clips/indexes against that surface. Existing
frame APIs construct the original 256x240 view.

Opening an owned target validates its VRAM rectangle and initializes scratch
pixels from a clear color or fully resident owned texture words. Drawing leaves
texture memory unchanged until commit. Cancellation discards scratch changes.
Only one target may be active. The borrowed surface expires on completion or
owner destruction; callers must not retain it. Original private input views stay
immutable. Direct-color texture sampling can read committed target pixels,
including VRAM row 511.

Ordered actor commands begin the target at its priority, apply the original
coordinate offset, then restore the main frame after committing. Missing restores,
invalid bounds and nesting fail explicitly. The frame itself remains 256x240;
HUD and original simulation timing use their existing paths. This bounded native
adapter does not reproduce all PS1 GPU draw-environment or texture-cache behavior.

## Validation and scope

Public authored regressions cover normal/transformed sprite and quad drawing at
the target's lower edge, sampling its final VRAM row, retained target pixels,
commit/cancellation, nested/malformed operations and short output buffers.
Existing sprite/quad/line/framebuffer tests exercise the unchanged frame APIs.

A muted private BO0 scene check inserts synthetic ordered begin/paint/restore/
sample commands, writes VRAM row 511, and verifies the displayed result. Removing
the fixture restores a byte-identical ordinary frame. The fixture does not advance
an actor or simulate boss completion. Private inputs and helpers remain ignored.

Olrox's transformation, articulated second form, final defeat/reward and connected
travel still need integration. This is renderer infrastructure, not a completed
encounter or console pixel-parity claim. Existing selected skull-column/native
checks remain the gameplay regression reference.

## Original environment replacement

The selected [monster laser components](olrox-monster-attacks.md) add ordered
replaceable environments, using target modes 3/4. A replacement commits the
current replaceable interval, then opens the next target. Repeated restores to
the main surface succeed. The cursor owns the current borrowed surface and its
coordinate offsets; the game presenter delegates those transitions to the render
service. Strict modes 1/2 still reject nesting and unmatched restores, and the
underlying owned-target API still permits only one open scratch view.

Public pixel tests check replacement across distinct VRAM regions, retained
commits, strict/mismatched operations, repeated restores and invalid bounds.
Private native and actual CMake-app diagnostics exercise the original command
ordering, including final VRAM row 511. These are controlled rendering checks,
not a completed monster encounter or full GPU/texture-cache parity claim.
