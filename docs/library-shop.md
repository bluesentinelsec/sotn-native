# Selected Library shop integration

The core shop service reads the original LIB stock and unlock tables from the
user's verified overlay. It preserves original stock order, milestone unlocks,
completed-game unlock, Jewel of Open, purchased-map and learned-scroll exclusions.
No stock rows, prices, text, artwork or reveal rectangles enter public source.

Purchases retain the original 99-copy limit including equipped copies in either
hand/accessory slot. Gem sales use only unequipped inventory and cap gold at
999999. Equipment acquisition retains existing inventory order. Relics use the
original found/active rule. Map purchase reveals high map bits without awarding
visited rooms. Scrolls retain original learned-spell identity and acquisition order;
this does not implement casting or the spell menu.

Every trade validates current eligibility, quantity, price, stock and funds.
Character/world drafts commit together after all metadata and map checks pass.
Malformed private input, unavailable offers and insufficient funds leave gold,
inventory/order, relics, spells and map progress unchanged. Ordinary pickup caps
remain independent of the stricter shop-owned-copy limit.

[Version 8 ordinary saves](native-saves.md) retain spell ownership/order and the
original completed-game unlock flag, with explicit migration from versions 1–7.
Map/relic ownership and purchased-map flags use the existing world payload.
Normal play does not grant the completed-game flag merely to unlock stock.

The placed Librarian now opens the private normal/completed-game option profile,
animates the selected original main/list panel geometry, and accepts buy/sell
navigation, quantity changes and shoulder paging. Confirm purchases or sales;
cancel returns to the main choices, then exits with the original farewell and
player-control release. Returning to the chair runs the original greeting and
can open the menu again. Held buttons use the original independent per-button
17-tick initial / six-tick repeat cadence; confirms remain edge-triggered.

The selected item list reads original equipment/relic/document names and sold-out
text. Glyphs use private original artwork and palettes; native glyph quads replace
temporary PS1 render textures. Main-panel collapse crops to the selected row.
The three shop speech requests use the existing XA owner, separately from SEQ;
stopping an old speech request does not retire the Librarian's sequence.

Full preview/detail panels, description fonts/icons, exact wipes/priority ties,
tactics, enemy list and sound test remain unintegrated. Optional service choices
report their missing support and remain in the menu instead of pretending success.
Library travel remains disabled. Unsupported purchased equipment can be owned,
but equipping it still fails explicitly until its behavior is integrated.

## Evidence and provenance

Public synthetic tests cover stock/unlock filtering, both-hand ownership limits,
equipped-gem exclusion, gold saturation, one-shot relic/map/scroll purchases,
malformed metadata/rectangles, atomic failure and independent save migrations.
Private native checks use this project's CMake libraries and the user's dump with
an explicit unlocked/gold fixture. They exercise original offers and v8 codec
roundtrips. A native room-4 sequence then exercises the placed introduction,
relic/map/scroll purchases, a two-copy gem sale, all three shop speech requests,
return of control, a second greeting and repeated exit. Debug and ASan/UBSan agree
at 1927 menu-sequence ticks / 481 renders, with successful process exits. A
subsequent explicit room-14 start writes slot 15 through the ordinary save-room UI
in an isolated local directory; fresh Continue retains gold, the sold gem count,
relic ownership, purchased-map flag and learned-spell order/mask. The real save
bank is untouched. This uses an explicit gold/gem fixture and room starts; it
does not establish uninterrupted-route or console parity.

Rules adapt `st/lib/e_shop.c`, `dra/5D5BC.c:LearnSpell` and
`dra/5087C.c:func_800F2288` at pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).

## Ordering correction found during presentation checks

The selected-row label was hidden because the native shared renderer sorted
original priorities backwards. The original `dra/42398.c` calls forward
`ClearOTag`, then `DrawOTag` at the first bucket; larger priorities draw later.
The native command sorter, actor list and player/weapon insertion now follow
that direction. Synthetic overlapping-color coverage checks actual foreground
composition. The private Lesser Demon arena/reward/save/fresh-Continue regression
still completes at its prior 3670 ticks / 918 renders. This corrects a shared
presentation defect; it does not prove all GPU packet ties or complete effects.
