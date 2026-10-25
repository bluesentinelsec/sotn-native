# Selected original poison mechanics

Normal Alucard now receives poison from supported original contact definitions,
including Corpseweed seeds. A surviving, non-stone, unprotected hit starts
`max(256, 4095 - CON * 16)` simulation ticks and requests the original reaction
voice. Constitution uses equipment-adjusted attributes. Curse takes precedence
when one contact carries both ailments. Equipment immunity/absorption runs before
this status path.

Poison halves newly created hand attack power, including unarmed attacks and the
selected ordinary shield. A weapon keeps its initial power if poison expires
while that attack is active. Ordinary incoming damage doubles after subtracting
defense and before applying the minimum of one. Stone and environmental damage
remain distinct paths. Poison does not inflict periodic HP loss.

The timer advances only with simulation. Inventory pause freezes it; room changes
retain it; the connected-stage adapter copies poison and curse independently.
Ordinary v8 saves retain character progress, not these transient timers: fresh
Continue starts without either ailment. No save-format or application-version
change.

## Evidence and limits

Public synthetic tests cover normal/poisoned defense arithmetic and caps,
immunity/absorption boundaries, invalid input without mutation, constitution
bounds, protected/lethal contacts, exact countdown without periodic HP loss,
curse coexistence, room changes, weapon-power snapshot/expiry and recovery.

Native LLDB checks insert and position original Corpseweed seed actors in an
explicit safe Library room, using a protected 999-HP development character.
The first contact removes 28 HP and starts 3999 ticks; a second poisoned contact
removes 56 HP. The sequence checks half-power Gladius attacks, inventory pause,
explicit stage arrival with both timers, expiry, recovered attack strength, and
an isolated ordinary save/fresh Continue that clears active poison. Debug and
sanitizer results are recorded separately; no real save slots are overwritten.
These inserted contacts and explicit arrivals are not an ordinary castle route
or console replay comparison.

Full poison palette/blink/text/recovery effects, medicine items, other elemental
reactions and general damage-kind/special-equipment parity remain incomplete.
Selected combat mechanics do not establish complete audiovisual/timing parity.

The implementation adapts `dra/5D5BC.c:{GetStatusAilmentTimer,GetEquipProperties,
HandleDamage}`, `dra/71830.c` and `dra/692E8.c` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later). Original
weapon, enemy, animation and sound data remain private runtime inputs.
