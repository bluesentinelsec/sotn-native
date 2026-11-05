# Selected owned food use

The 42 original weapon21 items29..70 now connect owned inventory selection,
ordinary hand input, shared factories/children, private sprites, collision and
resource feedback. This includes Peanuts35, Toadstool36 and Meal Ticket70.
Development starts grant no food. Ordinary savev9 fields and resources stay unchanged.
All six [Alucard completion gates](alucard-status.md) remain open.

The original hand helper counts actual same-item/same-hand children before curse
rejection; two block another use. Pending roots do not count. Both-hand presses
retain the ordinary left-hand choice. Curse rejects use through its actual optional
outline57/profile1 and failure recovery without spending food. Actual eight-root
exhaustion also leaves inventory unchanged. Successful requests consume once after
the player update, animation and collision succeed. Spare counts exclude equipped
copies; the last copy clears its hand. Either equipped Duplicator84 preserves food.
No attack mode, startup timer, MP charge or cast state is introduced. Standing
selects animation60 plus its ledge variant; crouching selects62, with the original
uncramped crouch2 return to standing. Walking/air and substate40 retain their gates.
Food and medicine show zero weapon attack, independently of raw healing power.

Factory50 uses actual slots8..15 and weapon children17..31. Its origin2 follows the
live player at the factory callback. Children retain packed fractions, signed word/
fixed-point wrap, original facing/priority and tile/slope collision. Ordinary birth
consumes motion randomness followed by the actual GetEquipProperties random call;
Peanuts consume their additional motion call. Healing uses raw item attack without
STR, poison, Ankh or Bloodstone scaling. Catch tests precede movement. A first pending
kind1 heal wins; occupied healing hides the food and retries every eight callbacks.
The next successful player update applies the existing cap, number, sound and Blink
owners. Dead states retain pending healing without revival.

Peanuts require the actual look-up frame7B/animation0 and original narrow mouth
window; a catch writes pose3/timer8, including when healing is already pending.
Their bounce and retirement remain separate from ordinary floor food. Toadstool
writes the original20-frame simulated input, Down in air and zero on the ground.
After12 callbacks it writes raw hit81/power0 and clears the actual player hitbox
word. The following normal damage owner handles poison, protection and HP loss;
this is neither a direct ailment grant nor a fabricated positive-power contact.
Soul Steal retains hitbox82 on its separate attack child; it does not overwrite the
player body word. Stage contacts respect the player's resulting bit1 after the
ordered shared callbacks.

Meal Ticket consumes one random choice among the41 ordinary food IDs, then retries
an actual optional factory56 until available. The child is an inventory pickup,
including when the selected item is Peanuts or Toadstool. It uses its original512
floor countdown, expiry-before-contact and no grounded horizontal deceleration.
Collection queues the original item sound, applies capped inventory/acquisition
ordering and replaces the actual lower-left text lease. These changes are staged
with character RNG and the shared primitive plan. Text allocation occurs at this
child's point in slot order, allowing later effects to reuse released old rows.
Exhaustion omits the optional new text while preserving the item/sound request.
There is no separate food entity or primitive pool.

The global text uses actual existing primitive rows, private encoded names/font
artwork and original G4/GT4/SPRT-derived packets. Its304-update lifetime follows
ordinary/frozen stage callbacks; menus, release-only frames and draws hold it.
Room entry clears it. Replacing text preserves unrelated rows and actual generation
checks. A failed late native operation restores the preceding text lease, packets,
serial, player rows, character inventory and RNG before a retry. This selected
rollback does not establish whole-frame HP/map/clock/audio atomicity or reverse a
previously successful player's item consumption.

Food draws use actual private weapon21 artwork and its384 palette words, including
the source's adjacent bytes. The last equipped copy may be consumed while its
sprite is in flight: the original resident bank remains bound until another real
weapon load. Ordinary floor food begins original slot/engine-clock blinking at24;
real offscreen checks retire its slot. Camera translation preserves the hidden
Ticket and newly born cold factory's original screen positions. Menus/frozen
callbacks/draws never advance food controllers, and room rebases add no callback.
Room entry retires these room-local children and nonretained factories. Actual
menu hand changes retire that hand's weapon range and both original special ranges;
the global text remains independently owned.

Private instruction comparisons use revision
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`: `weapon/w_021.c`,
`weapon/shared.h`, `dra/6E42C.c`, `dra/5D5BC.c`, `dra/menu.c`,
`dra/7879C.c` and `st/st_update.h`. Original resources and raw traces stay private.
Debug and ASan traces match byte for byte:

| Compared owner | Scope |
| --- | --- |
| Food callbacks and catch helper |830 contexts,144,308 callbacks and244,129 actual native collision-provider queries; all42 primary items and41 inventory children |
| Ordinary hand helper |30,240 complete original calls; both hands, normal states, crouch2/recovery40, ledges, chain counts0/1/2, allocation and curse |
| ReduceWeapon |67,200 complete calls; all42 items, both hands, spares0..99, zero/one/two Duplicators and other-hand retention |
| Pickup text |All41 names and481 complete original packets, converted to the native quad representation |

RNG/CLZ, collision-provider, sprite-binding, destruction, factory, sound, inventory,
text, reservation/consumption and stat-recalculation services retain explicit
adapters where applicable. Component comparisons do not establish ordinary
acquisition, native right-overlay loading, full raster/audio timing or console parity.

Native checks separately use real ordinary input for all42 foods in both hands:
84 births,80 raw heals, two zero-power poison reactions and two Meal Ticket
inventory/text pickups. Peanut catches use actual look-up input. Another protocol
selects through the native menu, rejects an injected late post-row upload, verifies
rollback and retries once, then changes hands through actual menu navigation and
checks retirement with retained global text. Stock, health, protection, position,
arrival and failure fixtures are explicit; these are not acquisition, uninterrupted
routes, whole-pass atomicity or console evidence. Both actual CMake-app builds also
select all42 foods through Start/confirm/directional input, verify zero displayed
weapon attack and consume the selected food once through ordinary hand input.
Those app checks supply bag stock and protection, with no direct equip, pose,
position, HP/MP, clock, factory or save fixture. Additional native checks hold
live food through menu/draw and actual33-frame Summon freeze plus release, then
round-trip the existing4096-byte v9 record through an in-memory transfer to Library
save room14 and construct the actual saved session. Equipped food and spares return,
transient food/text stay clear and restored hand use consumes the last copy. This
uses an explicit save-room transfer fixture and writes no user save file. Remaining consumable/weapon/
equipment families, full shared contention and the other milestone gates remain open.
