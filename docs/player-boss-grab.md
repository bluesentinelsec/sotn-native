# Selected boss grab service

The native normal-form player accepts Hippogryph's original start, held and damage
handshake. Start enters player step 12 on the next simulation tick, cancels the
hand attack, selects the original grabbed animation and pain sound, and stops
both velocity axes. The boss can align whole-pixel position without losing either
fraction. While held, player input cannot move or attack; collision does not move
the player out of the boss's hold. Pause remains unavailable.

Each damage pulse now uses the selected original [held-grab damage feedback](player-grab-damage.md):
actual defense/prior poison, inclusive Talisman roll, unconditional type0 number,
surviving Blood Cloak/Fury requests and the existing lethal dispatch. Ordinary hit
protection does not block a pulse. Guard/dodge results retain the held state and
grab palette/sound without ordinary contact visuals. Ordinary contact and new
stone/curse/poison cannot interrupt a pending or active hold.

Hippogryph clears the handshake at the top of its flight. The next player tick
enters hit substep 1; normal hit recovery then resumes gravity and landing. This
is different from the thrown-release command used by other bosses, which remains
unsupported. Stone-reaction grabs, remaining dependent effects and complete death
presentation/parity remain incomplete.
Normal grab births and damage pulses now supply their original red CLUT to the
[owned timer2/palette presentation](player-script-control.md). The subsequent feedback owner adds actual normal-grab damage numbers and selected
equipment requests; stone-grab remains unwired.
No transient grab state is added to ordinary saves. This is a dependency for the
boss integration, not evidence that Hippogryph or connected Chapel travel works.

Synthetic player regression tests cover acknowledgement order, capture during
an attack and existing protection, held position/collision/input, successive
damage pulses, defense, poison, minimum damage, drop/landing, lethal completion,
unsupported requests and atomic animation/alignment failure. They require no
original game data and do not establish console or whole-encounter parity.

Private Debug and macOS ASan/UBSan checks compare 2,328 ticks in 24 sequences
against the pinned `PlayerStepBossGrab` C function with isolated damage, effect and
kill service adapters. They vary health, defense, raw damage and poison, and use
the private DRA animation timelines. This comparison covers the grab function,
not the complete original frame update. A separate injected native Laboratory
fixture advances 417 ticks and 104 regular renders, checks three damage pulses,
HUD health, held pause/map/input gates, drop/landing and lethal game over. Its
captured grabbed pose was inspected. These are component checks, not a natural
Hippogryph encounter, listening review, continuous route or console comparison.

Adapted from `src/dra/71830.c:PlayerStepBossGrab`, selected `PlayerStepKill`,
`src/dra/692E8.c` update/status/collision ordering and
`src/dra/5D5BC.c:HandleDamage` at sotn-decomp revision
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
The US BO5 state-18 instruction trace establishes the selected clear-to-drop
request. All animation, sprite and sound data remain private runtime inputs.
