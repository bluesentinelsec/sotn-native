# Selected ordinary fists

The actual shared [ordinary hand owner](shared-ordinary-hands.md) now accepts the
two original weapon1 fists without weapon specials: Knuckle duster85 (descriptor0)
and Jewel knuckles97 (descriptor1). Iron Fist102 and Fist of Tulkas120 share the
program; their rushes and fireball are documented in [fist specials](player-fist-specials.md).

Owned selection uses the ordinary inventory menu and either hand. Definitions
validate the actual fist category3, family1, kind0, animation0x24, blueprint48,
variant byte and absent special items. Defaults, grants and v9 saves are unchanged.
No new weapon, effect or primitive pool is introduced.

## Callback ownership

The punch has no sprite of its own: the original callback sets no animation set,
palette, priority or blend. Its visible part is four textured GT4 trails on page
0x1A with the descriptor's absolute CLUT and authored vertex colors. Those four rows
are mandatory: they are reserved from the real shared stage pool before properties,
and exhaustion destroys the entity before its flags, group or property RNG, without
retrying. Unarmed-style animation, hitboxes and the swing sound follow the private
descriptor tables.

Each visible callback places every row from the current screen position, private
per-animation offsets and one libc call for its trailing edge. The scale flag
lowers it three pixels. Animation5's follow-through rotates every vertex with the
original SDK square root, quantized atan2 and quarter-wave trigonometry. Other
poses hide the rows without moving them. Native positions are world coordinates;
the callback subtracts the actual camera, matching the original screen words.

## Player attack state

Fists are kind0, so the player uses the original substep0x40 handler (shared with the
empty hand and kind25): a two-pose crouch window, the fixed0x24 stand-up from a crouched
attack, and the pose7 combo continuation while the attack button stays held. Native
previously keyed that handler on item id0, so fists took the weapon branch. The falling
Down variant likewise needs Down plus Left/Right for kind0 items.

## Rendering, holds and failures

Weapon1's original palette table entries are NULL, so its palette callback returns
without writing; the native loader binds the program and texture but leaves the
hand CLUT untouched. Draws enqueue the visible rows at their own priority and apply
the current shake once. Room rebases translate all four vertices without a callback.
Draws never tick. Active attacks refuse the pause menu. Form entry retires the fist
and releases its real rows after a successful player update. Curse failure keeps its
original animation and creates no owner. In-memory v9 Continue rebinds the program.

## Evidence and remaining work

Public tests cover mandatory-row exhaustion without flags or RNG, birth packets,
per-variant phase gates, camera/facing/scale placement with per-row RNG, follow-
through table failure atomicity, the excluded special descriptor, slot16 runtime
rows/groups/commit/retirement, rebase translation, definition validation and the
NULL palette table.

The private original-instruction comparison runs 17,040 complete callbacks in 896
contexts with the real DRA animation helper and SLUS rand/ratan2/rcos/rsin/
SquareRoot12 code, including 4,928 square roots and 67,712 full GT4 packet
comparisons. It caught one ordering bug, now fixed: native assigned flags before
the allocation could fail. Debug and ASan traces are byte-identical.

Native protocols (Debug and ASan, LLDB, muted) exercise real menu equipment in both
hands, visible trail pixels, real enemy HP damage, row release, mandatory-row
exhaustion, Mist/Bat/Wolf retirement, draw holds, pause refusal, curse failure and
in-memory v9 Continue with mixed pairs. Supplied stock, protection, position,
occupancy, curse and save-room contexts are not acquisition, uninterrupted-route or
console evidence. Other weapon families, full sharing and all six
[milestone gates](alucard-status.md) remain open.

Adapted AGPL-3.0-or-later algorithms: `weapon/w_001.c`, `weapon/shared.h` and
`dra/alu_anim.c` at sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`.
