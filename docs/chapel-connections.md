# Connected Royal Chapel progression

Laboratory now connects to Royal Chapel through the original private US teleport
records. Chapel's selected actors, scenery, mechanisms, Maria scene and save rooms
run within the existing native session. The original two boss overrides enter
[Hippogryph's BO5 overlay](hippogryph-body.md) from either side until its defeat
record exists. The arena's two exits return to Chapel; later crossings use the
ordinary Chapel room without replaying the boss or post-boss Maria event.

This remains a development checkpoint, not a complete Chapel or authentic game.
Exits to Olrox's Quarters, Colosseum and Castle Keep still report an unsupported
destination. Forms, some player abilities, full effects/audio, original title/
prologue initialization and whole-route/timing/console parity remain incomplete.
Neither Chapel nor BO5 gains a separate public diagnostic gameplay start.

## Ownership, saves and entry

The existing [connected-session transaction](connected-stages.md) validates the
destination before replacing the current session. One pending arrival latches
simulation catch-up. Character progress, world events, exploration, RNG streams,
clock, verified music container and ordinary save-store ownership carry across;
destination actors and sound voices are recreated. Failed destination loading
preserves the old owner and ordinary slots and reports the error in the native shell.

Original boss routing consults the Hippogryph defeat record, not its visited bit
or cutscene-completion mask. Post-boss Maria writes flags 98/133 and completion
bits 0x300. [Version-nine ordinary saves](native-saves.md) retain that mask with
explicit v1–v8 migrations. In-session Continue unions the previous visited and
completion masks with the selected slot; the slot supplies other progress.

Fresh Laboratory room-one development starts now place Alucard on the right-hand
approach. The earlier left-side spawn could open Maria's door before her entrance
event and stop with a checked state error. Normal connected arrivals continue to
use the original private coordinates; this corrects the development start rather
than changing the original event, door timing or collision rules.

## Validation and limits

All 100 public synthetic test groups pass in Debug, Release and macOS ASan/UBSan.
`make package` produces the self-contained macOS Release ZIP. Route
tests distinguish both Chapel boss-entry directions, visited/completion settings
and an actual defeat record. Stage tests cover Chapel connectivity, its two save
rooms and BO5's lack of ordinary save rooms. Private runtime inputs and captures
remain ignored; public fixtures contain no game records or assets.

A private native LLDB sequence starts explicitly at Laboratory room one, lets
its original Maria script/speech complete, and walks Laboratory–Chapel–Laboratory–
Chapel. It then walks into Chapel's adjacent save room and uses its ordinary
save UI and isolated slot 15 Continue. Both Debug and macOS ASan/UBSan pass 3,343
input ticks/835 renders. The 70-HP development character receives no protection
or equipment/stat grants. The sequence checks exploration, character and save-
store retention and failed-destination ownership. It is an explicit-start route,
not an uninterrupted castle replay or prologue-derived character proof.

Separate protected, strength-99 fixtures enter BO5 from opposite Chapel sides,
defeat Hippogryph through ordinary Gladius hits, collect Life Max Up, complete
Maria's private event/speech/pictures, and leave through the opposite exit. They
cross the defeated ordinary arena both ways without replaying the encounter.
The scene portions take 2,935 ticks/734 renders in Debug and 2,953/739 under
macOS ASan/UBSan; these totals exclude additional travel/save ticks. Boss HP and
defeat flags are not forced. Protection and strength remain explicit test grants,
so these checks do not prove ordinary-character encounter balance or console parity.

Those boss fixtures then transfer explicitly to another Chapel save room to
check earned progress, fresh byte-identical v9 restoration, in-session completion
unions and a missing-slot failure. That transfer is not walking evidence. All
writes use isolated ignored directories; existing user slots remain untouched.
Dummy SDL audio checks decoding and ownership, not real-device listening or PCM
parity. The earlier stalled exit controllers omitted jump input; passing checks
jump onto the raised exit terrain without changing the original collision data.

Route/boss selection follows AGPL-3.0-or-later `dra/5087C.c` at pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`. All room, teleport, actor, artwork and
sound tables remain private runtime inputs. Session/save ownership is native C
code maintained in this repository.
