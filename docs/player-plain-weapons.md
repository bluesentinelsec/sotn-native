# Selected Morningstar, Mablung and Takemitsu

The actual shared [ordinary hand owner](shared-ordinary-hands.md) now accepts three
more original weapons without weapon specials: Morningstar129 (weapon3), Mablung
sword124 (weapon57, descriptor3) and the two-handed Takemitsu27 (weapon36). Their
ordinary callbacks are identical after symbol normalization: Mace's weapon2 template
from its own private descriptor, plus additive blending (BLEND_TRANSP|BLEND_ADD).
Nunchaku's weapon33 callback differs (pose12 double sound) and remains unsupported,
as do Red Rust's failure chance and Muramasa's attack-dependent behavior.

Owned selection uses the ordinary inventory menu and either hand. Definitions
validate each item's actual category, family, kind, animation, blueprint48, variant
and absent special items. Takemitsu shares Namakura's two-hand ownership: one owned
copy occupies both actual slots, menu confirmation returns displaced items, and
character/player/development bindings reject an unpaired Takemitsu before grants.
Defaults, grants and v9 saves are unchanged. No new pool is introduced.

The callback uses the real slot16 owner, factory48, group1/2, actual property RNG,
private animation/hitbox/sound tables and per-hand sprite banks/palettes.
Morningstar adds its descriptor palette5. Native per-hand program data is untouched.

## Rendering

Original LoadWeaponPalette copies 24x16 words for each. Takemitsu's copy crosses its
program's live g_HandId word, so the native loader binds the actual hand in a
temporary upload copy, as for the basic shields; immutable program and artwork bytes
stay untouched. Morningstar and Mablung copies end before their hand words. Nonzero
palette indices fail before any upload.

## Evidence and remaining work

Public tests cover descriptor locations, additive blending versus Mace, variant
bounds and untouched program data; Takemitsu's paired reservation and unpaired-
restore rejection; and the live hand word in its palette upload.

Private original-instruction comparisons run 8,000 complete callbacks in 192
contexts for all three programs (Debug and ASan traces byte-identical) and six full
384-word palette copies. The compiled native loader matches that reference in both
hands, changing only its texture and CLUT words.

Native protocols (Debug and ASan, LLDB, muted) exercise real menu equipment in both
hands, private sprite pixels, real enemy HP damage, Mist retirement and in-memory
v9 Continue (Mablung+Morningstar and paired Takemitsu), including unpaired-save
rejection. Supplied stock, protection, position, relic, arrival and save-room
contexts are not acquisition, uninterrupted-route or console evidence. Other
weapon families, specials, full sharing and all six [milestone gates](alucard-status.md)
remain open.

Adapted AGPL-3.0-or-later algorithms: `weapon/w_002.c`, `w_003.c`, `w_036.c`,
`w_057.c` and `weapon/shared.h` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`.
