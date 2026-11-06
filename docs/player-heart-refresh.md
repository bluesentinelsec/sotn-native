# Selected Heart Refresh

Heart Refresh (item142) is a kind131 consumable running the weapon44 program (`w_044`) in either
hand. Each use freezes the scene, plays a full-screen heart image over a wave of light and
refills 500 hearts (capped at the maximum). Defaults, grants and saves are unchanged; the stock
is ordinary inventory.

## Use

`func_8010EDB8` treats item142 like the other blueprint items:

- CheckChainLimit with the item's chain limit 0: the use is refused while any entity in
  slots16..63 carries equip id 142 with this hand's parameter bit (one live use per hand).
- A curse takes the ordinary failure path (`FACTORY(BP_57, 1)` outline, VO_ALU_PAIN_A).
- Otherwise blueprint50 with the hand bits (`FACTORY(50 + (hand+1)<<12, hand<<7)`, equip id 142)
  creates the E0/F0 entity in slots17..31.
- `ReduceWeapon` consumes one after the factory succeeds (Duplicator keeps it). Natively the
  throwing owner commits the consumption after the player's successful update.
- Case131 animates only a standing player: `0xBE` plus the ledge variant, with no grunt or roll.

## Effect

`EntityWeaponAttack` (role0) allocates seventeen GT4s; without them it is destroyed at once. The
native runtime leases the rows before the callback and never runs it without them.

- **Rows 0..15** are vertical bands 8 pixels wide across x64..192. Each samples an 8-pixel column
  of the hand's weapon texture (tpage 0x19, the `v + 0x80` band for the right hand), with clut
  0x110/0x128, priority 0x1BC and a semi-transparent drawMode.
- **Row 16**, the heart, starts hidden: u0..0x37, clut+1, priority 0x1BE.
- At birth the entity is `KEEP_ALIVE_OFFCAMERA|HAS_PRIMS|0x10000`, plays TRANSFORM_3X and sets
  `g_unkGraphicsStruct.unk28 = 2`. This is the spell freeze without func_8010E0D0's BlinkWhite
  births; the engine still resets the afterimages and keeps updating flag0x10000 entities. The
  native scene freeze and runtime input now accept request 2 like requests 1 and 3.

Every update ends with a band pass. Each band's top and bottom edges move by
`(rsin(angle) >> 5) * unk94 / 256` from y64 (and y64 + unk82). Its brightness is
`((rsin(angle) + 0x1000) >> 6) * unk90 / 256`. The angles are the program's sixteen
`D_138000_8017A260` words, each advanced by 0x40 after use. Band 15's right edge therefore reads
angle 0 already advanced. Those words are program data: they keep advancing across uses until
the overlay reloads.

1. **Swell:** brightness rises 4 per update to 0x100 while the wave amplitude falls 4 per update
   from 0x180, until it is below 0x40.
2. **Settle:** the amplitude falls 1 per update. Below zero, `func_80118C28(2)` flashes the
   screen, the bands switch to the image columns (u i*8..i*8+8, v 0..0x47, DRAW_DEFAULT) and the
   band moves to y56 with height 0x48. The original's following `i == 15` trim sees i == 16 and
   never runs.
3. **Hold:** 32 updates.
4. **Heart:** the heart row shows a quad around (0x80, 0x68), with corners at rcos/rsin of
   135/45/225/315 degrees scaled by scaleX. scaleX grows by a speed that starts at 10 and rises by
   2. Past 0x200, scaleY (the heart's colour, from 0x80) falls 1 per update. Past 0x380:
   `g_Status.hearts += 500`, clamped to heartsMax, then TELEPORT_BANG_B.
5. **Fade:** the heart keeps growing and dimming, the bands return to semi-transparent, the
   brightness falls 3 and the wave rises 6 per update. Below zero brightness: `unk28 = 0` and
   DestroyEntity.

## Graphics

The program's palette table (0x8017A2A0) uploads twenty-four palettes. As in the original, the
upload reads past the seventeen defined palettes. The hand word is at 0x8017A2A4. All rows are
screen-space GT4s; the entity draws no sprite.

## Evidence

Public tests cover:

- The program's angles: the first read, advancing every pass, and persistence across uses.
- Birth rows and words, each phase's duration and transition, the flash and image switch, the
  heart quad and its colour, the refill and its sound, and retirement with the freeze released.
- A shared-runtime use in both hands: seventeen leased rows, one quiet freeze with no BlinkWhite
  births, 286 frozen updates, the flash, hearts refilled to the maximum, and every row released.
- The player's use: the refused second use while one is live, consumption without a grunt, and
  the standing animation.

A private MIPS oracle executes the original callback over 1,722 traced callbacks in six contexts
(both hands, continued and reloaded programs, capped and uncapped refills). It runs SLUS
rsin/rcos and checks every row word, the angles, unk28 and g_Status hearts. Debug and ASan traces
are byte-identical.

A private native LLDB probe in Laboratory room 3 uses real Heart Refresh from the equipment menu
in both hands, by actual pad input. The scene holds the quiet freeze for 286 updates while the
bands and the heart draw; the screen flashes, hearts refill from 3 to the maximum, one is
consumed and the freeze releases. These fixtures supply items, stock, hearts and protection;
they are not acquisition, route or console evidence.

Adapted from `src/weapon/w_044.c` and `src/dra/6E42C.c` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, under AGPL-3.0-or-later. All original definitions,
tables, palettes and sounds remain private.
