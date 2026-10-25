# Selected player petrification

The native player now enters the original selected stone state, recoils, lands,
and waits for directional taps. CON and the owned player RNG select the escape
count. Holding a direction does not repeat escape inputs. Individual taps or a
selected contact impact advance escape; the original private rotation cursor
animates each wiggle. The selected escape animation returns to ordinary control.
Attacks, shields, pause and save-room activation remain unavailable while stoned.

Contact first applies the equipment element prepass. A new stone hit subtracts
twice physical defense and may petrify with zero HP loss. Contact while already
stoned uses one eighth maximum HP. The selected lethal flow reaches the existing
game-over service, with the new-hit landing/wait and selected stone-death clocks.
Ordinary saves do not serialize an active transient stone state.

Common private palettes provide the selected stone tint; CPU sprite rotation
uses the original pivot and checked private wiggle table. No rotation/palette data
is embedded. This remains a selected integration: original statue/dissolve child
entities, rare statue variants, smoke/fragments, camera shake, the full set of
lethal stone branches and complete GTE/lighting/audio/timing parity remain
incomplete. In particular, hiding the player at the selected death transition
does not establish that its original fragment presentation has been integrated.
[Selected poison mechanics](player-poison.md) and curse now run separately; complete
ailment effects and other elemental reactions remain incomplete.

Synthetic public tests cover zero-damage petrification, landing, disabled actions,
held versus tapped input, impact/escape, selected lethal completion, atomic corrupt
animation failure, palette selection/mirroring/clipping and invalid transforms.
Private native LLDB checks inject explicit zero-damage and lethal hits in an
original room and protect the escape sequence from other enemies. They verify
settling, palette/wiggle drawing, escape and selected game over in Debug and macOS
ASan/UBSan. They are component fixtures, not ordinary Medusa encounters, continuous
castle traversal or console parity comparisons.

Adapted from `src/dra/71830.c:PlayerStepStoned` and selected `PlayerStepKill`,
`src/dra/5D5BC.c:HandleDamage` / `GetStatusAilmentTimer`, and selected timer
semantics from `src/dra/6DF70.c` at sotn-decomp revision
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
Users supply all original game data privately.
