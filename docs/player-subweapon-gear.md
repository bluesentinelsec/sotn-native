# Selected Brilliant mail and Staurolite

Brilliant mail (body item 20) and the Staurolite (body item 61) change subweapon attack in
`func_800FE3C4`. Both are now supported. Defaults, grants and saves are unchanged.

## Subweapon attack

For an explicit subweapon (the path every thrown subweapon entity takes through
`func_8011A290`), `func_800FE3C4` copies the definition and then applies, in order:

1. Brilliant mail in the armor slot adds ten.
2. In the US build, the cross (4) and subweapon 12 are doubled by one Staurolite in an accessory
   slot and tripled by two.
3. One libc roll adds `(INT * 2 + rand() % 12) / 10`.

`SotnSubweapons_Power` reproduces this sequence, and the subweapon owner uses it for every birth.
Its 16-bit attack wraps like the original's. Staurolite now multiplies both attacks of the
[Cross](player-cross.md): its beam (4) and its giant crosses (12). Both records otherwise carry
ordinary additive fields.

## Evidence

- Public tests cover the plain roll, Brilliant mail's +10, Staurolite's ×2/×3 on the cross and
  subweapon 12, no Staurolite effect on other subweapons, and a rejected subweapon 0.
- A private MIPS oracle runs DRA's real `func_800FE3C4` over 5,376 contexts: every subweapon
  1..12 against armor/accessory pairs, INT values up to 1104 and libc cursors. It matches every
  attack word and the single roll. Debug and ASan traces are byte-identical, and the oracle
  catches a deliberate mutation.
- A private native LLDB probe throws a real dagger by Up+attack in Laboratory room 3, with and
  without Brilliant mail, at the same libc cursor. The mail adds exactly ten beside its own INT
  bonus's effect on the roll term. These fixtures supply items, the subweapon and hearts; they
  are not acquisition, route or console evidence.

Adapted from `src/dra/5D5BC.c` and `src/dra/7879C.c` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, under AGPL-3.0-or-later. All original definitions and
tables remain private.
