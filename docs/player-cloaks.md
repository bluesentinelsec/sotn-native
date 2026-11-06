# Selected Reverse and Royal cloaks

The Reverse cloak (body item 50) and the Royal cloak (body item 53) are now supported. Defaults,
grants and saves are unchanged.

`UpdateCapePalette` looks a cape up in `g_CapePaletteDefs` and queues its palette operation:

| Cloak | Operation |
| --- | --- |
| Reverse cloak | 0x40B, or 0x40C when `g_Settings.isCloakLiningReversed` is set |
| Royal cloak | 0x40D |

Both operations are the same one-step cape-colour copy that the native cape owner already applies
for the supported capes (`SotnAlucard_CapePalette`). Their records otherwise carry only a CON
bonus.

The native port has no configuration menu yet, so the lining setting keeps its default (off) and
the Reverse cloak always uses 0x40B. The Crystal cloak (an animated palette operation) and
Joseph's cloak (colours from the configuration menu) stay unsupported until those owners exist.

## Evidence

- Public tests cover equipping both cloaks in the cape slot and the Royal cloak's CON bonus.
- A private native LLDB probe in a real Laboratory session equips each cloak. The live player
  palette's four cape colours match each cloak's original operation, the two differ, and a
  rendered frame shows the colours. These fixtures supply the items; they are not acquisition,
  route or console evidence.

Adapted from `src/dra/5F60C.c` at sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, under
AGPL-3.0-or-later. All original definitions and palettes remain private.
