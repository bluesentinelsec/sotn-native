# Outer Wall Medusa Heads

The Wall profile now runs its original selected Medusa spawners and blue/gold
heads. Spawners retain their private camera/vertical eligibility windows, bounded
head pools, countdown/retry rules and gold chance. The libc roll is consumed even
for a zero-gold-chance profile. New heads choose an original screen edge from
player position/facing, rebase it into room coordinates, then run the original
alternating vertical acceleration and animation-facing rules. Defeated heads
transition into the shared explosion path.

Gold contact retains the original stone element and reaches the selected
[player petrification service](player-petrification.md); it is not reduced to
ordinary physical contact. That service still has explicit statue/fragment,
lethal-variant and presentation gaps. Complete encounter/route, effects/audio,
contact-priority and original-console timing parity remain unproven.

Public synthetic tests cover both colors, private-table bounds, screen-edge
rebasing, upward/downward flight, contact element propagation, destruction,
full-pool retry without consuming RNG, paused out-of-window countdown and invalid
signed speeds. Private LLDB checks use an explicit tower start that activates the
original spawner. Starting midway between placement activation windows can omit
that spawner in a diagnostic room start; it does not establish ordinary-route
behavior. The fixture observes original-profile blue/gold spawning, positions one
gold head for deterministic contact, clears fixture protection for that contact,
then verifies stone/escape and forced destruction cleanup. Debug and macOS
ASan/UBSan checks use private original data and dummy audio. They are protected
component fixtures, not a continuous route or console comparison.

Adapted from `src/st/e_medusa_head.h` at sotn-decomp revision
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, under AGPL-3.0-or-later.
All original spawn profiles, initialization/animation tables and artwork remain
private runtime inputs; no game assets are distributed.
