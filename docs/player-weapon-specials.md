# Weapon special inputs

Original hand use (`dra/6E42C.c`) can replace the equipped item with a virtual special
item. A completed quarter-circle-forward command selects the item's `specialMove`
(definition byte24) when grounded and affordable; otherwise a completed back-forward
command selects its `unk17` (byte23); otherwise the ordinary item attacks. Virtual items
have their own definitions, blueprints, player animation kind, lock and MP cost.

## Native ownership

The native player now runs the original back-forward recognizer (`CheckBackForwardInput`,
`dra/704D0.c`) once per update after the existing QCF recognizer, whenever actual hand
ownership is bound. Back then forward, relative to the facing captured when the command
started, opens a fifteen-update window; exact directions only; restoration cramping
clears a completed command. The command is transient and never saved.

Hand use resolves a completed back-forward command after the curse and Alucard-sword
QCF paths and before the actual factory reservation. Chain limit 0x80 allows the
special whenever no attack is active. The virtual item reserves its own blueprint
parameter through the shared slot16 owner and drives the generic attack animation,
substep, attack mode and lock. Kinds 8, 9, 10 and 25 enter the original `block_98`
directly and therefore skip the casting grunt.

Selected virtual items:

- Basilard18's back-forward special174: kind8, the actual weapon31 program's descriptor1,
  animation0x76, lock8, attack mode0x8009. Previously the same input produced an ordinary
  Basilard swing.
- Holy rod130's special170 and Star flail131's special171: kind4 (keeps the casting grunt),
  each program's descriptor1, animation0x56, lock13, attack mode0x8005.
- Gurthang121's QCF special195: kind1, weapon54 descriptor1; it raises Dark Metamorphosis
  and requests an outline (see [Gurthang](player-gurthang.md)). QCF resolution runs before
  the back-forward check, requires the ground, chain limit0x80 and enough MP.
- Combat knife20's special175 uses the original slot16 alternation quirk before the QCF and
  back-forward checks (see [template weapons](player-plain-weapons.md)).
- Holy sword115's QCF special196: kind28 with five MP, a ten-update pose0 hold and SFX_VO_ALU_
  ATTACK_B instead of the casting grunt; weapon55 descriptor1 spawns the first native
  [weapon secondary](player-holy-sword.md).
- Rapier23's special191 and Were bane22's special190: kind26 lunges that hold pose6 for up
  to forty updates with their own spark secondaries (see [lunges](player-lunge-weapons.md)).

- Katana100's QCF special189: kind1, weapon53 descriptor1 with the chained GT4 slash
  secondary (see [Katana](player-katana.md)).

- Thunderbrand112's QCF special178: kind19, a forty-update pose0 hold with lightning and a
  ring (see [Thunderbrand](player-thunderbrand.md)).

- Osafune139's back-forward slash192 and QCF warp193, and Masamune140's QCF warp194: kind135
  sword-warp afterimages with slash roots (see [Osafune/Masamune](player-katana-warp.md)).

- Claymore98/Flamberge101/Zwei hander103/Obsidian sword107 back-forward specials182..185:
  kind23 slot16 specials with a fan of blades, costing five MP only when MP covers it (see
  [weapon51 swords](player-weapon51-specials.md)).

- Iron Fist102's QCF special186 and Fist of Tulkas120's back-forward special187: kind24
  punch rushes in slot16; Tulkas's QCF special188: kind25 descriptor4 fireball (see
  [fist specials](player-fist-specials.md)).

The other virtual specials (remaining brands, Yasutsuna, rods, Shotel,
Sword of Dawn and others) need their own player states and callbacks. Their owning items
stay unavailable, so no input path silently degrades to an ordinary attack.

## Related correction

The generic attack's falling Down variant now follows `block_98`: kind0 items need Down
plus Left/Right for variant6, other kinds only Down. Native previously tested the item id,
so kind0 fists took variant6 from Down alone.

## Evidence

Public tests cover the recognizer window, captured facing, exact directions, cramped
reset and expiry; Basilard's special selection, blueprint parameter, omitted grunt and
attack state against the ordinary attack; and kind0 airborne variants. Mutations of the
selection and the variant test are caught. The private original-instruction comparison
matches `CheckBackForwardInput` over 3,456 phase/timer/direction/facing/cramped contexts
with no adapters. Weapon31 descriptor1's callback is covered by the existing shared-hand
comparison. A native Debug/ASan protocol performs the real input sequence in both hands
and checks the weapon31 child, attack state, private sprite pixels and real enemy damage
against the ordinary attack. Supplied stock, protection and position contexts are not
acquisition, route or console evidence.

Adapted AGPL-3.0-or-later algorithms: `dra/704D0.c` and `dra/6E42C.c` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`.
