# Selected stat-buff ownership and Fury Plate

Owned Fury Plate now supplies its original defense-buff producer. Actual equipped
armor refreshes the defense counter to at least512 after a surviving handled
contact, including a water update that loses no HP. Lethal, immune and absorbed
contacts do not refresh it. Longer existing buffs remain longer. This adds no
starting equipment, attributes, relics or ordinary save fields.

The native session owns the original sixteen DRA counters. Player updates borrow
their actual context and emit minimum-duration requests; they do not mutate that
owner while a working player update can still fail. The native caller commits
requests only after success. Character stats retain a derived active-bit snapshot,
without a second timer pool. New/Continue play initializes counters to zero;
connected stage replacement copies them after bootstrap without an extra update.
Room changes retain them. They are runtime state, never version-nine save data.

All sixteen nonzero words decrement once at the original HUD boundary, after
player and stage updates. The source switch's default includes the four words
without selected stat effects. A fresh Fury refresh therefore becomes511 at the
end of its birth frame. Spell-frozen updates and the separate release frame also
advance this HUD clock, while gameplay/player clocks remain held. Menus, map and
repeated drawing do not advance it. Dead-player timer holds do not stop this
separate HUD owner. Game-over presentation stops the ordinary HUD update.

The selected derived-stat service adds defense/attack bonuses before original
signed-word and0..999 clamps, and strength/intelligence/luck bonuses before the
99 equipment-bonus cap and original stat multipliers. Resistance buffs retain
their original element bits; US stone resistance supplies immunity. Current
attributes bind before damage, so expiry does not leave a stale defense bonus.
Defense also uses original positive-word SDK square-root normalization rather
than generic integer square root. Its normalized table value is computed with
integer arithmetic; no private table is embedded. This preserves the original
high-CON rounding, including841, and the original signed-word defense wrap.

These APIs reconstruct selected stat calculations and counter operations. Only
Fury supplies a new live item producer in this batch. Consumables, other equipment,
all weapon-specific stat rules and other buff producers remain incomplete. A
mask or Give operation is not an implemented item, control, acquisition or effect.

Public authored tests cover all sixteen counter boundaries, minimum versus4096,
invalid-state rollback, pre-clamp negative defense, signed wrap, high-CON SDK
rounding, capped stat bonuses, original multipliers, attack/defense clamps, US
stone immunity, unchanged ordinary character state, actual-context requirements,
surviving/lethal water requests and late failed-player request rollback.

Private Debug and ASan/UBSan traces match byte for byte. Original instructions
match5,880 stat contexts:4,096 actual private Fury/empty-hand masks and1,784
authored-copy CON0..891/defense-word boundary cases. They also match2,048 whole
sixteen-word HUD, Give, Clear, Reset, Fury-minimum and Get contexts. Comparisons
execute original stat/SDK math with equipment-count and GTE leading-zero adapters.
The original source image remains immutable; authored metadata copies are separate
fixtures. These are selected comparisons, not all-equipment or console evidence.

Muted native Debug and ASan/UBSan checks equip actual private Fury definitions,
verify zero-loss water refresh512→511 and defense+20, repeated-draw/menu holds,
34 real Summon Spirit frozen/release HUD updates, actual connected stage handoff,
expiry with fresh player/character defense, and fresh in-memory Continue reset.
They use explicit equipment/Holy grants, water/dry fixed-floor poses, a queued
arrival, expiry protection and an explicit Give4096 fixture. A separate ordinary
Laboratory zombie encounter, with only an explicit Fury equipment grant, verifies
actual queued damage, surviving refresh, protection and no replay. No check writes
user saves or establishes ordinary acquisition, uninterrupted routes, complete
equipment/Alucard/castle or console parity.

Provenance: selected `dra/5D5BC.c` stat-buff/HandleDamage operations,
`dra/5F60C.c:DrawHudSubweapon`, `dra/menu.c` calculations, `dra/game_handlers.c`
play reset and selected original SDK math, pinned `sotn-decomp`
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
Definitions, artwork, private SDK bytes and comparison traces remain private.
