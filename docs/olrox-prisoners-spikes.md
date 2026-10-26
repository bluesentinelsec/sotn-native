# Olrox's Quarters prisoners and spikes

The internal stage-four profile now advances original NO2 prisoner `0x28` and
spike services `0x21`/`0x22`/`0x23`/`0x31`. This extends the
[NO2 foundation](olrox-quarter-runtime.md); ordinary connected travel remains
disabled while other enemies, mechanisms and Olrox still need integration.

## Behavior and ownership

Prisoners retain their original appearance chance, horizontal displacement,
strict 64-pixel distance thresholds, appearance/rattle/retreat phases and private
animation streams. Their adjacent shadows have checked slot/generation ownership.
Required occupied reservations fail explicitly; orphaned shadows retire instead
of following replacement actors. Re-entry replaces only a matching owned child.

The original factory copies whole positions with zero fractional coordinates.
Subsequent shadow following changes only the high halves, preserving the shadow's
own fractions. Opacity arithmetic wraps at eight bits; the first appearance lasts
128 ticks and a normal later appearance lasts 129. Disabling the shadow's opacity
flag does not reset that stored byte. Rattle requests use positional sound.

NO2 spikes scan on initialization and create temporary damage actors in the
original bounded pool. They differ from Chapel spikes, which retain their owned
adjacent damage actor and orientation recipes. NO2 contact uses the original
environmental element and existing maximum-HP damage service, independent of
ordinary physical defense. Hit detection precedes actor updates: initialization
arms a damage actor for the next contact pass, then the actor retires.

Equipped Spike Breaker replaces the three supported original tile types with
their broken counterparts and creates selected rotating parts, explosion and dust.
NO2 parts retain their own initialization frame, downward launch and private sine/
animation tables. Owning an unequipped armor copy does not activate destruction.
Broken tiles remain transient and restore on fresh room entry; saves remain v9.

Keep NO2's mixed indexing: the initial index uses the actual room width, while
the three-cell scan and effect placement use a fixed 96-tile stride. The ordinary
prisoner room is 112 tiles wide. Normalizing these strides changes original behavior.

Mandatory native safety adaptations omit out-of-map reads at room edges, check
effect-position arithmetic and reject unknown broken-tile replacements before
mutating any selected tiles. The original replacement switch leaves unknown
hazard types uninitialized; native C must not reproduce that undefined read.
Unbreakable artwork carrying hazard collision definitions is not assigned an
invented replacement. Exhausted optional effect/damage pools do not overwrite
other actors. Complete environmental reaction/effect parity remains unproven.

## Validation

Public synthetic tests cover prisoner phases, both random masks, all retreat
branches, distance boundaries, repeated fades, byte wrapping, high-half movement,
owned replacement/orphans, malformed animation/state and normal actor-loop updates.
Spike cases cover initialization/contact/retirement order, environmental element,
equipped destruction, all three replacement tiles, mixed room stride, debris/dust,
full pools, stale owners, room edges, invalid tiles and fixed-point position bounds.
Existing Chapel spike regressions remain enabled. No public fixture contains game
tables, artwork or sound.

An ignored private comparison checks 24,000 native/original instruction ticks
across four RNG seeds and every prisoner gameplay state/retreat branch. It executes
the original animation, random, factory and high-half motion instructions with
isolated initialization, destruction and sound adapters. This is component
evidence, not full original-engine or console parity.

Private native LLDB checks use original placed actors and this repository's
CMake-built libraries/rendering. A protected room-15 start with a seeded actor RNG
observes prisoner appearance, owned shadow and rattle for 650 ticks/163 rendered
frames. A separate explicit ceiling start checks eight-HP environmental damage
despite high defense, then granted/equipped Spike Breaker destruction, selected
debris/dust/explosion, escape and fresh-room tile restoration. These fixtures do
not establish ordinary ability access, balance, uninterrupted traversal or console
parity. Audio uses SDL's dummy device for deterministic ownership checks; listening,
reverb and complete audio parity remain separate.

Provenance: `st/e_prisoner.h`, `st/e_spikes.h`, NO2 wrappers/initializers and shared
services at sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`
(AGPL-3.0-or-later). Original runtime arrays and comparison inputs remain private.
