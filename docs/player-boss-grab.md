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
is different from the thrown-release command used by other bosses. The controller
now also consumes that original release4: the boss writes its actual velocity
and raw retained power, then the next player update selects hit15/animation2E,
clears the handshake and plays6ED. Release itself causes no HP loss; subsequent
collision/recovery and secondary impact use the existing hurt owner. No velocity
is guessed from facing or damage. Darkwing Bat and Akmodan II's source producers
remain separate unfinished boss integrations; Hippogryph still uses clear-to-drop.
Normal grab births and damage pulses now supply their original red CLUT to the
[owned timer2/palette presentation](player-script-control.md). The subsequent feedback owner adds actual normal-grab damage numbers and selected
equipment requests. START can also capture the original separate stone reaction2:
normal reset retains the existing animation ID, without the normal grab's animation,
timer8 or pain voice. A surviving stone pulse sets timer4/stone palette8161 without
the normal pulse's voice. Later commands retain this reaction word; the timer owner
applies the palette on the following update. This is a controlled reaction owner,
not an ordinary stone-grabbing actor. The source inventory currently finds no US
stone-reaction producer. Actual producer, dead-release and broader interruption/
effect/sharing contexts remain incomplete.
No transient grab state is added to ordinary saves. This is a dependency for the
boss integration, not evidence that Hippogryph or connected Chapel travel works.

Synthetic player regression tests cover acknowledgement order, capture during
an attack and existing protection, held position/collision/input, successive
damage pulses, defense, poison, minimum damage, drop/landing, lethal completion,
unsupported requests and atomic animation/alignment failure. They require no
original game data and do not establish console or whole-encounter parity.

The later reaction/release batch adds authored normal/stone entry, retained reaction,
release velocity/power, no immediate HP loss, invalid-command and failed-release-
animation rollback coverage. A private instruction comparison matches6,000 complete
`PlayerStepBossGrab`/`HandleDamage` contexts across16 phase/command/reaction
combinations, with recalculation/equipment-count/number/audio adapters. Debug and
ASan traces are identical. All211 tests pass in Debug, Release and macOS ASan/UBSan.

Four muted native protocols and four actual CMake app protocols separately pass
normal/stone reactions in Debug/ASan with normal exits. From default70HP/20MP,
supplied requests hold actual input/collision, apply16 damage through a real20-row
number and set palette8166/8161 on the timer boundary. The apps render327/313
number pixels. Supplied boss velocities of plus/minus6 horizontal and minus2
vertical enter actual hurt15/2E with retained power30 and unchanged54HP. Subsequent
real wall collision produces sound644, a15-point secondary hit and39HP, then
normal recovery. Native recovery takes47/58 updates respectively. Grab reaction,
velocity and protection are explicit fixtures; no HP/MP, clock, RNG, gear, pose or
position grants or save writes are used. These are not ordinary throwing bosses,
natural stone grabs, acquisition, routes or console evidence.

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
