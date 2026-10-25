# Selected character ailments

The native player now applies selected curse contacts after the scene resolves
immunity, absorption and physical defense. A living, vulnerable player receives
the original `max(64, 1024 - CON * 4)` simulation-tick duration. Invulnerable,
already-reacting, petrified and lethal contacts do not add a new curse. Its
original status bits reach stage/enemy observations. Selected physical hurt
behavior remains the existing adapter; complete damage-kind/effect parity is
not established by this addition.

Cursed equipped weapons use the original standing, crouching and airborne failed
attack animations and voice. Their weapon/shield hitboxes remain inactive until
the failure finishes. A held selected shield can attempt and fail again as in
the original path. Empty hands and selected subweapons remain usable; blocking
all attacks would change the original game. Supported equipment still determines
what can be used outside this status. Consumable cures and full item families
remain unintegrated. Petrification cancels the failed weapon state without
leaving the player locked after escape.

The timer advances once per player simulation tick and freezes in the native
inventory/map pause. Adjacent room rebases preserve it. Connected stage arrivals
carry the remaining timer explicitly when the player service is recreated.
Ordinary Continue starts a fresh transient state; the timer is not serialized
into v7 ordinary saves, and their format/version stays unchanged. Restoring HP/MP
at a save point does not add an invented curse cure. Native stage adapters retain
the existing save and ownership failure behavior.

The sound path now carries the hurt request followed by the separate curse
voice, rather than replacing one with the other. Full curse palette/blink,
outline, status text, expiry effects and complete audiovisual parity remain
pending. Do not call the ailment or Library area complete.

Public synthetic cases cover duration bounds, invalid-input rollback, lethal and
invulnerable contacts, status expiry, all three failed-attack poses, shields,
unarmed/subweapon exceptions, room transfer, and petrification interruption and
escape. Protected native LLDB checks position an original Library Ectoplasm for
contact, then disable later contacts to isolate the timer. They verify 1,000
initial ticks, original failed equipped attack, inventory pause, an explicitly
selected stage-arrival adapter retaining 73 ticks, expiry and recovered weapon
use. Debug and macOS ASan/UBSan draw 36 frames and exit successfully. This is an
injected contact/arrival fixture, not ordinary travel or a console parity replay.
All 64 public groups pass in both builds; release packaging and source policy pass.

The selected algorithms adapt pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later):
`dra/5D5BC.c:GetStatusAilmentTimer`, `dra/71830.c:AlucardHandleDamage` and selected
stand/crouch/jump failure substeps, `dra/692E8.c` timers/status,
`dra/6E42C.c` equipped/unarmed/subweapon dispatch, and `dra/62DEC.c` save-point
resource restoration. Animation, audio and equipment data stay in private files.
