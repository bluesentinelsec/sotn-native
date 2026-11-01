# Selected held-grab damage feedback

Normal held boss grabs now consume their original kind-two damage result and
always request the original type0 HP number before the lethal branch. Each number
uses the existing entity19/slots56..63/twenty-row owner, captured pre-stage-alignment
fractions and original unsigned-word/four-digit presentation. A zero dodge result
uses the existing blue zero; positive and wrapped negative words use red digits.
Number exhaustion omits only the optional visual.

The pulse subtracts actual defense and applies the prior poison status, including
the final poison-expiry frame. The full requested amount remains available for
lethal overkill instead of the old9999 cap. An owned Talisman consumes one player
RNG roll for either one or two accessories, with the original inclusive comparison.
A successful roll retains HP and the pre-minimum signed number word. Nonpositive
ordinary damage becomes one; high defense returns guard1. Guard and dodge keep
the grab held and still request its number and ordinary grab palette4/sound6E7.
They do not dispatch ordinary-contact guard/dodge visual owners.

Surviving actual HP loss now requests Blood Cloak hearts and Fury Plate's minimum512
through their existing owners. Pre-add full-heart suppression, uncapped requested
hearts, longer Fury buffs,512-to511 HUD birth and commit after player success retain
their original boundaries. Dodges and lethal pulses request neither resource.
Earlier deferred healing still runs first and retains its independent number.
No pool, equipment/default grant, clock or ordinary-savev9 field is added.

The subsequent [HUD/secondary-impact owner](player-hud-hp-hold.md) connects
surviving non-dodged grab notifications to the actual64-if-zero displayed-HP
hold and shares kind-two calculations with secondary impacts.

An explicit captured boss-grab provenance bit distinguishes these number contexts
from contact or secondary-impact results. Shared validation requires the real held
state for surviving guard/dodge/hurt results, or the actual lethal dispatch. A
forged ordinary-contact response cannot gain the grab's zero/full-word allowance.
Failed animation/control updates retain HP, RNG, pending grab and all feedback.
Room rebases retain the captured source without ticking. Existing number freeze/
menu/draw/release lifetimes and actual row retirement remain with the common owner.

Authored tests cover surviving, guard, zero/negative dodge, lethal overkill,
poison expiry, deferred healing, Cloak full suppression, Fury requests, captured
fractions, provenance rejection and failed-update rollback. Private comparisons
match6,480 contexts through actual `PlayerStepBossGrab`, full `HandleDamage` and
actual `AddHearts` instructions:3,600 dodges,672 guards,1,392 lethal dispatches and
2,160 poison-expiry contexts. Number/factory/audio, recalculation/equipment/HUD,
BIOS RNG and physical-death boundaries have explicit service adapters. Debug and
ASan traces are byte-identical. Tables, artwork and raw traces remain private.

Muted Debug/ASan native checks use the actual placed Hippogryph's START/DAMAGE/NONE
callbacks with explicit attack18 positioning and a supplied held-flight substep3/
timer64/retained velocity. Four real pulses keep default70HP/20MP starts: the normal
context reaches6HP with330 number pixels; owned Cloak/Fury reaches66HP/14 hearts,
refreshes Fury512-to511 and contributes150 pixels. Two owned Talismans produce one
HP-preserving dodge among four displayed numbers, reach22HP and contribute330 pixels.
Each number owns20 actual rows. Pre-pin fractions, actual player RNG, repeated-draw
holds, natural retirement/row release and no replay pass. The phase/velocity fixture
establishes no ordinary attack selection or uninterrupted encounter.

Both actual CMake apps separately exercise four supplied grab pulses,70-to6HP,
20-row numbers/330 pixels, clear-to-drop recovery, retirement and normal SDL quit.
Their supplied grab/protection requests are distinct from the native boss-producer
checks. All208 tests pass in Debug/Release/macOS ASan-UBSan, with final focused
provenance checks;34 Python tests, policy906, formatting, static SDL linkage and
ZIP packaging pass. Exact-head platform results are recorded with the PR.

Supplied gear/protection/position/attack/phase/timer/velocity or grab-request
fixtures are separate from ordinary attack
selection, acquisition, uninterrupted encounters/routes and console evidence.
Thrown release, stone-grab reactions, other boss producers, remaining death/fade,
general shared timing/contention and [milestone1](alucard-status.md) remain open.

Provenance: `dra/71830.c:PlayerStepBossGrab`, `5D5BC.c:HandleDamage`/`AddHearts`,
`7879C.c:CreateHPNumMove`/`EntityNumberMovesToHpMeter` and actual BO5 grab callbacks,
sotn-decomp commit `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`.
