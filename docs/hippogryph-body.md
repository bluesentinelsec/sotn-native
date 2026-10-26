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

This branch is not a completed encounter. Defeat presentation remains an explicit
development error; reward/arena/cutscene flow, BO5 rendering/audio setup, after-boss
Maria and connected Chapel travel still need integration. The BO5 profile remains
disconnected, without a public gameplay start. Do not merge or describe this
checkpoint as a playable Hippogryph milestone while those paths remain incomplete.

Public synthetic tests exercise activation/owned child creation, takeoff/flight,
landing, distance selection, breath pools, object initialization, lunge timing,
stagger, grab/pinning/drop, linked damage/cooldown/stun/generation handling and
invalid state/data/motion. Private component checks compare 12,160 ticks across
76 seeded sequences with the original US main-body instructions. They execute the
original movement, animation, random, floor/sensor and particle-factory routines,
with isolated initialization, empty collision, audio, event-visit and player
acknowledgement adapters. The comparison includes spawned children and retained
grab values. It does not establish realistic tile/room traversal, full original
frame order, rendering/listening, a natural encounter or console parity.

Provenance: US BO5 `func_us_801A1C14`, `func_us_801A19FC`, `CheckHippogryphFloorCollision`
and the linked-parent paths in `src/st/collision.h`, at sotn-decomp revision
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
Only adapted native C algorithms and table metadata are public; instruction traces,
runtime tables, game artwork and comparison inputs remain private.
