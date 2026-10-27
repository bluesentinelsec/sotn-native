# Selected playable Bat

Owned, enabled Soul of Bat now connects original entry, ordinary flight, collision,
MP use, damage and restoration to the native session. E or Xbox RB toggles it;
the Relics panel can enable or disable an owned Soul of Bat. Ordinary starts grant
no abilities. [Bat command/motion components](bat-command-motion.md) are available,
but Wing Smash, Fire of Bat, Echo of Bat and Wolf remain unfinished in gameplay.

## Flight and real restoration

Entry retains the original strict MP eligibility and sixteen-tick lockout. The
[actual BlinkWhite child](player-bat-presentation.md) signals entry phases two
and three; no timer substitutes for its readiness. Only the actual transition
switches from normal artwork to private bank thirteen. Bat uses original flight
animations, steering, rotation, facing, jump-button braking and settling offset.
Its compact sensors block ordinary and Mist-only walls and retain selected
one-way behavior. Original simulation timing charges MP once to character progress;
transformed states suspend normal recovery. Bat remains vulnerable to enemy damage.

Cancellation, water or resource exhaustion starts the actual return child. Original
sensors grow before the player signals readiness; only the real distortion's
completion restores normal control. The independent cramped-space flag retains
stuck/crouch recovery. Normal return applies the original twenty-four-tick lockout.
Per-frame rotation flags remain distinct from the movement state, including the
first cancellation frame and the sensor-ready frame. Damage/script/grab/stone
adapters restore normal artwork and retire transformed ownership.

The session reserves two actual selected visual owners and updates newer children
first, retaining shared Blink phases and lock ordering. Body/effect packets use
original texture pages, palettes, priorities, target offsets and clip. Raw body
packets ignore color bytes; only original DRAW_COLORS packets modulate texture.
Camera/room changes translate geometry without advancing effects a second time.
Full stage initialization restores normal Alucard and retires transient owners.

Bat-to-Mist captures the actual bank-thirteen pose. Mist-to-Bat waits on the real
Mist child and barrier escape; it creates no invented Bat-entry child. The shared
settling offset survives cross-form entry. Ordinary saves retain owned/disabled
relics and character progress, with no transient form serialization or version-nine
format change. Fresh Continue starts normal Alucard.

## Evidence and remaining scope

All 132 public test groups pass in Debug, Release and macOS ASan/UBSan;
`make package` passes. Authored public fixtures exercise ownership, disabled relics, strict MP limits,
real effect entry/return, flight/braking/rotation, natural MP exhaustion, water,
wall collision, cramped recovery, physical damage and lockouts. Animation fixtures
verify that mutable form poses do not change borrowed private DRA bytes; raster
fixtures distinguish raw texture packets from colored/hidden packets.

Private original-instruction comparisons cover 2,304 supplied ordinary Bat
controller contexts and 5,760 restoration contexts. They execute original motion,
rotation, resource and return helpers; factory/sound adapters and a bridge to this
project's collision decoder remain explicit. This is neither independent PS1
collision validation nor a full original simulation replay. Debug and sanitizer
controller/restoration traces are byte-identical. Command families are
excluded from these ordinary-flight comparisons.

Muted LLDB checks use this project's CMake libraries and executable with explicit
relic grants and ordinary development health/stats/MP. They cover actual entry,
flight, MP use, sensor/visual return and child retirement; a separate sequence
crosses Bat/Mist/Bat before returning normally. Explicit connector entry covers
connected Laboratory room travel with the live generation retained. An isolated
save-room transfer exercises the actual slot UI and fresh byte-identical version-nine
load, healing MP and retaining Soul of Bat without transient children. Ordinary
user slots remain untouched. These fixtures do not prove ordinary acquisition,
uninterrupted castle traversal or console parity. All runtime inputs, artwork,
traces and screenshots remain private.

Shared factory/primitive contention, afterimages, command abilities, complete
water volumes, full effects/audio and raster/timing parity remain incomplete.
Do not describe Bat, Alucard, any area, the castle or the game as complete.

Provenance: selected `dra/692E8.c`, `71830.c`, `6DF70.c`, `7E4BC.c`, `7879C.c`,
`4DA70.c` and `5087C.c`, pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
