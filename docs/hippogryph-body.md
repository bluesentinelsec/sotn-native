# Hippogryph main body: work in progress

The internal BO5 body now advances its original idle/activation, takeoff, hovering,
landing, distance-based attack choice, flame breath, ground run, lunge, grabbing
flight, object spawning, swoop, stagger and movement-particle states. Timelines,
choice tables, initialization and sensors remain private runtime inputs.

The original selected quirks remain intact: lunge does not decrement its local
timer, pose-ending transitions retain their original update order, and clearing
grab does not overwrite the previous raw damage value. A captured player follows
the boss's whole-pixel position while retaining its own fractions. The stage
adapter delivers the start, damage and clear commands to the normal-form player
service on the next simulation tick. The original visited event is index seven.

The auxiliary hurtbox routes damage and hit information to the matching owned
body. Both parts share hand/projectile cooldowns, including the original extra
tick for the later part. Hits through the auxiliary part suppress stun as in the
original; direct body hits share stun. Generation checks prevent a stale child
from damaging or following a replacement actor.

The original defeat now clears the grab, records the event, lands, draws its two-line
flash and 111-part ray/spark burst, fades and hands off to the reward/dialogue actors.
The shared reward service uses BO5's own initialization and private drop table to
produce its permanent Life Max Up. Optional effect allocation follows the original
fallbacks; actor-pool exhaustion retries reward birth, and fixed-slot ownership checks
protect unrelated actors. The original fractional velocity clear, asymmetric X tests,
separate-half reverse speed, retained timer and final opacity remain intact.

This branch is not a completed encounter. Arena/cutscene flow, BO5 rendering/audio
setup, after-boss Maria and connected Chapel travel still need integration. The BO5
profile remains disconnected, without a public gameplay start. Do not merge or describe this
checkpoint as a playable Hippogryph milestone while those paths remain incomplete.

Public synthetic tests exercise activation/owned child creation, takeoff/flight,
landing, distance selection, breath pools, object initialization, lunge timing,
stagger, grab/pinning/drop, linked damage/cooldown/stun/generation handling and
invalid state/data/motion, defeat effects/pool pressure and reward conversion. Private
component checks compare 12,160 ticks across 76 seeded sequences with the original US
main-body instructions. They execute the
original movement, animation, random, floor/sensor and particle-factory routines,
with isolated initialization, empty collision, audio, event-visit and player
acknowledgement adapters. The comparison includes spawned children and retained
grab values. It does not establish realistic tile/room traversal, full original
frame order, rendering/listening, a natural encounter or console parity.

A separate defeat comparison matches 36,000 original instruction ticks across 120
cases, including both facings, grabbed/free players, boundary positions, allocation
failures and a counterfactual timer rewind exercising the reverse-speed branch.
It compares parent/child state, all 111 effect records, geometry, fractional motion,
RNG, audio requests and event timing. It executes the original private SDK sine/cosine
routines with isolated allocation, initialization, empty-collision, audio and event
adapters. This is component evidence, not a rendered encounter or console replay.

A private native actor sequence also completes the projected reward and collects it
through normal contact handling, increasing maximum HP by five and releasing all
effect records. It uses verified BO5/DRA/SDK inputs with synthetic empty placements
and a flat floor; it does not load or overwrite ordinary saves.

Provenance: US BO5 `func_us_801A1C14`, `func_us_801A19FC`, `CheckHippogryphFloorCollision`,
shared `src/st/e_life_up.h` and the linked-parent paths in `src/st/collision.h`, at
sotn-decomp revision
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
Only adapted native C algorithms and table metadata are public; instruction traces,
runtime tables, game artwork and comparison inputs remain private.
