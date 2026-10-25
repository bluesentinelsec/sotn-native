# Selected Chapel mechanisms and scenery

The internal [Royal Chapel checkpoint](chapel-runtime.md) now runs the original
Colosseum shortcut block/statue, selected gargoyle tongue/block states, castle
wall quads and screen-space staircase shadow. Chapel connected travel remains
disabled. Tower-exit routing, bells, spikes, remaining scenery/enemies, Maria and
boss progression are separate integration work.

The placed room-6 block creates its adjacent statue with four HP. Initialization
from the left disables that statue's hitbox, preserving the original approach-side
rule. Attacks from the other side can destroy it with its original sound/explosion;
the block then records castle flag 96 and follows its private floor sensors at
one-quarter pixel per tick. Its two collision boxes preserve top and side shoves.
Re-entry with the flag set starts at the original (+32,+32) opened position and
creates no statue. Preserve original quantization: the moving block can settle a
pixel beyond that fresh-entry pose. This is not a reason to alter its movement.

Adjacent children retain native slot/generation ownership. Re-entering roots can
replace a surviving matching owned child; placed or unrelated actors are protected.
Lost ownership fails explicitly rather than silently opening the shortcut. Version
8 saves already carry the castle flags: no layout/version change is needed. Save
validation now rejects shortcut values above one; cross-stage save tests retain it.

Gargoyle tongues create their original adjacent block, frame/priority and right-side
offset. Selected down/up states retain 1.125-pixel motion, clamping at 70/129 pixels,
and the four original room-4 foreground tile changes. The private native checks
inject those movement states; they do not establish an original trigger route.

Two-piece castle walls and the repeated three-column wall retain original texture
direction, dimensions, primitive budgets and priority 160. The untextured staircase
triangle retains screen geometry, priority 94, player-screen-Y adjustment and dark
blue backdrop. This is presentation; the staircase's terrain comes from its private
collision map. Optional primitive exhaustion retires these scenery actors.

## Validation

All 80 public test groups passed in Debug and macOS ASan/UBSan, and release packaging
passed. Synthetic cases cover the shortcut's statue, flag, sensor motion, re-entry,
approach-side restriction, child replacement/ownership failure, gargoyle clamps/tile
bounds, wall orientation, shadow scrolling and primitive exhaustion. Ordinary-save
tests retain the shortcut flag across the supported stage/save-room profiles.

A private LLDB sequence uses an ordinary 70-HP character to break the placed statue
and settle its block at tick 183. It uses the real save-room UI in an isolated slot
15, fresh Continue and an explicit room-6 transfer to verify permanent restoration.
It then checks both placed gargoyles with injected movement states and explicit
wall/stair views. Debug and sanitizer runs each render 51 frames and exit successfully.
Existing preference save slots are untouched. Dummy audio checks decoding/ownership,
not listening or console PCM. These controlled starts/transfers are not uninterrupted
traversal, connected-stage, whole-area or console parity evidence.
