# Selected Gurthang

The actual shared [ordinary hand owner](shared-ordinary-hands.md) now accepts Gurthang121
through its own weapon54 program, together with its QCF [weapon special](player-weapon-specials.md)
195 (descriptor1, kind1). Muramasa shares the same attack callback but also spawns a secondary
once its attack reaches 30. Its attack adds the square root of a persistent blood counter
(`g_Status.D_80097C40`) that ordinary v9 saves do not store, so it stays unavailable pending a
save-format decision.

## Callback ownership

Gurthang runs the sword template with additive blending plus twelve optional LINE_G2 trail
rows. The rows are reserved from the real shared stage pool before properties; each row
consumes its original birth rand() calls (four rows take two for the length, all take one
for the shared green/blue level), so a successful birth uses 28 row calls before the
property roll. Exhaustion omits only the rows and their calls. On the last frame of each
attack (0xD, 0x22, 0x2C, 0x3E) every row is placed from the private offsets in screen
space, its accumulated delta and growing length update, and it becomes visible. Draws
render the opaque lines with the current shake once; room rebases shift both endpoints.

While the player's Dark Metamorphosis timer (word11) runs, the callback selects descriptor2.
For descriptors1 and 2 the property attack doubles under an active timer, and a timer below
0x38 is raised to 0x38; descriptor1 also requests the original FACTORY(61,0x11) outline from
the weapon's position. The runtime reports the timer floor, and the session raises the actual
player timer only after the player update; the Dark spell's glow and palette owners are
untouched. The outline uses the existing profile-0x11 outline owner.

## QCF specials

Hand use now resolves a completed grounded QCF command for any equipped specialMove whose
chain limit is 0x80, before the back-forward check, as `dra/6E42C.c` block_32 does. The MP
cost is charged after the actual factory succeeds; an unaffordable special falls back to the
back-forward and ordinary checks. The QCF recognizer now runs whenever ordinary hand
ownership is bound, not only for the Alucard sword.

## Evidence

Public tests cover Gurthang's optional rows and exact birth RNG, Dark descriptor2 and attack
doubling, timer-floor/outline events, trail placement on a trigger frame, grounded versus
airborne QCF selection with the casting grunt, and the bounded timer floor. Mutations of the
doubling and the grounded check are caught. The private original-instruction comparison runs
56,848 complete callbacks in 2,240 contexts (descriptors0/1, five Dark timer values, row
exhaustion, both hands and facings, nonzero cameras), including 487,008 line-row comparisons
of which 273,024 are visible placements; Debug and ASan traces are byte-identical. The
compiled palette loader matches the original copy. Native Debug/ASan protocols exercise the
ordinary attack, the real QCF special with the timer floor and real outline child, the
Dark-active doubled attack, visible trail pixels and real enemy damage in both hands.
Supplied stock, protection, position and timer contexts are not acquisition, route or
console evidence.

Adapted AGPL-3.0-or-later algorithms: `weapon/w_054.c`, `weapon/shared.h` and `dra/6E42C.c`
at sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`.
