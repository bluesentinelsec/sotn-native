# Selected ordinary weapon0 swords

The actual shared [ordinary hand owner](shared-ordinary-hands.md) now accepts
twenty original one-hand weapon0 items: the existing Gladius86 plus Tyrfing83,
Scimitar87, Cutlass88, Saber89, Falchion90, Broadsword91, Bekatowa92, Damascus93,
Hunter94, Bastard96, Talwar99, Sword of Hador104, Luminus105, Harper106, Gram108,
Mormegil110, Terminus Est116, Dark Blade118 and Mourneblade122.

Owned selection uses the ordinary inventory menu and either hand. The private
definition supplies original attack, defense, elements, critical chance, lock,
invincibility and factory metadata. Selected definitions validate their actual
one-hand category, animation41, blueprint48, chain128 and absent special/stock/MP
cost. Existing grants and v9 saves are unchanged. No new weapon or effect pool is
introduced.

## Callback, properties and palette ownership

The actual blueprint48 parameter retains variant0/1/2 and hand identity through
reservation, child birth, validation and callback. The current shared slot16 owner
reads that variant's private descriptor, animation, sound and hitbox streams.
Legacy raw-frame callers also resolve the supplied descriptor variant. Allocation
failure, casting-grunt order, actual property RNG, group1/2 cooldowns and replacement
retain the ordinary hand owner's boundaries.

Signed base attack is retained until original stat/body/buff calculation and caps.
Tyrfing's negative base attack is not replaced by an invented HP condition.
Hunter sword retains its actual hand defense contribution. Mormegil and Terminus
Est use the original elemental words through the existing native enemy damage
owner. Their metadata alone does not establish all enemy reaction/palette parity.

Weapon0 palette selection uses the actual definition's index into its bounded
five-entry private pointer table. Each load copies all384 words, including adjacent
private data beyond the seven named color arrays, as the original callback does.
Terminus Est selects index2 and Mourneblade index1. All twenty items retain the
normal callback's original destination palette words and per-hand texture bank.
The other hand's resident colors and unrelated VRAM remain owned by their existing
producers. Bad palette references fail before either upload or program binding.

Mourneblade hit effect130 reaches the existing real common actor7/Soul-orb owner.
The collision dispatcher creates an actual orb before enemy damage or immunity.
Original homing, age16 contact, stage-row retirement and first-pending deferred
eight-HP healing remain with [Soul orbs](soul-steal-orbs.md). There is no immediate
HP addition, replacement healing effect or guaranteed allocation.

Same-family menu changes still replace the actual hand entity and load the newly
selected palette without reinitializing an unrelated weapon program. Menu/draw/
freeze holds add no callback or RNG update. Full Continue reconstructs the actual
equipped item, private resource binding and next ordinary control through the
existing v9 restore path; no program/palette data is serialized.

## Evidence and open gates

Public authored tests cover actual variant2 shared factories and malformed
parameter rejection, negative attack caps, distinct raw-frame descriptors,
palette2 and its twenty-fourth row, the other hand's colors, invalid palette
index and truncated full-copy failure without replacing resident colors.

Private property traces exercise64,000 supplied contexts across all twenty items,
base STR/LCK, actual opening body equipment, buff masks, poison and critical RNG.
Debug, Release and ASan traces agree byte for byte and match complete original
GetEquipProperties/CalcAttack execution with the private SDK root. Equipment-count,
RNG-result and CLZ services remain explicit adapters. Separate original palette
comparisons cover44 complete callbacks for all22 original family0 items/both hands,
their actual384-word destination writes, upload requests and untouched CLUT words.
The upload is an explicit adapter. Six compiled native palette-loader contexts
separately compare all384 actual private words against those original results,
including full owner-memory/surrounding-VRAM sentinel preservation and byte-identical
Debug/ASan outputs. These supplied component contexts and original comparisons do
not enable the two excluded items.

Native protocols exercise owned menu use, private sprite pixels and real enemy
HP/death/EXP for each selected item in either hand; Mourneblade's actual orb,
contact and deferred healing through ordinary and supplied immune targets; selected
late effect failure/retry; menu/draw holds and in-memory v9 Continue. Stock, health,
protection, arrival, enemy-position/immunity, save-room and injected failure contexts
are supplied fixtures. They are not ordinary acquisition, uninterrupted routes,
whole-pass rollback, full contention or console evidence. No user save is written.

Selected [Namakura84/two-hand ownership](player-two-hand-equipment.md) now supplies
its actual inventory/menu/restoration and shared callback owners. Selected
[playtime/Badelaire125](player-playtime-badelaire.md) now supplies its actual
four status words, hours-dependent attack and shared variant3 owner. Other weapon families,
equipment specials, complete enemy feedback, familiars, shared timing/presentation,
Alucard and castle remain incomplete. All six [milestone gates](alucard-status.md)
and the full [cold New Game gate](cold-start.md) remain open.

Adapted AGPL-3.0-or-later algorithms: `weapon/w_000.c`, `weapon/shared.h`,
`dra/menu.c:InitWeapon/CalcAttack`, `dra/5D5BC.c:GetEquipProperties`,
`dra/6E42C.c` and `st/collision.h` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`. Private metadata, tables and artwork
remain external immutable inputs.
