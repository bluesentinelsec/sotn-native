# Selected Muramasa

Muramasa141 is a two-handed kind29 sword running the weapon56 program. Its attack grows with
the original blood counter `g_Status.D_80097C40`, which ordinary saves now keep in
[version 10](native-saves.md). Its palette table and live hand word (D_8017B058/D_8017B05C)
load through the existing palette owner. Defaults and grants are unchanged.

## Blood counter

In `dra/692E8.c`, `func_8010A3F0` runs before healing in each living, non-teleporting player
update. When both hands run weapon56 (two-handed Muramasa), it holds Dark Metamorphosis'
absorption timer at 0x20, so enemy blood is absorbed and the absorption glow shows. On an update
with blood contacts (`g_Player.unk10`), it adds one to the counter, saturating at 0xFFFFFFFF.
The contact counter then clears every living update, whatever is equipped. Native code reports
the request with the player state, and the session commits it to the character after a
successful update.

`menu.c` CalcAttack adds `SquareRoot0(counter)` to Muramasa's attack in its signed 16-bit sum
before the 0..999 clamp, for both the displayed attack and GetEquipProperties. Very large
counters can therefore wrap the sum negative (0x7FFFFFFF's root 46340 does). The SDK root takes
a signed input. From 0x80000000 it counts leading sign bits and shifts arithmetically, indexing
entries before its private table. Native code computes nonnegative roots without the table and
reads the bound SDK image for that range; without the image it fails explicitly.

## Swing

Muramasa's attack is Gurthang's callback over its own tables (descriptors 0xF64, row offsets
0xF94/0xF9C). It runs descriptor2 whenever the Dark timer is set, which two-handed Muramasa
always ensures. Twelve optional LINE_G2 blood rows reserve before their 28 birth rand calls and
are placed on trigger frames. After SetWeaponProperties, a rolled attack of 0x1E or more
requests `WFACTORY(56,0)`. There is no Dark doubling, timer floor or outline.

In `dra/6E42C.c`, case29 runs GetEquipProperties after the factory: one critical rand, then
the attack, halved by poison. At 11 or more it swings normally. Below 11 it takes Red Rust's
`rand()&7` roll, where zero is the failure branch (no casting grunt).

## Slash chain

The `WFACTORY(56,0)` root and its chain run Katana's slash callback over Muramasa's tables, eight
bytes later (rows 0xFCC, CLUT schedules 0xFA4/0xFB8). Muramasa's table is shorter. Chained
parameters past it read its live g_HandId at 0x105C (row10's CLUT flag and lifetime) and then
program words, as the original does. Roots copy the raw parent's equip id and enemyId. Each slash
owns one GT4, follows the player and retires after nine callbacks.

## Evidence

Public tests cover the counter's attack bonus, signed-word wrap, saturation, poisoned
properties and signed-range binding. They cover v10 encode/decode, v9 migration and reserved
bytes. The case29 roll is checked for strong, weak and poisoned attacks. The Dark timer hold,
blood request and contact clearing are covered, as are the descriptor2 swing with the 0x1E slash
threshold and Muramasa's table offsets. Slash tables, the hand word in row10 and Katana's
schedule are also covered. All twenty targeted mutations are caught.

Private original-instruction comparisons cover 145,248 complete attack callbacks (1,394,112
blood-row comparisons, 3,024 slash requests) and 6,880 slash callbacks over all fifty
parameters. They also cover 270,009 inputs to the original SLUS SquareRoot0, including the
signed range. Both hands' palette copies match the compiled loader. Debug and ASan traces are
byte-identical. Native Debug/ASan protocols equip two-handed Muramasa through the menu. They see
the held Dark timer and absorption status, weak swings with failure rolls and no slashes, and
real cutting hits on Laboratory zombies growing the counter. At counter 900 they see the slash
chain kill a target, and an in-memory v10 Continue keeps the counter and attack. Supplied stock,
protection and counter values are fixtures, not acquisition, route or console evidence.

Adapted AGPL-3.0-or-later algorithms: `weapon/w_056.c`, `weapon/w_053.c`, `weapon/w_054.c`,
`dra/692E8.c`, `dra/6E42C.c`, `dra/5D5BC.c` and `dra/menu.c` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, and the US SLUS SquareRoot0.
