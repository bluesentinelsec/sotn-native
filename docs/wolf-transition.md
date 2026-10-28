# Wolf entry and return transitions

The transition component reconstructs selected original entry substates zero/eight
and return substates zero/one/two. It consumes actual factory success, actual Mist
completion and the real effect signal. A pending entry factory leaves the player
unchanged. Successful entry reports original weapon clearing and replacement of
body slots 17..30, saves the old drawing bank/frame, and selects the hidden Wolf
pose. It does not allocate those entities or signal their readiness.

Preserve the sixteen-bit shared signal, the maximum of signed original timer14
and four, all three movement words, source priority, zeroed entry velocity and
one-pixel floor restoration. Mist entry waits for the real child and requests the
original finishing flash. Ordinary entry waits for actual signal three. Neither
path substitutes an elapsed-time threshold for the effect handshake.

Return decelerates before dispatch, holds when floor and ceiling touch together,
and restores ceiling sensors one pixel per tick. Reaching the default on this tick
is distinct from starting the tick at the default. Only after all four sensors are
ready does the original normal bank/CA pose and signal-one handoff occur. A real
signal three reports the normal-return or stuck-return helper boundary; the native
owner must invoke the actual helper and its remaining original tail. A reported
request does not complete a return, set a lockout or retire an actual child.

Public authored tests cover exhaustion, retry, original timer maxima and word wrap,
normal/Bat/Mist entry, floor/ceiling waits, both-contact cramped recovery, signed
deceleration and invalid-state atomicity. Private muted LLDB instruction comparisons
cover 16,384 supplied contexts, 824 pending factory results and 1,301 normal/stuck
helper boundaries. Original animation, sensor, timer, priority and deceleration
instructions execute; factory, weapon/destruction, Mist-ready and final helper
boundaries use explicit adapters. Debug and ASan/UBSan traces match.

This component enables no controls or relic switches. Actual Wolf body generations,
shared primitives, transformation/render/combat ownership and full Alucard/game/
console parity remain incomplete. Ordinary relic ownership and v9 saves are unchanged.
All source images, runtime contexts and traces remain private.

Provenance: selected `dra/wolfForm.c:WolfStepS_0/WolfStepS_8/PlayerStepUnmorphWolf`,
pinned sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
