# Selected Laboratory subweapons

Hold Up and tap either attack button (keyboard Z/D or Xbox X/B). Dagger, axe,
holy water, and stopwatch now use private original definitions for attack power,
elements, heart cost, chain limits, and hit timing. Insufficient hearts or a full
chain permits the original ordinary hand-attack fallback. Heart broach discounts
apply in either accessory slot, with a minimum cost of one heart. Projectile kills
use the same damage, enemy death, and experience path as hand attacks.

The bounded service owns sixteen projectile slots. Reusing a slot changes its
generation, so a previous projectile's target cooldown cannot suppress a new
throw. Room entry clears transient projectiles and the stopwatch flag. Menu
pause stops their motion, lifetimes, damage cadence, and heart spending. Saved
character progress includes hearts and the selected subweapon, not projectiles.

Selected reconstructed paths preserve dagger speed, terrain sweeps, impact delay,
and wall bounce; axe velocity/gravity and hard-hit bounce; holy-water terrain
resolution, bottle lifetime and four delayed flames; and the stopwatch rise,
countdown, exit, and actor freeze exceptions. Flame damage remains periodic.
A rounded mathematical sine reproduces all 1,025 entries of the pinned original
quarter-wave table in a private comparison, without embedding that table.

This is a selected attack checkpoint, not complete subweapon parity. Cross,
Bible, rebound stone, vibhuti, and agunea can be collected but their attacks warn
once and remain unintegrated, without spending hearts. Full projectile trails,
glass/debris effects, original color modulation, throw grunts/shared random
consumption, item crashes, and all transform/form interactions remain incomplete.
Texture/palette bytes and flame UV animation data stay in private runtime files.
The native affine primitive renderer presents selected original projectile art;
complete GPU ordering and effect blending are separate work.

For a controlled native test, append `--laboratory-test-profile dagger`, `axe`,
`holy-water`, or `stopwatch` to a fresh Laboratory launch. These named development
fixtures grant the selected subweapon and fill current hearts to their maximum;
they are not original new-game starts. `inventory` grants selected test equipment;
`progression` starts one EXP below the next level. Continue rejects fixture options
and restores only saved progress.

Public synthetic regressions cover heart costs/discounts, chain rejection and
unchanged RNG, projectile motion/impacts, flame creation, stopwatch duration,
room reset, invalid data, repeated casts after animation transitions, and affine
texture bounds. Private LLDB tests ran this project's native scene: dagger, axe,
and holy water each killed an original Bloody Zombie; stopwatch activation and
expiry ran with original artwork/audio. Those targeted checks do not prove full
original gameplay or audiovisual parity.

Adapted source: `src/dra/6E42C.c`, `5D5BC.c`, `7879C.c`, `84B88.c`, and
`8A0A4.c` at the pinned reconstructed revision
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
