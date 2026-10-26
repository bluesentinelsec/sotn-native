# Selected relic pause controls

The native pause menu now opens Relics and switches owned Cube of Zoe, Gravity
Boots, Leap Stone and Jewel of Open on or off. Enter/Xbox Start opens or closes
pause; X/Space/Xbox A selects, and keyboard A/Xbox Y returns. Gameplay and elapsed
ticks remain paused while the menu owns control; music continues.

Navigation follows the US two-column, 28-position order. Up/down moves within a
column and stops at its ends; left/right changes columns. Original identities
23/24, the Japanese-exclusive familiar cards, are omitted. Selection survives
returning to the main pause panel. Unfound relics show no name or icon and cannot
be activated. Found but unintegrated abilities remain visible and unavailable.
Familiar activation is deferred with its runtime, mutual exclusion and sword
familiar equipment effects; the menu never claims to implement those behaviors.

A successful switch preserves the found bit and XORs the original active bit.
The ordinary world owns this state; the player receives its updated validated
snapshot before gameplay resumes. Switching never grants ownership, resets a
jump interval, refills MP or changes equipment. Existing version-nine ordinary
saves retain both enabled and disabled relics without a format change. Earlier
save migrations remain unchanged. See [relic progression](relic-progression.md),
[Leap Stone](player-leap-stone.md) and [Gravity Boots](player-gravity-boots.md).

Names, icons and palettes come from bounded private DRA records. The renderer
uses the existing private menu font and original two-column/scroll equations.
Its simplified panels, ASCII switch labels and cropped long names remain native
checkpoint presentation; original switch artwork/pulsing, descriptions, panel
transitions and complete menu/GPU parity remain incomplete. No game tables,
strings or artwork are embedded in public source or packages.

## Validation

Public synthetic tests cover all US cursor positions, omitted identities,
navigation boundaries, press-versus-hold behavior, all relic ownership-bit
combinations, supported toggles, unavailable familiar/ability rejection, malformed
metadata and unchanged progress after failure. Rendering tests exercise every
cursor and malformed state without replacing caller output on failure.

Muted LLDB checks of the CMake-built Debug and macOS ASan/UBSan executable cover
normal pause navigation, unavailable selection, repeated Gravity Boots switches,
Leap Stone disabling, frozen gameplay ticks and immediate player snapshots. A
separate native check renders all 28 selections, disables all four supported
relics, uses the actual save-room UI with isolated slot 15 and verifies a fresh
byte-identical version-nine restore with disabled player snapshots. Existing
user slots remain untouched. These checks use explicit relic grants, room starts
and a save-room transfer; they do not establish ordinary acquisition, continuous
castle traversal or console parity. Automated checks use SDL dummy audio.

Selected rules derive from `dra/menu.c:MENU_STEP_RELIC`, `DrawRelicsMenu` and
`MenuHandleCursorInput` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
