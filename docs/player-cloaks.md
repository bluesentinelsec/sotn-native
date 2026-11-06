# Selected Reverse, Royal and Crystal cloaks

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
the Reverse cloak always uses 0x40B. Joseph's cloak (colours from the configuration menu) stays
unsupported until that menu exists.

## Crystal cloak

The Crystal cloak (body item 52) queues operation 0x410, a `PAL_UNK_OP3` palette rotation. Each
palette-job pass of `func_800EA7CC` copies the four cape colours of the current source, waits
`(op >> 16) - 1` passes and advances, returning to the first source at the -1 terminator. The
operation's interval is 1 and its second colour set is all zeros, so the cape flickers between
its colours and transparency on every pass.

`SotnAlucard_CrystalTick` reproduces this rotation. Changing to the cloak resets it to the queued
state without copying anything (the original copies only at the next pass). The session runs one
pass per update in `TickCape`, the same cadence as the Twilight cloak's native glow. The job
lasts until the cape changes, which clears it like `func_800EA538(6)`.

## Evidence

- Public tests cover equipping both cloaks in the cape slot and the Royal cloak's CON bonus.
- A public test covers the Crystal rotation's copy, countdown, advance, wrap and its rejection of
  a malformed cursor or operation kind.
- A private MIPS oracle runs DRA's real `g_CapePaletteDefs` lookup (52 → 0x410),
  `func_800EA538(6)`, `func_800EA5E4(0x410)` and 40 `func_800EA7CC` passes. The cape colours
  match the native rotation on every pass, and the oracle catches a deliberate mutation.
- A private native LLDB probe in a real Laboratory session equips each cloak. The live player
  palette's four cape colours match each cloak's original operation, the two differ, and a
  rendered frame shows the colours. With the Crystal cloak, consecutive updates alternate between
  its two colour sets. These fixtures supply the items; they are not acquisition, route or
  console evidence.

Adapted from `src/dra/5F60C.c` at sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, under
AGPL-3.0-or-later. All original definitions and palettes remain private.
