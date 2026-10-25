# Library shop transaction foundation

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

This is a transaction service, not a playable shop menu. The placed Librarian
introduction still reaches its shop-ready gate; menus, preview/detail panels,
tactics, enemy list, sound test, purchase speech and exit are subsequent integration.
Library travel remains disabled. Unsupported purchased equipment can be owned,
but equipping it still fails explicitly until its behavior is integrated.

## Evidence and provenance

Public synthetic tests cover stock/unlock filtering, both-hand ownership limits,
equipped-gem exclusion, gold saturation, one-shot relic/map/scroll purchases,
malformed metadata/rectangles, atomic failure and independent save migrations.
Private native checks use this project's CMake libraries and the user's dump with
an explicit unlocked/gold fixture. They exercise original offers and v8 codec
roundtrips; they do not establish menu, uninterrupted route or console parity.

Rules adapt `st/lib/e_shop.c`, `dra/5D5BC.c:LearnSpell` and
`dra/5087C.c:func_800F2288` at pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
