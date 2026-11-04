# Selected template weapons, Combat knife and Great Sword

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
program's descriptor1 (kind4, lock13, casting grunt kept). Great Sword127 (weapon51
descriptor0, two-handed) draws from DRA sprite bank10 without a per-hand bank offset; its
siblings' descriptors1-3 cycle palettes and belong to special-bearing items that stay
unavailable. Combat knife20 (weapon32) draws in front of the player (the special behind),
adds SFX_VO_ALU_ATTACK_B after a SWISH_B descriptor sound, becomes a hit-state4 parry on
pose6 within its first four animations, and when its attack ends it lingers four updates
in slot16 with no sprite or hitbox instead of retiring.

The player resolves Combat knife's special175 (both its back-forward and QCF item) with the
original quirk: a lingering ordinary knife in slot16 makes the next swing special175, a
lingering special makes it the ordinary knife, and only an empty slot16 falls back to the
QCF/back-forward checks. The session supplies slot16's actual item before each player update.
Kinds9/10 skip the casting grunt. [Muramasa](player-muramasa.md)'s
attack-dependent roll is described separately.

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

Private original-instruction comparisons run 13,520 complete callbacks in 480
contexts across all nine programs (including Combat knife's linger, parry and voice, and
Great Sword's descriptor0; Debug and ASan traces byte-identical), and twenty full 384-word
palette copies. The compiled native loader matches that reference in both hands.

Native protocols (Debug and ASan, LLDB, muted) exercise real menu equipment in both
hands, private sprite pixels, real enemy HP damage, Mist retirement and in-memory
v9 Continue (Mablung+Morningstar and paired Takemitsu), including unpaired-save
rejection, Red Rust's actual-cursor failure roll in both hands, and Holy rod/Star flail
ordinary attacks plus their real back-forward specials with real enemy damage, Great
Sword's paired sprite and damage, and Combat knife's real slot16 alternation. Supplied stock, protection, position, relic, arrival and save-room
contexts are not acquisition, uninterrupted-route or console evidence. Other
weapon families, specials, full sharing and all six [milestone gates](alucard-status.md)
remain open.

Adapted AGPL-3.0-or-later algorithms: `weapon/w_002.c`, `w_003.c`, `w_004.c`, `w_005.c`, `w_032.c`, `w_033.c`, `w_035.c`,
`w_051.c`,
`w_036.c`, `w_057.c`, `weapon/shared.h` and `dra/6E42C.c` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`.
