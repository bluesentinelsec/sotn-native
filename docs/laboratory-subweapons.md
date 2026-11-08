# Selected Laboratory subweapons

Hold Up and tap either attack button (keyboard Z/D or Xbox X/B). Every subweapon now runs through
the shared player runtime with its original definition (attack, elements, heart cost, chain
limit), factories, primitive rows, attack groups and casting grunt:

- the [dagger](player-dagger.md);
- the [axe](player-axe.md);
- [holy water](player-holy-water.md);
- the [Cross](player-cross.md);
- the [Bible](player-bible.md);
- the [stopwatch](player-stopwatch.md);
- [Rebound Stone](player-rebound-stone.md);
- [Vibhuti](player-vibhuti.md);
- [Agunea](player-agunea.md).

Insufficient hearts or a full chain permits the original ordinary hand-attack fallback. Heart
broach discounts apply in either accessory slot, with a minimum cost of one heart. Subweapon kills
use the same damage, enemy death and experience path as hand attacks.

The earlier bounded subweapon service (sixteen projectile slots, its renderer and its actor
attack channels) has been removed; `subweapon.c` keeps only the definition read, the Heart broach
cost and `func_800FE3C4`'s attack roll. Saved character progress includes hearts and the
selected subweapon, not live subweapon entities.

This is a selected checkpoint, not complete subweapon parity. Item crashes, the stopwatch's
CD muting (see its document) and transform/form interactions remain incomplete.
Texture/palette bytes and UV tables stay in private runtime files.

For a controlled native test, append `--laboratory-test-profile dagger`, `axe`,
`holy-water`, or `stopwatch` to a fresh Laboratory launch. These named development
fixtures grant the selected subweapon and fill current hearts to their maximum;
they are not original new-game starts. `inventory` grants selected test equipment;
`progression` starts one EXP below the next level. Continue rejects fixture options
and restores only saved progress. These fixtures exercise the shared-runtime subweapons.

Each subweapon's document lists its public tests, private emulator comparison and LLDB probe.
Those targeted checks do not prove full original gameplay or audiovisual parity.

Adapted source: `src/dra/6E42C.c`, `5D5BC.c`, `7879C.c`, `84B88.c`, and
`8A0A4.c` at the pinned reconstructed revision
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
