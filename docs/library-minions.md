# Library summoned minions

The Library adapter now runs Mudman and its separate attack child, plus the
summoned Skeleton appearance/cleanup path. Ectoplasm and Skeleton share the
original Library opacity/blink helper. Mudman keeps its own floor-emergence and
sinking shapes. These services prepare Lesser Demon's minions; the boss controller
and its summoning beam still remain pending. Library travel stays gated.

Mudman retains delayed floor emergence, the colored private-texture shape,
walking/attack timelines, frame-selected separate hitboxes, obstacle sinking and
four independently falling death pieces. The original effect and child pools are
bounded; optional exhaustion omits children or retires unavailable shapes. Child
slot/generation checks prevent following a replaced body. Death pieces copy only
the original presentation fields, flags and private motion; they cannot inherit
the body's active hitbox or primitive ownership. Retained emergence variations
remain selected state paths rather than additional ordinary encounters.

Summoned Skeletons retain parameter 0x10, grounding before opacity, the original
palette blink and return to ordinary Skeleton behavior. The boss cleanup signal
sets their original death/hit flags and retires them through their normal pieces.
Ectoplasm uses the same helper without grounding and retains its existing selected
curse and cleanup behavior. Runtime animation, sensors, hitboxes and fragment
profiles remain private.

Library effect cadence now uses a separate full simulation-frame phase with the
original unsigned 32-bit wrap. The clock's frame within its current second remains
separate. In particular, Spellbook death sounds no longer restart their 16-tick
cadence every sixty ticks. Tests cross the 60/64/128 boundaries and 32-bit wrap.
The native phase follows simulation ticks; global console menu/pause/load phase
and complete timing parity remain unproven.

Public synthetic cases cover both Mudman directions, emergence, attack child
poses, sinking, fragments, stale owners, malformed hitboxes, allocation failures,
checked motion, Skeleton fade/grounding/cleanup and shared helper failures. All 67
public groups pass in Debug and macOS ASan/UBSan; release packaging passes.

Private LLDB fixtures insert each summoned type in the original boss room 0 with
a protected character. Ordinary movement/attack inputs begin after appearance.
Mudman completes combat at tick 200, Skeleton at 112 and Ectoplasm at 242, totaling
141 native renders in both Debug and macOS ASan/UBSan, with successful exits. Immediate Mudman/Skeleton retirement requires a new kill,
the death sound and the expected fragment family; disappearance alone cannot pass.
Ectoplasm exposes its HP/dead state before its later retirement. These checks do
not establish the boss's summon decision, an ordinary route, an unmodified-character
playtest or audiovisual/console parity.

Algorithms adapt pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, `st/lib/e_mudman.c`, the selected
Library branches of `st/e_skeleton.h`, `st/lib/e_lesser_demon.c:func_us_801BB8DC`
and `st/e_misc.h:MakeEntityFromId` (AGPL-3.0-or-later). No original data arrays or
artwork enter the public source or release packages. Ordinary saves remain v7.
