add_library(sotn-core STATIC
  collision.c
  animation.c
  decompress.c
  stage.c
  actor_layout.c
  room_actors.c
  player.c
  player_sensors.c
  player_internal.h
  include/sotn/collision.h
  include/sotn/animation.h
  include/sotn/decompress.h
  include/sotn/stage.h
  include/sotn/actor_layout.h
  include/sotn/player.h
)
sotn_configure_c_target(sotn-core)
target_include_directories(sotn-core PUBLIC include)
target_link_libraries(sotn-core PUBLIC sotn-data)
