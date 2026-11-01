# Selected playable Axe Lord Armor

Owned body item25 now enters the actual weapon45 form. It supplies original
stand40, walk41, jump42, hit43 and simulated-input hold50, private multipart body
and shadow artwork, shared factories and native enemy combat. Ordinary starts
still grant no armor. Metadata support alone does not enter the form.

## Equipment, control and damage

The native owner validates virtual216/weapon45 metadata and loads its real graphics
before binding the equipped form. Ordinary selection destroys the actual slot16
weapon, cancels the prior form through its existing owners, requests WHAT and uses
the existing32-update simulated-input interval. Full new/Continue restoration binds
owned armor before player control, without replaying the equip sound or input hold.
Unequip restores the normal bank/palette/pivots and ground/fall recovery. Entry
from upside-down High Jump retains its actual facing/rotation/pivot; the later
normalization helper sees the new Axe state and must not flip facing again. The
ordinary frame clears draw flags before dispatch; callbacks re-enable only their
original flags, while frozen updates retain the preceding display. Same-family
hand changes and script reentry retain the original overlay words; an actual family
reload initializes those words from the private overlay. No extra timer or pool is
introduced and no transient form fields are serialized.

The controller preserves direction/attack/jump priority, original half-speed
rounding, turning's preceding-facing speed, air steering/gravity and landing attack
recovery. Walking requests the original stomp/smoke on absolute modulo8. The attack
uses Square/left-hand input, original animation CE and factory87. Script50 holds
zero velocity until the raw mode word changes, even after its input timer expires;
its CF recovery preserves palette changes made by the common timers. Only the four
overlay callbacks rebind weapon45's palette on each update.

Surviving queued damage selects43/D1, original side selection (including kinds17/18),
voice6ED, shadow85 and protection20. Guard, new curse/poison and stone convert to
original kind1 hurt; zero-damage stone still reacts and produces its blue zero
number. Earlier immunity, absorption and Talisman retain their owners. Actual
fractional origins, Blood Cloak, prior ailments/healing and lethal/water exclusions
retain their established order. The hit controller preserves angle-word wrap,
ceiling/floor boundaries, pivot20, rotations512/896, byte-wrapped SDK sine opacity,
48-update ground recovery, five-update rise and loss-of-floor recovery. Physical
and water death restore the normal body owner. Other damage kinds/effects and
familiar revival remain separate unfinished work.

## Shared combat and presentation

Factory87 replaces actual slot16 with child50. Spell11 metadata/INT/libc/Mojo work
runs once at birth, including a birth whose sprite immediately retires. The
original left-hand group1 and its actual enemy cooldown persist across attacks.
Current sprite5/6 supplies the original hitbox; captured facing and child fractions
survive live high-word following. Native enemy HP/death/EXP use the existing combat
owner. Root exhaustion occurs before spell RNG; unrelated external slot ownership
fails explicitly. No substitute attack or primitive allocator is used.

Factory85 creates actual rowless child49. It captures original bank/palette/frame7,
facing/priority, follows player high words with retained child fractions and uses
rotation-dependent offsets plus gameplay-clock sway. It mirrors the player's
opacity flag/byte and retires on actual hurt/form exit. Initialized children hold
through spell freeze. Room/camera rebases translate their sources without another
update; menus, map, release-only frames and repeated draws retain existing clocks.
Failed player updates retain requests/HP; failed shared updates retain children/RNG.

The body/shadow use private weapon45 multipart frames and two palette rows. Owned
bank substitution restores the selected hand bank after drawing. Rotated body
parts use original SDK SquareRoot12/ratan2/trig, all four vertices, retained pivots,
word wrap and original mirrored UV bytes. Missing SDK/invalid pivots fail before
pixels change, including explicit zero-angle rotation. The CPU rasterizer remains
an adaptation; matching packet geometry is not GPU/console raster parity. Original
Blink/outline exclusion of Axe status remains in force.

## Evidence and boundaries

Public authored tests cover entry/exit/bootstrap, original priorities and signed
motion, script palette/release, same-family words, raw ailment/stone/guard/kind17/18
conversion, lethal exit, late failure, actual slot16/group1/metadata, exhausted root
records before RNG, foreign ownership refusal, shadow fractions/freeze/rebase/
retirement and renderer SDK failure atomicity. Fixtures contain no original tables
or assets.

Private comparisons match60,000 original overlay entry/stand/walk/jump/hit contexts,
4,096 actual native shared child49/50 contexts and1,008 private multipart rendering
contexts/2,016 complete SDK/unrotated packets. They cover motion/animation/retained words,
ordered requests, geometry/UV/page/palette/opacity and original child birth/live
fields/retirement. Bank/palette/factory/audio, child Spell11/destruction and renderer
CLUT/GTE-leading-zero contexts use explicit adapters; native metadata/combat and
actual graphics ownership are tested separately. These are scoped instruction and
packet comparisons, not whole-frame or console claims.

Muted LLDB native checks retain default70HP/20MP. Owned armor selection exercises
32-update hold, stand/walk/jump, actual slot16 attack, enemy24HP defeat/27EXP, menu
holds, ordinary zombie hurt, real shadow and48-update recovery/unequip. Measured
body/shadow contributions are730/433 pixels. The actual CMake app separately checks
these controls, raw-mode2 hold/release, ordinary contact and SDL exit. Connected
stage replacement preserves owned armor/resources; isolated v9 write/read/fresh
bootstrap restores the exact record after the original64 RNG initialization calls,
without stale children or equip holds. Since original Axe standing has no Up/look
state, the ordinary save-room UI check unequips first, retains owned25 through
Continue, then re-equips with the real32-update interval.

Owned-item, spawned/positioned target, protection, raw-mode and destination/save-room
transfer fixtures are separate from acquisition, uninterrupted routes and console
proof. User save slots are untouched. Final sanitizer/platform results are recorded
in the feature PR. Milestone1 now dispatches31/34 states;17/18/39, common damage/
attack/equipment/familiar branches and integrated acceptance remain open. Alucard
and castle are incomplete; default resources/grants and ordinary saves remainv9.

Provenance: weapon/w_045.c callbacks00/04/08/0C/10; dra/692E8.c equipment/status/frame;
71830.c reaction/script50;7E4BC.c children49/50;4B758.c multipart rendering; menu.c
LoadWeaponPrg. Pinned AGPL-3.0-or-later sotn-decomp
7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Tables/artwork remain private.
