# Selected Heaven sword

Heaven sword119 is a kind129 sword thrown through blueprint50, like
[Runesword](player-runesword.md). It runs the weapon13 program hosted by the shared weapon-secondary
owner outside slot16. Its palette table and live hand word (D_8017A5FC/D_8017A604) load through
the existing palette owner. Defaults, grants and v9 saves are unchanged.

## Throw

The hand-use path matches Runesword: CheckChainLimit (chain1), `FACTORY(50 | hand,
variant + hand*128)` with equip id119, the casting grunt and animation 0x5D. Its isConsumable byte
is clear, so the hand keeps the sword. CheckChainLimit counts only the root; afterimages never take
an equip id.

## Root

Blueprint50 creates the E0/F0 root in slots17..31. It owns no primitives. At birth it takes the
hand's bank1 frame6 (palette 0x110/0x128, unk5A 0x64/0x66) at the player's animation hitbox, 16
pixels up (8 while the player step is Player_Crouch). Its speed is SetSpeedX(6) plus
PLAYER.velocityX. It runs SetWeaponProperties (least-used group 3..6, own item) and
DestroyEntityWeapon(1), so a raised slot16 shield retires. It takes an 18x4 hitbox, plays
SWISH_C, starts its swish volume at 0x50 and sets player timer10 to4.

After twenty-two updates (DecelerateX(FIX(1)) below seven), it rises backwards: velocityY -1.5
with gravity 1/16 for twenty-four updates, spinning by -11.25° with a 12x12 hitbox. It then
steers toward the player's hitbox at four pixels per update. Each turn is limited to a step that
grows by 0x20 from 0x80, and never overshoots a smaller heading change. It retires inside half of
the combined hitbox sizes. During both phases, every even g_GameTimer requests a
`WFACTORY(0x3A,0)` afterimage. Its childPalette follows the private fourteen-step cycle
(D_8017A5E0), and every eighth timer plays PlaySfxVolPan(SWISH_A) at the fading volume.

## Afterimages

Blueprint0x3A creates E1/F1 afterimages in slots32..63. On their first callback they retire if the
raw parent slot is empty. Otherwise they copy its frame, animSet, unk5A, facing, childPalette and
rotation, add ENTITY_OPACITY to its drawFlags and sit two priorities lower. They fade by eight
from 0x80 and retire below 48. They never take an equip id, properties or hitbox. Native drawing
shades them by entity opacity.

The both-buttons item202 special (two Heaven swords, func_ptr_80170008/8017000C) is a
CheckWeaponCombo selection shared with Shield rod and its shields. It remains unavailable until that
combo owner exists.

## Evidence

Public tests cover the root's birth fields and origin, deceleration, rise, gravity, spin,
afterimage/swish schedules, palette cycle, volume fade and clamp, homing turn limit and exact
snap, retirement window and phase bounds; afterimage copies, fade and orphan retirement; the
runtime's root, slots, raw-parent naming, palette range and cadence; and the throw's
non-consumption, grunt and chain limit. All fifteen targeted mutations are caught.

The private original-instruction comparison runs complete w_013 callbacks: 4,134 root callbacks
(1,589 afterimage requests, 437 sounds, eight shield retirements, thirty-six homing retirements)
and 340 afterimage callbacks. Both hands' full palette copies match the compiled loader. Debug and
ASan traces are byte-identical. Native Debug/ASan protocols equip Heaven sword through the menu in
either hand. They throw it, measure sprite and afterimage pixels, damage a target, confirm the
chain-limit block, non-consumption, row release and a rethrow, and throw while the other hand's
shield is raised. Supplied stock, protection and position contexts are not acquisition, route or
console evidence.

Adapted AGPL-3.0-or-later algorithms: `weapon/w_013.c`, `weapon/shared.h`, `set_speed_x.h`,
`decelerate.h`, `dra/6E42C.c` and `dra/menu.c` (CheckWeaponCombo) at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`.
