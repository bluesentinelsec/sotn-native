# Selected ordinary basic shields

The actual shared [ordinary hand owner](shared-ordinary-hands.md) now accepts three
more original shields: Knight shield6 (weapon family9), Axe Lord shield8 (family11)
and Shaman shield12 (family26). Their ordinary callbacks match Alucard shield16's
family52 callback after symbol normalization; each reads its own private animation,
hitbox and palette tables. Their shield-rod spells are separate and unavailable.
Other shield families (leather, iron, herald, dark, goddess, fire, medusa, skull)
differ and remain unsupported; metadata alone never enables them.

Owned selection uses the ordinary inventory menu and either hand. Definitions
validate the actual shield category9, family, kind128, animation99, blueprint48 and
absent special/stock/MP bytes. Defaults, grants and v9 saves are unchanged. No new
weapon, effect or primitive pool is introduced.

## Callback, properties and guarding

The callback runs the original raise/hold/movement/lower animation, signed wraps,
phase4 knock-away with its double timer decrement and the final drawFlags overwrite.
The private animation table differs per family; hitboxes sit 0x48 bytes before it.
Basic shields leave per-hand program data untouched: only family52 caches its
attack and overrides attack to one. The actual SetWeaponProperties hit-state4 rule
gives basic shields attack255.

GetEquipProperties and the displayed attack are zero for base-ATK1 shields, even
with Attack buffs or poison, matching original CalcAttack's early return. The
critical/property RNG still runs. Raw GetProps ATK0 and the later hit-state4
attack255 belong to different owners and stages.

Native collision now follows original `collision.h`: any hit-state4 attacker is a
guard, not only item16. Blockable (state8) projectiles die without DealDamage.
Other state4 targets still run DealDamage, including its critical roll, then
discard the result; enemy HP is never reduced. This also corrects Alucard shield16,
which previously skipped that roll.

## Rendering

Original LoadWeaponPalette always copies 24x16 words. For families9/11/26 that copy
crosses the program's live g_HandId word. The native loader binds the actual hand
in a temporary upload copy; immutable program and artwork bytes stay untouched.
Family52's copy ends before its hand word. Palette indices other than zero fail
before any upload.

## Persistence and failure boundaries

Unbound legacy player paths reject basic shields: they require the actual hand
factory context. The legacy raw shield tick remains item16-only. Menus, maps,
draws and freezes add no callback updates; form entry retires the shield after a
successful player update. Curse failure keeps its original animation and raises
no guard. In-memory v9 Continue rebinds the resident programs.

## Evidence and remaining work

Public tests cover all three families in both hands, distinct table locations,
attack255 without the family52 program write, held/lower/retirement, malformed
table atomicity, definition validation, zero ATK under buffs and poison, slot16
runtime binding, wrong-program rejection, the generalized collision guard roll,
and hand-word palette binding.

Private instruction comparisons cover 8,136 complete callbacks in 96 contexts,
5,808 private animation calls and 96 property calls, plus eight full384-word
palette copies. The compiled native palette loader matches that reference for all
four families and both hands, changing only its texture and CLUT words.

Native protocols (Debug, LLDB, muted) exercise real menu equipment in both hands,
private sprite pixels, raised guards, real enemy-contact knock-away without enemy
HP loss, destruction of a real Laboratory Skeleton bone (with an unraised-shield
control that is hit), Mist/Bat/Wolf retirement, menu/draw holds, curse failure and
in-memory v9 Continue with mixed pairs. Supplied stock, protection, position,
curse and save-room contexts are not acquisition, uninterrupted-route or console
evidence. Shield spells, other shield families, full sharing and all six
[milestone gates](alucard-status.md) remain open.

Adapted AGPL-3.0-or-later algorithms: `weapon/w_009.c`, `w_011.c`, `w_026.c`,
`w_052.c`, `weapon/shared.h`, `st/collision.h`, `dra/5D5BC.c:DealDamage`,
`GetEquipProperties` and `menu.c:CalcAttack` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`.
