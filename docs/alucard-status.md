# Alucard milestone status

This is the consolidated status and completion checklist for the Alucard milestone,
reviewed on 2026-11-03. The goal remains original Alucard behavior. Selected playable
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
| Selected consumables | Owned Library Card travel; weapon20 Karma Coin; weapon15 Shuriken, Cross shuriken, Buffalo star, Flame star and TNT; weapon16 Bwaka knife, Boomerang, Javelin, Fire boomerang and Iron ball; weapon22 Magic Missile; weapon17 Pentagram/Bat Pentagram, weapon19 Neutron Bomb and weapon18 Power of Sire;42 weapon21 foods/Peanut/Toadstool/Meal Ticket;17 equippable medicines, actual cures/buffs/healing/MP and shared effects; Duplicator/Ankh | [Karma Coin](player-karma-coin.md), [Stars/TNT](player-star-weapons.md), [Throwing items](player-thrown-weapons.md), [Magic Missile](player-magic-missile.md), [Pentagrams](player-pentagrams.md), [Power of Sire](player-power-of-sire.md), [Neutron Bomb](player-neutron-bomb.md), [Food](player-food.md), [medicines](player-medicines.md), [Library Card](player-library-card.md), [shared teleport effects](teleport-effects.md); other-castle triggers/stages remain unavailable |
| Water | Actual Entrance volumes/currents/effects, player drag/gravity/form gates, Wolf swimming, bubbles, ordinary HP loss and native water death | [Stage actors](native-water-actors.md), [player feedback](player-water.md), [bubbles](normal-water-bubbles.md), [damage/contact ordering](player-water-contact.md), [water death](player-water-death.md) |
| Recovery and presentation | Afterimages, original16 player timer words, selected status masks, poison/curse/stone behavior, blood/orb healing and numbers, original HUD MP recovery, Fury Plate, Healing Mail and Blood Cloak | [Afterimages](player-afterimages.md), [player timers](player-timers.md), [status](player-status.md), [poison](player-poison.md), [curse](character-status.md), [stone](player-petrification.md), [blood](enemy-blood.md), [orbs](soul-steal-orbs.md), [healing numbers](player-healing-numbers.md), [contact numbers](player-contact-numbers.md), [neutral hurt feedback](player-neutral-hurt.md), [Fury](stat-buff-ownership.md), [HUD/Mail](hud-healing-resources.md), [Blood Cloak](player-blood-cloak.md) |
| Combat and equipment | Owned Axe Armor movement/hurt/attack, actual shared slot16 empty-hand/Basilard/22 weapon0 swords including paired Namakura and clock-bound Badelaire/Mace/Short sword/Alucard sword/shield, the Jewel sword's sparkles and special198 gold coins, the Stone sword, kind134 Holbein dagger slashes and Blue knuckles' arc, Dynamite's sparks, explosion and flames, Monster vial 1's fire-breathing monster, Monster vial 2's homing bat, Monster vial 3's skeleton archer, Heart Refresh's 500-heart refill, Sun/Moonstone hour and Vlad relic stat bonuses, CalcDefense's Shield rod/Medusa/Fire shield terms, Walk armor's explored-room defense and additive body records, Brilliant mail/Staurolite subweapon attack, Vibhuti and Rebound Stone on the shared player runtime, Reverse/Royal/Crystal/Joseph's cloak colours, Alucart sword/mail with the Alucart set's LCK30, the Sword Familiar (ATK + the Sword's level), Leather/Knight/Iron/Herald/Dark/Medusa/Skull/Fire/Axe Lord/Goddess/Shaman/Alucart shield, Knuckle duster/Jewel knuckles, Iron Fist/Fist of Tulkas rushes and fireball, and Morningstar/Mablung/Holy rod/Star flail/Combat knife and paired Takemitsu/Nunchaku/Red Rust/Great Sword/Claymore/Flamberge/Zwei hander/Obsidian sword/Estoc owners with weapon51 glows and specials, Gurthang, Holy sword with its weapon-secondary owner, Rapier/Were bane lunges, Katana slash chain, Osafune/Masamune warps, Thunderbrand lightning, Firebrand/Marsil fire and Icebrand frost specials, Moon rod crescents, Shield rod's orb, the thrown Runesword and Heaven sword (with dual Heaven swords' both-buttons sweep and strike swords), Chakram/Vorpal blade/Crissaegrim, thrown Yasutsuna, Shotel's boomerang/punch, Sword of Dawn's summoned squad, every Mablung+shield combo spell (Leather/Knight/Iron/Herald/Dark/Medusa/Skull/Fire/Axe Lord/Goddess/Shaman/Alucard/Alucart, including Alucard's armed shield), blood-grown Muramasa, back-forward/QCF specials, ordinary sword TILEs and shield knock-away, selected command attacks; selected equipment attributes/passives and absorption/immunity circlets, high-defense guard, Talisman dodge and Ballroom Mask reduction | [Shared ordinary hands](shared-ordinary-hands.md), [weapon special inputs](player-weapon-specials.md), [Gurthang](player-gurthang.md), [Holy sword/weapon secondaries](player-holy-sword.md), [Rapier/Were bane](player-lunge-weapons.md), [Katana](player-katana.md), [Osafune/Masamune](player-katana-warp.md), [Thunderbrand](player-thunderbrand.md), [Firebrand/Marsil](player-firebrand-marsil.md), [Icebrand](player-icebrand.md), [Moon rod](player-moon-rod.md), [Shield rod](player-shield-rod.md), [Alucart set](player-alucart-set.md), [Runesword](player-runesword.md), [Heaven sword](player-heaven-sword.md), [weapon12](player-weapon12.md), [Muramasa](player-muramasa.md), [ordinary weapon0 swords](player-basic-swords.md), [two-hand ownership](player-two-hand-equipment.md), [basic shields](player-basic-shields.md), [basic fists](player-basic-fists.md), [fist specials](player-fist-specials.md), [weapon51 swords](player-weapon51-specials.md), [Morningstar/Mablung/Takemitsu](player-plain-weapons.md), [playtime/Badelaire](player-playtime-badelaire.md), [Axe Armor](player-axe-armor.md), [Combat](laboratory-combat.md), [Short sword](short-sword.md), [Jewel/Stone swords](player-jewel-stone-swords.md), [Holbein/Blue knuckles](player-holbein-blue-knuckles.md), [Dynamite](player-dynamite.md), [Monster vial 1](player-monster-vial-1.md), [Monster vial 2](player-monster-vial-2.md), [Monster vial 3](player-monster-vial-3.md), [Heart Refresh](player-heart-refresh.md), [stat bonuses](player-stat-bonuses.md), [defense terms](player-defense-terms.md), [subweapon gear](player-subweapon-gear.md), [Vibhuti](player-vibhuti.md), [Rebound Stone](player-rebound-stone.md), [cloaks](player-cloaks.md), [opening equipment](opening-equipment.md), [sword command](player-sword-warp.md), [inventory](character-inventory.md), [absorption](player-contact-absorption.md), [immunity](player-contact-immunity.md), [guard](player-contact-guard.md), [Talisman](player-contact-talisman.md), [Ballroom Mask](player-ballroom-mask.md) |

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
  32 states now have selected ordinary native producers. The
  [bound state17 response](player-fairy-rescue.md),
  [native Fairy/FFF dispatch](fairy-native-dispatch.md) and
  [owned load-boundary API](familiar-lifecycle.md) now provide real resource/root,
  item, cleanup, restoration and release/recovery producers for17. Ordinary card
  menu activation, saved progression, general audio/cutscene and whole-pass gates
  still prevent ordinary17 availability. [Sword Brothers39](sword-familiar.md#sword-brothers)
  now runs with the Sword summoned; the common/cross-owner gates remain open; Life Apple rescue17 was previously labeled global-fade.
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
  Life Apple/Hammer hand use is forbidden by the original menu. [Selected food](player-food.md)
  now supplies42 owned items with actual weapon21 factories, private rendering, raw healing,
  Peanut/Toadstool and Meal Ticket inventory/text owners. [Selected Neutron Bomb71](player-neutron-bomb.md)
  now supplies actual hand input/consumption, shared weapon19 child/row, private
  SDK presentation and one native global damage pulse. [Selected Power of Sire72](player-power-of-sire.md)
  now supplies weapon18 parents/next-pass bursts, eight gated global pulses, private
  bat/portrait/burst rendering and actual shared flash/shake/profile12 particles.
  [Selected Pentagram73/Bat74](player-pentagrams.md) now supplies actual weapon17
  hand input/consumption, shared25-row GT4/GT3 rendering, per-hand mutable scratch,
  original SDK/GTE breakup and24-frame native global attacks. Familiar use, remaining
  consumables and weapon/equipment families still require real owners.
  [Selected Magic Missile25](player-magic-missile.md) now supplies actual weapon22
  bow/arrow input/stock, raw item/hand chain4, shared50/56 factories, private sprites,
  physical combat and conditional shield retirement. [Selected weapon16 throwing items](player-thrown-weapons.md)
  now supply80/81/82/134/135, actual kind129/130 hand input/stock, roots and raw-parent
  trails/flames/particles, private SDK sprites, native physical combat and Iron-ball
  collision/shake/dust. [Selected weapon15 stars/TNT](player-star-weapons.md) now supplies75..79,
  actual optional TNT32-row sparks, split explosion factories, native physical
  damage/forced raw groups and separate secondary-bank rendering. Other equipment
  families remain open. [Selected Karma Coin24](player-karma-coin.md) now supplies
  weapon20 input/stock, mandatory26-row ownership, both actual outcomes, internal
  tail hitboxes, native head-hit orbs and real per-hand mutable program data.
  [Selected ordinary weapon0 swords](player-basic-swords.md) now add19 owned one-hand
  items through shared16, actual variants/palette indices/full24-row color copies,
  signed attack/defense/elements and Mourneblade's real orb healing. Selected
  [Namakura/two-hand ownership](player-two-hand-equipment.md) now adds actual paired
  reservation, ordered stock returns/menu confirmation and v9 restoration. Other
  two-hand families remain open. [Actual playtime/Badelaire125](player-playtime-badelaire.md) now supplies the four status words, original signed hours bonus and real variant3 hand owner; the [selected main-loop tail](main-loop-tail.md) now supplies its actual map word and native libc-adapter call, while full BIOS/shared RNG/SDK timing remain open.
- [ ] **Familiars and remaining spells:** finish actual familiar lifecycle,
  ordinary controls, behavior/combat/progression and Sword Brothers' familiar-dependent
  execution. Its existing recognizer is not an available spell. See
  [command boundaries](normal-spell-commands.md) and [relic controls](relic-menu.md).
  The [foundation](familiar-foundation.md) supplies verified private resources,
  original animation and actual bank20/VRAM/bank2 services. Selected Fairy
  [controllers](fairy-controller.md), [room events](familiar-room-events.md),
  [effects](fairy-effects.md), [CD audio](familiar-cd-audio.md),
  [four-slot rendering](fairy-slot-runtime.md),
  [inventory/translation](familiar-inventory-translation.md),
  [shared cleanup](familiar-rescue-cleanup.md) and
  [FFF/player17 dispatch](fairy-native-dispatch.md) now connect their bound native owners.
  The [Sword familiar component](sword-familiar.md) reconstructs its init, entrance, follow,
  GTE model, trail, halo, events and speech. The session now loads, ticks, translates and draws
  it through the familiar load API (card22, record4, real row leases). Its component now also
  runs the target search and both attacks (circle below level 70, dive from 70), and the session
  connects their marks, voices and one-update DE hitboxes to native combat and familiar EXP. The
  relic menu switches the Sword Card; its level-50 gift and farewell run, and the gift (the
  Sword Familiar, item 126) is an ordinary weapon whose attack adds the Sword's level. Sword
  Brothers runs with its D4 ring and global DF attack.
  The [selected load/retirement owner](familiar-lifecycle.md) now provides owned0/3
  resource initialization, root/wings, cached reselection, actual item/rescue producers
  and deselection through a native load-boundary API. Detached graphics/SPU/CD
  preparation preserves live owners on failure. Complete namespace validation also
  fixes late-sibling partial retirement in room translation. Actual connected-room entry
  now retains initialized root/wings and history/progress by the original player screen
  displacement, retires temporary rows before destination callbacks and avoids a second
  familiar update, initializer or camera shift.
  Selected full-engine/session initialization also restores active-card mode0 after
  bootstrap, transfers all seven records/selection through connected-stage candidates
  without menu-load increments, and retains cached-none overlay history.
  The CD service also reconstructs selected managed mode1 continuation/mode0 loop
  commands with actual transports. Selected native stage90..94 volume requests now
  bind actual command8, separate fade acknowledgement and original activity snapshots
  through ordinary/frozen/FFF scheduling. Remaining general stage music/dialogue,
  remaining master controls, retargeted queues, CD reverb and general sharing remain open;
  this does not enable ordinary familiar selection.
  An already bound native CD owner also receives selected stage music/speech,
  stop, pause/restore and real-profile restart requests without overlapping legacy
  decoders. Laboratory/Library boss fades and shared dialogue92 retain their actual
  identities; the Laboratory music-transition victory preserves its original state
  increment. Selected common CD bootstrap now supplies actual initial stage requests,
  flag20 silence and no-card game-over routing. First familiar bank loading rebinds
  the same CD to a prepared SPU while retaining live/saved decoders and queues.
  Save-room stop/restart uses actual XA activity; it creates no legacy overlap.
  Full sound ordering and cross-session CD/SPU continuity still require their
  remaining owners. Selected
  [bound game-over audio](laboratory-game-over.md) now uses separate81/11 frames,
  original acknowledgement/XA gates,12/B cleanup, actual mute and33e. SPU updates
  continue without legacy overlap; original entity/GPU cleanup remains open.
  **Ordinary relic-menu activation remains disabled.** Saved familiar records, remaining
  general audio/cutscene lifecycle, whole-pass rollback and
  integrated acceptance must finish before enabling it. Seven signed-word character
  records already receive actual raw enemy rewards and menu load counts, and ordinary v11
  saves persist them. The Sword familiar, including Sword Brothers39, now runs; other
  familiar owners remain unavailable. Supplied resource/card/item/health/clock
  fixtures remain separate from ordinary selection, acquisition, routes and console proof.
- [ ] **Shared runtime and presentation:** finish real factory/entity/primitive/
  attack-group sharing across all supported producers, remaining palette/flash/
  number/particle/shake/capture/audio paths and original ordering/timing. Selected
  [native SPU voice reverb](spu-reverb.md) now binds actual private SDK Studio B,
  per-tone sends, selected SEQ depths and shared work RAM with menu/freeze/draw/load
  boundaries. Bound CD expiry/loop depth work now reaches that actual owner between
  command updates and sample rendering, with prepared sound rebinding and retained
  pre-SEQ lease spans. Bound80..84 now use actual master gain, depth0, SEQ close,
  three-update all24 RR8 cleanup and queued CdFadeOut1 (command3)/reset127.
  Natural SEQ end retains its access and fourth script gate; explicit close
  releases all24. Selected bound
  7/E/F/A1/202/204 now share the actual CD sound ring after script updates, retaining
  duplicate-XA holds, delayed resume and pre-command lease spans. The
  [effect ring](laboratory-audio.md) now retains original256 read/write slots,
  last-duplicate order/volume/pan and wrap-to-empty behavior without a capacity
  failure. Standalone control/ambience compatibility APIs remain separately scoped.
  Actual inventory-menu entry/exit now queuesA7/A3/E andF/A4/A8 through that ring.
  Bound1/A0..A8 retain conditional release/rekey, current private definitions and
  gain parameters; stage22 gain updates globals before its deferred command1.
  An earlier queued clear still permits the original private-entry0 gain work.
  Map/spell-freeze remain separate. Full SDK flush/timing and session continuity
  still require their actual owners. Selected direct-effect identities now use original same/free/strict-higher/equal
  priority and open-SEQ18/19 restrictions; actual ENV words reclaim direct/script
  leases before effects. Group3 remains excluded from ordinary equal replacement.
  Script notes retain the actual cached-status/even-voice/global-cursor fallback
  across ranges and mute. A1 clears20/21 identity while retaining gain/distance.
  Full SDK voice pressure/key timing, remaining global controls, CD sends,
  original full-engine/room-fade timing, other presets, remaining mixing
  and cross-session audio remain unfinished. Native sessions now initialize the
  common CD owner before familiar loading; no-card deaths use its actual game-over
  lifecycle. Selected
  [MainGame tail](main-loop-tail.md) now binds actual global map words and the
  trailing native libc-adapter call across startup/gameplay/pause/freeze/death,
  retaining pre-tail packet phases and no-tick session transfer. Full BIOS/shared
  RNG/SDK and whole-pass gates remain open. Selected
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

The requested [original cold-start flow](cold-start.md) now has its own source-backed
implementation slices and acceptance gate. Selected `--cold-start` now creates the
original title before any gameplay session, then offers an explicit native
filesystem Continue adapter with actual saved-session ownership and recoverable
load failures. New Game/name entry/prologue/original SEL menus and return-to-title
remain unavailable; the original cold-start gate stays open.

Castle actors, routes, remaining original startup/prologue initialization, inverted castle and
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
