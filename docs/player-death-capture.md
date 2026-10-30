# Owned death image and decay

The selected common death-image component now reconstructs the original packed
texture decay. The renderer supplies real StoreImage/LoadImage operations against
owned display-texture memory, using the bounded rectangle parsed from private DRA
data. These APIs supply capture operations. The selected
[native water-death owner](player-water-death.md) now supplies actual C1 capture
timing, committed decay uploads and game-over completion for a lethal-result boundary.

Capture copies the complete5,120-byte packed image after checking that every source
word is resident. It initializes the common cursor and64-update lifetime. Each
simulation update clears four groups of40 packed nibbles, retains the original
35-step byte cursor wrap, and requests an upload. The final update clears the full
image. Zero remaining time requests the original completion; it does not itself
stop velocity, clear player color blending, advance the death substate or start
game over. The actual owner must upload successfully before committing a copied
cursor. Menus, scene freeze, rebasing and repeated drawing must not advance decay.

The source is the current uploaded player texture, not the immutable ARC image or
the composited game framebuffer. Capture preserves packed pixels and margins.
Uploads change only the original owned texture rectangle; they never rewrite
private artwork. Missing residency, malformed dimensions/bounds and invalid
capture/cursor state fail explicitly without changing output or owned pixels.
Both operations remain separate from other capture targets and private asset views.

Public authored tests cover exact nibble coverage, cursor wrap,64-update completion,
alias-safe initialization, source immutability, actual texture store/load, unrelated
words, missing residency and malformed-input rollback. Private Debug and ASan/UBSan
traces match byte for byte. An original-instruction comparison checks every packed
byte and common cursor across eight supplied captures and512 updates, including
the original final substate/color request and one LoadImage call per update. The
SDK upload is adapted; these are component comparisons, not console evidence.

Muted native checks start the ordinary Entrance and upload actual private Alucard
artwork, then exercise64 real capture/decay/upload/readback updates and128 native
draw packets. Both Debug and ASan/UBSan reduce724 visible body pixels to zero,
preserve the private source and keep repeated drawing independent of decay. They
use an explicitly supplied decay/packet sequence outside ordinary gameplay, write
no saves, and do not exercise an actual dying player or game-over transition.

The [water-death controller](water-damage-death.md), [flashes](water-blink-effects.md)
and [particles](player-death-particles.md) now connect through that native owner.
Selected [ordinary water HP/contact ownership](player-water-contact.md) now runs;
global-fade recovery remains unwired.
The existing physical-death presentation is a separate selected path,
not a substitute for water death. Ordinary starts, grants and version-nine saves
remain unchanged by these capture APIs. Alucard, the castle, full shared effects
and console parity remain incomplete.

Provenance: selected `dra/71830.c:PlayerStepKill` phase2 and the original death-image
StoreImage/LoadImage boundaries, pinned `sotn-decomp`
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
Original rectangles, images, artwork and traces remain private runtime inputs.
