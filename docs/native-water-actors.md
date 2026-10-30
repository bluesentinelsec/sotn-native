# Native Entrance water actors

The two Entrance variants now run their actual placed water controllers and
main, surfacing, side, small-drop and large-drop actors. The owner consumes the
[controller plans](water-controller.md) and [effect components](water-effects.md)
through original stage identities and private tables. This is a stage integration
checkpoint. Selected [player feedback](player-water.md) now binds drag, form gates,
swimming and bubbles; water damage still requires its own owner.

Controllers inspect real auxiliary slots224..255, preflight the planned children
and commit actual reservations before publishing cooldown, three distinct depths
or current displacement. Current movement uses the existing bounded stage-object
motion owner, so subsequent actors see the displaced player. Creation clears child
fractions while retaining the original priority and surface metadata. Exhaustion
never overwrites unrelated actors or starts a fictitious surfacing cooldown.

Every effect reserves its actual shared primitive rows. The large drop requires33
contiguous rows with its real sentinel; a fragmented pool is insufficient. Native
updates preserve separate stage Random and libc-rand streams, birth ordering,
optional drop creation, original volume/pan requests and owned retirement. Main
splashes create the following drop without inheriting priority; its actual common
initialization supplies the original default. Invalid late initialization releases
only that birth's rows and preserves shared randomness.

Owned GT4 colors, texture coordinates and ordering priorities feed stage rendering.
Tiles use their original blend mode and sentinel visibility. Camera projection
translates cached body/packet coordinates once before simulation and draws apply
no second camera offset. Initialized actors retain ordinary freeze rules; menus
do not tick them. Room entry retires transient rows and clears retained water
depths, matching the room initialization boundary. Nothing new enters saves.

Public authored tests cover both stage identities, actual auxiliary generations,
shared2/33-row creation, current/wall feedback, pre-current sampling, inherited
metadata, camera movement, generation failure, occupied auxiliaries and fragmented
primitive exhaustion. Private native checks use actual room7 in both variants,
explicit position/contact protection and real updates, drawing and sound requests.
They are not ordinary acquisition, uninterrupted traversal or console evidence.
Debug and ASan/UBSan checks also retain initialized water actors, depths and random
streams through20 frozen updates and20 paused native draws, then verify real room
entry releases stage rows and clears depths. The CMake-built Debug app exercises
the actual controller/splash/drop path under muted LLDB and exits normally through
an SDL quit event. Existing user save slots are never written by these probes.

Selected player consumers now run; water damage/death and further stage-specific
water implementations remain unfinished. Ordinary starts and version-nine saves remain unchanged; Alucard and
the castle are incomplete. Provenance follows the pinned original functions
listed in the component documents. All game data and runtime traces stay private.
