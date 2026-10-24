add_library(sotn-game STATIC
  laboratory.c
  input.c
  input.h
  include/sotn/laboratory.h
)
sotn_configure_c_target(sotn-game)
target_include_directories(sotn-game PUBLIC include)
target_link_libraries(sotn-game PUBLIC sotn-render sotn-audio)
