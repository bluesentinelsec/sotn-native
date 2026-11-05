# Selected owned medicine use

The 17 equippable medicines in IDs 144..162, excluding Life Apple146 and Hammer147,
now connect normal input, actual inventory consumption, factory55/rowless child39
and their original resources and presentation. Library Card166 retains its
[separate travel owner](player-library-card.md). Ordinary starts and savev9 are
unchanged; this integration grants no medicine or accessory.

Medicine input precedes curse rejection. Standing, walking, crouching and ordinary
air control retain the original input gates. Only standing with substate below
hex40 selects animationBE plus its ledge variant. Medicine use introduces no attack
mode, cast state, startup timer or velocity clear. An actual entity39 in slots32..63
blocks another use; a pending factory alone does not. Actual eight-root exhaustion
omits use without cost or replacement. Successful requests consume once, after the
player update, animation and collision succeed. Spares exclude the equipped copy;
the last copy clears that hand. Either Duplicator84 suppresses consumption. Native
failed-player handling restores the real factory reservations.

Actual child39 retains the private schedule, signed timer/index and next-pass
factory order. It creates real circle53/38, outline40/31, Blink44/11 and label47/37
owners in the existing shared slots and stage pool. Resources commit after actual
row work. Buffs write the original4096 counters, leaving4095 after the same HUD
frame; derived attributes bind before the next player control. Cures set actual
poison/curse words to2. Potion, High Potion and Elixir overwrite pending kind/amount
with kind1; blood's first-pending API is not substituted. Ankh of Life80 adds half
to Potion/High Potion healing once, including with two copies; Bloodstone82 does
not scale them. Manna Prism refills actual character MP and binds player MP,
including during death. Dead states retain pending healing without revival.
Existing deferred-healing owners supply caps, numbers, sound and Blinks. Healing
Mail's later HUD write retains its overwrite priority.

Hammer uses the separate original signed DRA word. Its effect sets64 and one shake
only in actual stone state11. Full player callbacks decrement the word before
early teleport/death handling; frozen animation, menus and draws hold it. Stone
consumes the remaining word through ordinary forced directional taps and escape.
Native session replacement restores this global after bootstrap, without adding
ordinary save fields. The original menu forbids equipping Life Apple146/Hammer147;
neither has ordinary hand use. Their metadata/effect APIs and supplied factory
checks do not establish a familiar. The actual Fairy producer, familiar lifecycle
and Life Apple state17 remain unavailable.

Circle38 owns 32 real GT4s in two sixteen-row rings. Both directions retain private
profiles/SDK trig, the inner ring's next-angle offsets, signed extension words,
packed fractions and original colors/UVs. Afterimage reset precedes optional
allocation failure; failure retires the child without RNG. Initialized circles
advance in selected spell freeze. Original235 packets have neither camera2 nor
room100 translation. Room rebases and actual stage shoves retain their separate
entity owners. The two stat-label GT4s and sixteen trail tiles share the existing
18-row lease. Labels8..19 retain their private left/right suffix, packed target and
draw flags, including the hidden second row's ordinary camera translation.

Ordinary room entry retains initialized20000 medicine/circle children and prunes
cold ones. It clears actual factory records lacking20000 before destination
allocation, covering all eight roots independently of remaining child cleanup.
The [departing callback order](player-room-ordering.md) precedes lookup; entering
adds no second effect or resource update.

Private original-instruction comparisons cover the following scopes. Debug and
ASan traces match byte for byte.

| Owner | Compared scope |
| --- | --- |
| Ordinary hand helper | 5,440 complete calls: all17 items, both hands, five normal states, recovery/ledge, active39, allocation and curse boundaries |
| Consumption | 27,200 complete ReduceWeapon calls: spare counts0..99, both hands, zero/one/two Duplicators and other-hand retention |
| Equipment selection | 240 complete calls, including owned Life Apple/Hammer error, unchanged counts and suppressed stat preview |
| Medicine schedule | 360 contexts / 38,844 callbacks, including actual Give, protection, ailment-duration and equipment-count helpers |
| Circle | 176 contexts / 14,234 callbacks / 454,080 complete GT4s |
| Labels | 192 contexts / 24,708 callbacks / 46,856 complete GT4s and sixteen tiles |
| Blink | 112 normal-image contexts / 8,368 complete callbacks; no Bat-image claim |
| Room cleanup | 8,304 complete calls across real root flags/slots and circle/Pink lifecycle states |

Allocation, destruction, audio, reservation/consumption and menu/stat presentation
retain explicit service adapters where applicable. These comparisons alone do not
complete native resources, leases or menus. Profiles, artwork and raw traces stay
private.

Native checks separately exercise all17 equippable medicines in both hands, real
scheduled children/rows/rendering, one-use inventory behavior,4095 buff birth,
cures, deferred healing and fresh MP binding. Further protocols cover Ankh versus
Bloodstone, dead pending healing/MP, actual stone Hammer escape with frozen/menu/
session holds, departing callback/room cleanup, Healing Mail overwrite and late
row-commit rejection. Effect rollback restores its runtime and character snapshot
after the successful player's cost; it is not whole-frame HP/map/clock/audio
atomicity or a reversal of already committed consumption.

Both actual CMake-app builds select all17 medicines through ordinary Start/confirm/
directional input, reject Life Apple/Hammer without changing copies and then use
the menu-selected Potion. These checks grant bag stock and protection, with no
direct equip call.

Further actual-app protocols cover ordinary owned stat buff, enhanced Potion,
Uncurse and Manna Prism, real rendered contributions, complete runtime/all16-word/
game-clock menu holds, repeated draws and actual retirement. Equipment, HP/MP,
protection, ailment, clock, factory and position contexts retain their individual
evidence boundaries. They are not ordinary acquisition, uninterrupted castle
routes or console comparisons. Full child cleanup, GPU/audio timing, whole-native
rollback, remaining equipment/familiars and all six
[Alucard milestone gates](alucard-status.md) remain open.

Provenance: `dra/6E42C.c`, `dra/menu.c:func_800FB23C`,
`dra/7E4BC.c:EntityPlayerPinkEffect/EntityTransparentWhiteCircle`,
`dra/7879C.c:EntityGuardText/EntityPlayerBlinkWhite`, `dra/5D5BC.c`,
`dra/692E8.c:EntityAlucard`, `dra/71830.c:PlayerStepStone` and
`dra/5087C.c:RunMainEngine`, pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
