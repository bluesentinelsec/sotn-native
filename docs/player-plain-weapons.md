# Selected Morningstar, Mablung, Takemitsu, Nunchaku, Red Rust, Holy rod and Star flail

The actual shared [ordinary hand owner](shared-ordinary-hands.md) now accepts five
more original weapons without weapon specials: Morningstar129 (weapon3), Mablung
sword124 (weapon57, descriptor3) and the two-handed Takemitsu27 (weapon36),
Nunchaku21 (weapon33) and Red Rust26 (weapon35). Each runs Mace's weapon2 callback
template from its own private descriptor. Morningstar, Mablung and Takemitsu add
additive blending (BLEND_TRANSP|BLEND_ADD); Red Rust adds plain transparency;
Nunchaku adds none but repeats its swing sound and adds SFX_WEAPON_SWISH_B at
pose12, queued in that order. Holy rod130 (weapon4, palette1) and Star flail131
(weapon5, palette3) add SFX_ANIME_SWORD_B after their descriptor sound; Star flail
also widens its hitbox X and width by seven on poses6 and7 after the hitbox write.
Their back-forward [weapon specials](player-weapon-specials.md) 170/171 use each
program's descriptor1 (kind4, lock13, casting grunt kept). Muramasa's attack-dependent
behavior differs and stays unsupported.

Red Rust's player-side hand use follows dra/6E42C.c: after the actual hand factory,
one libc rand() from the character cursor decides the swing. A zero low-three-bit
result takes the same failure branch as curse (animation B5/B6/B7, 0x8012 lock,
voice 0x6E7) with no casting grunt; the spawned child then retires without
properties. Otherwise the ordinary grunt and attack follow. Exhausted factories never
roll.

Owned selection uses the ordinary inventory menu and either hand. Definitions
validate each item's actual category, family, kind, animation, blueprint48, variant
and absent special items. Takemitsu, Nunchaku and Red Rust share Namakura's two-hand ownership: one owned
copy occupies both actual slots, menu confirmation returns displaced items, and
character/player/development bindings reject an unpaired two-hand item before grants.
Defaults, grants and v9 saves are unchanged. No new pool is introduced.

The callback uses the real slot16 owner, factory48, group1/2, actual property RNG,
private animation/hitbox/sound tables and per-hand sprite banks/palettes.
Morningstar adds its descriptor palette5. Native per-hand program data is untouched.

## Rendering

Original LoadWeaponPalette copies 24x16 words for each. Takemitsu's, Nunchaku's and
Red Rust's copies cross their
programs' live g_HandId words, so the native loader binds the actual hand in a
temporary upload copy, as for the basic shields; immutable program and artwork bytes
stay untouched. Morningstar and Mablung copies end before their hand words. Nonzero
palette indices fail before any upload.

## Evidence and remaining work

Public tests cover descriptor locations, each blend, Nunchaku's pose12 sound pair,
variant bounds and untouched program data; Takemitsu's paired reservation and
unpaired-restore rejection; the live hand word in its palette upload; and Red Rust's
factory-then-roll ordering, failure state, omitted grunt and exhaustion.

Private original-instruction comparisons run 12,208 complete callbacks in 384
contexts across all seven programs (including both Holy rod/Star flail descriptors,
Star flail's widening and every extra sound; Debug and ASan traces byte-identical),
and fourteen full 384-word palette copies. The compiled native loader matches that
reference in both hands, changing only its texture and CLUT words.

Native protocols (Debug and ASan, LLDB, muted) exercise real menu equipment in both
hands, private sprite pixels, real enemy HP damage, Mist retirement and in-memory
v9 Continue (Mablung+Morningstar and paired Takemitsu), including unpaired-save
rejection, Red Rust's actual-cursor failure roll in both hands, and Holy rod/Star flail
ordinary attacks plus their real back-forward specials with real enemy damage. Supplied stock, protection, position, relic, arrival and save-room
contexts are not acquisition, uninterrupted-route or console evidence. Other
weapon families, specials, full sharing and all six [milestone gates](alucard-status.md)
remain open.

Adapted AGPL-3.0-or-later algorithms: `weapon/w_002.c`, `w_003.c`, `w_004.c`, `w_005.c`, `w_033.c`, `w_035.c`,
`w_036.c`, `w_057.c`, `weapon/shared.h` and `dra/6E42C.c` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`.
