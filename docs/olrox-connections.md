# Connected Olrox’s Quarters progression

Olrox’s Quarters now connects to Marble Gallery and Royal Chapel through the
original private US teleport records. Its original boss override enters BO0 from
the eastern approach until Olrox’s defeat time exists. The boss arena exits return
to NO2; subsequent crossings use the ordinary NO2 arena without replaying the boss.
The encounter retains [both forms and reward](olrox-encounter.md) and
[breakable furnishings/sitting](olrox-furnishings.md).

This remains a development checkpoint. Exits to Colosseum and the teleport room
still report an unsupported destination. Forms, remaining scenery/effects/audio,
original new-game/prologue initialization and whole-route/console parity remain
incomplete. Neither NO2 nor BO0 gains a public diagnostic gameplay start. Connected
loading does not establish a complete area, Alucard, castle or game.

## Routes, ownership and saves

The existing [session transaction](connected-stages.md) validates destination data
before replacing the active owner. Character progress, world events, map discovery,
owned RNG streams, elapsed time, verified music container and save-store ownership
carry across. Transient actors, projectiles and voices are recreated. A pending
arrival stops catch-up; held movement continues, while UI/action buttons require
release. Missing destination inputs retain the old session and ordinary slots.

Boss routing reads the original private override table and consults the Olrox
defeat time. A visited bit or cutscene-completion setting alone never bypasses the
encounter. No additional opposite-side override is invented. Return travel and
fresh NO2 visits retain the earned defeat record. The original arena’s high exits
still require suitable movement; the native port does not lower walls or grant
abilities to make them reachable by the development character.

The NO2 save room uses [ordinary version-nine saves](native-saves.md), with no
format change. It retains supported inventory, relic ownership, defeat times,
events and exploration; existing v1–v8 migrations remain explicit. User slots are
never used by automated checks.

## Validation and limits

All 123 public test groups pass in Debug, Release and macOS ASan/UBSan; release
packaging passes. Authored route fixtures distinguish NO2’s boss override,
visited/completion state and actual defeat, preserve output on absent overrides,
and cover connected profiles/save-room identities. They contain no game records.

A muted private native sequence performs seven actual Marble–NO2 and Chapel–NO2
crossings in Debug and ASan/UBSan using the ordinary 70-HP development character
without protection or
stat/equipment grants. It verifies catch-up latching and retained ownership,
then checks the ordinary save UI, isolated slot 15, fresh byte-identical v9 restore
and a missing-destination failure. Connector starts and transfer to the save room
are explicit fixtures; that transfer is not walking evidence. Jump input clears
the Chapel connector’s raised terrain without changing its collision map.

The actual CMake executable independently traverses both connector sequences
in Debug and ASan/UBSan under muted LLDB and closes through normal SDL quit,
without writing save slots. Both also verify the original eastern NO2-to-BO0
override in the actual application. Debugger-driven input advances the real game APIs; these accelerated checks do
not establish normal presentation/audio timing or uninterrupted route parity.

A separate Debug and ASan/UBSan connected boss fixture enters BO0 through its eastern NO2 approach,
defeats both forms through physical Gladius attacks, collects the original +5 HP
reward, exits west and crosses the defeated ordinary NO2 arena both ways. It then
uses an explicit save-room transfer to verify the earned boss record through the
ordinary UI and fresh byte-identical v9 restoration. Strength 99 and contact
protection are explicit combat fixtures; Gravity Boots ownership is explicitly
supplied for the high return exits. No boss HP/death signal, player position or
MP is forced after entry. This does not prove ordinary ability acquisition or
encounter balance. The separate ordinary-damage encounter evidence remains in
[encounter validation](olrox-encounter.md).

Route selection follows `dra/5087C.c` at pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later. Runtime
teleports, room data, actors, artwork and sound stay private. Transactional
session/save ownership remains this repository’s native C implementation.
