# Selected weapon51 two-handed swords

The actual shared [ordinary hand owner](shared-ordinary-hands.md) now accepts the rest of the
weapon51 program shared with Great Sword127: Claymore98, Flamberge101, Zwei hander103 and
Obsidian sword107 (descriptors0..3), their back-forward specials182..185, and Estoc95. All
five use Namakura's [paired two-hand reservation](player-two-hand-equipment.md). Defaults,
grants and v9 saves are unchanged.

## Ordinary swings and the glow

Descriptors1 and2 draw additively, descriptor3 semi-transparently, descriptor0 opaquely.
Descriptors1..3 restart a glow from three words read at D_8017AD58[subType*16-2], [subType*16]
and [subType*16+2], which lie past that table in colour_table. Each update, func_8017B1DC
steps the red, green and blue words by two toward the row's limits, then back down. It writes
the result to colour11 of the descriptor's own palette (0x112/0x114/0x116, plus 0x18 for the
right hand) in g_Clut and uploads the 256x3 CLUT rows. After both phases it stops writing. The
words are the program's own BSS, shared by its attack and special and zeroed when the program
loads. Native code commits each write to the renderer's palette store only after the
runtime pass succeeds.

Estoc's variant8 uses D_8017AD58 itself as its descriptor (frameStart0xD6, sound0x6A2, the
attack frame table and hitboxes). Its glow reads colour_table row7, so it writes a zero colour
to colour11 of palette0x120 (0x138 right) twice. That palette belongs to no weapon51 sprite.

## Kind23 and kind27 player states

`dra/6E42C.c` selects a back-forward special without checking MP, then calls HasEnoughMp(mp,1)
after the factory for whichever item was chosen: with five MP the special costs five, with
fewer it costs nothing. Native code now applies that rule to every selected item; QCF
specials still require their MP before selection. Kind23 stands, turns to the input, sets
speed5, plays SFX_VO_ALU_ATTACK_B without the casting grunt and enters block_98, which halves
the speed (substep0x57). Kind27 (Estoc) turns, sets speed4 halved, plays the same voice, stays
in Player_Jump when airborne and keeps the ledge variant (substep0x5B). In a cramped space,
Estoc keeps its factory and MP but starts no attack.

In `71830.c`, substeps0x57/0x5B glide with 1/16 deceleration and no landing transition until
pose5. They raise FACTORY(69,1) dust on odd game-timer updates while grounded and six poses
short of the lock. Leaving the ground at pose4's last update switches to animation0x6C pose5
in Player_Jump. There they decelerate again. Animation0x6C's end resumes the ordinary fall or
jump, and the back-forward recognizer holds in its 0xFE state for sixteen updates.

## Special and fan

Blueprint54 replaces slot16 with w_051's E1/F1 special, whose equip id is the factory argument
(182..185). It runs func_ptr_80170004: the special descriptors (frameStart0xD6) with palette
0x110+subType*2, retiring outside animations0xD6..0xD7 without checking the attack mode. It
plays no descriptor sound and requests WFACTORY(0x3E,0) on pose1's last update. Its glow holds
on pose4 once the colour has started. It keeps the hand group and attacks on the hand channel.

Blueprint62 creates the fan in slots17..31. Each blade needs one mandatory GT4 (white-fill
palette, page0x19, UVs from the hand and the animation's flip, gouraud tint from the slot16
child's colour index plus a libc random 0..15 per channel, additive drawMode0x177, or0x117 for
colour3). It starts at the player, 20 (colour0) or 30 pixels ahead and 18 up. It slides out at
speed6 plus rand/32768, decelerating by 0x2800, with corners from its private rotation and
radius through the SDK rcos/rsin. The root (param0) copies its raw parent's equip id and
enemyId, attacks with a 30x16 box and requests FACTORY(0x3E,age+1) children every callback.
Blades with params divisible by four play SWISH_C. Every blade retires on its ninth callback.
Corners are stored relative to the blade, like the Katana slash.

## Evidence

Public tests cover the glow schedule, colour words and palette indices, blends, Estoc's
past-table descriptor and zero glow, the special's palette, window, factory and pose4 hold,
the fan's birth, tints, motion, root properties and chains, mirrored corners and lifetime;
kind23's reservation, voice, speed, MP rule, deceleration and dust, Estoc's grounded/airborne/
cramped starts, the jump hand-off to the 0xFE recognizer hold, the runtime slot16 E1 identity,
the fan's raw-parent copy and row release, and the CLUT write bounds. Mutations of each are
caught.

The private original-instruction comparison runs complete w_051 callbacks: 38,920 attack
callbacks (descriptors0..3 and variant8), 14,016 special callbacks and 3,040 fan callbacks.
It uses the real DRA UpdateUnarmedAnim and the program's own func_8017B1DC g_Clut writes
(13,248 uploads) and SetSpeedX/DecelerateX. Debug and ASan traces are byte-identical. Native
Debug/ASan protocols equip each sword through the menu in either hand and run every ordinary
swing with its glow writes, Estoc's lunge and the real back-forward specials (slot16 sprite,
five MP, fan pixels for all nine blades, fan damage and row release). Supplied stock,
protection and position contexts are not acquisition, route or console evidence.

Adapted AGPL-3.0-or-later algorithms: `weapon/w_051.c`, `weapon/shared.h`, `set_speed_x.h`,
`decelerate.h`, `dra/6E42C.c`, `dra/5D5BC.c`, `dra/704D0.c` and `dra/71830.c` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`.
