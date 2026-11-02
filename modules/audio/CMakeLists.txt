add_library(sotn-audio STATIC
  playback.c
  playback_internal.h
  include/sotn/playback.h
  sequence.c
  include/sotn/sequence.h
  sound_sequence.c
  sound_internal.h
  sound.c
  sound_pause.c
  reverb.c
  reverb.h
  music.c
  cd_audio.c
  include/sotn/cd_audio.h
  voice.h
  gauss.h
  include/sotn/sound.h
  include/sotn/music.h
)
sotn_configure_c_target(sotn-audio)
target_include_directories(sotn-audio PUBLIC include)
target_link_libraries(sotn-audio PUBLIC sotn-data)
