# Selected playable Wing Smash

Owned, enabled Soul of Bat now connects Wing Smash to the native session. While
flying, hold Jump and enter Up, Up-back, Back, Down-back, Down, Down-forward and
Forward, then release Jump. Back/forward are relative to facing. Each direction
must match exactly. The US repeat command can cast again during the attack.
Ordinary starts grant no relics, MP or learned spells.

The controller runs after Bat's own resource gate. A successful command spends
its private spell cost once and records the learned spell in existing version-nine
progress. Original speed, vertical steering, slope order, duration, four-tick
protection refresh and wall-impact sound connect to normal collision and return.
Forced return retains the original thirty-two-tick input hold and actual
transformation/sensor handshake; it invents no completion timer.

## Owned factories, attacks and presentation

The session reserves actual factory slots 8..15 and updates selected children in
original slot order across 16..63. Original private blueprint ranges select each
slot. Critical transformation factories wait for a real free child; optional
requests can fail without inventing an attack or flash. Generation ownership
survives room rebases, and full stage initialization retires transient owners.
Gas Cloud records its actual slots 32..47 and exchanges occupied masks with this
owner, including retained disabled children.

Wing Smash's invisible child uses original private spell power, INT, RNG, Mojo
Mail adjustment, critical-hit rate, element, stun and enemy invincibility data.
It follows Alucard's whole position while retaining its own fractional position.
Actual birth consumes one spell RNG value and selects the least-used original
group 3..6. Enemy cooldown belongs to that group, surviving child replacements;
separate groups remain independent. Native damage, enemy death and EXP use the
existing combat service.

The real `0x5C` BlinkWhite fan and shrinking blue trails sample current private
Bat artwork. Duplicate flashes compare full parameters. Shared phase/lock ordering,
original `0x7009` fallthrough and retained phase four remain unchanged. Trails
capture facing/rotation but follow the live frame, scaling before SDK rotation.
Camera/room rebases move sources and packets without advancing them again.

## Validation and remaining scope

All 136 public test groups pass in Debug and macOS ASan/UBSan; Release tests and
`make package` pass. Authored fixtures cover real player/factory/effect entry,
Wing commands and US repeats, MP/learning, group assignment, duplicate suppression,
wall/duration returns, critical allocation retries, actual slot contention,
room rebases and atomic malformed-data/ownership failures. Actor fixtures cover
same-group cooldown across generations and independent-group damage/death.

Private original-instruction checks cover 240 supplied factory/capacity contexts,
including 192 actual attack births, spell initialization, group selection,
fraction-preserving follow and removal. They execute original factory, spell and
group instructions with explicit RNG, equipment-count and destruction adapters.
Debug and sanitizer traces match byte-for-byte. Earlier command and visual
comparisons retain their narrower evidence in the linked component documents.

Muted LLDB checks of this project's CMake libraries and actual executable execute
the full command, spend MP, learn Wing Smash, damage/kill placed Laboratory enemies,
award EXP, draw real flash/trails and return normally with all children retired.
They use an explicit Soul of Bat grant and ordinary development HP/MP/stats, with
no player-position or enemy-state injection. An explicit save-room transfer uses
an isolated slot fifteen and a fresh byte-identical version-nine load, retaining
the learned spell and relic while restoring normal form. Existing user saves stay
untouched. Ordinary Bat flight, Bat/Mist transitions and connected room travel
also retain their regression checks.

This remains selected integration. Other projectiles, dive kick, Gravity Boots,
familiars and spells do not yet share one global factory, primitive or attack-group
allocator. The new owner's duplicate scan covers its selected children, not every
stage actor. Afterimages, floor/ceiling dust, camera shake, full water volumes,
freeze/effect contention and complete audio/raster/timing remain incomplete.
Fire/Echo of Bat and Wolf remain unavailable. These controlled starts and relic
grants do not establish ordinary acquisition, uninterrupted route or console
parity, or complete Bat, Alucard, castle or game support.

Provenance: selected `dra/71830.c:ControlBatForm/CheckWingSmashInput`,
`7879C.c:EntityFactory/EntityPlayerBlinkWhite/EntityWingSmashTrail/func_8011B480/func_80118894`
and `5D5BC.c:CastSpell/GetSpellDef`, pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
