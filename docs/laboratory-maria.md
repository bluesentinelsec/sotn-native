# Laboratory Maria encounter

Entering Maria's original Laboratory room now runs its private dialogue/event
streams. Maria opens the left red door, walks to Alucard, gestures during the
conversation, and leaves to the right. The original player manager walks Alucard
into position, holds control during the conversation, and releases it at the end.
The original met-Maria castle flag prevents replay after returning or continuing.

The shared bounded script interpreter reads text, portrait addresses, speaker
palettes, event timing and sound IDs from private stage data. The five original
US XA speech clips stream through the native audio service. Music fades in the
original 12-unit steps, pauses with its decoder position retained, and resumes
after the conversation. Native CD seek/command timing and full dialogue effects
remain incomplete; this does not establish console timing parity. Cutscene skip
and the original actor-name strip are not integrated.

Behavior follows `st/nz0/maria.c`, `st/nz0/cutscene.c` and `dra/sound.c` at the
pinned sotn-decomp commit, under AGPL-3.0-or-later. Assets and scripts remain private.

Synthetic regression tests cover dialogue waits, speech selection, palette bounds,
control release, door dependencies, animation transitions and departure flags.
Private native LLDB checks run the original room encounter through all five speech
clips and verify completion/control release. They use an explicit room start,
dummy audio and an isolated save; they are not a continuous route parity replay.
