# Olrox articulated geometry components

Selected original helpers now convert the owned monster joint state to four
textured packets per leg and two torso packets, with the original palette and
priority roles. The torso also positions its generation-owned head/attachment
roles. A separate helper copies a single-part bank-seven sprite to an owned
packet while preserving the caller's visibility, texture page and blend state.
These components do not register or advance the monster actor. The playable
transformation, body, attacks, defeat/reward and connected travel remain incomplete.

Joint state stays separate from primitive packets. Geometry uses the original
SDK lookup inputs and signed word coordinates, including the distinction between
subtracting a rounded product and rounding a negative product. Feet retain their
original circular floor projection, toe threshold, fixed transformation scroll
origin and half-angle division. Four private torso profiles determine attachment
distances, angles and texture windows; no profile or artwork is public.

The original sprite helper deliberately ignores part cropping, selects bank seven
and uses 127-unit texture offsets. It reads the selected part's palette rather
than the actor palette. Do not replace these rules with the ordinary sprite path.
Owned child references and primitive ranges must validate before presentation;
reused child generations fail explicitly.

## Validation and scope

Public authored tests cover torso corners and attachment fractions, facing,
leg/foot projection, palette/priority roles, preserved packet attributes, the
sprite helper's crop/offset rules, malformed ranges, missing private input and
reused owners. Fixtures contain independent synthetic tables and artwork metadata.
Private Debug and macOS sanitizer comparisons match 7,680 original-instruction
snapshots across the leg, torso and sprite helpers, both facings, camera origins,
all four private torso profiles, transformation scroll and floor projection.
The original SDK and helper instructions execute; a leading-zero instruction
adapter supplies the PS1 GTE operation used by square root. Seeded joint/body
fixtures supply component inputs. These are not actor integration, ordinary
encounter traversal, framebuffer/console parity or a full-game claim.

Provenance: reconstructed BO0 `313A8.c` routines `func_us_801B13A8`,
`func_us_801B2690` and `func_us_801B30AC` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
Private profiles, SDK tables and artwork remain user-supplied runtime data.
Authoring disassembly, generated analysis and private traces are not shipped.
