# Selected high-defense guard

Actual queued contacts now select original response1 when the signed damage
before the minimum-one rule is nonpositive, DEF is at least100, and the contact
has neither poison nor curse. Immunity, absorption, stone and ordinary water keep
their earlier priority. Existing poison doubles the signed amount without changing
its sign. Environmental kinds use their original HP-max fraction, so their rare
zero fraction retains the same guard test. The surviving result loses exactly one
HP and keeps normal control, momentum and animation instead of starting recoil.
One-HP results delegate to the existing selected death owner, with remaining
interrupted/elemental death contexts still incomplete.

The result preserves existing ailments, copies original effect bits, requests
voice6E7 and a red type0 number1, sets protection12 and starts timer3 at six. The
palette uses the actual original `high_jump_timer` word, already owned by Gravity
Boots; no parallel palette timer or substitute word is added. Birth does not
immediately apply the palette because the common timer pass already ran.
The guard voice is a separate one-update request, queued before later control
sounds. A same-frame backdash or spell cannot overwrite it; repeated identical
sound IDs also retain their distinct requests. Original
high-jump entry still resets its shared word at its own boundary.

Timer3 runs after timer2, before timer4 and the later protection/afterimage timers.
It applies the retained palette word and refreshes timer15 to twelve before its
actual common-owner decrement to eleven. On timer3 expiry, the palette restores
to8100. The prior-frame stone/poison/curse status decides whether expiry also
requests timer4/protection/timer15 at twelve. Newly requested timer4 runs and
decrements in that same common pass; protection/timer15 likewise become eleven.
Actual gameplay-clock SDK colors and timer4's final color-blend clear retain their
existing owner. Capture the prior status before ailment decrements; recomputing it
after an ailment expires changes this original boundary.

## Resources and shared ownership

Deferred healing still commits first and may coexist with a guard's independent
red number. Surviving guard damage retains Blood Cloak's pre-add full-heart check
and one-heart request, and Fury Plate's original minimum512 refresh, including a
guard while an earlier Hit state remains. The HUD's actual counters still own
512-to511 birth and derived DEF; no cooldown replaces them. Guard acceptance and
feedback consume no new RNG and do not cancel hands/forms or clear unrelated
resources. The captured number/heart origins retain pre-controller fractions.

The existing entity19 owner now represents original type0 as well as green type1.
Nonzero damage numbers use red palette/trails; zero immunity numbers remain blue.
[Queued hurt/lethal/stone numbers](player-contact-numbers.md) now join guard and
immunity as selected live type0 producers. Secondary impact/grab and MP-number
producers remain unfinished. First-free slots56..63, twenty
real shared rows, original digit/motion/SDK math, trail ordering and retirement
remain shared. Factory/child/row exhaustion omits only the optional visual.

Owned God's Garb24 and Nauglamir87 now use their original additive private stats
through normal equipment selection. They introduce no new grants or save fields.
Their availability does not establish acquisition or remaining item effects.
Existing global Fury buffs survive supported armor selection; the new guard never
invents a buff, recalculates from a copied defense override or grants resources.

Failed player updates retain HP, contacts, RNG, counters and prior requests. Shared
updates retain their copied runtime/row transaction on failure. Room rebases add no
timer update. Menus/maps/draws hold player and effect owners; initialized numbers
advance on actual frozen effect frames while player timers/protection hold. The
separate release-only frame stays held. New/Continue restoration clears transient
player feedback using the existing initialization boundaries; saves remain v9.

## Evidence and remaining work

Authored public regressions cover thresholds99/100, attack equal/above DEF,
existing poison, immunity/absorption priority, one-HP death delegation, prior
healing, retained fractions, actual guard timer/SDK work, prior-status expiry,
Blood Cloak/Fury/full hearts, failed-player and missing-SDK rollback, same-frame
backdash voice retention, high-only water replacement, coexistence of green/red
numbers, actual shared leases, freeze, optional exhaustion and row retirement.
Synthetic owned-item records establish selection without copying private data.

Private original comparisons match11,200 contact contexts including1,580 guard
results;3,072 timer2/3/4/14/15 contexts with shared words/prior ailments; and672
red-number contexts/73,248 controller/digit/trail updates. Explicit recalculation,
equipment-count/HUD/AddHearts/factory/number/sound and afterimage-service adapters
isolate these algorithms. These are not complete original frames or console replay.

Muted native checks use actual Laboratory zombie contacts. Owned Fury gear and a
CON99 fixture derive DEF69; the first real hit activates the real Fury512-to511
owner and DEF89. A real menu selection replaces Fury with God's Garb, retaining
the actual buff and deriving DEF102 from private God's Garb/Nauglamir/equipment
metadata. The next queued contact loses one HP without recoil or RNG, emits the
original voice and leases a real red twenty-row number contributing five pixels.
An observation-only native audio hook also verifies the actual ordered
voice6E7/backdash617 pair before playback. Native palette/protection/afterimage
expiry values, menu/repeated-draw holds,
actual spirit freeze/release and number retirement are checked. Explicit CON,
equipment grants/selections and post-hit protection fixtures are not ordinary
acquisition, uninterrupted routes or console evidence. No HP/MP, queued contacts,
stat masks, clocks or user saves are granted/written by this check.

Final native/app/platform results are recorded in the feature PR.
[Selected Talisman dodge](player-contact-talisman.md) now shares the actual recovery
timer owner. Remaining hurt/kind promotion, elemental/interrupt deaths, stone variants and
other effects retain the [damage inventory](player-contact-absorption.md).
Milestone1 remains open at26/34 states; full damage/equipment/sharing and Alucard
remain incomplete.

Provenance: `dra/5D5BC.c:HandleDamage`, `692E8.c` response1 and timer loop,
`6DF70.c:func_8010E168`, `7879C.c:EntityNumberMovesToHpMeter` and the existing
actual HUD/stat/afterimage owners, pinned AGPL-3.0-or-later `sotn-decomp`
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`. Artwork and tables remain private.
