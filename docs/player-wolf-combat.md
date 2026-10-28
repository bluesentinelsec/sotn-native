# Selected Wolf attack ownership

The shared ability owner now initializes actual front-part rush and head
charge/bite attacks from private spell definitions thirteen, four and fourteen.
It consumes requests in original front-before-head order, with one RNG value per
actual initialization, original INT/Mojo Mail scaling and private critical-hit
metadata. The body's current final pose supplies hitbox geometry. Spell lookup
does not depend on MP; the original front-hit cost still commits exactly once
through the shared character transaction. Failed later initialization rolls back
body state, MP and RNG together.

Preserve the original allocation quirk: each spell initialization increments the
least-used group3..6, then Wolf overrides its enemy identity to three. The final
per-tick scan counts actual retained Wolf and selected Bat child identities.
Front/head attacks therefore share group-three enemy cooldown, including after
child replacement, with selected Wing children assigned that same group. Do not
reset enemy cooldown on a new body generation. Gas Cloud and other legacy owners
still need complete global group sharing.

Native enemy combat reads generation-owned Wolf poses, converts their screen
coordinates through the current camera and preserves fractional positions. It
applies selected physical damage/death/EXP and reports actual front contact for
the next body's rush cost/skid decision. Retired bodies expose no active attacks.

Authored tests cover front/head shared cooldown, new front generations, camera
coordinates, enemy death, ordered spell/RNG initialization, once-only hit costs,
transactional failure and the next allocator's retained group counts. A private
muted LLDB session fixture enters Wolf using an explicit Soul of Wolf grant,
steers and bites placed Laboratory zombies, receives two kills and 54 EXP, then
returns and retires the body. It retains ordinary character stats but deliberately
orchestrates components without player damage or rendering; it is not a native
controls playtest, unmodified encounter or console replay. It writes no saves.

Native scene/presentation, live controls/relic switching, water/cross-form paths,
real bubble/controller child factories and complete shared effects remain
incomplete. Ordinary starts grant no relics and saves remain version nine. Do not
call Wolf, Alucard or the game complete.

Provenance: selected `dra/wolfForm.c`, `7879C.c:func_8011A328/func_80118894`
and `5D5BC.c:GetSpellDef`, pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
Private spell/artwork tables, runtime contexts and traces remain ignored.
