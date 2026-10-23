add_library(sotn-core STATIC
  collision.c
  animation.c
  decompress.c
  stage.c
  include/sotn/collision.h
  include/sotn/animation.h
  include/sotn/decompress.h
  include/sotn/stage.h
)
sotn_configure_c_target(sotn-core)
target_include_directories(sotn-core PUBLIC include)
target_link_libraries(sotn-core PUBLIC sotn-data)
