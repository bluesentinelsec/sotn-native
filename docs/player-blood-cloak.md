# Selected Blood Cloak recovery

Owned Blood Cloak now refunds the selected surviving queued contact's HP loss as
hearts, capped at the actual character maximum. The normal player borrows current
cape/heart values before control. A successful player update emits the request;
the native session commits its resource change before room/stage transition can
replace the session. Failed player updates retain HP, queued contact, RNG and the
previous request. No new equipment/relic grants or version-nine save fields are added.

Lethal, immune and absorbed contacts do not refund hearts. Ordinary water response9
is excluded, including zero-loss exposure. Stone contacts use their real selected
damage branch: a surviving zero-loss stone contact still calls AddHearts(0), which
requests presentation when hearts were below maximum. Full hearts suppress both
factory and pickup sound. Poison and environmental fractions use the selected
actual damage result. This does not extend the existing damage service to every
original damage kind, grab, weapon or equipment special.

Presentation requests original shared factory99, actual child51 and pickup sound67A.
Optional factory/child exhaustion omits the visual without undoing the committed
resource change or suppressing its sound. There is no separate effect pool or
primitive lease. The child uses private bank3/frame14 artwork and SDK trigonometry,
a motionless birth at the captured pre-reaction/collision origin with Y-16,
priority player-2, retained facing/fractions and original wrapping arithmetic.
Its128 motion updates rise by half a pixel, apply cosine sway and decrease sway
speed only on gameplay-clock multiples of four. Blink begins from the original
pre-decrement timer<32 test; drawing uses actual child-slot XOR engine-clock parity.
Native drawing preserves clipping, target offsets and one presentation shake.

Menus/map/repeated draws hold lifetime; initialized children hold through spell
freeze. Camera/room rebases translate without ticking, and ordinary offscreen
bounds retire the real child after update. New stage/Continue owners clear transient
children while ordinary heart progress retains its existing save/session behavior.
The component, factory and resource APIs alone do not establish another item or
producer's availability. Familiars, full equipment/damage behavior, complete shared
contention, Alucard, the castle and console parity remain unfinished.

Provenance: selected `dra/5D5BC.c:HandleDamage/AddHearts`,
`dra/7879C.c:EntitySmallRisingHeart` and `dra/4B758.c` Blink rendering,
pinned `sotn-decomp` `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`
(AGPL-3.0-or-later). Definitions, artwork, SDK tables and comparison traces stay private.

## Validation and evidence limits

Public authored coverage exercises surviving/poison/stone/lethal/water/full-heart
requests, invalid resource snapshots, next-update clearing, freeze holds, room
translation and failed-player rollback. Shared-owner tests cover capped/zero/invalid
resource operations, optional factory/child exhaustion, private-data failure
rollback, motionless birth,128-update lifetime, Blink threshold, signed wrapping
and exact offscreen boundaries. The full188-test Debug and Release suites and
release packaging passed; final player/resource changes also passed26 focused
Debug tests and four macOS ASan/UBSan groups. All34 Python policy tests passed.

Private Debug and ASan/UBSan traces are byte-identical. Original instructions match
4,608 supplied selected HandleDamage/AddHearts cases and8,976 rising-heart updates
across784 contexts, including64 complete lifetimes and authored signed-word
boundaries. The comparisons adapt equipment-count lookup, stat recalculation,
factory/sound calls and destruction. They verify selected branches and private
SDK motion, not the full damage service, stage simulation or a console replay.

Muted native Debug and ASan/UBSan checks equip an actual private Blood Cloak,
reach a real Laboratory zombie contact, verify HP70→64/hearts10→16 exactly once,
and render factory99/child51 with its private sprite contributing11 frame pixels.
They verify menu/repeated-draw holds,34 actual Summon frozen/release updates,
128-motion retirement and a connected stage replacement retaining hearts/cape
while clearing transient children. The CMake-built application separately confirms
ordinary zombie contact, the real child birth and20 updates without refund replay,
then exits normally through SDL quit in both configurations.

These checks use an explicit cape grant and post-contact protection; the stage
check also supplies a queued arrival. Their initial health/stats/position/relics
remain ordinary development values. They write no user saves and do not establish
ordinary equipment acquisition, uninterrupted routes, full equipment/Alucard or
console parity. Large regenerated traces and probe binaries need not be retained
once their concise results are recorded; source/probe scripts remain available.
