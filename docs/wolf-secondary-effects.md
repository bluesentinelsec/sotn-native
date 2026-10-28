# Selected Wolf secondary effects

The shared ability owner now consumes the Wolf controller's original factory and
sound requests. Ground/slide puffs use actual entity two and private animation;
wall/ceiling particles and underwater bubbles use actual entity five with private
profiles one, nine and thirteen. Effects use the same factory8..15 and child16..63
availability as other selected abilities. A full optional factory/child range
creates no successful effect. Bubble factories born during head20 wait until the
next original factory update rather than producing a child in the same tick.

Controller origins retain the camera at command creation, then apply the room
transfer before current-camera projection. Body-created bubble origins use the
current body camera. Room/camera changes rebase retained particle and puff fractions
without advancing animation, lifetime or RNG. Sounds retain request ordering;
shake requests remain explicit reports for the native camera owner to consume.

Particle children reserve their private count plus the original sentinel row from
the actual stage pool. There is no second primitive pool. Exhaustion retires the
child before consuming RNG. Whole commits validate particle rows alongside Bat,
Echo and Wolf reservations and reject duplicate/stage collisions without mutation.
Private profile pointers, counts, geometry and selected behavior identities are
checked before use. The native draw owner must render the retained tile/puff state;
factory ownership alone does not establish visible effects.

Preserve the original bubble delays, byte colors, surface hiding, signed-word
positions and double lifetime decrement through case fallthrough. Smoke retains
its private position/speed/scale tables, slope division, facing and post-behavior
animation ordering. The original US smoke birth reads an unwritten parameter when
player status0x20000 is set. That unsupported branch fails explicitly here instead
of reading uninitialized memory or inventing a deterministic original result.

Public authored tests cover capacity failure, real shared reservations, RNG,
origin conversion, rebases, timelines, retirement and malformed private inputs.
Private muted LLDB traces compare 72 particle contexts/2,568 frames/24,576 particle
records and 144 puff contexts/3,744 complete behavior/animation frames against
original instructions. Allocation/RNG/destruction use explicit adapters for the
particle comparison; the puff comparison adapts destruction only and excludes
the undefined US status branch. These are component comparisons, not native-app
or console replays. Private inputs and traces stay ignored; no saves are written.

Native scene rendering, actual water providers, camera shake and complete shared
effects remain incomplete. Live Wolf controls/relic switches stay disabled;
ordinary starts grant no relics and saves remain version nine. Do not call Wolf,
Alucard or the game complete.

Provenance: selected `dra/7879C.c:EntitySmokePuff`, `7E4BC.c:func_8011E4BC`,
`alu_anim.c:UpdateAnim` and Wolf request ordering, pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
Original tables and artwork remain private runtime inputs.
