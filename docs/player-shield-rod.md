# Selected Shield rod

Shield rod4 is a kind2 rod that runs the weapon7 program (`w_007`) in slot16 through the shared
ordinary hand owner, in either hand. Its back-forward special173 releases a guarding orb that
sheds sparkles. Its combo main bits (0x3FFE) pair it with every shield, like Mablung, so both
buttons cast each owned shield's [spell](player-shield-combos.md). Beside any shield its attack
gains `CalcAttack`'s five, on the status sheet and every swing ([details](player-alucart-set.md)).
Defaults, grants and v10 saves are unchanged.

## Swing

Both descriptors at 0xC28 start at animation 0x48 with palette 0x0F, SFX_WEAPON_SCRAPE_ECHO on
pose3 and `w_002`'s template: player-locked flags, two priorities behind the player, additive
blend (BLEND_TRANSP|BLEND_ADD) and SetWeaponProperties. Kind2 stops a grounded player in substep
0x42 with lock13. The private palette table (g_WeaponCluts at 0x8017AC94) loads through the existing
palette owner, with the live hand word at 0x8017AC9C.

The back-forward command selects special173 (descriptor1). On pose4, while the player's pose timer
is7, descriptor1 requests `WFACTORY(0x4A, anim)` from the swing. That factory carries the swing's
animation index (0..6) in its params.

## Orb

Blueprint0x4A creates the E1/F1 orb (`func_ptr_80170004`) in slots28..31. At birth it retires if
its raw slot16 parent is empty. Otherwise it copies the parent's facing, animSet, unk5A and palette,
then offsets itself by `D_8017AC48[params>>7]` (mirrored when facing left). It sits at zPriority 0xA8
and launches at SetSpeedX(4.75); animations3 and6 launch at 3.75 with velocityY 2.75 instead. It
starts at 1/16 scale with CAMERA_LOCKED|0x100000 (its four-pose animation loops through UpdateAnim).
It takes fixed attack words: a 9x9 hit type4 hitbox with attack1, four stun and invincibility
frames, hitEffect1 and no critical rate. func_80118894 gives it the least-used group, and it plays
UI_ALERT_TINK. It never takes an equip id.

It decelerates both velocities by 7/32 while growing by 0x10 per update to full scale. It then
counts 0x200 updates down, bobbing on rsin of an angle that grows by 0x20 (applied twice per update)
and requesting `WFACTORY(0x40,0)` sparkles on every eighth count. The countdown's end or a hit
starts the shrink by 0xC per update; the orb keeps bobbing and shedding sparkles until it drops
below 0x10. A hit on the update its growth ends leaves the original's unhandled step4: the orb holds
still until the camera culls it.

Hit type4 makes the orb a guard, like a raised shield. It only meets entities whose hitboxState
carries bit4 (blockable projectiles). Those die without DealDamage, and other targets roll and
discard DealDamage. collision.h writes the attacker's hitFlags as soon as the boxes overlap,
before the guard handling. Native combat now does the same for every hit-state-4 attacker, so the
orb learns of the contact and shrinks.

## Sparkles

Blueprint0x40 creates E2/F2 sparkles (`func_ptr_80170008`) in slots32..63. Each takes one GT4;
allocation failure retires it before its two rand calls. It scatters by `rand()%16-8` horizontally
and `rand()%16` down, sinks at 0.75 pixels per update and draws a 6x6 quad (4x4 from frame3) from
`D_8017AC64`'s UVs with clut0x1B0, tpage0x1A, priority+4 and additive drawMode0x31. Its frame
advances every second update and frame6 retires it. Quads are screen pixels placed with the
pre-scroll camera, as the original's posX is during the callback.

## Evidence

Public tests cover the swing descriptors, blend, palette and the pose4/timer7 factory with its
animation index. They also cover the orb's origins, speeds, scale, attack words, group, sound,
growth, deceleration, bob, sparkle cadence, hit and countdown shrinks and the stranded step4; the
sparkles' allocation failure, rand scatter, frames and retirement; and the runtime's slot16-to-orb
factory, raw-parent copies, sparkle leases and row release. Further tests cover the back-forward
selection, Shield rod with Leather and Alucard shield combos, and guard hitFlags feedback in
native combat. All 45 targeted mutations are caught. The private original-instruction comparison runs complete `w_007` callbacks:
15,280 plain-weapon callbacks including w_007's factory requests, 22,798 orb callbacks (1,994
sparkle requests, 28 stranded orbs, 28 hit shrinks) and 276 sparkle callbacks (three failed
allocations). Debug and ASan traces are byte-identical. Native Debug/ASan protocols equip Shield
rod through the menu in either hand, swing it, perform the back-forward special and measure orb and
sparkle pixels, and cast Leather shield's spell with it. Supplied stock, protection and position
contexts are not acquisition, route or console evidence.

Adapted AGPL-3.0-or-later algorithms: `weapon/w_007.c`, `weapon/shared.h`, `set_speed_x.h`,
`decelerate.h`, `st/collision.h` and `dra/6E42C.c` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`.
