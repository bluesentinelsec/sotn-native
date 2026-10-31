# Alucard milestone status

This is the consolidated status and completion checklist for the Alucard milestone,
reviewed on 2026-10-30. The goal remains original Alucard behavior. Selected playable
paths are implemented; complete Alucard, castle progression and console parity are
unfinished. There is no measured completion percentage or reliable finish estimate.

Individual documents preserve the scope and evidence of their implementation batch.
Some older documents list abilities that were integrated by later batches. Use this
index to find current availability, then read the linked ownership document for
its supported context, tests and remaining limitations. Component APIs, supplied
fixtures and command recognizers do not establish playable availability by themselves.

## Current playable scope

| Area | Implemented selected paths | Ownership and limits |
| --- | --- | --- |
| Normal movement | Walking, jumping/crouching, backdash, double jump, dive kick and Gravity Boots | [Movement](laboratory-movement.md), [backdash](player-backdash.md), [Leap Stone](player-leap-stone.md), [dive kick](player-dive-kick.md), [Gravity Boots](player-gravity-boots.md) |
| Forms | Mist/Gas Cloud, Bat flight/Wing Smash/Fire/Echo, Wolf movement/bite/sprint/charge, direct form conversions | [Mist](player-mist.md), [Gas Cloud](player-gas-cloud.md), [Bat](player-bat.md), [Wing Smash](player-wing-smash.md), [Fire](player-fire-of-bat.md), [Echo](player-echo-of-bat.md), [Wolf](player-wolf.md), [conversions](player-wolf-cross-forms.md) |
| Normal spells | Dark Metamorphosis, Summon Spirit, Tetra Spirit, Hellfire and Soul Steal | [Dark](player-dark-metamorphosis.md), [spirits](player-spirit-spells.md), [Hellfire](player-hellfire.md), [Soul Steal](player-soul-steal.md) |
| Water | Actual Entrance volumes/currents/effects, player drag/gravity/form gates, Wolf swimming, bubbles, ordinary HP loss and native water death | [Stage actors](native-water-actors.md), [player feedback](player-water.md), [bubbles](normal-water-bubbles.md), [damage/contact ordering](player-water-contact.md), [water death](player-water-death.md) |
| Recovery and presentation | Afterimages, selected status masks, poison/curse/stone behavior, blood/orb healing, original HUD MP recovery, Fury Plate, Healing Mail and Blood Cloak | [Afterimages](player-afterimages.md), [status](player-status.md), [poison](player-poison.md), [curse](character-status.md), [stone](player-petrification.md), [blood](enemy-blood.md), [orbs](soul-steal-orbs.md), [Fury](stat-buff-ownership.md), [HUD/Mail](hud-healing-resources.md), [Blood Cloak](player-blood-cloak.md) |
| Combat and equipment | Selected empty-hand, Basilard, Gladius, Mace, Short sword and ordinary Alucard sword/shield paths; selected equipment attributes/passives | [Combat](laboratory-combat.md), [Short sword](short-sword.md), [opening equipment](opening-equipment.md), [inventory](character-inventory.md) |

Owned relics and equipment gate these paths. Development starts retain their
existing resources and grants. A granted item/relic or positioned encounter test
is separate evidence from ordinary acquisition or uninterrupted traversal.

## Completion checklist

All six workstreams below remain open. This is the milestone's completion gate;
finishing one additional passive item does not close a workstream. Each workstream
needs a source-backed inventory of its remaining branches and an evidence record.

- [ ] **Normal control and state transitions:** audit the original player states,
  substates, input/cancel windows, collision/restoration and status/timer boundaries
  against the native dispatch. Implement uncovered branches and verify transitions
  among normal movement, attacks, forms, scripts, grabs and death. Relevant starting
  points: [movement](laboratory-movement.md), [forms](player-wolf-cross-forms.md),
  [status](player-status.md), [boss grabs](player-boss-grab.md).
- [ ] **Damage, ailments and death:** finish remaining original damage kinds,
  elemental reactions, invincibility/helper timing, stone variants, death/fade
  branches and dependent effects. Verify contact/healing priority and failure
  rollback with the real owners. Selected ordinary physical and water deaths do
  not establish the remaining branches. See [contact ownership](player-water-contact.md),
  [stone](player-petrification.md) and [death](laboratory-game-over.md).
- [ ] **Weapons, subweapons, consumables and equipment:** inventory original families
  and special behaviors, distinguish metadata support from live attacks/effects,
  and implement remaining hand/shield specials, consumable use, item passives and
  buff producers with their actual resources and effects. Verify ordinary owned
  selection/use, costs, caps, expiry and save/Continue behavior. See
  [inventory](character-inventory.md), [combat](laboratory-combat.md),
  [subweapons](laboratory-subweapons.md) and [buff ownership](stat-buff-ownership.md).
- [ ] **Familiars and remaining spells:** implement actual familiar lifecycle,
  controls, behavior/combat/progression and Sword Brothers' familiar-dependent
  execution. Its existing recognizer is not an available spell. See
  [command boundaries](normal-spell-commands.md) and [relic controls](relic-menu.md).
- [ ] **Shared runtime and presentation:** finish real factory/entity/primitive/
  attack-group sharing across all supported producers, remaining palette/flash/
  number/particle/shake/capture/audio paths and original ordering/timing. Exercise
  simultaneous producers and optional exhaustion without substitute pools or
  replacing unrelated actors. See [factories](player-ability-factories.md),
  [shared runtime](wolf-shared-runtime.md) and each producer's ownership document.
- [ ] **Integrated acceptance:** run repeatable sequences in the actual native app
  covering movement/forms/combat/resources/ailments/death, pauses/freezes, room and
  stage handoffs, and ordinary save/Continue. Add ordinary acquisition/traversal
  checks as supported castle routes permit. Compare original behavior and record
  remaining concessions explicitly; controlled component tests and passing CI
  alone do not close this gate.

Castle actors, routes, original prologue/title initialization, inverted castle and
endings retain their separate [project acceptance gates](native-port-plan.md).
An unavailable route can block acquisition evidence even when a selected ability
works in a controlled context. Report that limitation explicitly; do not change
default grants or reduce the Alucard milestone to the currently supported paths.

## Work and validation discipline

Keep focused feature branches and PRs. Merge dependencies first only after every
required CI check passes on the exact current head commit. A push or rebase requires
checking the new head again. Preserve private comparisons and actual native checks
alongside public authored tests; publish no private data or traces.

After recording concise results, remove standalone probe binaries/debug symbols,
large raw captures/traces and obsolete build trees. Keep source/probe scripts,
small result logs, private inputs and user saves. Maintain a current local handoff
with pending work and validation limits so an interrupted session can resume.
