# Entrance water-room rocks

Both sides of the water-room boulder now take their original two strikes, update
foreground collision tiles, spawn selected gray explosions/falling debris and retain
broken-side flags. The left side drops the original pot roast. Re-entry restores
the broken collision layout from saved state. NO3 and NP3 retain separate private
table offsets and their original actor identities.

Stage mechanisms now own a checked copy of background tile IDs as well as foreground
tiles. Original room resources remain immutable. The boulder’s background changes
and selected Jewel Sword door restoration update those copies; room replacement
and destruction release them. Missing/truncated layers fail explicitly.

The later Jewel Sword ritual still requires original wolf and bat contact. Those
transformations remain unavailable, so breaking the rock never fabricates their
completion bits. The door’s saved/open path and two-layer changes run when the
original prerequisite bits exist; this does not make the full ability puzzle playable.
Version-six saves retain the four-bit puzzle state and boolean door flag without
changing format. Invalid serialized values fail before replacing loaded state.

Public synthetic tests cover first/second strikes, the food factory, both stage
profiles, tile/background ownership, re-entry, gated door changes, falling/bouncing
fragment cleanup, invalid variants and malformed saved flags. Private native LLDB
checks strike and traverse both barriers, then save/fresh-load in isolated slots.
They use explicit protected fixtures; they are not continuous-route or console
parity evidence. A separate ordinary-health input sequence reaches this room from
the corrected castle entry. Full effects, transformations and parity remain incomplete.

Adapted from `st/e_jewel_sword_puzzle.h` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, under AGPL-3.0-or-later.
Original tile recipes, fragment data, graphics, sound and game binaries stay private.
