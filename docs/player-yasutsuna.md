# Selected Yasutsuna

The two-handed Yasutsuna165 runs the weapon53 program shared with [Katana](player-katana.md),
hosted by the shared weapon-secondary owner outside slot16. It uses Namakura's
[paired two-hand reservation](player-two-hand-equipment.md), and both buttons throw it.
Defaults, grants and v10 saves are unchanged.

## Use

Yasutsuna takes the thrown-item path in `dra/6E42C.c`. It runs CheckChainLimit (chain 1), the
blueprint68 factory, the casting grunt and the throw animation 0x5D. Its isConsumable byte is
clear, so the hands keep it. The root carries its item from birth, so CheckChainLimit counts it;
the chained slashes never do.

## Slash

Blueprint68 creates one E3/F3 child (`func_ptr_8017000C`) in slots 17..31. The right hand's
parameter carries 0x8000, which w_053 masks off with `(params >> 8) & 0x7F`.

Unlike Katana's roots, this slash stays at its factory origin, with no 20-pixel forward offset.
It moves 11 pixels down when crouched, otherwise 5 up, and never follows the player. It reserves
one mandatory GT4 before SetWeaponProperties.

Only parameter 0 rolls properties and chains:
- SetWeaponProperties takes the least-used group, which the raw-parent copy then overwrites with
  the player entity's enemyId (zero).
- Each callback requests `WFACTORY(0x38, age+1)`, which produces Katana's role1 slashes with
  indices 1..9. Their raw parent is the root's slot.

Other parameters, including Katana's 0x28..0x31 range, neither roll nor chain. The hitbox X is
always 0x12. The CLUT steps through weapon53's schedule (plus 0x18 for the right hand), and the
slash retires after its ninth callback.

## Evidence

Public unit tests cover both hands:
- the fixed origin and crouch offset;
- the single property roll;
- nine chain requests;
- the CLUT schedule and retirement;
- non-root parameters.

Runtime tests cover both buttons: the blueprint68 root with its hand bit, group zero, the nine
chained indices and the release of every row. Hand use covers the packed factory, grunt,
non-consumption and chain limit. Fifteen targeted mutations are caught.

The private original-instruction comparison runs 6,880 complete w_053 callbacks. Debug and ASan
traces are byte-identical.

Native Debug/ASan protocols equip Yasutsuna through the menu and throw it from either button.
They measure root and chained-slash pixels, real enemy damage, non-consumption and row release.
Supplied stock, protection and position contexts are not acquisition, route or console evidence.

Adapted AGPL-3.0-or-later algorithms: `weapon/w_053.c` and `dra/6E42C.c` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`.
