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

Selected ordinary weapon attacks include empty hands, Basilard, Gladius, Short sword,
Mace, and Alucard sword/shield through the actual [shared hand owner](shared-ordinary-hands.md).
The selected [ordinary weapon0 swords](player-basic-swords.md) add19 one-hand items;
[Namakura/two-hand ownership](player-two-hand-equipment.md) now adds one-copy/both-word
reservation, ordered displaced returns and paired restoration. The native menu
performs actual confirmation, including spare decrement before a capped return.
Programmatic equipment setting remains idempotent. Virtual empty rows stay out of
physical first-empty searches without receiving stock. No default grants or v9
fields are added; remaining two-hand families and equipment effects stay open.
Selected [basic shields](player-basic-shields.md) add Leather5, Knight6, Axe Lord8, Goddess11,
Shaman12 and Alucart167
through the same owner; their shield spells and other shield families stay unavailable.
Selected [basic fists](player-basic-fists.md) add Knuckle duster85 and Jewel knuckles97;
special-bearing Iron Fist and Fist of Tulkas stay unavailable.
Selected [plain weapons](player-plain-weapons.md) add Morningstar, Mablung, Takemitsu,
Nunchaku, Red Rust, Holy rod, Star flail, Combat knife and Great Sword; [Gurthang](player-gurthang.md) adds its
QCF special; the two-handed ones use Namakura's paired reservation.
The original opening body equipment is supported; see [opening scope](opening-equipment.md).
Spike Breaker now equips and drives the selected [Chapel spike corridor](chapel-spikes.md).
Its original ownership, armor stats and equipped slot use the existing save fields;
other stages still require their own spike-controller integration.
Owned Bloodstone now equips as an accessory and doubles selected
[blood absorption requests](enemy-blood.md) from selected playable
[Dark Metamorphosis](player-dark-metamorphosis.md).
Owned [Fury Plate](stat-buff-ownership.md), [Healing Mail/Mystic Pendant](hud-healing-resources.md)
and [Blood Cloak](player-blood-cloak.md) now have their selected actual gameplay
owners. Their availability does not establish all equipment effects; see the
[current Alucard checklist](alucard-status.md).
Owned [Library Card166](player-library-card.md) now selects and consumes through
ordinary hand input and actual Library travel. Duplicator84 preserves this selected
consumable in either accessory. The subsequent [owned medicine owner](player-medicines.md)
adds17 equippable items144..162, excluding Life Apple146/Hammer147 as required by
the original menu. Either Duplicator preserves them; Ankh of Life80 scales Potion/
High Potion healing once even with two. Their cures, buffs, HP/MP and shared effects
now run. [Owned food29..70](player-food.md) now adds actual weapon21 use, raw healing,
Peanuts, Toadstool and Meal Ticket inventory/text pickup. Food and medicine retain
the original zero displayed weapon attack; raw food attack remains healing power.
[Owned Pentagram73/Bat Pentagram74](player-pentagrams.md) now selects and consumes
through either hand, with actual weapon17,25 shared rows, private GT4/GT3 drawing,
original SDK/GTE breakup and24-frame native global attacks. Duplicator preserves
both items; last-copy use retains the resident program.
[Owned Power of Sire72](player-power-of-sire.md) now selects and consumes through
either hand, with actual shared weapon18 parents/bursts, native global attacks,
private presentation and real shared flash/shake/particles. Its live weapon attack
uses retained metadata even after last-copy consumption.
[Owned Neutron Bomb71](player-neutron-bomb.md) now selects and consumes through
either hand, with its actual shared weapon19/GT4, private animation/rendering and
global damage pulse. Its live STR/body/buff/poison-scaled weapon attack remains
distinct from food's raw healing value. Duplicator preserves the bomb.
Other consumable/equipment effects and the familiar completion gates stay open.
Owned Gold/Ruby/Opal/Topaz/Beryl and Cat-eye circlets now connect their actual
[contact-absorption owner](player-contact-absorption.md), with unchanged ordinary
ownership and save fields. This does not establish ordinary acquisition routes.
Owned God's Garb and Nauglamir now use their actual additive attributes through
selection; the [selected guard owner](player-contact-guard.md) consumes real high
DEF without granting stats or items in ordinary starts.
Owned [Talisman](player-contact-talisman.md) now uses the actual accessory count,
derived LCK and one libc roll for contact dodge, with shared visuals and original
recovery. This establishes no acquisition or remaining injury parity.
Owned [Ballroom Mask](player-ballroom-mask.md) now applies its original conditional
contact reduction as well as its existing additive attributes. Its single actual
head slot, pre-defense ordering and existing inventory/save ownership remain intact.
Ordinary armor/head/cape/ring records with integrated additive effects can be
equipped; conditional/transforming items remain owned but explicitly unavailable
for use. This does not claim every passive effect, item category, transformation,
spell, or weapon is implemented. Stage-session temporary candle/enemy flags stay
separate from permanent item progress.

[Owned Magic Missile25](player-magic-missile.md) now selects and consumes through
either hand with original chain4, shared bow/arrow factories, private weapon22
sprites/animation and native physical damage. Raw right-hand arrows retain
params0 and count toward the left-hand item25 limit. Last-copy use retains the
actual loaded hand program. No ordinary starts or v9 save fields change.

## Native equipment loop

Enter/Xbox Start opens or closes the supported pause menu when normal control is
available and a hand attack/hit reaction has finished. X/Space or Xbox A selects;
keyboard A or Xbox Y goes back. Equipment exposes the original seven slots.
Arrows/d-pad select a slot or owned item; unsupported selections show an error
without consuming stock. [Selected relic controls](relic-menu.md) expose owned
supported relics.
Spells/settings/familiars remain visibly unavailable.
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
invoking undefined C division. Selected [queued contacts](player-water-contact.md)
now connect poison/curse/stone and selected equipment passives; full elemental
reactions and other special item behavior remain unfinished.

The [progression service](character-progression.md) now awards EXP and updates
levels/resources. Version-nine [native saves](native-saves.md) retain this supported
progress; [selected subweapons](laboratory-subweapons.md) consume hearts and damage
enemies. Versions 1/2 migrate explicitly from their earlier limited profile.

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
