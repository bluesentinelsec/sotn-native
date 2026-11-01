# Alucard milestone status

This is the consolidated status and completion checklist for the Alucard milestone,
reviewed on 2026-11-01. The goal remains original Alucard behavior. Selected playable
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
| Selected consumables | Owned Library Card travel;17 equippable medicines, actual cures/buffs/healing/MP and shared effects; Duplicator/Ankh | [Medicines](player-medicines.md), [Library Card](player-library-card.md), [shared teleport effects](teleport-effects.md); other-castle triggers/stages remain unavailable |
| Water | Actual Entrance volumes/currents/effects, player drag/gravity/form gates, Wolf swimming, bubbles, ordinary HP loss and native water death | [Stage actors](native-water-actors.md), [player feedback](player-water.md), [bubbles](normal-water-bubbles.md), [damage/contact ordering](player-water-contact.md), [water death](player-water-death.md) |
| Recovery and presentation | Afterimages, original16 player timer words, selected status masks, poison/curse/stone behavior, blood/orb healing and numbers, original HUD MP recovery, Fury Plate, Healing Mail and Blood Cloak | [Afterimages](player-afterimages.md), [player timers](player-timers.md), [status](player-status.md), [poison](player-poison.md), [curse](character-status.md), [stone](player-petrification.md), [blood](enemy-blood.md), [orbs](soul-steal-orbs.md), [healing numbers](player-healing-numbers.md), [contact numbers](player-contact-numbers.md), [neutral hurt feedback](player-neutral-hurt.md), [Fury](stat-buff-ownership.md), [HUD/Mail](hud-healing-resources.md), [Blood Cloak](player-blood-cloak.md) |
| Combat and equipment | Owned Axe Armor movement/hurt/attack, selected empty-hand, Basilard, Gladius, Mace, Short sword and Alucard sword ordinary/command attacks and shield paths; selected equipment attributes/passives and absorption/immunity circlets, high-defense guard, Talisman dodge and Ballroom Mask reduction | [Axe Armor](player-axe-armor.md), [Combat](laboratory-combat.md), [Short sword](short-sword.md), [opening equipment](opening-equipment.md), [sword command](player-sword-warp.md), [inventory](character-inventory.md), [absorption](player-contact-absorption.md), [immunity](player-contact-immunity.md), [guard](player-contact-guard.md), [Talisman](player-contact-talisman.md), [Ballroom Mask](player-ballroom-mask.md) |

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
  [status](player-status.md), [boss grabs](player-boss-grab.md). The
  [control inventory](player-control-audit.md) lists all 34 original dispatch states,
  normal substates and cross-workstream dependencies. The selected
  [scripted input/theft reaction](player-script-control.md) adds state48 and its
  actual input/palette/factory/render/recovery owners. The selected
  [Alucard sword command/recovery](player-sword-warp.md) adds state15, and
  [owned Axe Armor](player-axe-armor.md) adds40/41/42/43/50; selected
  [Library Card](player-library-card.md) adds18 with actual item/effect/travel owners;
  32 states now dispatch. Fairy Life Apple rescue17 (previously labeled global-fade),
  Sword Brothers39 and the common/cross-owner gates remain open. The actual Fairy
  producer, item consumption, scene handshake and cleanup are required for17.
  Other-castle triggers/native TOP/RTOP remain unavailable; all-six-substate
  component comparisons do not establish those routes.
- [ ] **Damage, ailments and death:** finish remaining original damage kinds,
  elemental reactions, invincibility/helper timing, stone variants, death/fade
  branches and dependent effects. The [selected hurt owner](player-hurt-motion.md)
  now supplies kind promotion, physical motion/recovery, actual shared impacts and
  palette counters; [poison/curse presentation](player-ailment-effects.md) now owns
  their actual new text/Blink producers, and [cut feedback](player-cut-effects.md)
  owns its selected particles/Blink. [Surviving holy hits](player-holy-hit.md)
  now own actual shared GT4s/Blink, and [surviving dark hits](player-dark-hit.md)
  own their actual timed sprites/shared blend phase/Blink. [Surviving fire hits](player-fire-hit.md)
  now own both timed sprite streams/Blink and actual parent-bound particles.
  [Surviving thunder hits](player-thunder-hit.md) now own their actual four-child
  six-row effects/sounds/Blink. [Surviving ice hits](player-ice-hit.md) now own
  actual24-row G4 shards, SDK geometry and selected room-entry cleanup.
  [Normal held-grab damage feedback](player-grab-damage.md) now owns actual numbers
  and selected Talisman/Cloak/Fury work. The [grab controller](player-boss-grab.md)
  also consumes supplied actual boss release velocities and stone reaction2;
  throwing-boss/stone producers and dead-release contexts remain open.
  [HUD HP hold/secondary impacts](player-hud-hp-hold.md) now own actual64-if-zero
  health-display delay, save-room16 overwrite and signed secondary results.
  [Shared stone-effect support](stone-effect-support.md) now supplies its actual
  selected text/smoke/tile/entity24 owners. The [shared dissolve owner](stone-dissolve.md)
  supplies actual36-row entity32, resident capture and uploads. [Selected stone control](player-petrification.md)
  now connects queued entry/landing/wiggle/escape and selected stone deaths to actual
  factories, sounds, shake, palettes and rendering. Remaining stone/grab/Hammer,
  damage/death/fade and general shared cleanup gates stay open. The subsequent
  [medicine owner](player-medicines.md) supplies the actual Hammer word/stone escape
  protocol; the Fairy producer and Life Apple rescue remain unavailable. [Selected native
  physical/elemental death](player-physical-death.md) now supplies actual normal/fire/
  thunder/ice C0 effects, resident capture and64 real decay uploads; remaining
  interrupted/fade/grab contexts and whole-stage atomicity remain open.
  Verify contact/healing priority and failure
  rollback with the real owners. Selected ordinary physical and water deaths do
  not establish the remaining branches. See [contact ownership](player-water-contact.md),
  [stone](player-petrification.md), [death](laboratory-game-over.md), and the
  [source-backed damage inventory/selected absorption](player-contact-absorption.md).
- [ ] **Weapons, subweapons, consumables and equipment:** inventory original families
  and special behaviors, distinguish metadata support from live attacks/effects,
  and implement remaining hand/shield specials, consumable use, item passives and
  buff producers with their actual resources and effects. Verify ordinary owned
  selection/use, costs, caps, expiry and save/Continue behavior. See
  [inventory](character-inventory.md), [combat](laboratory-combat.md),
  [subweapons](laboratory-subweapons.md) and [buff ownership](stat-buff-ownership.md).
  [Selected owned medicines](player-medicines.md) now supply17 equippable items,
  actual inventory/cures/4096 buffs/deferred healing/full MP and shared presentation;
  Life Apple/Hammer hand use is forbidden by the original menu. Familiar use, food,
  remaining consumables and weapon/equipment families still require real owners.
- [ ] **Familiars and remaining spells:** implement actual familiar lifecycle,
  controls, behavior/combat/progression and Sword Brothers' familiar-dependent
  execution. Its existing recognizer is not an available spell. See
  [command boundaries](normal-spell-commands.md) and [relic controls](relic-menu.md).
- [ ] **Shared runtime and presentation:** finish real factory/entity/primitive/
  attack-group sharing across all supported producers, remaining palette/flash/
  number/particle/shake/capture/audio paths and original ordering/timing. Selected
  [room callback/cleanup ownership](player-room-ordering.md) now runs departing
  callbacks before lookup and prunes32/33/65 before destination allocations. The
  medicine batch extends actual root cleanup to every eight-slot record by flags,
  retaining initialized20000 circle/Pink children and retiring cold38/39; other
  cleanup and full room loading/fade/engine timing remain open. Exercise
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
