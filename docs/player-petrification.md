# Selected player petrification

Queued stone contacts now run the original selected entry, landing, ordinary/rare
statue, directional/contact wiggle, escape and stone-death paths through actual
shared effects. CON and the player RNG select the escape count. Holding a direction
does not repeat taps. Attacks, shields, pause and save-room activation remain
unavailable while stoned. Original entry clears actual poison/curse/hit words0/1/2;
Axe-converted stone remains a separate hurt path. Ordinary savev9 fields and default
resources/grants remain unchanged.

Entry owns palette8161, original recoil and text3. The actual heavy-landing helper
runs before stone landing: sound647 and smoke blueprint0 precede shake1, sound644,
dissolve39 and variant smoke. Ordinary statues use38/39 and smoke3; rare3A uses
smoke7 and retains protection4 through every handler branch. Airborne statue gravity
has its own protection4 and no landing-helper ceiling clamp. Actual palette words
and original SDK rotation/pivots drive rendering, including captured resident art.

A directional tap or accepted contact impact forces all four directional tap bits
before the shared dissolve callback. Remaining wiggles create actual ordinary
particle3/smoke3 or rare particle8/smoke7 factories. The private signed rotation
cursor owns the wiggle; its draw flag is separate from the retained angle word.
Escape selects3B before testing the live animation, so both variants follow the
original ordinary smoke3 branch, pain sound and hurt6 recovery. Three bounded
sound requests preserve landing644, wiggle608 and escape6E7 when they coincide.
Factory sources retain pre-collision fractions and actual call order.

The [shared support owners](stone-effect-support.md) and
[dissolve owner](stone-dissolve.md) fulfil actual factories, child slots, primitive
leases, image services and drawing. Landing transfers its preceding smoke into
the existing ordered reaction list; no second entity/primitive pool is introduced.
Allocation failure preserves each owner's original optional/retry behavior and RNG
boundary. Menus, freeze/release and draws retain their documented clocks.

New stone damage subtracts twice physical defense and may petrify with zero loss.
Already-stone contact uses one eighth maximum HP. A lethal new stone hit lands,
waits32 grounded updates and enters death16. An already-stone lethal contact queries
actual generation-owned entity32 slots16..63, including phase0 children; pending
factories do not qualify. With a child it enters stone death; without one it
delegates to the selected physical C0 owner. That delegation does not complete the
remaining physical capture, elemental deaths or global-fade branches.

Stone death waits80 updates, writes five actual Up taps and restores the original
five-wiggle word. At the break transition it selects hidden palette810D, animation3E
and smoke3. The original numeric draw flag4 is entity rotation in this context.
The actual break animation has a zero sprite: dissolve first performs its secondary
copy, then retires before packet motion rather than fabricating visible shards or
48 decay uploads. Actual smoke supplies the visible break effect, followed by real
game over. The surviving escape path retains its actual textured slices and decay.

Authored tests cover ordered landing factories/sounds, ordinary/rare particles,
automatic impact taps, simultaneous final-wiggle sounds, airborne protection,
signed private rotation, actual child versus pending-factory death choice, failed
query rollback and five death taps. Existing tests retain zero-loss stone numbers,
contact/healing priority, held input, corrupt-animation rollback and no revival of
actual dead states. Source comparison matches12,000 original stone/stone-death
handler contexts, including107 escapes,3,984 supplied rare-animation contexts and
703 forced-tap outputs. Another720 original Kill0 comparisons exercise all48
slots and phases0..4; the no-child comparison stops at physical-reset delegation.
RNG/factory/audio/shake services have explicit adapters. Debug/ASan handler traces
match byte for byte. Another560 original PlayAnimation updates match complete
cursors/frames/hitboxes for38..3E, including80 zero-sprite break frames, without
helper adapters. All three Debug/ASan trace pairs match; private data and raw traces
are not published.

Muted native Debug/ASan checks start with70HP/20MP and supply raw stone contacts,
without factory39 requests or health/MP/clock grants. Actual entry text, landing
order,36-row resident capture, wiggle factories, escape, retirement and drawing run.
Ordinary escape contributes746 secondary pixels and266 text pixels. A separate
explicit rare-selection RNG fixture contributes2,990 secondary pixels, with actual
rare particle/smoke requests and normal escape smoke. New lethal and repeated
already-stone contacts reach game over after five Up taps; break smoke contributes
113/83 pixels. A scoped cleanup fixture releases the actual dissolve rows before
repeated contacts and verifies physical C0 delegation; it is not a room transfer.
Natural enemy contacts may damage the player after escape.

Both actual CMake apps independently pass the surviving path, including746
secondary pixels, repeated-draw holds, actual inventory open/hold/close, Summon
Spirit's33 frozen updates plus release, retirement and normal SDL quit. Separate
actual-app lethal checks exercise entry/capture/death taps, actual secondary copy,
zero-frame row retirement, hidden palette, smoke, real game over and normal quit.
Both lethal apps measure28 break-smoke pixels after drawing the current frame.
Raw-contact/protection/RNG/cleanup fixtures remain distinct from ordinary Medusa
producers, acquisition, uninterrupted routes and console evidence. No user saves
are written by these protocols.

All210 tests pass in Debug/Release/macOS ASan-UBSan, with Python/policy/formatting,
static SDL linkage and ZIP packaging; exact-head platform checks accompany the PR.
Remaining stone grab/thrown contexts, familiar Hammer's actual producer, general
room cleanup, complete damage/death/fade, whole native failure atomicity and shared
RNG/timing/contention remain open. All six [Alucard gates](alucard-status.md) remain
open at31/34 dispatched states. Passing selected handlers does not complete Alucard.

Provenance: `dra/71830.c:PlayerStepStoned`/`PlayerStepKill`, `6FDF8.c` landing,
`5D5BC.c` damage/ailment duration, `alu_anim.c`, actual shared effects and the
`692E8.c` frame at pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
