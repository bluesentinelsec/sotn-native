# Ordinary spell command component

The core now reconstructs the six original normal-form command recognizers for
Dark Metamorphosis, Summon Spirit, Hellfire, Tetra Spirit, Soul Steal and Sword
Brothers. This is an input/resource component. It enables no live spell controls,
allocates no player/effect/combat children, grants no spells or relics, and writes
no saves. [Selected playable Dark Metamorphosis](player-dark-metamorphosis.md) now owns
its command; [selected playable Summon/Tetra](player-spirit-spells.md) now bind
their actual freeze/capture/factory/target/combat owners. Other native spell paths
remain to be integrated.

Call it once at the original special-command point, after earlier Gravity Boots/
weapon commands have had their priority. It preserves the six spells' original
relative order: Hellfire, Tetra, Soul Steal, Sword Brothers, Summon, Dark. A successful
request stops later recognizers for that tick. The owner supplies an availability
bit only when it can fulfill that spell's actual native state/effect/combat path;
zero availability makes no resource requests. The component still tracks inputs.

Pressed/tapped fields retain full US words. Direction-masked, subset and exact
word tests remain distinct. Facing captures follow each original recognizer;
Sword Brothers uses current facing. Signed sixteen-bit counters/timers retain
word wrap, with independent terminal and expiration behavior. Preserve:

- Dark's inclusive backward start and exact full-word Up step.
- Summon's exact backward start and original neutral intermediate acceptance.
- Hellfire's 21-tick Down transition, cramped cancellation and zero-timer branch.
- Tetra's 32-tick hold, release/precheck shortcut and expiration-frame cast.
- Soul Steal's original right-facing 24/left-facing 20 start-window asymmetry.
- Sword Brothers' actual familiar gate, 64-tick Up hold, completion0xFF and
  delayed familiar notification. Resetting an inactive familiar still runs the
  original phase-zero Down test; it never permits the sequence to complete.

Private spell definitions supply MP costs. Exact-cost casts succeed; insufficient
MP preserves resources and the original command retry window. A successful request
debits only the copied frame and reports its cost/identity. The caller must commit
MP and learned-spell progress exactly once with its real player/child transaction.
No request represents effect readiness or successful allocation. A later spell
cast preserves an earlier Sword Brothers completion notification on the same tick.
Malformed state or required private data preserves commands, frame and outputs.

Public authored replays complete all six commands in both facings, including
exact MP, holds, terminal notifications, timing boundaries, action locks, word
comparisons, neutral acceptance, cramped space, signed wrap, priority and failure
atomicity. Private comparisons match 54,432 original command instruction contexts,
including actual original CastSpell MP debits. Perform/Learn/diagnostic calls use
explicit boundary adapters; their resulting player states, learned progress and
children are not established by these comparisons. Debug and ASan traces match.
Private tables/traces remain ignored. This does not establish live spell, full
Alucard, whole-route or console parity.

Further integration must supply actual spell controllers, shared factories/children,
primitives, artwork/palettes, combat/healing, learning and their lifetimes before
making another command available in gameplay. Selected Dark now supplies those
owners; selected Summon/Tetra now supply theirs too. Other availability bits stay disabled. Existing saves and ordinary starts
remain unchanged.

Provenance: selected `dra/704D0.c:{CheckDarkMetamorphosisInput,CheckSummonSpiritInput,
CheckHellfireInput,CheckTetraSpiritInput,CheckSoulStealInput,CheckSwordBrothersInput,
CheckSpecialMoveInputs}` and `5D5BC.c:CastSpell`, pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
