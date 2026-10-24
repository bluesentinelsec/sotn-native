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

The supported attack implementations remain empty hands, Gladius, and Mace.
Ordinary armor/head/cape/ring records with integrated additive effects can be
equipped; conditional/transforming items remain owned but explicitly unavailable
for use. This does not claim every passive effect, item category, transformation,
spell, or weapon is implemented. Stage-session temporary candle/enemy flags stay
separate from permanent item progress.

This PR establishes the tested ownership and calculation boundary. The next
stacked integrations connect room pickups, equipment presentation and combat,
subweapon/experience behavior, and versioned save migration. Read the final
stack's updated scope before describing any of these as playable features.

Public tests use synthetic definitions and cover ownership, duplicate hand use,
stock limits, slot compatibility, stats, malformed text, inventory-order
validation, and unchanged state after failed operations. A private check decoded
all 259 item records from the user's dump and verified Gladius/Hide cuirass
calculations; it does not establish complete original-game parity.

Selected logic derives from `src/dra/5D5BC.c:AddToInventory` and
`menu.c:UpdateEquipStatBonuses`, `CalcAttack`, and `CalcDefense` at the pinned
reconstruction, under AGPL-3.0-or-later. Conditional effects remain explicit
integration work rather than silently borrowing unrelated item behavior.
