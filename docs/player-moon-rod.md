# Selected Moon rod

The actual shared [ordinary hand owner](shared-ordinary-hands.md) now accepts Moon rod132 through
the weapon6 program, with its QCF special172. Its palette table and live hand word
(D_8017AC34/D_8017AC3C) load through the existing palette owner. Defaults, grants and v9 saves
are unchanged.

## Swing and special start

Moon rod's swing uses descriptor0 (frameStart0x48, additive, SFX at pose3). Special172 is kind5
with blueprint72, chain limit1 and five MP. The QCF selects it only while `CheckChainLimit`
allows: limited specials count the live entities carrying the special's equip id with that
hand's params bit, so a second cast falls back to the swing while a crescent lives. Unlimited
specials keep their attack-mode check. Native code counts each hand's special children for the
player. MP is still charged after the factory.

In `dra/6E42C.c`, case5 stops a standing, walking or crouching player, takes the ledge variant
and enters substep0x45. An airborne start keeps its state. Both keep the casting grunt. In
`71830.c`, substep0x45 has no movement window and stands up at the END.

## Crescents and sparkles

Blueprint72 creates five E1/F1 crescents straight from the player factory, one every three
updates in slots17..31. Each uses its own equip id172 and takes its least-used group with no
raw-parent enemyId copy. A crescent starts at the player's animation hitbox offsets
(PLAYER.hitboxOffX/Y). Its horizontal speed is SetSpeedX(rand-4) and its vertical speed comes
from the private launch table (D_8017ABF0) plus rand&0x1FFF. It plays VO_ALU_ATTACK_A (first
crescent only) and SetWeaponProperties. It calls DestroyEntityWeapon(true), the conditional
slot16 retirement, and sets player timer10 to4.

The crescent decelerates by0x2800 while growing a scale step per update. Once stopped, it spins
up by8 for twenty-four updates and plays SFX_TRANSFORM. It then grows by two and accelerates
away, requesting a WFACTORY(0x40,0) sparkle every other update. Every update it cycles its
palette through the private fourteen-step table, sizes its box at scale*3/32 and plays SWISH_B
every ten updates (first crescent only). An actual collision hit turns it white
(PAL_FILL_WHITE, transparent) and retires it eight updates later. It draws the program's bank1
frame0x42 with SDK scale and rotation. Without KEEP_ALIVE it is culled off screen.

Each sparkle (E2/F2) needs one mandatory GT4. It moves ±12 randomly, then falls half a pixel per
update through func_8017B6A0's six two-placement UV rows (half-extent six, then four) and retires
when the rows end.

## Evidence

Public tests cover the crescents' birth, launch RNG, deceleration, spin-up, shedding schedule,
swish rule, hit flash and retirement; the sparkles' offsets, fall and lifetime; the runtime's
blueprint72 crescents, own item, least-used groups, shield-clear/timer10 commits, hit feedback,
forged-ownership rejection and row release; and kind5's grounded/airborne starts and
CheckChainLimit fallback. Mutations of each are caught.

The private original-instruction comparison runs complete w_006 callbacks: 8,176 attack, 9,600
crescent (including forty hit flashes) and 212 sparkle callbacks, with w_006's own
SetSpeedX/DecelerateX/DestroyEntityWeapon. Its 36 full palette copies match the compiled
loader. Debug and ASan traces are byte-identical. Native Debug/ASan protocols equip Moon rod
through the menu in either hand and run its swing and the real QCF special: five MP, crescent
and sparkle pixels, crescent damage and row release. Supplied stock, protection and position
contexts are not acquisition, route or console evidence.

Adapted AGPL-3.0-or-later algorithms: `weapon/w_006.c`, `weapon/shared.h`, `set_speed_x.h`,
`decelerate.h`, `dra/6E42C.c` and `dra/71830.c` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`.
