add_library(sotn-audio STATIC
  playback.c
  playback_internal.h
  include/sotn/playback.h
  sound.c
  music.c
  voice.h
  gauss.h
  include/sotn/sound.h
  include/sotn/music.h
)
sotn_configure_c_target(sotn-audio)
target_include_directories(sotn-audio PUBLIC include)
target_link_libraries(sotn-audio PUBLIC sotn-data)
