# Original player timer ownership

The player now owns one bank of the original sixteen timer words. Existing poison,
curse, weapon, spell, protection and afterimage names alias their actual indices;
`SotnPlayerState.player_timers` is an immutable copy. The HUD's sixteen stat-buff
counters retain their separate original owner. No counter bank is serialized and
ordinary saves remain version9.

The native bank retains raw sixteen-bit representations. Countdown wraps as the
original signed word, and the protection helper compares signed values before
applying its minimum. This preserves both positive durations above255 and signed
boundary behavior without C signed overflow.

| Word | Original role and current boundary |
| --- | --- |
| 0/1 | Actual poison/curse durations; aliases replace their separate counters |
| 2 | Damage palette, with normal8100 on expiry |
| 3 | Shared high-jump word used as palette; afterimage hold and eligible recovery |
| 4 | Actual private SDK four-corner recovery colors and terminal blend clear |
| 5/6/7 | Coyote grace, fall animation expiry and drop-through interval |
| 8 | Original impact word; remaining hurt/impact producers are still unavailable |
| 9 | Actual weapon startup and original status400 |
| 10 | Actual subweapon/selected Fire of Bat use interval and status800 |
| 11 | Actual Dark Metamorphosis duration; its existing native palette job consumes it |
| 12 | Actual spell-use interval/status1000 |
| 13 | Original flashing-invincibility word/expiry; no new Blink15 or consumable producer |
| 14 | Actual common protection minimum, including damage/forms/scripts/freeze release |
| 15 | Actual afterimage disable interval and enable at expiry |

## Frame order and real owners

Each ordinary living update processes a word's action, decrement and expiry before
moving to the next word. Healing still precedes the timer pass, which precedes
physical/simulated input and contact consumption. The existing afterimage owner
processes word15 after word14; unbound component callers still count down that
player word but cannot enable a scene display.

Poison expiry uses prior status and starts color4, protection14 and afterimage15
at12. Their later passes in that same update leave each at11. The original curse
expiry condition also tests the preceding curse bit; this boundary is preserved.
Palette3 similarly uses prior stone/poison/curse masks. Word13's expiry grants
word14 sixteen before word14 decrements to15. Longer positive protection survives,
including ongoing hurt and landing. Negative retained words follow the original
signed comparison. Word6 retains the original fall animation/carry-clear gate.

Dark's duration is now word11. Its existing successful-player/native palette owner
still starts/stops the actual shared job; engine-clock palette animation continues
through pause while simulation duration holds. No second duration or visual pool
is introduced. Subweapon use now occupies its original word10 instead of the
native byte-array's former placeholder0. Stone entry's original0/1/2 clear now
clears actual poison/curse/hit-palette words. Axe's converted stone response keeps
its existing ailments because it never enters the stone handler.

Freeze, release-only frames, menu/map and repeated draws hold the timer pass.
Actual shared-effect requests may still set their original words during frozen
updates; a held countdown does not suppress those producers. Dead updates retain
all counters and pending healing without revival. Failed player updates retain
the complete bank, resources, RNG and queued requests. Room rebases add no tick.
Connected-stage restoration retains the existing explicit ailment carry after
bootstrap; other transient words use their actual new-stage owners. Fresh Continue
rebuilds transients and preserves ordinary character/world progress.

## Evidence and limits

Authored public tests cover all16 words at zero, expiry,255/256 and signed-word
boundaries; exact aliases/immutable copies, signed minimum and13-to14 handoff;
ordered poison/curse/palette/fall expiry; all16 freeze/release/dead/rebase holds and
late SDK failure rollback. Existing weapon/curse/stone fixtures now expect the
original stone cure and wait for actual poison-expiry protection before a later
hit. Guard's unbound fixture counts word15 too. Longer hurt protection and Axe
ailment retention have explicit regressions.

Private comparisons match20,000 complete original16-word timer-loop contexts and
3,072 retained guard/palette contexts, including actual private SDK colors,
signed wrap/minimum, helper ordering, fall animation and movement flags. Original
protection/afterimage reset/enable instructions execute in the reference; Dark's
palette service has an explicit adapter. Counter comparisons alone establish no
new effect, item or ability availability.

A muted native sequence retains default70HP/20MP, uses a spawned/positioned actual
Library seed's queued contact for70-to42 HP and the real3,999-update poison duration,
then checks six menu holds and actual Summon5MP/freeze33/release1. Connected-stage
replacement carries the actual remaining poison count without an extra tick.
Natural expiry starts words4/14/15 at11, draws612 changed recovery-color body pixels
and completes their eleven remaining passes. The actual Gladius attack owns word9
four and retires it; HUD buff0 remains independent. An isolated ordinary save UI
heals HP/MP through its real owner and fresh v9 Continue rebuilds transient words.
Seed, destination/save-room and later protection fixtures are separate from
acquisition, uninterrupted routes and console proof. No HP/MP/RNG/clock grants or
user-slot writes establish this sequence. Final sanitizer/platform and additional
native/app results are recorded in the feature PR.

This closes the selected word-bank/expiry owner, not the remaining hurt kinds,
impact effects, consumables, familiar lifecycle or cross-owner acceptance.
[Milestone1](alucard-status.md) remains open at31/34 dispatched states;17/18/39 and
its other completion gates remain unfinished. Alucard and castle are incomplete.

Provenance: `game.h:AluTimers/PlayerState`, `dra/692E8.c` timer loop and status,
`6DF70.c` protection/afterimage helpers, `71830.c` stone entry, and the existing
native afterimage/Dark owners. Pinned AGPL-3.0-or-later sotn-decomp
7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. All original tables/artwork remain private.
