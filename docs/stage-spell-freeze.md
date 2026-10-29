# Selected frozen stage updates

The core actor owner now exposes the original selected frozen-stage dispatch used
by spirit casting. This is an integration interface; the native engine has not yet
connected spirit controls, scene freeze, capture or spirit combat. Selected Dark
Metamorphosis remains the only live ordinary spell.

Frozen updates visit the actual 192 stage slots in original order. Supported live
step-zero actors initialize even without flag0x10000. Initialized actors without
that flag retain their entire state. Eligible actors run their real existing
behavior and clear their hit parameters/flags afterward. Before a nonzero-step
behavior, the original low-nibble hit-palette countdown selects the private color,
decrements the flags and restores the saved palette on completion. Invalid palette
indices fail before changing that actor. This supports retained palette metadata;
the complete ordinary-hit palette producer/presentation remains incomplete.

This path performs no layout scrolling, camera displacement/culling, hit detection,
player contact generation, ordinary stun/cooldown decrement, control-timer decrement
or game-clock update. Existing hits can still affect an eligible behavior: freeze
does not discard queued contacts before its update. Real stage behaviors may emit
sounds, allocate/init later slots, retire their own effects and update supported
world state. The owner consumes those events once. Engine input edges/repeats still
observe current physical input. Unsupported behaviors retain their existing
explicit omission; this interface implements none of the missing actors.

The palette view resolves from the pinned private overlay's exported
UpdateStageEntities entry and its bounded original LUI/ADDIU reference. It reads
exactly22 private words; no game palette is embedded. The export, instruction
shape, address alignment and both code/data bounds are validated. Other layouts
fail with an actionable error rather than guessing. This layout has been checked
against the twelve currently available private overlays. It is not evidence for
unavailable versions or other stages.

Public authored tests cover palette-reference/bounds failures, all-or-nothing
field updates, new/initialized eligibility, palette restore and malformed indices.
An actual synthetic stage test verifies unchanged offscreen enemies/queued hits,
continued eligible candle animation, new actor initialization, real hit clearing,
retained stun/cooldowns/control/game clock and absence of fresh combat sounds.
Normal actor tests exercise the shared frame setup and retirement path too.

Private comparisons match41,472 slot snapshots across216 complete original
192-slot dispatches from twelve overlays. The complete original dispatcher executes,
including callback order and absent callbacks, with an explicit behavior boundary
adapter. Actual behavior parity is not established by that adapter. Debug and
ASan/UBSan traces match. A separate selected native check uses this repository's
CMake libraries, actual Laboratory data and renderer: it retains a real zombie and
player/character clocks/resources for90 frozen updates while a candle with an
explicit permitted-effect flag animates. A real unflagged newborn explosion
initializes and then retains its full state; after an explicit eligibility-flag
fixture it advances, retires and draws. It writes no saves and runs muted through
LLDB. This is a selected dispatcher check, not a live spirit spell, whole-scene,
whole-route, Alucard or console parity claim.

The next owner must connect the [scene-freeze boundaries](spirit-casting.md),
actual player/effect factories, ordered framebuffer capture, targets and native
combat before enabling [spirit spell controls](spirit-spells.md). Ordinary starts
and version-nine saves remain unchanged.

Provenance: selected `st/st_update.h:UpdateStageEntities`, pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
Original palettes, overlay images, traces and runtime inputs stay private.
