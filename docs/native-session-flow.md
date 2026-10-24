# Native launch and Continue flow

`--play --game-data DIR` opens the native launch/Continue shell. Select Continue,
choose one of fifteen ordinary slots, and confirm. Slots show validated level,
HP, and stage/save room; missing and invalid slots stay distinguishable.
An unavailable slot shows an error and never starts a fabricated game. The other
choice explicitly starts the existing Laboratory development checkpoint.

After the original game-over presentation finishes, the application returns to
this shell. Continue restores ordinary progress in a fresh scene, without
requiring a process restart or a command-line slot argument. Existing direct
`--laboratory` and `--laboratory-continue SLOT` entries remain supported.

Keyboard: arrows select, X/Space or Enter confirms, A cancels, Escape quits.
Xbox: d-pad/stick selects, A confirms, Y cancels. Held selection/confirm inputs
are suppressed across screen/session transitions until released. A requested load
stays pending across catch-up ticks until the application consumes it once. Hotplug and
window lifecycle use the existing SDL input/OpenGL presenter.

This is a native filesystem adaptation with the private common font. It is
**not** the original title screen, memory-card port/file menu, or a full new-game
start. Prologue/Richter, the original initial character profile, original opening
menus and title music still need integration. The selected Entrance/Death checkpoint
and connected Laboratory route are described in [connected stages](connected-stages.md). The UI labels the development
checkpoint explicitly rather than pretending to offer those unimplemented paths.

The app scans slots read-only. It checks the selected file again at load time;
a file replaced/corrupted after enumeration cannot bypass validation. A fresh
scene must initialize successfully before replacing the old owner. A failed
read/resource initialization shows an explicit error while retaining the old
scene and saves. On success, the save-store owner transfers once, the old scene
is released, and borrowed handles expire. Loading never writes or migrates a
file on disk. Ordinary save writes still require the save-room confirmation flow.
Native browser persistence remains unavailable until its adapter is implemented.

Public tests cover held input, navigation/wrapping, unavailable slots, cancel,
one load request, renderer/resource/text bounds and CLI conflicts. A private
native LLDB sequence exercised slot scanning, a changed-file failure, valid
load, death/game over, return to the menu, another Continue, and byte-identical
save retention using an isolated directory. Private captures and game inputs
remain excluded from source, CI and release archives.
