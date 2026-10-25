# Original opening equipment

Fresh first-visit Entrance checkpoints now equip Alucard sword, Alucard shield,
Dragon helm, Alucard mail, Twilight cloak, and Necklace of J. Explicit starts
after Death's room use the corresponding empty equipment slots and completed
Death flag. Continue and connected arrivals retain their saved equipment.
Laboratory's direct development entry remains explicit. Resources/base stats
still use the documented development profile until prologue initialization is
integrated; this is not the original new-game flow.

Ordinary Alucard sword attacks use its own private overlay, frame streams,
hitboxes, sound, artwork, and palette. The ordinary shield has an independent
raise/hold/movement/lower cursor. It follows walking/crouching/jumping and can
coexist with the other hand's attack. Its private hitboxes block susceptible
projectiles before player contact; ordinary shield contact deals no enemy damage.
Dragon helm halves enemy defense during player damage resolution. Mail/necklace
attributes use the existing original definition reader. Twilight cloak applies
its partial palette copy and advances its two-color glow once per simulation tick.

Death removes the equipped original items, including the correct head/armor
slots, and immediately recalculates attributes/artwork. An equipped copy takes
precedence over inventory stock; unrelated gear and duplicate stock survive.
Earlier checks granted items only to inventory and missed reversed head/armor
IDs. Public regression coverage now checks equipped theft as well.

Alucard sword's command attack/sparkles, shield-rod spells, knocked-away shield
presentation, full elemental/status behavior, and exact GPU blending remain
incomplete. No substitute special attacks are exposed. Opening relics and the
original character's prologue-derived resources remain separate progression work.

Public synthetic tests cover shield cursor transitions, held movement frames,
malformed pointers, projectile blocking in both orientations, zero kill-count
side effects, supported equipment bounds, equipped theft, and palette glow timing.
A private native LLDB check exercises the actual sword/shield while moving,
completes all five Death speech clips, and checks all six equipped items disappear.
Private checks also run with macOS ASan/UBSan and dummy audio; they do not establish
full timing/audiovisual parity or real-device audio listening.

Adapted AGPL-3.0-or-later algorithms: `src/weapon/w_043.c`, `w_052.c`,
`src/dra/6E42C.c`, `71830.c`, `5D5BC.c:DealDamage`, `5F60C.c:InitStatsAndGear`,
`4A538.c:PAL_GLOW_ANIM`, and `src/st/collision.h` at reconstructed revision
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`. Runtime arrays, palettes, frames,
sprites, and sounds stay in verified private inputs.
