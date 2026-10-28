# Owned Wolf scene presentation

The native scene renderer now accepts owned Wolf player/body snapshots and places
their original bank-fifteen packets in the foreground ordering table. It retains
SDK pivots, cropping, raw-color behavior, capture copies and reverse emission
order at equal priority. Original primitive images draw before entity packets at
equal priority, matching the original RenderEntities/RenderPrimitives insertion
order. The player can still emit its bank-fifteen packet after body retirement
until the actual return switches back to ordinary artwork.

The common environment begins at priority0x190 and restores at0x1B0. It draws into
the owned 63x63 target at512,448 with the original background-clear bit, through
the same target cursor as stage commands. Current capture duplicates draw inside
that interval; earlier retained images and flashes sample the previous capture.
The outer row/column remain untouched. No capture is performed before its ordered
world consumers.

Before a simulation frame's first draw, the renderer retains the previous target
pixels. Repeated or paused draws restore that same input before rerunning the
ordered capture, so they change neither the displayed result nor the next capture.
Pending Wolf environment writes join stage target writes in the pre-update flush:
skipped presentations cannot discard a simulation frame's capture. The temporary
snapshot belongs to the session, remains outside ordinary saves and retires at
full stage initialization.

Owned puffs and nonhidden particle tiles now join the same scene draw list. Tile
indices and actual child/row metadata are validated before access; sentinel rows
are never drawn. The shared update owner applies original offscreen retirement
after initialization, preserving consumed RNG while releasing actual particle
reservations. Drawing advances no behavior or animation.

Authored tests cover draw-command bounds and post-initialization offscreen
retirement. A private muted LLDB fixture runs this repository's real player,
factories, stage pool, placed enemies and scene renderer through transformation,
movement, two zombie defeats/54 EXP, return and retirement. It grants only Soul
of Wolf to ordinary stats and deliberately omits player damage. A separate seeded
prior-capture/rush-image visual fixture proves that sampling the current image
changes the screen; repeated draws using the retained prior image preserve both
screen and target. Entry/active/return capture checks write no saves. These are
controlled session checks, not an unmodified encounter, native-input playtest or
console replay.

The [selected native session](player-wolf.md) now binds Wolf contexts/body feedback,
owned controls and relic switches. Original complete shared GPU packet
capacity, all primitive/stage ties, cross-form/water/shake integration and full
raster/audio parity remain incomplete. Selected Wolf packets retain the bounded
768-packet component input; this does not prove global original GPU contention.
Ordinary starts grant no relics; saves remain version nine. Do not call Wolf,
Alucard or the game complete.

Provenance: selected `dra/{42398,4B758,4DA70,692E8,wolfForm}.c` presentation
ordering and previously reconstructed Wolf components at pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
Original artwork, runtime snapshots, traces and captures remain private inputs.
