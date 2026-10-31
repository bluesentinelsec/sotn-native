# Selected contact absorption and damage inventory

Queued elemental absorption now owns the original immediate response 5: HP gain,
silent Blink44 followed by Blink48, a green HP number and twelve updates of the
existing protection timer corresponding to original timer 14. It preserves normal
control without starting a hurt reaction. Owned Gold, Ruby, Opal, Topaz and Beryl
circlets now equip and use their actual private absorption attributes. Owned
Cat-eye Circlet converts kind 7 contacts into twice their adjusted attack power.
Ordinary grants, equipment ownership and version-nine saves remain unchanged.

Weakness/resistance run before immunity and absorption. US stone immunity removes
the stone effect before the remaining absorption test. Ordinary elemental
absorption takes priority over Cat-eye, so its amount is never doubled by that
item. A nonimmune Cat-eye kind 7 result doubles the adjusted power and then takes
the original minimum of one. Neither branch subtracts defense, doubles for poison,
applies a new ailment, grants Blood Cloak hearts or refreshes Fury Plate.
Existing poison and curse timers continue; absorption does not cure them.

Actual pending blood/orb/Mail healing still applies first. Absorption is a separate
same-update contact result, so both sources retain their own original numbers and
sound behavior. Full HP still requests both absorption flashes and its number;
stone also retains both absorption flashes. The latter differs from deferred
healing's stone suppression. The amount is captured before the HP cap, alongside
the pre-controller fractional position. Numbers preserve the original unsigned
sixteen-bit interpretation and four least significant decimal digits, including
amounts above 32767. The selected maximum 39,996 can arise from weakness and Cat-eye.

Player-update failure retains HP, pending healing, queued contact, RNG, protection
and the prior request. Shared-runtime failure preserves its own copied children
and row reservations. Actual factories, slots 56..63 and twenty-row number leases
retain their optional exhaustion rules; no separate pool is introduced. Duplicate
flashes share the existing full-parameter suppression. Freeze/release clear the
one-update request while holding HP, queued damage and protection; effect updates
retain the original flag10000 rules. Menus/map/draws hold gameplay, and room rebases
translate captured sources without another update. Stage/Continue reset transient
effects while preserving ordinary equipped items and committed HP.

## Remaining damage ownership

The following inventory is based on `dra/5D5BC.c:HandleDamage` and its
`dra/692E8.c` dispatch, not an assertion of complete reactions. Each selected
owner retains its own evidence boundary.

| Original response | Current native boundary | Remaining work |
| --- | --- | --- |
| 0, immunity | Element prepass suppresses HP loss | Original text/Blink43+51, zero number and helper timing |
| 1, high-defense guard | Ordinary minimum-one damage remains selected | Distinct guard outcome, palette/timers, voice and number |
| 2, Talisman dodge | No live Talisman producer | Actual count/LCK/RNG selection, effects and recovery |
| 3, hurt | [Queued contact owner](player-water-contact.md) delegates to selected physical/ailment helpers | Original kind promotion0/1→2/3/4, distinct kinds2/3/4/5/16..18, collision/recovery and all dependent effects |
| 4, lethal | [Selected physical death](laboratory-game-over.md) | Full original elemental deaths, capture/effect/fade branches and numbers |
| 5, absorption | This selected owner, actual circlet attributes and Cat-eye kind 7 | Ordinary acquisition/encounter breadth and all-ability contention acceptance |
| 6, lethal water | [Native water death](player-water-death.md) | Global fade recovery and remaining presentation/timing boundaries |
| 7/8, stone/new or existing | [Selected petrification](player-petrification.md) | Statue variants, children/fragments, full lethal branches and numbers |
| 9, water pain | [Ordinary water contact](player-water-contact.md) | Remaining dependent effects/timing and broader original contexts |

Ballroom Mask's conditional reduction and remaining special equipment also need
actual owners. This batch does not replace the selected physical-damage adapter
or implement ordinary damage/guard number types. Full damage/ailments/death,
equipment/familiars/sharing, Alucard, castle and console parity remain unfinished.

## Validation and evidence

Public authored coverage exercises actual owned selection, prepass priority,
Cat-eye eligibility, minimum-one results, twelve-update protection/expiry,
existing ailments, uncapped/full-HP/stone requests, simultaneous deferred healing,
room translation, freeze holds and late-player failure. Shared-owner tests cover
two independent numbers, duplicate flashes and correct pickup-sound provenance,
child/row exhaustion and rollback. All 190 tests pass in Debug, Release and macOS
ASan/UBSan; final targeted tests, packaging and 34 Python checks also pass. An
initial Release CLI process-start timeout under concurrent validation passed on
the full rerun without changing its timeout or acceptance criteria.

Private Debug/ASan traces match byte for byte. Comparisons match 5,184 supplied
contact contexts, including 1,728 response 5 results, against original early
HandleDamage and actual response 5/helper14 instructions. Extended number checks
match 672 contexts and 73,248 updates with all four GT4 and sixteen tile packets.
Recalculation, equipment-count, HUD, factory, number and sound boundaries are
adapted; these are not full original frames or console replays.

Muted native checks start at the original Library room 9 Corpseweed placement and
wait for its AI to create a real seed. A positioned seed's actual queued poison
contact with owned Topaz restores HP 20→48, requests protection 12 and both real
eight-row flashes, omits poison/recoil/pickup sound and births an actual twenty-row
number contributing ten frame pixels. Supplied Cat-eye kind 7 plus actual deferred
Mail produces independent 6+1 numbers, including at full HP. Menus/repeated draws
hold them; a real Summon sequence advances effects across 33 frozen updates while
holding protection and preserves the release-only hold. Number retirement,
connected-stage HP/head retention and an explicit in-memory version-nine
save-room Continue with cleared transients are checked without user-save writes.

The CMake-built application separately exercises the real placed-plant/seed/contact
path and shared number/flashes, then checks 120 updates without replay and normal
SDL quit. Equipment/health, stage-arrival, projectile-position, post-contact
protection and supplied kind 7 fixtures are separate evidence from ordinary item
acquisition, uninterrupted traversal or complete encounter/console parity.

Provenance: selected `dra/5D5BC.c:HandleDamage`, `dra/692E8.c` response 5,
`dra/6DF70.c:func_8010E168` and existing shared effect/number owners, pinned
`sotn-decomp` `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`
(AGPL-3.0-or-later). Item definitions, artwork, SDK tables and comparison traces
remain private.
