# Leap Stone double jump

Normal Alucard now uses the original Leap Stone jump when the saved relic is
found and active. Tap Jump again in the air: X/Space on keyboard or Xbox A.
The existing relic collector and version-nine ordinary saves retain ownership;
no new-game or development profile receives the relic automatically.

The movement service copies thirty validated found/active bytes from owned
session progress before simulation. It retains no world pointer. Unfound or
found-but-disabled Leap Stone does not enable the jump. Changing rooms, updating
the snapshot, or disabling/re-enabling the relic does not refill a used airborne
jump. A subsequent ordinary ground jump or walk-off initializes the original
movement flags again.

The selected reconstructed path retains Jump versus Fall, horizontal velocity,
attack word, original animation 32 and the one-use flag. Vertical speed becomes
-4.25 pixels/tick; a tight space still consumes the jump but sets speed to zero.
Original ledge grace and attack-startup priority remain ahead of this transition.
Jump retains early-release behavior; Fall retains its separate gravity path.
The ordinary airborne movement substate runs after a double jump even if an
unlocked attack word remains; the reset substate must not dispatch weapon logic.

## Evidence and remaining scope

Public authored fixtures cover ownership, disabled relics, copied state, atomic
rejection of invalid relic/animation data, held input, one use, room changes,
landing, falling, ledge grace, ceilings, tight spaces, attack priority and damage.
All 108 Debug, Release and macOS ASan/UBSan test groups and asset-free release
packaging pass.

A private instruction comparison matched 576 transitions against the user's
original `func_8010E940` and `SetPlayerAnim`, spanning Jump/Fall, movement flags,
horizontal speeds, substates, tight spaces and retained attack words. Native
follow-up ticks also retain ordinary movement after the reset substate. This is
component evidence, not a comparison of the complete original player update.

A muted native LLDB check uses this repository's built services and private DRA
animation in an explicitly selected NO2 save room. An explicit private relic
grant distinguishes absent, active and disabled states. The active jump remains
available after the actual isolated save-room UI and fresh byte-identical v9
load. Existing user slots are untouched. This does not prove ordinary castle
acquisition, uninterrupted traversal or console parity.
Both Debug and macOS ASan/UBSan native probes exit normally, with 112 draws each
and SDL dummy audio explicitly verified before simulation.

Dive kicks, Gravity Boots, transformations, other abilities, their complete
effects and remaining castle routes still require integration. Do not describe
this double-jump slice as complete Leap Stone combat or complete Alucard abilities.

## Provenance

`modules/core/player.c` adapts `dra/6FDF8.c:func_8010FDF8`,
`6E42C.c:func_8010E940` and selected `71830.c:PlayerStepJump/PlayerStepFall`
from the [pinned reference](../config/reference-us.json), under AGPL-3.0-or-later.
Animation tables and other original data remain private runtime inputs.
