# Doppleganger connected progression

Outer Wall crossings now consult the original private boss-transfer table before
ordinary room selection. An undefeated event selects BO4 and its original arrival;
a recorded victory returns the player to normal Outer Wall rooms. The selected
native entrance, physical combat, seals, music and Life Max Up reward use the
existing character/world/time/RNG transfer path. BO4 has no save room or separate
public gameplay start. Other stages' boss overrides remain a separate integration
boundary; parsing an override does not establish destination gameplay support.

World progress now owns the original 32 HHMMSS time-attack records and independent
visited mask. Visiting alone does not defeat a boss. Setting a nonzero record does
not overwrite it; original time zero remains zero. Laboratory paired-boss progress
uses original event 6; Doppleganger uses event 2. New [v7 ordinary saves](native-saves.md)
retain all records. Explicit v4–6 migration converts the old Laboratory seconds and
visited boolean; earlier formats initialize missing events to zero. Readers never
rewrite a slot. Retired fields, reserved bytes and minute/second digits are checked.

The native Continue/development start now centers supported save rooms at X=128.
Its earlier Laboratory-specific start selection put Outer Wall save rooms at X=32,
which could fall outside their supported collision route. Ordinary room/stage
arrivals retain their original coordinates.

Synthetic tests cover zero/first/existing event records, the highest visited bit,
HHMMSS bounds, all 32 save records, malformed/reserved fields, frozen v3/v4/v5/v6/v7
checksums and all six legacy migrations. Boss-transfer tests distinguish visited
from defeated, preserve always-active events, validate source identity/indices,
and reject truncated or unterminated tables without changing outputs. No runtime
transfer/event table is embedded in public fixtures.

Private LLDB checks use protected explicit Outer Wall starts. They cross into BO4,
run its placed opening and selected combat/reward, leave through a reopened seal,
and reenter the normal Wall room without repeating the boss. An explicit native
save-room start then runs the ordinary save UI into an isolated local slot and
fresh Continue retains the boss record. This is not an uninterrupted castle route,
an unmodified-character playtest or console replay. Other missing actors still
warn explicitly. Full mist/elemental effects and timing/audiovisual parity remain
incomplete; do not call the boss or the Outer Wall complete.

The left-entry Debug and macOS ASan/UBSan sequences finish their
connected/reward path at tick 4,446, retain record 57, and draw 1,112 native frames.
The separate right-entry Debug sequence reaches tick 4,149 and retains record 53.
Both test ordinary save/Continue in Wall room 17; Debug also checks a centered,
stable room-14 Continue start. All 59 public groups and release packaging pass.

The event rules adapt `dra/timeattack.c` and the transfer parser adapts
`dra/5087C.c:FindBossTeleport` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
Runtime tables/art/audio remain private. Native save encoding/migration and
transactional session ownership are this repository's C code.
