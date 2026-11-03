# Main-loop playtime and selected Badelaire

The native session owns the original four signed32 status words for hours,
minutes, seconds and frames. Badelaire125 now uses that owner for ordinary
owned inventory selection, either hand, actual shared slot16 weapon0 variant3,
private sprites/palette/sounds and native enemy damage. No item is granted.

## Clock and attack boundaries

`dra/42398.c:MainGame` updates these words after the game/render preparation.
The SDK count is normalized as signed32 `(vcount + 255) >> 8`. Each word performs
one carry, preserving original signed wrap and single-carry behavior even for
supplied large counts. An hour carry reaching100 sets99:59:59; it does not
normalize an arbitrary loaded hours word on read. The remaining frame word
continues cycling at the cap.

The native fixed60 adapter supplies count256 for each successful engine frame.
This is a nominal engine-frame adapter, not an implementation of the console's
SDK vertical counter or variable raster/render stalls. Post-update inventory and
map flags hold the clock. Menu/map closing frames advance it even though ordinary
gameplay remains held. The native instant menu remains an explicit adaptation;
this does not reconstruct the original inventory fade flags. Ordinary spirit
freeze, its separate release frame, bound Fairy FFF, dying players and Game Over
advance playtime while retaining their separate gameplay/resource gates. Repeated
draws and pending-stage catch-up calls never add time. Failed native updates do
not commit these four words. Earlier player/stage changes are not a whole-pass
transaction; the existing rollback boundaries remain unchanged.

Gameplay `g_GameTimer` still owns MP/water and selected subweapon draw phases.
Engine `g_Timer` owns paused presentation phases, generic Wolf Blink and stage
service phases. Stage time-of-day services receive the actual HMS/frame snapshot
separately from that wrapping engine word. The remaining legacy per-entity pickup
flicker approximations are not converted by this batch. The original MainGame
map-cursor update and unconditional trailing `rand()` remain separate unowned
work; these clock comparisons do not establish whole-main-loop RNG/timing parity.

Badelaire adds the actual low signed word of `timerHours` before attack caps,
poison and the original retained-property buff calculation. Display attack and
new attacks update after the committed hour change, before the next player
control. An existing sword retains its birth properties across that boundary.
Raw metadata stays unavailable until a character binds its real time context;
raw player callers also need the actual hand factory/property owner. A metadata
row or supplied hours word is not playable availability by itself.

## Save and session lifetime

The existing v9 aggregate tick field serializes the actual live clock without
new fields. Legacy native saves up to99999 hours remain accepted: loading does
not silently clamp their hour word to99. The existing aggregate ceiling still
applies when writing that field, without changing live words. Connected-stage
candidates carry all four live words directly, including fractional time beyond
the aggregate ceiling. Bootstrap, full Continue and stage handoff add no clock
update; owned Badelaire binds its time context before equipment restoration and
actor initialization. Fresh development starts begin at zero and retain their
existing grants/resources. This does not supply original New Game/prologue state.

## Evidence and remaining scope

The public `main-loop-playtime` group covers holds, carries, cap, signed overflow,
SDK-count normalization, single-carry hitches, unchanged aggregate compatibility,
context-gated Badelaire, signed attack and separate stage/engine clock services.

Private comparisons execute367,416 supplied four-word/count/flag contexts against
the original MainGame clock tail, stopping before its independent RNG call.
Another35,200 supplied contexts compare actual Badelaire GetEquipProperties and
CalcAttack across signed hours, base stats, equipment, buffs, poison and critical
RNG. Equipment-count, selected RNG-result and SDK root/CLZ services remain
explicit adapters. Native protocols separately exercise ordinary menu selection,
private sword pixels, enemy HP, menu/map/frozen/release/FFF/Game Over boundaries,
an in-memory hour-boundary Continue, actual Library Card stage replacement,
Marble Clock consumers, cold title/Continue with saved hours and
selected player/late-effect failures. Validation results are recorded locally;
original private input bytes and traces remain outside public source/artifacts.

Owned-stock, clock, protection, position, save-room, bound Fairy/load API and
failure contexts are explicit fixtures. They establish no acquisition,
uninterrupted routes, elapsed-hour playthrough, ordinary Fairy activation,
whole-pass rollback, full sharing or console parity. No user save is written.
All233 public groups pass in Debug, Release and macOS ASan/UBSan. Twenty-four
sequential native protocols pass across Debug and ASan, each with an explicit
pass, normal inferior exit and no sanitizer diagnostic. Python34, repository
policy1019, formatting, static SDL linkage and Release ZIP/private-input checks
pass. Both existing play-test snapshots remain unchanged.

All six [Alucard gates](alucard-status.md), remaining equipment/familiar/shared
owners and the full [cold New Game gate](cold-start.md) remain open.

Adapted AGPL-3.0-or-later algorithms: `dra/42398.c:MainGame`,
`dra/menu.c:CalcAttack`, `dra/5D5BC.c:GetEquipProperties`, `weapon/w_000.c`,
`dra/4B758.c` and `dra/84B88.c` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`. All original data remains private.
