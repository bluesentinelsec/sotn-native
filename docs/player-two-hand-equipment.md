# Selected two-hand equipment ownership

Owned Namakura84 now selects through the ordinary equipment menu and attacks
through the existing real [weapon0/shared slot16 owner](shared-ordinary-hands.md).
Its actual category5 definition, negative base attack, lock18, animation41,
blueprint48, variant0, palette0, properties, sprites and groups1/2 remain private
metadata or existing native services. Either attack button uses the corresponding
actual hand callback. No new attack, row pool, grant or ordinary-save field is added.

## Equipment and inventory

One spare Namakura reserves both original equipment words. It consumes one copy,
returns the selected hand's previous item first, then returns the other hand's
item unless that item was already a two-hand weapon. Replacing or unequipping a
two-hand weapon clears its other word and returns the one owned copy once.
Changing from one two-hand item to another does not return its duplicated word
as a second physical copy. Other two-hand weapon families remain unavailable.

The native menu now invokes actual confirmation bookkeeping. A confirmation of
an already worn item decrements the spare byte before returning the displaced
item. Above99, original AddToInventory refuses another increment, preserving the
unchecked byte rather than clamping it. Reselecting a worn item with255 spares
therefore leaves254, as the original menu does. The existing programmatic equipment
setter remains idempotent when the item is already selected. The native item
list requires an owned spare or its fixed virtual empty row.

Displaced items return through the existing acquisition-order owner. Physical
returns retain the original first-empty selection and type filtering. Virtual
empty hand/head/armor/cape/accessory rows stay out of that search; their native
stock bytes stay unchanged. This retains their fixed-row adaptation without
adding original virtual stock indicators to default grants or saves. A failed
confirmation retains the full character, stock, ordering and derived stats.

Shop capacity deliberately counts both actual equipped words, as the original
Librarian does, even though the weapon represents one physical copy. Selling
continues to remove only spare inventory. Accessory84/Duplicator remains a separate
body-table identity and confers no effect from holding Namakura.

Character validation rejects an unpaired Namakura. Native player and development
equipment binding require both actual words; an invalid development request
fails before collecting stock. Startup-style binding rereads committed equipment
between hand operations, so the second word does not grant another copy after
the first reservation has already filled both. v9 encodes the existing two words
and spare byte. No additional persistence format or program state is serialized.

## Shared native ownership

Both changed hand programs flow through the existing menu commit. Real old hand
entities are retired, including an actual raised shield, before subsequent control.
The native owner loads each hand's actual weapon0 program/palette binding and
retains the original single shared slot16 for attacks. Form entry and late effect
failure/retry preserve the earlier ordinary hand owner's boundaries. Continue
restores the paired equipment and next real control through the existing native
candidate/session path. These selected operations do not close whole-pass rollback.

## Evidence and limits

Public authored tests cover one-copy/two-word reservation, displaced sword/shield
returns, opposite-hand replacement, empty/same-item selection, real menu
confirmation, stock255/254, original shop capacity, paired restoration, unpaired
rejection and a late metadata failure after tentative inventory changes.

Private Debug/ASan traces match byte for byte across26,208 complete original menu
confirmations, real AddToInventory execution, all168 physical hand stock bytes,
both equipment words and the full169-entry ordering table. Contexts cover one-hand,
two-hand and empty items, both selected hands, identity/reversed orders, spare
counts1/2/98/99/100/200/255 and collateral stock through255. Cursor/stat-preview/
render/sound services remain explicit adapters. The original virtual-empty stock
indicator is normalized to1 only in the reference context; native virtual stock
is not granted or compared as physical inventory.

Separate comparisons cover30,720 complete actual paired-Namakura hand-helper
calls across normal states/substates, directions, chain/allocation/curse and all
eight grunt outcomes;3,200 complete property/stat/buff/poison/SDK-root contexts.
Reservations, stock service, equipment-count and RNG/CLZ adapters retain their
boundaries. These are component contexts, not ordinary acquisition or console proof.

Native protocols exercise both hands, actual private sprite pixels, real existing
enemy HP loss, ordinary menu displacement/reselection, raised-shield retirement,
Mist/Bat/Wolf entry, late effect rollback/retry, startup-style binding and in-memory
v9 Continue with fresh control. Stock/protection/arrival/save-room/position,
byte255 and injected-failure contexts are supplied fixtures. No user save is written.
They establish no uninterrupted acquisition route, complete retail inventory UI,
all-family contention, global timing or console parity.

[Badelaire125](player-playtime-badelaire.md) now has its actual playtime/stat owner. Other weapon families,
full damage/equipment/familiar/shared ownership, [Alucard milestone gates](alucard-status.md)
and full [cold New Game](cold-start.md) remain incomplete. Default resources,
ordinary grants and save version9 are unchanged.

Adapted AGPL-3.0-or-later algorithms: `dra/menu.c:func_800FB23C`,
`dra/5D5BC.c:AddToInventory/GetEquipProperties`, `dra/6E42C.c:func_8010EDB8`,
`st/lib/e_shop.c` and `weapon/w_000.c` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`. Metadata, artwork and original tables
remain immutable private inputs.
