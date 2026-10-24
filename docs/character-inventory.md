# Character inventory foundation

The character core owns ordinary inventory counts, acquisition order, seven
original equipment slots, and character progress. Inventory counts exclude
currently equipped copies. Picking up another copy adds stock up to 99; equipping
exchanges one owned copy with the previous item. Empty hand/head/armor/cape/ring
choices are virtual, not consumable inventory. Invalid, incompatible, unowned,
and unsupported equipment leaves state unchanged.

All 169 hand and 90 body definitions, names, descriptions, icons, and palettes
remain in private DRA data. Public C decodes bounded 52-byte hand and 32-byte body
records. It never casts PS1 pointers into native pointers. Original inventory
order changes, STR attack contribution, body stat bonuses, CON defense, and
attribute clamps follow selected reconstructed paths. The native Laboratory
profile remains explicit: level 1, 70 HP, 20 MP, base stats 6, and 10/50 hearts.
It is not the original new-game entry.

The supported attack implementations are empty hands, Basilard, Gladius, and Mace.
Ordinary armor/head/cape/ring records with integrated additive effects can be
equipped; conditional/transforming items remain owned but explicitly unavailable
for use. This does not claim every passive effect, item category, transformation,
spell, or weapon is implemented. Stage-session temporary candle/enemy flags stay
separate from permanent item progress.

## Native equipment loop

Enter/Xbox Start opens or closes the supported pause menu when normal control is
available and a hand attack/hit reaction has finished. X/Space or Xbox A selects;
keyboard A or Xbox Y goes back. Equipment exposes the original seven slots.
Arrows/d-pad select a slot or owned item; unsupported selections show an error
without consuming stock. Spells/relics/settings/familiars are visibly unavailable.
Gameplay and its elapsed timer stop while the menu owns control; music continues.

The renderer reads the original font, portrait, item icons, and palettes from
private inputs and uses the original 384-pixel menu resolution. It implements
selected status/equipment panels, not complete original menu layout, transition
animation, sorting, descriptions, settings, or game-specific display-aspect parity.
The supported cape choices update the original four cape colors in Alucard's
palette without modifying borrowed resources.

Globe tables now break and produce their original persistent pickups. Equipment
icons follow selected original fall/expiry/collection rules; acquisition and
permanent Laboratory item flags survive room revisits. Ordinary candle/enemy
flags remain transient. Room 10's globe yields Hide cuirass, room 5's yields
Cloth cape, and room 4 has Basilard. Development starts 4, 5, 6, 9, and 10 allow
focused checks; they do not run original stage entry. Room 10 starts at a clear
nearby location; jump left onto the ledge when collecting its item.

Body stats and effective LCK now feed attacks and ordinary physical damage.
Original critical selection uses base stats separately from effective LCK.
The zero-effective-LCK remainder edge remains explicitly unsupported rather than
invoking undefined C division. Elemental ailments and special item passives
remain outside the supported physical-contact slice.

The [progression service](character-progression.md) now awards EXP and updates
levels/resources. Save-format expansion and subweapon use follow in the next
stacked integrations. Until that migration, native version 1/2 saves retain only
the earlier checkpoint fields and do not preserve this new inventory.

Public tests use synthetic definitions and cover ownership, duplicate hand use,
stock limits, slot compatibility, stats, malformed text, inventory-order
validation, and unchanged state after failed operations. A private check decoded
all 259 item records from the user's dump and verified Gladius/Hide cuirass
calculations. Native LLDB checks broke a globe, collected/equipped Hide cuirass,
froze gameplay in the menu, collected/attacked with Basilard, and verified that
zombie contact removes 8 HP with Hide cuirass instead of 10. These checks do not
establish complete original-game parity.

Selected logic derives from `src/dra/5D5BC.c:AddToInventory` and
`menu.c:UpdateEquipStatBonuses`, `CalcAttack`, and `CalcDefense` at the pinned
reconstruction, under AGPL-3.0-or-later. Conditional effects remain explicit
integration work rather than silently borrowing unrelated item behavior.
