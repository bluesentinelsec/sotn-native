# Selected Iron Fist and Fist of Tulkas

The actual shared [ordinary hand owner](shared-ordinary-hands.md) now accepts Iron Fist102 and
Fist of Tulkas120 through the weapon1 program shared with the [basic fists](player-basic-fists.md).
Their ordinary punches use descriptors2/3. Iron Fist's QCF special186 and Tulkas's
back-forward special187 are kind24 punch rushes; Tulkas's QCF special188 is a kind25 punch
(descriptor4) that throws a fireball.

Definitions validate category3 (188: category2), family1, the special bytes, variants2/3/0/1/4,
kinds24/25 and, for 186/187, blueprint54. None costs MP. Defaults, grants and v9 saves are
unchanged.

## Kind24 punch rush

`dra/6E42C.c` case24 keeps an airborne player in Player_Jump (a grounded one in Player_Stand),
sets D_80139824 to 0x80, stops both velocities and plays SFX_VO_ALU_ATTACK_B without the
casting grunt, then follows block_98 into substep0x58. On the ground, `71830.c` rewinds pose4's
last update to pose2 while the hold lasts; pose6, a Down tap or holding backward resumes
substep0x40, and pressing Up still enters the stand press-up. In the air, PlayerStepJump skips
deceleration, the rising clamp and its transitions during substep0x58, so the player hovers
without gravity or landing. Pose3's last update rewinds to pose1; pose4 or a Down/Up tap
resumes substep0x40.

Blueprint54 replaces slot16 with w_001's E1/F1 rush, whose equip id is the factory argument
(186/187). It follows the player and retires once the player's animation leaves its
descriptor's seven-frame window (frameStart0x24) or the attack mode clears. It needs sixteen
mandatory GT4 rows (Alucard palette, page0x19, the private UVs with V+0x80 for the right hand,
player priority+4, hidden drawMode0x379); without them it retires before RNG. Its properties
come from its own item. As func_80118894 does for slot16, it takes the hand's group without a
least-used count. Its 18x10 hitbox sits at (28,-12), and its lifetime is 128 updates:
SWISH_C plays whenever the count's low three bits are7, and the final update plays
SFX_VO_ALU_ATTACK_C and retires. Each row waits r0 updates (initially 1..16), appears 16x16
at a random offset (two libc calls) around the player plus (±26,-6) (three more up when
Y-scaled), swaps to the second image after two updates, then swaps back and hides.

## Kind25 fireball

Kind25 enters block_98 directly: no grunt and the unarmed substep0x59 handlers. Descriptor4
plays SFX_VO_ALU_ATTACK_A after its trail rows, never shows trails, and requests
WFACTORY(0x3E,0) on pose2's last update. Blueprint62 creates the E2/F2 fireball in
slots17..31. It faces away from the player, moves 4.5 pixels per update (starting six steps
ahead, 13 pixels up, three more when Y-scaled), copies its raw parent's equip id before
SetWeaponProperties and that parent's enemyId after it, plays FIRE_SHOT and grows by0x10 per
update to X0x180/Y0x100. It draws DRA animSet9 frames1..8 with palette0x1B0 and additive
blending, CAMERA_LOCKED, runs post-callback UpdateAnim, and retires on its next callback after
any actual collision records hitFlags. Lacking KEEP_ALIVE, it is culled off screen.

## Related correction

Room entry translates each DRAW_UNK_200 primitive without a callback. Weapon secondaries
translated their LINE_G2 rows unconditionally, but weapon48's ring GT4s share that storage
since the stack-size fix, so a room change wrote row offsets into the ring's vertices. Now
the ring and the punch rush translate all four vertices of each owned GT4, LINE_G2 owners
translate their rows, and weapon53's entity-relative slash and the rowless fireball translate
neither.

## Evidence

Public tests cover descriptor4's sound/factory, the rush's rows, schedules, RNG, sounds,
lifetime and retirement, the fireball's birth, motion, scale caps, animation and hit
retirement; kind24's reservation, hold, pose rewinds, cancels and airborne hover; kind25's
selection; the slot16 rush's runtime identity, hand group and row release; and the fireball's
raw-parent copy and hit feedback; and room-entry translation of the ring and rush GT4s.
Mutations of each are caught.

The private original-instruction comparison runs complete w_001 callbacks: 4,260 descriptor4
attack callbacks, 4,600 rush callbacks (72,000 GT4 rows) and 668 fireball callbacks with the
real DRA UpdateAnim. Debug and ASan traces are byte-identical; UV, scale-cap, sound and
hit-flag mutations are caught. Native Debug/ASan protocols equip both items through the menu
in either hand and run ordinary punches, the real QCF186/back-forward187 rushes (GT4 pixels,
damage, row release, airborne hover) and the real QCF188 fireball (sprite pixels, damage,
retirement after its hit). Supplied stock, protection and position contexts are not
acquisition, route or console evidence.

Adapted AGPL-3.0-or-later algorithms: `weapon/w_001.c`, `weapon/shared.h`, `set_speed_x.h`,
`dra/6E42C.c`, `dra/7879C.c` and `dra/71830.c` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`.
