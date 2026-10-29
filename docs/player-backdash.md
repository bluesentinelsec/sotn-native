# Selected Alucard backdash and ordinary movement puffs

A / Xbox Y now reaches the original normal-form backdash through the existing
Triangle/cancel action. Menus retain their cancel action and block a held closing
button from triggering gameplay. Bat/Wolf keep their owned form-specific input.

Backdash enters original standing substate three, selects its private animation,
sets facing-relative speed -3.5 and zero vertical speed, captures a blueprint-zero
factory request, clears the active hand action and requests the original scrape.
It spends no MP and grants no invincibility. Preserve original input priority:
hand actions, jump and Down resolve before backdash. A locked hand action blocks
it; unlocked hand recovery can consume held input, otherwise the command requires
a fresh press. The active backdash animation prevents retriggering.

Standing decelerates by one eighth per update. Its first poses keep movement
locked; pose four (or original alternate animation) allows walking, while pose
seven/end permits ordinary standing/Up/walking. Jump and Down retain original
cancel paths. Actual player collision and live afterimages run at their existing
update points. This is a selected controller integration, not complete movement,
weapon-cancel or console parity.

Ordinary walk starts now request blueprint one/parameter five. Selected hard
landings and backdash request blueprint zero; normal landings retain the original
soft/hard sound identities. The captured fixed-point room-space source precedes
collision/camera follow, retains facing/priority and rebases without another
simulation update. A separate request bit keeps blueprint zero valid.

The existing shared factory/child owner consumes these optional requests before
its factory phase. It creates real entity-two smoke children with private timing,
positions and artwork. Walking profile five uses its original -6 offset, low
parameter scale and reversed velocity-table selection; only its original low
parameters 0..2 are accepted. Ground/slide profiles remain intact. The owner
shares actual slot availability with transformations and other selected effects,
uses no new pool, and omits an optional request if every factory is occupied.
Smoke consumes no RNG or MP. Late malformed profiles preserve runtime/character
snapshots. The original US unwritten status0x20000 birth branch still fails
explicitly instead of inventing an original value.

Public authored tests cover both facings, initial speed/source, held-input gates,
early/late direction, jump/crouch priority, airborne rejection, zero-blueprint
ownership, no-tick rebases, private profile-five indexing, real shared births,
factory exhaustion, unchanged resources and atomic failures. Existing keyboard and
virtual Xbox tests check the shared A/Y action. All 156 tests pass in Debug,
Release and macOS ASan/UBSan, alongside Python checks and release packaging.

Private muted instruction comparisons match 6,048 original backdash constructor/
standing-controller snapshots, including phase/end, facing, hold/tap and cancel
paths, captured factory origins and sound. Factory/sound calls use explicit
boundary adapters; animation/collision run separately. Another 180 supplied smoke
contexts cover 4,680 original entity/animation frames, both facings, slopes and
word/fraction wrap, with an explicit destruction adapter. Debug and ASan traces
match; private tables and traces remain ignored.

Native Step/Draw checks use the ordinary development character and enemies,
without relic, position, protection, stat or HP/MP grants. They verify one backdash
while holding the input, actual blueprint-zero/profile-five owners, live afterimages,
ordinary jump, unchanged MP and retirement. Seven sampled draws contribute 561
smoke pixel differences. Separate CMake-executable checks verify 190 contributed
smoke pixels and the same action/ownership/resource/retirement behavior before
ordinary SDL quit. Debug and ASan/UBSan pass. They write no saves and do not prove
an uninterrupted route, ordinary full-game initialization or console parity.

Normal status/palette/shake producers, water, spells, full equipment/familiar
behavior, all-ability/GPU contention and complete Alucard/game parity remain
incomplete. Ordinary starts and version-nine saves remain unchanged.

Provenance: selected `dra/6E42C.c:{func_8010FD88,func_8010E6AC}`,
`6FDF8.c:func_8010FDF8`, `71830.c:PlayerStepStand`, and
`7879C.c:{EntityFactory,EntitySmokePuff}`, pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
