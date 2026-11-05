# Selected Ballroom Mask contact reduction

Ballroom Mask28 was already equippable through its private additive metadata.
Actual queued contacts now also apply its conditional reduction in the original
`HandleDamage` order. The real head slot supplies the check; inventory copies and
other equipment slots do not count. One head slot permits one equipped Mask.
The original unreachable two-head count is not exposed as a native capability.

After weakness/resistance, US stone-immunity removal, immunity and absorption,
matching effects in mask `0xF980` reduce attack by `attack / 5`, using integer
truncation. The reduction runs once before defense, poison doubling and the
existing reaction selection. Subtracting one fifth preserves attacks below five;
it is not rounded percentage damage after defense. Immunity and absorption retain
their earlier results, including full-HP healing. New stone uses the modified
attack before twice DEF; existing stone and environmental kinds retain their
maximum-HP fractions. Ordinary water keeps its existing absolute-clock one-HP
owner. No new effect, sound, timer, RNG stream or resource pool is introduced.

Talisman still rolls at its original later boundary. Guard may now win when the
modified attack crosses DEF, while its poison/curse-bearing exclusion remains.
Surviving damage gives Blood Cloak the actual reduced HP loss and retains Fury's
existing minimum512 request. Deferred healing still commits first; lethal checks
use the resulting HP. Queued contacts retain strongest/first-equal raw power
priority before this modifier. A weaker raw contact cannot replace the winner
because the equipped Mask would reduce that winner.

The actual character snapshot binds equipped head and derived attributes before
consuming queued damage. A supported menu selection therefore affects an already
queued raw contact on the next ordinary player update. Menus/maps/draws hold that
contact and HP; frozen/release updates retain the existing pending-heal/contact
boundaries. Failed player updates retain HP, RNG, equipment snapshots and queued
requests. Unequipping the Mask removes its conditional modifier through the same
head-slot binding. Existing inventory and v9 saves retain the item; ordinary
starts and grants remain unchanged.

## Evidence and remaining scope

Public authored tests cover owned selection, matching and excluded effect bits,
integer thresholds, single reduction for compound elements, weakness/resistance order, defense/poison order, immunity and
absorption priority, US stone-immunity removal, new/existing stone, environmental
and ordinary-water results, guard crossing, Talisman, lethal avoidance, raw-contact
priority, prior healing, Blood Cloak/Fury and failed-update/freeze/release holds.
Fixtures contain no original item definitions or artwork.

Private comparisons match201,600 original full `HandleDamage` contexts, including
72,000 equipped inputs with matching elements. They compare actual HP and response,
immunity/absorption/stone/environment/poison/guard/lethal outcomes, Blood Cloak,
Fury and absence of modifier RNG. Explicit recalculation/HUD, single-head
CheckEquipmentItemCount and AddHearts adapters isolate the calculation. They do
not compare the unfinished native injury-kind animation/effects or full shared RNG.

Muted native checks use a placed Library Ectoplasm's real queued curse contact
(power6). While that contact waits in the actual menu, owned Mask28 selection binds
its private DEF2 and conditional reduction: HP70 becomes67, versus66 with the same
DEF and no conditional modifier. Existing curse/recoil/audio consume their original
selected two reaction RNG calls; the Mask adds none. Checks retain pending raw
contact/HP/gameplay clock through six menu updates, then verify120 updates without
replay, owned unequip and repeated-draw holds. Actual CMake app/platform results
are recorded in the feature PR. Explicit arrival, owned item, actor position and
later protection fixtures are not acquisition, uninterrupted routes or console
evidence; no HP/MP/contact/RNG/clock grants or user-save writes establish this check.

Remaining hurt promotion/kinds, elemental/interrupted death, stone breadth,
other equipment and shared contention retain the [damage inventory](player-contact-absorption.md).
Milestone1 remains open at26/34 handlers plus common/cross-owner work; Alucard and
castle remain incomplete.

Provenance: `dra/5D5BC.c:HandleDamage`, `692E8.c` contact dispatch and the existing
native equipment/damage owners, pinned AGPL-3.0-or-later `sotn-decomp`
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`. Private metadata/artwork stay private.
