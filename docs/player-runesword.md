# Selected Runesword

Runesword143 is a kind129 sword that Alucard throws through blueprint50. It runs the weapon14
program, which the shared [weapon-secondary owner](player-holy-sword.md) now hosts outside
slot16. Its palette table and live hand word (D_8017A5B0/D_8017A5B8) load through the existing
palette owner. Defaults, grants and v9 saves are unchanged.

## Throw

In `dra/6E42C.c`, kind129 shares the thrown-item path: CheckChainLimit (chain1), the factory
`FACTORY(50 | hand, variant + hand*128)` with equip id143, the casting grunt
(`func_8010EA54(8)`) and the stand/crouch animation 0x5D. Runesword's isConsumable byte is
clear, so ReduceWeapon leaves the hand unchanged; native use records `keep` and skips the
consumption commit. CheckChainLimit counts the root (equip id143 from birth) and its hitboxes
once their first callback copies the parent's id, so a second throw waits until all have
retired.

## Root

Blueprint50 creates the E0/F0 root in slots17..31. It reserves 48 mandatory GT4 rows before
anything else; without them it retires before properties or RNG. At birth it takes the hand's
bank1 frame8 (palette 0x11C/0x134, unk5A 0x64/0x66) and starts at the player's animation
hitbox: 16 pixels up, or 8 while the player step is Player_Crouch. Its speed is SetSpeedX(6)
plus PLAYER.velocityX, with velocityY one pixel. It runs SetWeaponProperties (least-used group
3..6, own item) and DestroyEntityWeapon(true): a raised shield in slot16 retires with its
actual owner. It has a 4x4 hitbox, rotation -157.5° and twenty-two updates of lifetime. It plays
SFX_RUNESWORD_ATTACK (volume 0x50, pan0) then SWISH_A, sets player timer10 to4 and requests its
`WFACTORY(0x72,0)` hitboxes.

The flight turns the blade 0x70 per update until level. Below lifetime7, DecelerateX(FIX(14))
stops it. At zero it drifts back at SetSpeedX(-3) for eight updates, unrotated. It then homes on
the player's hitbox at ten pixels per update with SDK ratan2/rcos/rsin, rotating to its heading.
It retires inside half of (hitboxWidth+16) by (hitboxHeight+4), before any trail update.

Every update uncovers one trail row: row i spans the blade (47 and 8 pixels out) at callbacks i
and i+1. Rows older than seventeen hide, and the newest sixteen share the texture's 128 U texels.
Rows use tpage0x19, palette 0x11D/0x135, V rows 0x60..0x7F (0xE0..0xFF right) and drawMode 0x173
(quarter-additive). Their DRAW_UNK02|UNK_100 vertices follow camera scroll and room entry like
the entity, so native code keeps the corners in world pixels and converts them at draw time.

## Hitboxes

Blueprint0x72 creates four E1/F1 hitboxes (paramsLo0..3) in slots17..47. Each retires as soon as
its raw parent slot is empty. On its first callback it copies the parent's facing, equip id and
enemyId, runs SetWeaponProperties and takes a 4x4 hitbox. Every callback places it 10/20/30/40
pixels along the blade, using the parent's rotation only while the parent draws rotated
(mirrored by facing).

## Evidence

Public tests cover the root's birth fields and origin, exhaustion before RNG, flight, stop,
drift, homing retirement, trail uncovering/hiding/UVs and phase bounds; the hitboxes' offsets,
rotation gate, raw-parent retirement and bounds; the runtime's blueprint50 root, own item, 48
rows, volume sound, hitbox group/item copy, forged-root rejection, room-entry trail translation
and row release; the throw's non-consumption, grunt and chain limit; and shield retirement at
the root's birth. All eighteen targeted mutations are caught.

The private original-instruction comparison runs complete w_014 callbacks: 2,578 root callbacks
(121,824 GT4 prim comparisons, eight shield retirements, thirty-six homing retirements and
trails past 330 updates) and 5,160 hitbox callbacks with sixteen parent retirements. Both hands'
full palette copies match the compiled loader. Debug and ASan traces are byte-identical. Native
Debug/ASan protocols equip Runesword through the menu in either hand. They throw it, measure
sprite and trail pixels, see all four hitboxes, damage a target, confirm the chain-limit block,
non-consumption, row release and a rethrow, and throw while the other hand's shield is raised.
Supplied stock, protection and position contexts are not acquisition, route or console
evidence.

Adapted AGPL-3.0-or-later algorithms: `weapon/w_014.c`, `weapon/shared.h`, `set_speed_x.h`,
`decelerate.h`, `dra/6E42C.c`, `dra/5D5BC.c` (ReduceWeapon) and `dra/5087C.c` (primitive camera
flags) at sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`.
