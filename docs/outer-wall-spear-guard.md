# Outer Wall Spear Guard

The native Wall profile now runs Spear Guard's patrol, approach/retreat, selected
melee attacks, spinning block, thrown-spear variant and original selected death
sequence. Its adjacent blocking actor has independent private hitboxes and uses
slot-generation ownership. The spear's launch checks its owner; a launched spear
can outlive that owner, following the original flight behavior.

The player snapshot exposes the existing four-tick subweapon-use interval so this
enemy can react to the supported player action at the original decision point.
Spells and other unsupported player actions do not produce fabricated status flags.
Original eight-bit patrol/attack counters retain their unsigned wrapping behavior.
The guard reads special attack power/elements, motion recipes, hitboxes and death
positions from the private original definitions rather than public replacement tables.

Public synthetic coverage checks patrol/attack transitions, byte-counter wrapping,
subweapon startup gating, the blocking hitbox/clang, special melee attack power,
throw/flight, owner loss and fade/death cleanup. The player regression checks the
subweapon interval begins and expires at its original four ticks.

A private native LLDB fixture exercises melee, rendering, a forced throw and death
against the user's original room data. It uses an explicit room and protected player;
the placed Wall guard's normal variant does not gain an additional throwing attack.
These checks do not establish ordinary combat, continuous-route or console parity.
Full status damage, hit flashes, effects and timing/audio parity remain incomplete.

Adapted from `st/e_spear_guard.h`, selected player subweapon timing and shared
services at sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, under
AGPL-3.0-or-later. All original animation, movement, attack, hitbox, graphics and audio
tables remain in ignored private runtime inputs. The native actor factory safely
checks projectile exhaustion where the original routine assumed a free slot.
