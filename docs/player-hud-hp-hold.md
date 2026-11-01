# Selected HUD health-display delay and secondary impacts

Actual player healing and selected damage now notify the existing HUD owner of
original `func_800FE8F0`. Its signed DRA word starts64 only while zero. Each actual
HUD update decrements a positive word while holding displayed HP; interpolation
resumes on the65th update. A later hit or applied heal does not refresh a positive
word. The existing displayed-HP digits and palette selection consume the held
value; actual HP, death checks and gameplay resources change immediately.

Applied deferred blood/orb/Mail healing notifies even at full HP. Absorption and
Cat-eye notify only when HP was below maximum. Surviving ordinary damage, guard,
stone and non-dodged held-grab loss notify. Ordinary immune/dodged/lethal results
and ordinary water response9 do not notify. An earlier applied heal retains its
request across a later excluded damage result. The kind5 secondary-impact helper
explicitly notifies even after its own dodge or lethal result, as in the source.
Requests commit after successful player work and before session handoff. Freeze
and release clear one-shot requests without replaying them.

Actual save-room healing overwrites the same word with16, even while positive;
its same-frame HUD update leaves15. The counter continues on ordinary, frozen
and release HUD updates. Inventory, map, repeated drawing and game-over hold it.
Room entry retains the HUD without an extra update. Fresh stage/HUD and Continue
initialization reset the word and initialize displayed HP from actual HP. There
is no second timer in the player, effect pool, cooldown, default grant or savev9
field. The player snapshot carries only the transient notification intent.

Held-grab and secondary-impact kind-two HP work share the existing player damage
owner. Secondary impact interprets its retained argument as a signed16 word before
dividing by two, subtracts defense, and uses the prior poison status even on its
expiry update. Ordinary queued normal/environmental damage uses that same prior
status. Original poison-expiry protection still gates queued contacts; an active
curse suppresses that protection in the explicit accepted-expiry test.

A secondary Talisman dodge retains the pre-minimum signed number word, including
zero or a wrapped negative value. Non-dodged nonpositive damage becomes one and
returns guard1 at defense100 or greater. These internal results always request
the existing type0 number without ordinary-contact guard/dodge effects. Captured
secondary provenance is exclusive with boss-grab provenance; the shared runtime
requires an actual continuing hit state or lethal dispatch before accepting its
zero/full-word result. Wall sound644 precedes the surviving secondary pain voice;
lethal dispatch consumes no extra pain RNG. Surviving loss retains actual Blood
Cloak/Fury owners and their full-heart, duration and commit boundaries.

Authored tests cover idle64/not-refreshing, save16 overwrite, hold/interpolation
boundaries, draw/invalid-status holds, fresh initialization, poison expiry,
full-HP healing/absorption, signed secondary dodge words and provenance rejection.
Private original-instruction comparisons match16,848 grab/secondary contexts:
9,360 dodges,1,824 guards,3,600 lethal dispatches,5,616 prior-poison expiry contexts
and11,856 actual HUD notifications. They execute full `HandleDamage`, actual
`func_800FE8F0` and `AddHearts`; recalculation/equipment, BIOS RNG, factory, number,
audio and physical-death services have explicit adapters. Another650 HUD contexts
match93,600 observations, including92,560 actual interpolation updates,2,730
original notification calls and1,040 held updates. Its260 supplied save16
assignments are a boundary adapter, separate from the actual native save UI.
Private tables, artwork and raw traces remain private.

The native HUD protocol starts with default70HP/20MP, accepts a supplied raw20
contact, holds display70 for64 updates after HP becomes50, then interpolates on
update65. Applied blood8 does not refresh the counter. Applied Mail1 supplies a
new hold; actual inventory/map/draw hold it, while actual Summon Spirit's33 frozen
updates and release update decrement it without replay. A separate selected
save-room23 context accepts raw20 damage, applies Mail1 to51HP, then actual Up
activation heals to70HP and overwrites the positive delay with16; its next
interpolation occurs after16 held updates. The UI is cancelled without saving.

Muted Debug and macOS ASan/UBSan protocols pass for the HUD sequence and actual
kind5 wall collision. The latter produces a real secondary number/twenty rows,
ordered impact/pain sounds and HUD counter62 after the second damage frame.
Default raw20 contact plus secondary10 leaves40HP. Actual owned Cloak54/Fury22
with supplied raw40 contact leaves46HP and34 hearts; its secondary loss1 uses the
real resource owners and Fury512-to511. Both actual CMake apps separately verify
70-to50HP, display70 through64 updates, display68 on65, repeated-draw holds,
settled50 and normal SDL quit. Debug/ASan comparison traces are byte-identical.
All208 tests pass in Debug/Release/macOS ASan-UBSan;34 Python tests, formatting,
repository policy, static SDL linkage and ZIP packaging accompany the batch.
Exact-head platform checks are recorded with the PR.

Supplied raw contacts, retained status/HP/gear, protection, arrival and deferred
healing fixtures do not establish ordinary damage producers, acquisition,
uninterrupted encounters/routes, complete physical death or console parity.
Remaining HUD, damage, equipment/familiar, general sharing and all six
[Alucard milestone gates](alucard-status.md) remain open at31/34 dispatched states.

Provenance: `dra/5D5BC.c:func_800FE8F0`/`HandleDamage`,
`5F60C.c:DrawHud`, `62DEC.c` save healing, `692E8.c` deferred healing and
`71830.c:func_80113D7C`, pinned sotn-decomp commit
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
