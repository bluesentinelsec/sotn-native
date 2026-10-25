# Connected Long Library travel

The selected native checkpoint now follows the original Outer Wall/Library
teleport records in both directions. Library entrance rooms 12 and 13, the
original door and stairs, stage-specific enemies/resources/music, selected
Librarian buy/sell interaction and room-14 ordinary saves share the existing
connected-session ownership path. The Library remains incomplete; enabling its
connector does not establish complete progression or console parity.

## Arrival correction

The original Library arrival begins inside a red-door column. Previously the
native startup advanced player collision before initializing that door. Closed
tiles displaced Alucard into the floor, and he fell through after the door opened.
Teleport arrivals now initialize stage actors and their owned tile changes before
the initial player collision. Direct development starts retain their existing
priming order. No collision rules, original arrival coordinates or private tile
data are changed to conceal the problem.

Character/world progress, exploration, owned RNG states, clock and ordinary save
ownership retain the existing [connected-stage rules](connected-stages.md).
Curse/poison remain transient and copy independently across live arrivals. Fresh
Continue reconstructs the recorded stage and clears transient ailments. Current
v8 slots and v1–v7 migrations are unchanged. Missing destination resources retain
the previous session and save store, then show the native error shell.

## Validation and remaining scope

All 77 public synthetic test groups pass in Debug and macOS ASan/UBSan, and release
packaging passes. Stage-profile tests now include Library connectivity and its
supported ordinary save-room identity. Private asset checks run through this
project's CMake libraries and LLDB, without publishing game inputs.

The ordinary-character connector sequence starts explicitly at the lower Outer
Wall entrance, walks into Library, descends/returns up its entrance stairs with a
held jump, crosses back to Wall, then enters Library again. It verifies the three
actual connector crossings, entry-door reopening, catch-up latching, unchanged
HP/equipment and save-store ownership, exploration and retained discovery.
An explicit transfer to Library room 14 then exercises the ordinary save UI in
an isolated directory and fresh Continue. A missing-input destination check
retains the original owner and checked slot. Debug and sanitizer sequences each
pass 1379 input ticks and 344 rendered frames. The discovery flag is an explicit
fixture; this sequence does not walk from the secret bookcase to the save room.

Regression checks also pass the Librarian introduction/buy/sell/return/save flow
(1927 ticks, 481 renders) and protected Doppleganger entry/combat/reward/return and
ordinary-save flow (4446 ticks, 1112 renders). These retain their existing explicit
starts and grants. They are not unmodified-character full routes or console replays.

Early failed travel probes caught the arrival-order defect and are not passing
coverage. A controller fixture that tapped jump for only one frame could not climb
the return stairs; the passing sequence holds jump for the original variable-height
jump. It does not change player movement to bypass the step.

Sitting, Mist-form interaction, full shop panels/optional services, medicine,
full effects/audio, remaining castle areas and whole-route replay parity remain
incomplete. The 70-HP development character is not prologue-derived initialization.

Route decoding and door behavior retain their pinned AGPL-3.0-or-later provenance
in [connected stages](connected-stages.md). Native transactional session ownership
and startup orchestration belong to this repository. All original route, tile,
actor, artwork and sound tables remain private runtime inputs.
